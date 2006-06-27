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
#include "ice-core.h"

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
        const string& get_cream_job_id( void ) const { return m_cream_job_id; };

        /**
         * Returns the status for this notification
         */
        api::job_statuses::job_status  get_status( void ) const { return m_job_status; };

        /**
         * Returns true iff the notification has an exit_code attribute
         */
        bool has_exit_code( void ) const { return m_has_exit_code; };

        /**
         * Returns the exit code
         */ 
        int get_exit_code( void ) const { return m_exit_code; };


        /**
         * Returns the worker node
         */
        const string& get_worker_node( void ) {return m_worker_node; };

        /**
         * Apply this status change notification to job j. This means
         * that the status of j (and possibly its exit code) is changed
         * according to this notification.
         *
         * @param j the job on which this status change notification
         * should be applied.
         */
        void apply_to_job( iceUtil::CreamJob& j ) const;

        const string& get_failure_reason( void ) const { return m_failure_reason; };

    protected:
        string m_cream_job_id;
        api::job_statuses::job_status m_job_status;
        bool m_has_exit_code;
        int m_exit_code;
        string m_failure_reason;
        string m_worker_node;
    };

    //
    // StatusNotification implementation
    //
    StatusNotification::StatusNotification( const string& ad_string ) throw( iceUtil::ClassadSyntax_ex& ) :
        m_has_exit_code( false ),
        m_exit_code( 0 ) // default
    {
        CREAM_SAFE_LOG(api::util::creamApiLogger::instance()->getLogger()->infoStream()
		       << "Parsing status change notification "
		       << ad_string
		       << log4cpp::CategoryStream::ENDLINE);

        classad::ClassAdParser parser;
        classad::ClassAd *ad = parser.ParseClassAd( ad_string );

        if (!ad)
            throw iceUtil::ClassadSyntax_ex( boost::str( boost::format("StatusNotification() got an error while parsing notification classad: %1%" ) % ad_string ) );
        
        if ( !ad->EvaluateAttrString( "CREAM_JOB_ID", m_cream_job_id ) )
            throw iceUtil::ClassadSyntax_ex( boost::str( boost::format( "StatusNotification(): CREAM_JOB_ID attribute not found, or is not a string, in classad: %1%") % ad_string ) );
        boost::trim_if( m_cream_job_id, boost::is_any_of("\"" ) );

        string job_status_str;
        if ( !ad->EvaluateAttrString( "JOB_STATUS", job_status_str ) )
            throw iceUtil::ClassadSyntax_ex( boost::str( boost::format( "StatusNotification(): JOB_STATUS attribute not found, or is not a string, in classad: %1%") % ad_string ) );
        boost::trim_if( job_status_str, boost::is_any_of("\"" ) );
        m_job_status = api::job_statuses::getStatusNum( job_status_str );

        if ( ad->EvaluateAttrInt( "EXIT_CODE", m_exit_code ) ) {
            m_has_exit_code = true;
        }

        ad->EvaluateAttrString( "FAILURE_REASON", m_failure_reason );
        boost::trim_if( m_failure_reason, boost::is_any_of("\"") );

        ad->EvaluateAttrString( "WORKER_NODE", m_worker_node );
        boost::trim_if( m_failure_reason, boost::is_any_of("\"") );

    };

    void StatusNotification::apply_to_job( iceUtil::CreamJob& j ) const
    {
        j.setStatus( get_status() );
        j.set_worker_node( m_worker_node );
        j.set_failure_reason( get_failure_reason() );
        if ( has_exit_code() ) {
            j.set_exit_code( get_exit_code() );
        }
    }


void iceUtil::eventStatusListener::createObject( void )
{
  try {
      m_myname = iceUtil::getHostName( );
  } catch( runtime_error& ex ) {
      m_isOK = false;
      return;
  }


  CREAM_SAFE_LOG(m_log_dev->info( "eventStatusListener::CTOR() - Listener created!" ));

  /**
   * Here we do not need to check if the creation of subscriptionManager
   * produces an error, because the main ice-core module did that as
   * preliminary init operation. We also do not need to lock the
   * ::getInstance() call, because the instance of singleton has been
   * already created.
   */
  m_subManager = subscriptionManager::getInstance();

  init();
}

