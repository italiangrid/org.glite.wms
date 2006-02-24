#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "classad_distribution.h"
#include "ClassadSyntax_ex.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "eventStatusListener.h"
#include "subscriptionCache.h"
#include "subscriptionManager.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include <unistd.h>
#include <string>
#include <iostream>
#include <cerrno>
#include <sstream>
#include <cstring> // for memset
#include <netdb.h>
#include "iceEventLogger.h"
#include <algorithm>
#include <set>
#include "boost/functional.hpp"
#include "boost/mem_fn.hpp"
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"
#include <stdexcept>

extern int h_errno;
extern int errno;

using namespace std;
namespace api = glite::ce::cream_client_api;
namespace iceUtil = glite::wms::ice::util;

boost::recursive_mutex iceUtil::eventStatusListener::mutexJobStatusUpdate;

namespace { // anonymous namespace

    /**
     * Utility function to return the hostname
     */ 
    std::string getHostName( void ) throw ( std::runtime_error& )
    {
        char name[256];
        
        if ( gethostname(name, 256) == -1 ) {
            throw runtime_error( string( "Could not resolve local hostname: ") + string(strerror(errno) ) );
//             // This error prevent the possibility to subscribe to a CEMon
//             // and receive notifications about job status
//             log_dev->fatalStream() << "eventStatusListener::CTOR - "
//                                    << "Couldn't resolve local hostname: "
//                                    << strerror(errno)
//                                    << log4cpp::CategoryStream::ENDLINE;
//             _isOK = false;
//             //exit(1);
//             return;
        }
        struct hostent *H=gethostbyname(name);
        if ( !H ) {
            throw runtime_error( string( "Could not resolve local hostname: ") + string(strerror(errno) ) );
//             // This error prevent the possibility to subscribe to a CEMon
//             // and receive notifications about job status
//             log_dev->fatalStream() << "eventStatusListener::CTOR - "
//                                    << "Couldn't resolve local hostname: "
//                                    << strerror(h_errno)
//                                    << log4cpp::CategoryStream::ENDLINE;
//             //exit(1);
//             _isOK = false;
//             return;
        }
        return string(H->h_name);
    }


    /**
     * This class represents status change notifications as sent by
     * CEMON to ICE. A Job status change notification is a classad
     * with several fields indicating che cream job id of the job
     * whose status changed, the new status, and the timestamp when
     * the change occurred.
     */
    class StatusNotification {
    public:
        /**
         * Builds a StatusNotification object from a classad.
         *
         * @param _classad the string representing a classad to build
         * this object from.
         */
        StatusNotification( const string& _classad ) throw( iceUtil::ClassadSyntax_ex& );
        virtual ~StatusNotification( ) { };

        /**
         * Returns the CREAM job ID for this notification
         */
        const string& getCreamJobID( void ) const { return cream_job_id; };

        /**
         * Returns the status for this notification
         */
        api::job_statuses::job_status  getStatus( void ) const { return job_status; };

        /**
         * Returns the time of the status for this notification
         */
        time_t getTstamp( void ) const { return tstamp; };
    protected:
        string cream_job_id;
        api::job_statuses::job_status job_status;
        time_t tstamp;
    };

