/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE Event Status change listener
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// ICE stuff
#include "eventStatusListener.h"
#include "subscriptionCache.h"
#include "subscriptionManager.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include "cemonUrlCache.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "iceLBLogger.h"
#include "iceLBEventFactory.h"
#include "iceUtils.h"

// CREAM stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

// other GLITE stuff
#include "classad_distribution.h"
#include "ClassadSyntax_ex.h"

// boost includes
#include "boost/functional.hpp"
#include "boost/mem_fn.hpp"
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"

// System includes
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <set>

using namespace std;
namespace api = glite::ce::cream_client_api;
namespace iceUtil = glite::wms::ice::util;

boost::recursive_mutex iceUtil::eventStatusListener::mutexJobStatusUpdate;

namespace { // anonymous namespace

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
        // time_t getTstamp( void ) const { return tstamp; };

        /**
         * Returns true iff the notification has an exit_code attribute
         */
        bool hasExitCode( void ) const { return has_exit_code; };

        /**
         * Returns the exit code
         */ 
        int getExitCode( void ) const { return exit_code; };

    protected:
        string cream_job_id;
        api::job_statuses::job_status job_status;
        // time_t tstamp;
        bool has_exit_code;
        int exit_code;
    };

    //
    // StatusNotification implementation
    //
    StatusNotification::StatusNotification( const string& _classad ) throw( iceUtil::ClassadSyntax_ex& ) :
        has_exit_code( false ),
        exit_code( 0 ) // default
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

        if ( ad->EvaluateAttrInt( "EXIT_CODE", exit_code ) ) {
            has_exit_code = true;
        }

    };

void iceUtil::eventStatusListener::createObject( void )
{
  try {
      myname = iceUtil::getHostName( );
  } catch( runtime_error& ex ) {
      _isOK = false;
      return;
  }


  log_dev->info( "eventStatusListener::CTOR() - Listener created!" );
  try {
    pinger.reset( new CEPing(proxyfile, "/") );
  } catch(exception& ex) {
    log_dev->fatalStream() << "eventStatusListener::CTOR() - "
			   << "Fatal Error creating a pinger object:"
                           << ex.what()
                           << log4cpp::CategoryStream::ENDLINE;
    _isOK = false;
    return;
  }

  /**
   * Here we do not need to check if the creation of subscriptionManager
   * produces an error, because the main ice-core module did that as
   * preliminary init operation. We also do not need to lock the
   * ::getInstance() call, because the instance of singleton has been
   * already created.
   */
  subManager = subscriptionManager::getInstance();

  init();
}

//______________________________________________________________________________
iceUtil::eventStatusListener::eventStatusListener(const int& i,const string& hostcert)
  : CEConsumer(i),
    iceThread( "event status poller" ),
    status(api::job_statuses::UNKNOWN),
    proxyfile(hostcert),
    tcpport(i),
    conf(iceUtil::iceConfManager::getInstance()),
    log_dev( api::util::creamApiLogger::instance()->getLogger() ),
    _lb_logger( iceLBLogger::instance() ),
    _isOK( true ),
    cache( jobCache::getInstance() )
{
  createObject();
}