//______________________________________________________________________________
iceUtil::eventStatusListener::eventStatusListener(const int& i,const string& hostcert)
  : CEConsumer(i),
    iceThread( "event status listener" ),
    m_conf(iceUtil::iceConfManager::getInstance()),
    m_log_dev( api::util::creamApiLogger::instance()->getLogger() ),
    m_lb_logger( iceLBLogger::instance() ),
    m_isOK( true ),
    m_cache( jobCache::getInstance() ),
    m_ice_manager( glite::wms::ice::Ice::instance() )
{
  createObject();
}

//______________________________________________________________________________
iceUtil::eventStatusListener::eventStatusListener(const int& i,
						  const string& hostcert,
						  const string& cert,
						  const string& key)
  : CEConsumer(i, cert.c_str(), key.c_str()),
    iceThread( "event status listener" ),
    m_conf(iceUtil::iceConfManager::getInstance()),
    m_log_dev( api::util::creamApiLogger::instance()->getLogger() ),
    m_lb_logger( iceLBLogger::instance() ),
    m_isOK( true ),
    m_cache( jobCache::getInstance() ),
    m_ice_manager( glite::wms::ice::Ice::instance() )
{
  createObject();
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::body( void )
{
    while( !isStopped() ) {
        CREAM_SAFE_LOG(m_log_dev->infoStream() 
                       << "eventStatusListener::body() - "
                       << "Waiting for job status notification"
                       << log4cpp::CategoryStream::ENDLINE);
        acceptJobStatus();
        sleep(1);
    }
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::acceptJobStatus(void)
{
  if( isStopped() ) {
    CREAM_SAFE_LOG(m_log_dev->infoStream()
		   << "eventStatusListener::acceptJobStatus() - "
		   << "eventStatusListener is ending."
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  }
  /**
   * Waits for an incoming connection
   */
    if( !this->accept() ) {
      CREAM_SAFE_LOG(m_log_dev->errorStream()
		     << "eventStatusListener::acceptJobStatus()"
		     << " - CEConsumer::Accept() returned false."
		     << log4cpp::CategoryStream::ENDLINE);
      return;
    }

    CREAM_SAFE_LOG(m_log_dev->infoStream()
		   << "eventStatusListener::acceptJobStatus() - "
		   << "Connection accepted from ["
		   << this->getClientName() << "] (" << this->getClientIP() << ")"
		   << log4cpp::CategoryStream::ENDLINE);

  /**
   * acquires the event from the client
   * and deserializes the data structures
   */
  if( !this->serve() ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "eventStatusListener::acceptJobStatus() - "
		   << "ErrorCode=["
		   << this->getErrorCode() << "]"
		   << " ErrorMessage=["
		   << this->getErrorMessage() << "]"
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  }

  if( m_conf->getListenerEnableAuthZ() ) {
    string remote_hostname = iceUtil::getNotificationClientDN( this->getClientDN() );
    if( !iceUtil::cemonUrlCache::getInstance()->hasCemon( remote_hostname ) ) {
      CREAM_SAFE_LOG(m_log_dev->warnStream() 
		   << "eventStatusListener::acceptJobStatus() - "
		   << "Remote notifying client is NOT recognized/authorized. Ignoring this notification..." 
		   << log4cpp::CategoryStream::ENDLINE);
      return;
    }
  }

  /**
   * Checks if the SOAP message was correct (i.e.
   * containing a non-null topic)
   */
  if(!this->getEventTopic()) {
    CREAM_SAFE_LOG(m_log_dev->warnStream() 
		   << "eventStatusListener::acceptJobStatus() - "
		   << "NULL Topic received. Ignoring this notification...." 
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  }

  /**
   * Ignores notifications generated by non-ICE topics
   */
  {
    //boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
    if(this->getEventTopic()->Name != m_conf->getICETopic()) {
      CREAM_SAFE_LOG(m_log_dev->warnStream() 
		     << "eventStatusListener::acceptJobStatus() - "
		     << "Received a notification with TopicName="
		     << this->getEventTopic()->Name
		     << " that differs from the ICE's topic "
		     << "official name. Ignoring this notification."
		     << log4cpp::CategoryStream::ENDLINE);
      return;
    }
  }

  const vector<monitortypes__Event>& evts = this->getEvents();

  /**
   * Loops over all events (event <-> job) For each event updates the
   * status of the related job in the jobCache getting the status from
   * last message of the event.
   */
  std::for_each( evts.begin(), 
                 evts.end(), 
		 boost::bind1st( boost::mem_fn( &eventStatusListener::handleEvent ), this ) );

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
      boost::recursive_mutex::scoped_lock cemonM( cemonUrlCache::mutex );
      for(jobCache::iterator it=m_cache->begin(); it != m_cache->end(); it++) {
          ceurl = it->getCreamURL();
          cemonURL =cemonUrlCache::getInstance()->getCEMonUrl( ceurl );
          CREAM_SAFE_LOG(m_log_dev->infoStream()
                         << "eventStatusListener::init() - "
                         << "For current CREAM, cemonUrlCache returned CEMon URL ["
                         << cemonURL<<"]"
                         << log4cpp::CategoryStream::ENDLINE);
          if( cemonURL.empty() ) {
              try {
                  api::soap_proxy::CreamProxyFactory::getProxy()->Authenticate( m_conf->getHostProxyFile() );
                  api::soap_proxy::CreamProxyFactory::getProxy()->GetCEMonURL( ceurl.c_str(), cemonURL );
                  CREAM_SAFE_LOG(m_log_dev->infoStream() 
                                 << "eventStatusListener::init() - "
                                 << "For current CREAM, query to CREAM service "
                                 << "returned CEMon URL ["
                                 << cemonURL << "]"
                                 << log4cpp::CategoryStream::ENDLINE);
                  cemonUrlCache::getInstance()->putCEMonUrl( ceurl, cemonURL );
              } catch(exception& ex) {
                  
                  cemonURL = ceurl;
                  boost::replace_first(cemonURL,
                                       m_conf->getCreamUrlPostfix(),
                                       m_conf->getCEMonUrlPostfix()
                                       );
                  
                  CREAM_SAFE_LOG(m_log_dev->errorStream() 
                                 << "eventStatusListener::init() - Error retrieving"
                                 << " CEMon's URL from CREAM's URL: "
                                 << ex.what()
                                 << ". Composing URL from configuration file: ["
                                 << cemonURL << "]" 
                                 << log4cpp::CategoryStream::ENDLINE);
                  
                  ceurls.insert( cemonURL );
                  cemonUrlCache::getInstance()->putCEMonUrl( ceurl, cemonURL );
              }              
          }
      }
  }

  /**
   * Now we've got a collection of CEMon urls (without duplicates,
   * thanks to the set's property) we've to check for subscription
   */
  for( set<string>::const_iterator it = ceurls.begin(); it!=ceurls.end(); it++) {
    CREAM_SAFE_LOG(m_log_dev->infoStream() 
		   << "eventStatusListener::init() - Checking subscriptions to ["
		   << *it << "]" 
		   << log4cpp::CategoryStream::ENDLINE);
      
      // Must lock the subscription manager due to its singleton nature
      boost::recursive_mutex::scoped_lock M( subscriptionManager::mutex );
      if( !m_subManager->subscribedTo(*it) ) {
	CREAM_SAFE_LOG(m_log_dev->infoStream() 
		       << "eventStatusListener::init() - Not subscribed to ["
		       << *it << "]. Subscribing to it..."
		       << log4cpp::CategoryStream::ENDLINE)

          if( !m_subManager->subscribe(*it) ) {
	    CREAM_SAFE_LOG(m_log_dev->errorStream() 
			   << "eventStatusListener::init() - Subscription to ["<< *it 
			   << "] failed. Will not receives status notifications from it."
			   << log4cpp::CategoryStream::ENDLINE);
          } else {
              boost::recursive_mutex::scoped_lock M( subscriptionCache::mutex );
              subscriptionCache::getInstance()->insert(*it);
          }
      } else {
	CREAM_SAFE_LOG(m_log_dev->infoStream() 
		       << "eventStatusListener::init() - Already subscribed to ["
		       << *it << "]"
		       << log4cpp::CategoryStream::ENDLINE);
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
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusListenre::handleEvent() - "
                           << "received a notification "
                           << *it << " which could not be understood; error is: "
                           << ex.what() << ". "
                           << "Skipping this notification and hoping for the best..."
                           << log4cpp::CategoryStream::ENDLINE);
        }
    }
    
    if( notifications.begin()->get_status() == api::job_statuses::PURGED ) {
	return;
    }
    
    boost::recursive_mutex::scoped_lock jc_M( jobCache::mutex );
    
    // NOTE: we assume that the all the notifications are for the SAME job.
    // This is important! The following code relies on this assumption
    
    jobCache::iterator jc_it( m_cache->lookupByCreamJobID( notifications.begin()->get_cream_job_id() ) );
    
    // No job found in cache. This is fine, we may be receiving "old"
    // notifications, for jobs which have been already purged.
    if ( jc_it == m_cache->end() ) {
        CREAM_SAFE_LOG(m_log_dev->warnStream()
                       << "eventStatusListener::handleEvent() - "
                       << "creamjobid ["
                       << notifications.begin()->get_cream_job_id()
                       << "] was not found in the cache. "
                       << "Ignoring the whole notification..."
                       << log4cpp::CategoryStream::ENDLINE);
        return;
    }
    
    // setLastSeen must be called ONLY if the job IS NOT in a TERMINAL state
    // (that means that more states are coming...),
    // like DONE-OK; otherwise the eventStatusPoller will never purge it...
    if( !api::job_statuses::isFinished( jc_it->getStatus() ) ) {
        jc_it->setLastSeen( time(0) );
        m_cache->put( *jc_it );
    }
    
    // Now, for each status change notification, check if it has to be logged
    vector<StatusNotification>::const_iterator it;
    int count;
    for ( it = notifications.begin(), count=1; it != notifications.end(); ++it, ++count ) {
    
        CREAM_SAFE_LOG(m_log_dev->debugStream() 
		       << "eventStatusListener::handleEvent() - "
		       << "Checking job [" << it->get_cream_job_id()
		       << "] with status [" << api::job_statuses::job_status_str[ it->get_status() ] << "]"
		       << " notification count=" << count
		       << " num already logged=" << jc_it->get_num_logged_status_changes()
		       << log4cpp::CategoryStream::ENDLINE);

        // If the status is "PURGED", remove the job from cache        
        if( it->get_status() == api::job_statuses::PURGED ) {
            CREAM_SAFE_LOG(m_log_dev->infoStream()
                           << "eventStatusListener::handle_event() - "
                           << "Job with cream_job_id = ["
                           << jc_it->getJobID()
                           << "], grid_job_id = ["
                           << jc_it->getGridJobID()
                           << "] is reported as PURGED. Removing from cache"
                           << log4cpp::CategoryStream::ENDLINE); 

            m_cache->erase( jc_it );
            return;
        }
        
        if ( jc_it->get_num_logged_status_changes() < count ) {
            iceUtil::CreamJob tmp_job( *jc_it );
            it->apply_to_job( tmp_job ); // apply status change to job
            tmp_job.set_num_logged_status_changes( count );
            m_lb_logger->logEvent( iceLBEventFactory::mkEvent( tmp_job ) );
            // The job gets stored in the jobcache anyway by the logEvent method...
        } else {
            CREAM_SAFE_LOG(m_log_dev->debugStream()
                           << "eventStatusListener::handleEvent() - "
                           << "Skipping current notification because contains old states"
                           << log4cpp::CategoryStream::ENDLINE);
        }

        m_ice_manager->resubmit_or_purge_job( jc_it );

    }
}

} // anonymous namespace