    //
    // StatusNotification implementation
    //
    StatusNotification::StatusNotification( const string& _classad ) throw( iceUtil::ClassadSyntax_ex& )
    {
        api::util::creamApiLogger::instance()->getLogger()->infoStream()
            << "Parsing status change notification "
            << _classad
            << log4cpp::CategoryStream::ENDLINE;
        
        classad::ClassAdParser parser;
        classad::ClassAd *ad = parser.ParseClassAd( _classad );
        
        if (!ad)
            throw iceUtil::ClassadSyntax_ex("The classad describing the job status has syntax error");
        
        if ( !ad->EvaluateAttrString( "CREAM_JOB_ID", cream_job_id ) )
            throw iceUtil::ClassadSyntax_ex("CREAM_JOB_ID attribute not found, or is not a string");
        boost::trim_if( cream_job_id, boost::is_any_of("\"" ) );
        
        string job_status_str;       
        if ( !ad->EvaluateAttrString( "JOB_STATUS", job_status_str ) )
            throw iceUtil::ClassadSyntax_ex("JOB_STATUS attribute not found, or is not a string");
        boost::trim_if( job_status_str, boost::is_any_of("\"" ) );
        job_status = api::job_statuses::getStatusNum( job_status_str );
        
        string tstamp_s;
        if ( !ad->EvaluateAttrString( "TIMESTAMP", tstamp_s ) )
            throw iceUtil::ClassadSyntax_ex("TIMESTAMP attribute not found, or is not a string");
        boost::trim_if( tstamp_s, boost::is_any_of("\"" ) );
        try {
            tstamp = (time_t)( lrint( boost::lexical_cast<double>( tstamp_s )/1000 ) );
        } catch( boost::bad_lexical_cast& c ) {
            throw iceUtil::ClassadSyntax_ex("TIMESTAMP attribute canno be converted to time_t" );
        }
        // FIXME: In the future, this timestamp will be relative to
        // UTC time, and probably should be converted to the local
        // (ICE-centric) timezone.
    };