//______________________________________________________________________________
iceUtil::eventStatusListener::eventStatusListener(const int& i,
						  const string& hostcert,
						  const string& cert,
						  const string& key)
  : CEConsumer(i, cert.c_str(), key.c_str()),
    iceThread( "event status poller" ),
    status(api::job_statuses::UNKNOWN),
    proxyfile(hostcert),
    tcpport(i),
    conf(iceUtil::iceConfManager::getInstance()),
    log_dev( api::util::creamApiLogger::instance()->getLogger() ),
    _lb_logger( iceLBLogger::instance() ),
    _isOK( true ),
    cache( jobCache::getInstance() )
{
  createObject();
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::body( void )
{
    while( !isStopped() ) {
        log_dev->infoStream() << "eventStatusListener::body() - "
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
            log_dev->errorStream()
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
    log_dev->errorStream() 
        << "eventStatusListener::acceptJobStatus() - "
        << "ErrorCode=["
        << this->getErrorCode() << "]"
        << " ErrorMessage=["
        << this->getErrorMessage() << "]"
        << log4cpp::CategoryStream::ENDLINE;
    return;
  }

  /**
   * Checks if the SOAP message was correct (i.e.
   * containing a non-null topic)
   */
  if(!this->getEventTopic()) {
    log_dev->warnStream() 
        << "eventStatusListener::acceptJobStatus() - "
        << "NULL Topic received. Ignoring this notification...." 
        << log4cpp::CategoryStream::ENDLINE;
    return;
  }

  /**
   * Ignores notifications generated by non-ICE topics
   */
  {
    boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
    if(this->getEventTopic()->Name != conf->getICETopic()) {
      log_dev->warnStream() 
          << "eventStatusListener::acceptJobStatus() - "
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

  string ceurl, cemonURL;
  ostringstream hostport;

  {
      // Scoped lock to protect concurrent access to the job cache
      boost::recursive_mutex::scoped_lock M( jobCache::mutex );
      
      { 
          boost::recursive_mutex::scoped_lock cemonM( cemonUrlCache::mutex );
          for(jobCache::iterator it=cache->begin(); it != cache->end(); it++) {
              ceurl = it->getCreamURL();
              cemonURL =cemonUrlCache::getInstance()->getCEMonUrl( ceurl );
              log_dev->infoStream()
                  << "eventStatusListener::init() - "
                  << "For current CREAM, cemonUrlCache returned CEMon URL ["
                  << cemonURL<<"]"
                  << log4cpp::CategoryStream::ENDLINE;
	    if( cemonURL.empty() ) {
              try {
                  api::soap_proxy::CreamProxyFactory::getProxy()->Authenticate( conf->getHostProxyFile() );
                  api::soap_proxy::CreamProxyFactory::getProxy()->GetCEMonURL( ceurl.c_str(), cemonURL );
                  log_dev->infoStream() 
                      << "eventStatusListener::init() - "
                      << "For current CREAM, query to CREAM service "
                      << "returned CEMon URL ["
                      << cemonURL << "]"
                      << log4cpp::CategoryStream::ENDLINE;
                  cemonUrlCache::getInstance()->putCEMonUrl( ceurl, cemonURL );
              } catch(exception& ex) {

		cemonURL = ceurl;
		boost::replace_first(cemonURL,
				     conf->getCreamUrlPostfix(),
				     conf->getCEMonUrlPostfix()
				     );

		log_dev->errorStream() 
		  << "eventStatusListener::init() - Error retrieving"
		  << " CEMon's URL from CREAM's URL: "
		  << ex.what()
		  << ". Composing URL from configuration file: ["
		  << cemonURL << "]" 
		  << log4cpp::CategoryStream::ENDLINE;
		
		ceurls.insert( cemonURL );
		cemonUrlCache::getInstance()->putCEMonUrl( ceurl, cemonURL );
              }              
	    }
          }
      } // lock on the cemonUrlCache
  }

  /**
   * Now we've got a collection of CEMon urls (without duplicates,
   * thanks to the set's property) we've to check for subscription
   */
  for( set<string>::const_iterator it = ceurls.begin(); it!=ceurls.end(); it++) {
      log_dev->infoStream() 
          << "eventStatusListener::init() - Checking subscriptions to ["
          << *it << "]" 
          << log4cpp::CategoryStream::ENDLINE;
      
      // Must lock the subscription manager due to its singleton nature
      boost::recursive_mutex::scoped_lock M( subscriptionManager::mutex );
      if( !subManager->subscribedTo(*it) ) {
          log_dev->infoStream() 
              << "eventStatusListener::init() - Not subscribed to ["
              << *it << "]. Subscribing to it..."
              << log4cpp::CategoryStream::ENDLINE;

          if( !subManager->subscribe(*it) ) {
              log_dev->errorStream() 
                  << "eventStatusListener::init() - Subscription to ["<< *it 
                  << "] failed. Will not receives status notifications from it."
                  << log4cpp::CategoryStream::ENDLINE;
          } else {
              boost::recursive_mutex::scoped_lock M( subscriptionCache::mutex );
              subscriptionCache::getInstance()->insert(*it);
          }
      } else {
          log_dev->infoStream() 
              << "eventStatusListener::init() - Already subscribed to ["
              << *it << "]"
              << log4cpp::CategoryStream::ENDLINE;
      }
  }
}


//______________________________________________________________________________
void iceUtil::eventStatusListener::handleEvent( const monitortypes__Event& ev )
{
    if( ev.Message.empty() ) return;

    // First, convert the vector of messages into a vector of StatusNotification objects
    vector<StatusNotification> notifications;

    for ( vector<string>::const_iterator it = ev.Message.begin(); it != ev.Message.end(); ++it ) {
        try {
            notifications.push_back( StatusNotification( *it ) );
        } catch( iceUtil::ClassadSyntax_ex ex ) {
            log_dev->errorStream()
                << "eventStatusListenre::handleEvent() - "
                << "received a notification "
                << *it << " which could not be understood; error is: "
                << ex.what() << ". "
                << "Skipping this notification and hoping for the best..."
                << log4cpp::CategoryStream::ENDLINE;
        }
    }
    
//    if ( notifications.empty() )
//        return;

    if( notifications.begin()->getStatus() == api::job_statuses::PURGED ) {
	return;
    }

    boost::recursive_mutex::scoped_lock jc_M( jobCache::mutex );

    // NOTE: we assume that the all the notifications are for the SAME job.
    // This is important! The following code relies on this assumption

    jobCache::iterator jc_it( cache->lookupByCreamJobID( notifications.begin()->getCreamJobID() ) );

    // No job found in cache
    if ( jc_it == cache->end() ) {
        log_dev->warnStream()
            << "eventStatusListener::handleEvent() - "
            << "creamjobid ["
            << notifications.begin()->getCreamJobID()
            << "] was not found in the cache. "
            << "Ignoring the whole notification..."
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }

    // setLastSeen must be called ONLY if the job IS NOT in a TERMINAL state
    // (that means that more states are coming...),
    // like DONE-OK; otherwise the eventStatusPoller wont never purge it...
    if( !api::job_statuses::isFinished( jc_it->getStatus() ) ) {
      jc_it->setLastSeen( time(0) );
      cache->put( *jc_it );
    }

    // Now, for each status change notification, check if it has to be logged
    vector<StatusNotification>::const_iterator it;
    int count;
    for ( it = notifications.begin(), count=1; it != notifications.end(); ++it, ++count ) {

        // If the status is "PURGED" the StatusPoller will remove it asap form
        // the cache. So the listener can ignore this job

        if( it->getStatus() == api::job_statuses::PURGED ) 
            return;

        log_dev->infoStream() 
            << "eventStatusListener::handleEvent() - "
            << "Checking job [" << it->getCreamJobID()
            << "] with status [" << api::job_statuses::job_status_str[ it->getStatus() ] << "]"
            << " notification count=" << count
            << " num already logged=" << jc_it->get_num_logged_status_changes()
            << log4cpp::CategoryStream::ENDLINE;
        
        if ( jc_it->get_num_logged_status_changes() < count ) {
            jc_it->setStatus( it->getStatus() );
            jc_it->set_num_logged_status_changes( count );
	    
	    
	    
            _lb_logger->logEvent( iceLBEventFactory::mkEvent( *jc_it ) );
            // The job gets stored in the jobcache anyway by the logEvent method...
        } else {
            log_dev->debugStream()
                << "eventStatusListener::handleEvent() - Skipping current notification because contains old states"
                << log4cpp::CategoryStream::ENDLINE;
        }
    }
}

} // anonymous namespace
