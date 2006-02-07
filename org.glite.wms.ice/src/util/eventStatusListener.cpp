#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "classad_distribution.h"
#include "ClassadSyntax_ex.h"
#include <boost/algorithm/string.hpp>
#include "eventStatusListener.h"
#include "subscriptionCache.h"
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
#include <functional>
#include <algorithm>

extern int h_errno;
extern int errno;

using namespace std;
namespace api = glite::ce::cream_client_api;
namespace iceUtil = glite::wms::ice::util;

boost::recursive_mutex iceUtil::eventStatusListener::mutexJobStatusUpdate;

namespace { // anonym namespace

    class StatusNotification {
    public:
        StatusNotification( const string& _classad ) throw( iceUtil::ClassadSyntax_ex& );
        virtual ~StatusNotification( ) { };

        const string& getCreamJobID( void ) const { return cream_job_id; };
        const string& getStatus( void ) const { return job_status; };
        long getTstamp( void ) const { return tstamp; };
    protected:
        string cream_job_id;
        string job_status;
        long tstamp;
    };

    // StatusNotification implementation
    StatusNotification::StatusNotification( const string& _classad ) throw( iceUtil::ClassadSyntax_ex& )
    {
        api::util::creamApiLogger::instance()->getLogger()->infoStream()
            << "Parsing status change notification "
            << _classad
            << log4cpp::CategoryStream::ENDLINE;

        classad::ClassAdParser parser;
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
    };


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
    grid_JOBID(),
    cream_JOBID(),
    status(glite::ce::cream_client_api::job_statuses::UNKNOWN),
    endaccept(false),
    subscriber(),
    subManager(),
    pinger ( 0 ),
    T(iceUtil::iceConfManager::getInstance()->getICETopic()),
    P(5000),
    proxyfile(hostcert),
    tcpport(i), 
    myname( ),
    conf(iceUtil::iceConfManager::getInstance()),
    log_dev( api::util::creamApiLogger::instance()->getLogger() ),
    _ev_logger( iceEventLogger::instance() ),
    _isOK( true )
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
    //exit(1);
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
    //exit(1);
    _isOK = false;
    return;
  }
  myname = H->h_name;
  log_dev->info( "eventStatusListener::CTOR - Listener created!" );
  T.addDialect(NULL);
  try {
    pinger = new CEPing(proxyfile, "/");
  } catch(exception& ex) {
    log_dev->fatalStream() << "eventStatusListener::CTOR - "
			   << "Fatal Error creating a pinger object:"
                           << ex.what()
                           << log4cpp::CategoryStream::ENDLINE;
    _isOK = false;
    return;
    //exit(1);
  }
  init();
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::operator()()
{
  endaccept=false;
  while(!endaccept) {
    log_dev->infoStream() << "eventStatusListener::()() - "
			  << "Waiting for job status notification"
			  << log4cpp::CategoryStream::ENDLINE;
    acceptJobStatus();
    sleep(1);
  }
  log_dev->info( "eventStatusListener::()() - ending..." );
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::acceptJobStatus(void)
{
  /**
   * Waits for an incoming connection
   */
  if(!this->accept() && !endaccept) {
    if(endaccept) {
        log_dev->infoStream()
            << "eventStatusListener::acceptJobStatus() - "
            << "eventStatusListener is ending"
            << log4cpp::CategoryStream::ENDLINE;
      return;
    } else
        log_dev->infoStream()
            << "eventStatusListener::acceptJobStatus()"
            << " - CEConsumer::Accept() returned false."
            << log4cpp::CategoryStream::ENDLINE;
	return;
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
    //this->reset();
    return;
  }
  
  /**
   * Checks if the SOAP message was correct (i.e.
   * containing a non-null topic)
   */
  if(!this->getEventTopic()) {
    log_dev->fatalStream() << "eventStatusListener::acceptJobStatus() - "
			   << "NULL Topic received. Ignoring this notification...." ;
    //this->reset();
    return;
  }

  /**
   * Ignores notifications generated by non-ICE topics
   */
  if(this->getEventTopic()->Name != conf->getICETopic())
    {
      log_dev->warnStream() << "eventStatusListener::acceptJobStatus() - "
			    << "Received a notification with TopicName="
			    << this->getEventTopic()->Name
			    << " that differs from the ICE's topic "
			    << "official name. Ignoring this notification."
			    << log4cpp::CategoryStream::ENDLINE;
      //this->reset();
      return;
    }

  const vector<Event>& evts = this->getEvents();

  /**
   * Loops over all events (event <-> job)
   * For each event updates the status of the related
   * job in the jobCache getting the status from last message of the 
   * event.
   */
  for ( vector<Event>::const_iterator it=evts.begin(); it != evts.end(); it++ ) {
      handleEvent( *it );
  } // Loop over all events
  //this->reset();
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::init(void)
{
  /**
   * This method is executed ONLY at startup of the listener
   * it collects all jobids, extracts the CEUrl they belong to,
   * checks for each CEUrl if the current ICE is subscribed to it.
   */
  map< string , int > tmpMap;
  string ceurl;
  ostringstream hostport;
  for(jobCache::iterator it=jobCache::getInstance()->begin();
      it != jobCache::getInstance()->end();
      it++)
    {
      ceurl = it->getCreamURL();
      boost::replace_first(ceurl,
                           conf->getCreamUrlPostfix(),
                           conf->getCEMonUrlPostfix()
			   );
      tmpMap[ceurl] = 1;
    }

  /**
   * Now we've got a collection of CEMon urls (without duplicates,
   * thanks to the map's property) we've to check for subscription
   */
  vector<Subscription> vec;
  vec.reserve(100);

  bool contacted;
  for(map<string, int>::iterator it = tmpMap.begin();
      it!=tmpMap.end();
      it++)
    {
      contacted = false;
      while(!contacted) {
        try {
          log_dev->infoStream() << "eventStatusListener::init() "
	                        << "- Authenticating with proxy ["
                                << proxyfile << "]"
                                << log4cpp::CategoryStream::ENDLINE;

	  subManager.authenticate(proxyfile.c_str(), "/");
	  vec.clear();
	  log_dev->infoStream() << "eventStatusListener::init() - "
                                << "Getting list of subscriptions from ["
                                << it->first << "]"
                                << log4cpp::CategoryStream::ENDLINE;

	  subManager.list(it->first, vec);
        } catch(ServiceNotFoundException& ex) {
	  // Service is not there. Let's try to ping it...
          log_dev->fatalStream() << "eventStatusListener::init() - "
	                         << ex.what()
				 << log4cpp::CategoryStream::ENDLINE;

	  // must retry to contact this cemon until it
	  // is up&running again
	  pinger->setServiceURL(it->first);
          if( !pinger->Ping() ) {
	    log_dev->warnStream() << "eventStatusListener::init() - CEMon at ["
	  			  << it->first <<  "] is not reachable. "
				  << "Waiting 5 seconds..."
				  << log4cpp::CategoryStream::ENDLINE;
	    sleep(5);
	    continue; // try another subscription list
	  } else {
	    // The CEMon is reachable (back)
	    // let's try to retrieve again the
	    // subscription list
	    continue;
	  }
        } catch(exception& ex) {
 	  // something wrong happened. Exit
          log_dev->fatalStream() << "eventStatusListener::init() - "
	                         << ex.what()
				 << log4cpp::CategoryStream::ENDLINE;
	  exit(1);
	}
	contacted=true;
      } // while(!contacted)
      if(!vec.size())
	{
	  /**
	   * there're no subs in that CEMon; so for sure we've to
	   * subscribe to it
	   */
	  ostringstream myname_url("");
	  myname_url << "http://" << myname << ":"
		     << conf->getListenerPort();
	  try {
	    subscriber.authenticate(proxyfile.c_str(), "/");
	    subscriber.setServiceURL(it->first);
	    subscriber.setSubscribeParam(myname_url.str().c_str(),
	                                 T,
					 P,
					 conf->getSubscriptionDuration()
					 );
            log_dev->infoStream() << "eventStatusListener::init() - "
				  << "Subscribing the consumer ["
                                  << myname_url.str() << "] to ["<<it->first
                                  << "] with duration="
                                  << conf->getSubscriptionDuration()
                                  << " secs"
                                  << log4cpp::CategoryStream::ENDLINE;
	    subscriber.subscribe();
	    log_dev->infoStream() << "eventStatusListener::init() - "
                                  << "Subscribed with ID ["
                                  << subscriber.getSubscriptionID() << "]"
                                  << log4cpp::CategoryStream::ENDLINE;
	    
	    subscriptionCache::getInstance()->insert(it->first);
	  } catch(AuthenticationInitException& ex) {
	    log_dev->fatalStream() << "eventStatusListener::init() - "
				   << "AuthN Error: "
				   << ex.what() 
				   << log4cpp::CategoryStream::ENDLINE;
	    exit(1);
	  } catch(exception& ex) {
	    log_dev->fatalStream() << "eventStatusListener::init() - "
				   << "Subscription Error: "
				   << ex.what() 
				   << log4cpp::CategoryStream::ENDLINE;
	    exit(1);
	  }
	} else 
	{
	  vector<string> _tmp; _tmp.reserve(2);
	  char name[256];
	  bool subscribed = false;
	  string consumerUrl;
	  for(vector<Subscription>::iterator sit=vec.begin();
	      (sit != vec.end()) && (!subscribed);
	      sit++)
	    {
	      _tmp.clear();
	      consumerUrl = (*sit).getConsumerURL();
	      boost::replace_first(consumerUrl, "https://", "");
	      boost::replace_first(consumerUrl, "http://", "");
	      boost::split(_tmp, consumerUrl, boost::is_any_of(":"));
	      memset((void*)name, 0, 256);
	      if(gethostname(name, 256) == -1)
		{
		  log_dev->fatalStream() << "eventStatusListener::init() - "
					 << "Couldn't resolve local hostname:"
					 << strerror(errno)
					 << log4cpp::CategoryStream::ENDLINE;
		  exit(1);
		}
	      struct hostent *H = gethostbyname(_tmp[0].c_str());
	      if(!H) {
		log_dev->fatalStream() << "eventStatusListener::init() - "
				       << "Couldn't resolve subscribed "
				       << "consumer's hostname: "
				       << strerror(h_errno)
				       << log4cpp::CategoryStream::ENDLINE;
		exit(1);
	      }
	      ostringstream consumerUrlOS("");
	      consumerUrlOS << H->h_name << ":" << _tmp[1];
	      consumerUrl = consumerUrlOS.str();
	      hostport.str("");
	      hostport << myname << ":" << conf->getListenerPort();
	      if( (consumerUrl==hostport.str()) &&
		  ((*sit).getTopicName()==conf->getICETopic()) )
		{
		  /**
		   * this ICE is subscribed to CEMon
		   */
		  subscribed = true;
		  log_dev->infoStream() << "eventStatusListener::init() - "
                                        << "Already subscribed to ["
					<< it->first << "]"
					<< log4cpp::CategoryStream::ENDLINE;
		  //cemon_subscribed_to[it->first] = true;
		  continue;
		}

	    }
	  if(!subscribed) {
              log_dev->infoStream() << "eventStatusListener::init() - "
                                    << "Must subscribe to [" << it->first
                                    << "]" << log4cpp::CategoryStream::ENDLINE;
	    subscriber.setServiceURL(it->first);
	    subscriber.setSubscribeParam(hostport.str().c_str(),
	                                 T,
					 P,
					 conf->getSubscriptionDuration());
	    log_dev->infoStream() << "eventStatusListener::init() - "
                                  << "Subscribing the consumer ["
                                  << hostport.str() << "] to ["<<it->first
                                  << "] with duration="
                                  << conf->getSubscriptionDuration()
                                  << " secs" 
                                  << log4cpp::CategoryStream::ENDLINE;
	    try {
	      subscriber.subscribe();
	      log_dev->infoStream() << "eventStatusListener::init() - "
                                    << "Subscription succesfull: ID="
                                    << subscriber.getSubscriptionID()<<"]"
                                    << log4cpp::CategoryStream::ENDLINE;
	      //cemon_subscribed_to[it->first] = true;
	      subscriptionCache::getInstance()->insert(it->first);
	    } catch(exception& ex) {
              log_dev->fatalStream() << "eventStatusListener::init() - "
		                       << ex.what()
				       << log4cpp::CategoryStream::ENDLINE;
	      exit(1);
	    }
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
    // tstamp = time(NULL);
}

void iceUtil::eventStatusListener::handleEvent( const Event& ev )
{
    for(unsigned jj=0; jj<ev.Messages.size(); jj++) {
    //      cout << "evt #"<<j<<".Message["<<jj<<"]="<<ev.Messages[jj]<<endl;
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
	log_dev->errorStream() << "Error parsing notification Message ["
	                       <<ev.Messages[ev.Messages.size()-1]
			       << "]. Ignoring it...";
	//this->reset();
	return;
    }


    jobCache::iterator it;

    try {
      boost::recursive_mutex::scoped_lock M( mutexJobStatusUpdate );

      it = jobCache::getInstance()->lookupByCreamJobID(cream_job_id);
      if( it == jobCache::getInstance()->end())
	{
	  log_dev->errorStream() << "eventStatusListener::acceptJobStatus() - "
				 << "Not found in the cache the creamjobid ["
				 << cream_job_id<<"] that should be there. Ignoring this notification..."
				 << log4cpp::CategoryStream::ENDLINE;
	  //this->reset();
	  return;
	  //exit(1);
	}
      log_dev->infoStream() << "eventStatusListener::acceptJobStatus() - "
      			    << "Updating job ["<<cream_job_id
			    << "] with status ["<<status<<"]"
			    << log4cpp::CategoryStream::ENDLINE;
      it->setStatus( api::job_statuses::getStatusNum(status), time(NULL) );
      _ev_logger->log_job_status( *it ); // FIXME
      jobCache::getInstance()->put( *it );
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



void iceUtil::eventStatusListener::handleEvent( const Event& ev )
{
    // First, convert the vector of messages into a vector of StatusNotification objects
    vector<StatusNotification> notifications;

    for ( vector<string>::const_iterator it = ev.Messages.begin();
              it != ev.Messages.end(); it++ ) {
        try {
            notifications.push_back( StatusNotification( *it ) );
        } catch( iceUtil::ClassadSyntax_ex ex ) {
            // FIXME!! Help!!!
        }
    }

    // Then, sort the list of StatusNotifications in nondecreasing
    // timestamp order
    sort( notifications.begin(), notifications.end(), less_equal_tstamp() );

    // For debug only...
    if ( log_dev->isInfoEnabled() ) {
        for( vector<StatusNotification>::const_iterator it;
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
    for ( vector<StatusNotification>::const_iterator it;
          it != notifications.end(); it++ ) {

        jobCache::iterator jc_it;
        
        try {
            boost::recursive_mutex::scoped_lock M( mutexJobStatusUpdate );
            
            jc_it = jobCache::getInstance()->lookupByCreamJobID( it->getCreamJobID() );
            if( jc_it == jobCache::getInstance()->end()) {
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
                    jc_it->setStatus( api::job_statuses::getStatusNum( it->getStatus() ), it->getTstamp() );
                    _ev_logger->log_job_status( *jc_it ); // FIXME
                    jobCache::getInstance()->put( *jc_it );
                } else {
                    log_dev->infoStream()
                        << "...NOT DONE, as notification is old"
                        << log4cpp::CategoryStream::ENDLINE;
                }
            }
        } catch(exception& ex) {
            log_dev->fatal( ex.what() );
            exit(1);
        }        
    }
}

} // anonymous namespace
