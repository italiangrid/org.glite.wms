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
#include "subscriptionManager.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEventFactory.h"
#include "iceUtils.h"
#include "ice-core.h"

// CREAM stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

// other GLITE stuff
#include "classad_distribution.h"
#include "ClassadSyntax_ex.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

// boost includes
#include "boost/functional.hpp"
#include "boost/mem_fn.hpp"
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"

// System includes
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <set>

#include <sys/select.h>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

extern int errno;

using namespace std;
namespace api = glite::ce::cream_client_api;
namespace iceUtil = glite::wms::ice::util;

#define ACCEPT_TIMEOUT 30

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
        // bool has_exit_code( void ) const { return m_has_exit_code; };

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
        bool m_has_failure_reason;
        int m_exit_code;
        string m_failure_reason;
        string m_worker_node;
    };

    //
    // StatusNotification implementation
    //
    StatusNotification::StatusNotification( const string& ad_string ) throw( iceUtil::ClassadSyntax_ex& ) :
        m_has_exit_code( false ),
        m_has_failure_reason( false ),
        m_exit_code( 0 ) // default
    {

        classad::ClassAdParser parser;
        classad::ClassAd *ad = parser.ParseClassAd( ad_string );
	

        if (!ad)
            throw iceUtil::ClassadSyntax_ex( boost::str( boost::format("StatusNotification() got an error while parsing notification classad: %1%" ) % ad_string ) );
        
	boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( ad );
	
        if ( !classad_safe_ptr->EvaluateAttrString( "CREAM_JOB_ID", m_cream_job_id ) )
            throw iceUtil::ClassadSyntax_ex( boost::str( boost::format( "StatusNotification(): CREAM_JOB_ID attribute not found, or is not a string, in classad: %1%") % ad_string ) );
        boost::trim_if( m_cream_job_id, boost::is_any_of("\"" ) );

        string job_status_str;
        if ( !classad_safe_ptr->EvaluateAttrString( "JOB_STATUS", job_status_str ) )
            throw iceUtil::ClassadSyntax_ex( boost::str( boost::format( "StatusNotification(): JOB_STATUS attribute not found, or is not a string, in classad: %1%") % ad_string ) );
        boost::trim_if( job_status_str, boost::is_any_of("\"" ) );
        m_job_status = api::job_statuses::getStatusNum( job_status_str );

        if ( classad_safe_ptr->EvaluateAttrInt( "EXIT_CODE", m_exit_code ) ) {
            m_has_exit_code = true;
        }

        if ( classad_safe_ptr->EvaluateAttrString( "FAILURE_REASON", m_failure_reason ) ) {
            m_has_failure_reason = true;
            boost::trim_if( m_failure_reason, boost::is_any_of("\"") );
        }

        if ( classad_safe_ptr->EvaluateAttrString( "WORKER_NODE", m_worker_node ) ) {
            boost::trim_if( m_failure_reason, boost::is_any_of("\"") );
        }

        // "Pretty print" the classad
        string pp_classad;
        classad::ClassAdUnParser unparser;
        unparser.Unparse( pp_classad, classad_safe_ptr.get() );
        if(!getenv("NO_LISTENER_MESS"))
	  CREAM_SAFE_LOG(api::util::creamApiLogger::instance()->getLogger()->infoStream()
			 << "StatusNotification::CTOR() - Parsed status change notification "
			 << pp_classad
			 << log4cpp::CategoryStream::ENDLINE);

    };

    void StatusNotification::apply_to_job( iceUtil::CreamJob& j ) const
    {
        j.setStatus( get_status() );
        j.set_worker_node( m_worker_node );
        if ( m_has_failure_reason ) {
            j.set_failure_reason( m_failure_reason );
        }
        if ( m_has_exit_code ) {
            j.set_exit_code( m_exit_code );
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
    m_conf(iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()),
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
    m_conf(iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()),
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
        if(!getenv("NO_LISTENER_MESS"))
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
  int localSocket = this->getLocalSocket();
  fd_set fds;
  struct timeval timeout;
  //sigset_t sig;
  int ret;
  FD_ZERO(&fds);
  FD_SET(localSocket, &fds);
  timeout.tv_sec = ACCEPT_TIMEOUT;
  timeout.tv_usec = 0;
  ret = select(localSocket+1, &fds, NULL, NULL, &timeout);

  if (ret == -1) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "eventStatusListener::acceptJobStatus() - "
		   << "select() error: "
		   << strerror(errno)
		   << log4cpp::CategoryStream::ENDLINE);
    //perror("select()");
    return;
  }
  else if (ret) { /* will do actualy the this->accept() call */ 
    CREAM_SAFE_LOG(m_log_dev->infoStream()
		   << "eventStatusListener::acceptJobStatus() - "
		   << "Listening socket received something to read: ready to accept a SOAP connection"
		   << log4cpp::CategoryStream::ENDLINE);
  }
    //printf("Data is available now.\n");
    /* FD_ISSET(0, &rfds) will be true. */
  else {
    //printf("No data within five seconds.\n");
    CREAM_SAFE_LOG(m_log_dev->infoStream()
		   << "eventStatusListener::acceptJobStatus() - "
		   << "No connection within " << ACCEPT_TIMEOUT << " seconds. Listening again."
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  }

    if( !this->accept() ) {
      //if(!getenv("NO_LISTENER_MESS"))
	CREAM_SAFE_LOG(m_log_dev->errorStream()
		       << "eventStatusListener::acceptJobStatus()"
		       << " - CEConsumer::Accept() returned false."
		       << log4cpp::CategoryStream::ENDLINE);
      return;
    }

    if(!getenv("NO_LISTENER_MESS"))
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

  { // start TIMING notification handling

    //    api::util::scoped_timer T("*** NOTIF HANDLING ***");

    if( m_conf->listener_enable_authz() && m_conf->listener_enable_authn() ) {
      string dn = this->getClientDN();
      if(!getenv("NO_LISTENER_MESS"))
	CREAM_SAFE_LOG(m_log_dev->infoStream()
		       << "eventStatusListener::acceptJobStatus() - "
		       << "Got DN ["
		       << dn << "] for host ["
		       << this->getClientName() << "]"
		       << log4cpp::CategoryStream::ENDLINE);
      
      if( !iceUtil::subscriptionManager::getInstance()->isAuthorized( dn ) ) {
	if(!getenv("NO_LISTENER_MESS"))
	  CREAM_SAFE_LOG(m_log_dev->warnStream() 
			 << "eventStatusListener::acceptJobStatus() - "
			 << "Remote notifying client DN has not been found is the"
			 << " subscriptionManager. Cannot authorize this notification. Ignoring it..." 
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
    if(this->getEventTopic()->Name != m_conf->ice_topic()) {
      CREAM_SAFE_LOG(m_log_dev->warnStream() 
		     << "eventStatusListener::acceptJobStatus() - "
		     << "Received a notification with TopicName="
		     << this->getEventTopic()->Name
		     << " that differs from the ICE's topic "
		     << "official name. Ignoring this notification."
		     << log4cpp::CategoryStream::ENDLINE);
      return;
    }
    
    //    const vector<monitortypes__Event>& evts;
    //    {
    //      api::util::scoped_timer T2("*** NOTIF HANDLING -CEConsumer::getEvents- ***");

    const vector<monitortypes__Event>& evts= this->getEvents();
      //    }
    
    /**
     * Loops over all events (event <-> job) For each event updates the
     * status of the related job in the jobCache getting the status from
     * last message of the event.
     */
    std::for_each( evts.begin(), 
		   evts.end(), 
		   boost::bind1st( boost::mem_fn( &eventStatusListener::handleEvent ), this ) );
  } // stop TIMING notification handling
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::init(void)
{
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::handleEvent( const monitortypes__Event& ev )
{
    if( ev.Message.empty() ) 
        return;

    std::string cream_job_id;

    // First, we need to get the jobID for which this notification 
    // refers. In order to do so, we need to parse at least the first
    // notification in the event.
    try {
        StatusNotification first_notification( *(ev.Message.begin()) );
        cream_job_id = first_notification.get_cream_job_id();
    } catch( iceUtil::ClassadSyntax_ex& ex ) {
        CREAM_SAFE_LOG( m_log_dev->errorStream()
                        << "eventStatusListener::handleEvent() - "
                        << "Cannot parse the first notification "
                        << *(ev.Message.begin())
                        << " due to error: "
                        << ex.what() << ". "
                        << "Skipping the whole monitor event and hoping for the best..." 
                        << log4cpp::CategoryStream::ENDLINE);
        return;
    }

    // Now that we (hopefully) have the jobid, we lock the cache
    // and find the job
    boost::recursive_mutex::scoped_lock jc_M( jobCache::mutex );    
    jobCache::iterator jc_it( m_cache->lookupByCreamJobID( cream_job_id ) );
    
    // No job found in cache. This is fine, we may be receiving "old"
    // notifications, for jobs which have been already purged.
    if ( jc_it == m_cache->end() ) {
        if(!getenv("NO_LISTENER_MESS"))
	    CREAM_SAFE_LOG(m_log_dev->warnStream()
                           << "eventStatusListener::handleEvent() - "
                           << "creamjobid ["
                           << cream_job_id
                           << "] was not found in the cache. "
                           << "Ignoring the whole notification..."
                           << log4cpp::CategoryStream::ENDLINE);
        return;
    }


    // Now, for each status change notification, check if it has to be logged
    // vector<StatusNotification>::const_iterator it;
    int count;
    vector<string>::const_iterator msg_it;
    for ( msg_it = ev.Message.begin(), count = 1;
          msg_it != ev.Message.end(); ++msg_it, ++count ) {

        if ( count <= jc_it->get_num_logged_status_changes() ) {
//             if (!getenv("NO_LISTENER_MESS")) {
//                 CREAM_SAFE_LOG(m_log_dev->debugStream()
//                                << "eventStatusListener::handleEvent() - "
//                                << "Skipping current notification because contains old states"
//                                << log4cpp::CategoryStream::ENDLINE);
//             }
            continue; // skip to the next job
        }

        boost::scoped_ptr< StatusNotification > notif_ptr;
        try {
            StatusNotification* n = new StatusNotification( *msg_it );
            notif_ptr.reset( n );
        } catch( iceUtil::ClassadSyntax_ex ex ) {
            if (!getenv("NO_LISTENER_MESS"))
                CREAM_SAFE_LOG(m_log_dev->errorStream()
                               << "eventStatusListenre::handleEvent() - "
                               << "received a notification "
                               << *msg_it << " which could not be understood; error is: "
                               << ex.what() << ". "
                               << "Skipping this notification and hoping for the best..."
                               << log4cpp::CategoryStream::ENDLINE);
            continue;
        }

        // If the status is "PURGED", remove the job from cache        
        if( notif_ptr->get_status() == api::job_statuses::PURGED ) {
            if (!getenv("NO_LISTENER_MESS"))
                CREAM_SAFE_LOG(m_log_dev->infoStream()
                               << "eventStatusListener::handle_event() - "
                               << iceUtil::describe_job( *jc_it )
                               << " is reported as PURGED. Removing from cache"
                               << log4cpp::CategoryStream::ENDLINE); 
            m_cache->erase( jc_it );
            return;
        }
        
        // setLastSeen must be called ONLY if the job IS NOT in a
        // TERMINAL state (that means that more states are coming...),
        // like DONE-OK; otherwise the eventStatusPoller will never
        // purge it...
        if( !api::job_statuses::isFinished( jc_it->getStatus() ) ) {
            jc_it->setLastSeen( time(0) );
            jc_it = m_cache->put( *jc_it );
        }
        
        if (!getenv("NO_LISTENER_MESS"))
            CREAM_SAFE_LOG(m_log_dev->debugStream() 
                           << "eventStatusListener::handleEvent() - "
                           << "Checking job [" << notif_ptr->get_cream_job_id()
                           << "] with status [" 
                           << api::job_statuses::job_status_str[ notif_ptr->get_status() ] << "]"
                           << " This is CEMON notification notification count=" << count
                           << " num already logged=" 
                           << jc_it->get_num_logged_status_changes()
                           << log4cpp::CategoryStream::ENDLINE);
        
        iceUtil::CreamJob tmp_job( *jc_it );
        notif_ptr->apply_to_job( tmp_job ); // apply status change to job
        tmp_job.set_num_logged_status_changes( count );
        iceLBEvent* ev = iceLBEventFactory::mkEvent( tmp_job );
        if ( ev ) {
            tmp_job = m_lb_logger->logEvent( ev );
        }
        jc_it = m_cache->put( tmp_job );
        m_ice_manager->resubmit_or_purge_job( jc_it ); // FIXME!! May invalidate the jc_it iterator
    }
}

} // anonymous namespace