    /**
     * This class is used to compare two StatusNotification objects
     * according with their timestamp. It is used to sort a vector of
     * notifications in nondecreasing timestamp order.
     */
    struct less_equal_tstamp : public binary_function< StatusNotification, StatusNotification, bool>
    {
        bool operator()(const StatusNotification& __x, const StatusNotification& __y) const 
        { 
            return __x.getTstamp() <= __y.getTstamp(); 
        }
    };


//______________________________________________________________________________
iceUtil::eventStatusListener::eventStatusListener(int i,const string& hostcert)
  : CEConsumer(i),
    iceThread( "event status poller" ),   
    grid_JOBID(),
    cream_JOBID(),
    status(glite::ce::cream_client_api::job_statuses::UNKNOWN),
    pinger ( 0 ),
    proxyfile(hostcert),
    tcpport(i),
    myname( ),
    conf(iceUtil::iceConfManager::getInstance()),
    log_dev( api::util::creamApiLogger::instance()->getLogger() ),
    _ev_logger( iceEventLogger::instance() ),
    _isOK( true ),
    cache( jobCache::getInstance() )
{
  char name[256];
  memset((void*)name, 0, 256);

  if(gethostname(name, 256) == -1) {
    // This error prevent the possibility to subscribe to a CEMon
    // and receive notifications about job status
    log_dev->fatalStream() << "eventStatusListener::CTOR - "
			   << "Couldn't resolve local hostname: "
                           << strerror(errno)
                           << log4cpp::CategoryStream::ENDLINE;
    _isOK = false;
    return;
  }
  struct hostent *H=gethostbyname(name);
  if(!H) {
    // This error prevent the possibility to subscribe to a CEMon
    // and receive notifications about job status
    log_dev->fatalStream() << "eventStatusListener::CTOR - "
			   << "Couldn't resolve local hostname: "
                           << strerror(h_errno)
                           << log4cpp::CategoryStream::ENDLINE;
    _isOK = false;
    return;
  }
  myname = H->h_name;
  log_dev->info( "eventStatusListener::CTOR - Listener created!" );
  try {
    pinger.reset( new CEPing(proxyfile, "/") );
  } catch(exception& ex) {
    log_dev->fatalStream() << "eventStatusListener::CTOR - "
			   << "Fatal Error creating a pinger object:"
                           << ex.what()
                           << log4cpp::CategoryStream::ENDLINE;
    _isOK = false;
    return;
  }

  {
    /**
     * Here we do not need to check if the creation of subscriptionManager
     * produced errors, because the main ice-core module did that as
     * preliminary init operation. We also do not need to lock the
     * ::getInstance() call, because the instance of singleton has been
     * already created.
     */
    subManager = subscriptionManager::getInstance();
  }

  init();
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::body( void )
{
    while( !isStopped() ) {
        log_dev->infoStream() << "eventStatusListener::()() - "
                              << "Waiting for job status notification"
                              << log4cpp::CategoryStream::ENDLINE;
        acceptJobStatus();
        sleep(1);
    }
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::acceptJobStatus(void)
{
  /**
   * Waits for an incoming connection
   */
    if(!this->accept() && !isStopped()) {
        if(isStopped()) {
            log_dev->infoStream()
                << "eventStatusListener::acceptJobStatus() - "
                << "eventStatusListener is ending"
                << log4cpp::CategoryStream::ENDLINE;
            return;
        } else {
            log_dev->infoStream()
                << "eventStatusListener::acceptJobStatus()"
                << " - CEConsumer::Accept() returned false."
                << log4cpp::CategoryStream::ENDLINE;
            return;
        }
    }

  log_dev->infoStream()
      << "eventStatusListener::acceptJobStatus() - "
      << "Connection accepted from ["
      << this->getClientIP() << "]"
      << log4cpp::CategoryStream::ENDLINE;

  /**
   * acquires the event from the client
   * and deserializes the data structures
   */
  if( !this->serve() ) {
    log_dev->errorStream() << "eventStatusListener::acceptJobStatus() - "
			   << "ErrorCode=["
			   << this->getErrorCode() << "]"
			   << log4cpp::CategoryStream::ENDLINE;

    log_dev->errorStream() << "eventStatusListener::acceptJobStatus() - "
			   << "ErrorMessage=["
			   << this->getErrorMessage() << "]"
			   << log4cpp::CategoryStream::ENDLINE;
    return;
  }
  
  /**
   * Checks if the SOAP message was correct (i.e.
   * containing a non-null topic)
   */
  if(!this->getEventTopic()) {
    log_dev->fatalStream() << "eventStatusListener::acceptJobStatus() - "
			   << "NULL Topic received. Ignoring this notification...." ;
    return;
  }

  /**
   * Ignores notifications generated by non-ICE topics
   */
  {
    boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
    if(this->getEventTopic()->Name != conf->getICETopic())
    {
      log_dev->warnStream() << "eventStatusListener::acceptJobStatus() - "
			    << "Received a notification with TopicName="
			    << this->getEventTopic()->Name
			    << " that differs from the ICE's topic "
			    << "official name. Ignoring this notification."
			    << log4cpp::CategoryStream::ENDLINE;
      return;
    }
  }

  const vector<monitortypes__Event>& evts = this->getEvents();

  /**
   * Loops over all events (event <-> job) For each event updates the
   * status of the related job in the jobCache getting the status from
   * last message of the event.
   */
  std::for_each( evts.begin(), evts.end(), boost::bind1st( boost::mem_fn( &eventStatusListener::handleEvent ), this ) );


  // The line above should be equivalent to the following:
  //  
  //   for ( vector<monitortypes__Event>::const_iterator it=evts.begin(); it != evts.end(); it++ ) {
  //       handleEvent( *it );
  //   } 

}

//______________________________________________________________________________
void iceUtil::eventStatusListener::init(void)
{
  /**
   * This method is executed ONLY at startup of the listener
   * it collects all jobids, extracts the CEUrl they belong to,
   * checks for each CEUrl if the current ICE is subscribed to it.
   */
  set< string > ceurls;

  string ceurl;
  ostringstream hostport;

  {
      // Scoped lock to protect concurrent access to the job cache
      boost::recursive_mutex::scoped_lock M( jobCache::mutex );

      for(jobCache::iterator it=cache->begin(); it != cache->end(); it++) {

          ceurl = it->getCreamURL();
          boost::replace_first(ceurl,
                               conf->getCreamUrlPostfix(),
                               conf->getCEMonUrlPostfix()
                               );
          ceurls.insert( ceurl );
      }
  }
  /**
   * Now we've got a collection of CEMon urls (without duplicates,
   * thanks to the set's property) we've to check for subscription
   */
  for( set<string>::const_iterator it = ceurls.begin(); it!=ceurls.end(); it++)
  {
    log_dev->infoStream() << "eventStatusListener::init() - Checking subscriptions to ["
                          << *it << "]" << log4cpp::CategoryStream::ENDLINE;
    {
      // Must lock the subscription manager due to its singleton nature
      boost::recursive_mutex::scoped_lock M( subscriptionManager::mutex );
      if( !subManager->subscribedTo(*it) ) {
        log_dev->infoStream() << "eventStatusListener::init() - Not subscribed to ["
			      << *it << "]. Subscribing to it..."
			      << log4cpp::CategoryStream::ENDLINE;
        if( !subManager->subscribe(*it) )
          log_dev->errorStream() << "eventStatusListener::init() - Subscription to ["
				 << *it << "] failed. Will not receives status notifications from it."
				 << log4cpp::CategoryStream::ENDLINE;
      } else {
        log_dev->infoStream() << "eventStatusListener::init() - Already subscribed to ["
			      << *it << log4cpp::CategoryStream::ENDLINE;
      }
    }
  }
}

//______________________________________________________________________________
#ifdef DONT_COMPILE
void iceUtil::eventStatusListener::parseEventJobStatus(string& cream_job_id, string& job_status, long& tstamp, const string& _classad)
    throw(iceUtil::ClassadSyntax_ex&)
{
    classad::ClassAd *ad = parser.ParseClassAd( _classad );
    double tstamp_d;

    if (!ad)
        throw iceUtil::ClassadSyntax_ex("The classad describing the job status has syntax error");

    if ( !ad->EvaluateAttrString( "CREAM_JOB_ID", cream_job_id ) )
        throw iceUtil::ClassadSyntax_ex("CREAM_JOB_ID attribute not found, or is not a string");

    if ( !ad->EvaluateAttrString( "JOB_STATUS", job_status ) )
        throw iceUtil::ClassadSyntax_ex("JOB_STATUS attribute not found, or is not a string");

    if ( !ad->EvaluateAttrReal( "TIMESTAMP", tstamp_d ) )
        throw iceUtil::ClassadSyntax_ex("TIMESTAMP attribute not found, or is not a number");
    tstamp = lrint( tstamp_d );
}

void iceUtil::eventStatusListener::handleEvent( const Event& ev )
{
    for(unsigned jj=0; jj<ev.Messages.size(); jj++) {
        log_dev->infoStream()
            << "Event dump follows for message[" << jj << "]: "
            << ev.Messages[jj]
            << log4cpp::CategoryStream::ENDLINE;
    }
    string Ad( ev.Messages[ev.Messages.size()-1] );
    string cream_job_id, status;
    double tstamp_d;
    long tstamp;

    // the following function extract from the classad Ad the
    // creamjobid and status and put them into the 1st and 2nd
    // arguments respectively
    try {
        parseEventJobStatus(cream_job_id, status, tstamp, Ad);
    }
    catch(iceUtil::ClassadSyntax_ex& ex) {
	log_dev->errorStream() 
            << "Error parsing notification Message ["
            <<ev.Messages[ev.Messages.size()-1]
            << "]. Ignoring it...";
	return;
    }


    jobCache::iterator it;

    try {
      boost::recursive_mutex::scoped_lock M( mutexJobStatusUpdate );

      it = cache->lookupByCreamJobID(cream_job_id);
      if( it == cache->end())
	{
	  log_dev->errorStream() << "eventStatusListener::acceptJobStatus() - "
				 << "Not found in the cache the creamjobid ["
				 << cream_job_id<<"] that should be there. Ignoring this notification..."
				 << log4cpp::CategoryStream::ENDLINE;
	  return;
	}
      log_dev->infoStream() << "eventStatusListener::acceptJobStatus() - "
      			    << "Updating job ["<<cream_job_id
			    << "] with status ["<<status<<"]"
			    << log4cpp::CategoryStream::ENDLINE;
      it->setStatus( api::job_statuses::getStatusNum(status), time(NULL) );
      _ev_logger->log_job_status( *it ); // FIXME
      cache->put( *it );
    } catch(exception& ex) {
      log_dev->fatal( ex.what() );
      exit(1);
    }
    
    log_dev->infoStream() << "eventStatusListener::acceptJobStatus() - "
                          << "Successfully updated JobID="
                          << cream_job_id <<" - STATUS=" << status
                          << log4cpp::CategoryStream::ENDLINE;
    /**
     * See CEConsumer's class for the purpose of this method
     */

}
#endif



void iceUtil::eventStatusListener::handleEvent( const monitortypes__Event& ev )
{
    // First, convert the vector of messages into a vector of StatusNotification objects
    vector<StatusNotification> notifications;

    for ( vector<string>::const_iterator it = ev.Message.begin();
          it != ev.Message.end(); it++ ) {
        try {
            notifications.push_back( StatusNotification( *it ) );
        } catch( iceUtil::ClassadSyntax_ex ex ) {
            log_dev->errorStream()
                << "eventStatusListenre::handleEvent() received a notification "
                << *it << " which could not be understood; error is: "
                << ex.what() << ". "
                << "Skipping this notification and hoping for the best..."
                << log4cpp::CategoryStream::ENDLINE;
        }
    }

    // Then, sort the list of StatusNotifications in nondecreasing
    // timestamp order
    sort( notifications.begin(), notifications.end(), less_equal_tstamp() );

    // For debug only...
    if ( log_dev->isInfoEnabled() ) {
        for( vector<StatusNotification>::const_iterator it = notifications.begin();
             it != notifications.end(); it++ ) {
            log_dev->infoStream()
                << "Notification: jobid=["
                << it->getCreamJobID() << "], tstamp=["
                << it->getTstamp() << "] status=["
                << it->getStatus() << "]"
                << log4cpp::CategoryStream::ENDLINE;
        }
    }

    // Now, for each status change notification, check if it has to be logged
    for ( vector<StatusNotification>::const_iterator it = notifications.begin();
          it != notifications.end(); it++ ) {

        jobCache::iterator jc_it;

        try {
            // boost::recursive_mutex::scoped_lock M( mutexJobStatusUpdate ); // FIXME: don't needed anymore???
            boost::recursive_mutex::scoped_lock jc_M( jobCache::mutex );
            
            jc_it = cache->lookupByCreamJobID( it->getCreamJobID() );
            if( jc_it == cache->end()) {
                log_dev->errorStream()
                    << "eventStatusListener::acceptJobStatus() - "
                    << "Not found in the cache the creamjobid=["
                    << it->getCreamJobID()
                    << "] that should be there. Ignoring this notification..."
                    << log4cpp::CategoryStream::ENDLINE;
            } else {
                log_dev->infoStream() 
                    << "eventStatusListener::acceptJobStatus() - "
                    << "Checking job [" << it->getCreamJobID()
                    << "] with status [" << it->getStatus() << "]"
                    << " notification tstamp=[" << it->getTstamp() << "]"
                    << " last updated on=[" << jc_it->getLastUpdate() << "]"
                    << log4cpp::CategoryStream::ENDLINE;
                if ( it->getTstamp() > jc_it->getLastUpdate() ) {
                    jc_it->setStatus( it->getStatus(), it->getTstamp() );
                    _ev_logger->log_job_status_change( *jc_it ); // FIXME
                    if ( (it+1) == notifications.end() ) {
                        // The cache is only modified for the last
                        // notification, for efficiency reasons.
                        cache->put( *jc_it ); 
                    }
                } else {
                    log_dev->infoStream()
                        << "...NOT DONE, as notification is old"
                        << log4cpp::CategoryStream::ENDLINE;
                }
            }
        } catch(exception& ex) { // FIXME: never thrown?
            log_dev->fatal( ex.what() );
            exit(1);
        }        
    }
}

} // anonymous namespace
