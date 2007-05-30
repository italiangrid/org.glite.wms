/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ICE command for updating job status
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceCommandUpdateStatus.h"

// ICE stuff
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEventFactory.h"
#include "iceUtils.h"
#include "ice-core.h"
#include "DNProxyManager.h"
#include "subscriptionManager.h"

// CREAM stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

// other GLITE stuff
#include "classad_distribution.h"
#include "ClassadSyntax_ex.h"

// boost includes
#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"

// System includes
#include <iostream>
#include <sstream>

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;

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
    StatusNotification( const string& _classad ) throw( ClassadSyntax_ex& );
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
    void apply_to_job( CreamJob& j ) const;

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
StatusNotification::StatusNotification( const string& ad_string ) throw( ClassadSyntax_ex& ) :
    m_has_exit_code( false ),
    m_has_failure_reason( false ),
    m_exit_code( 0 ) // default
{

    classad::ClassAdParser parser;
    classad::ClassAd *ad = parser.ParseClassAd( ad_string );
	

    if (!ad)
        throw ClassadSyntax_ex( boost::str( boost::format("StatusNotification() got an error while parsing notification classad: %1%" ) % ad_string ) );
        
    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( ad );
	
    if ( !classad_safe_ptr->EvaluateAttrString( "CREAM_JOB_ID", m_cream_job_id ) )
        throw ClassadSyntax_ex( boost::str( boost::format( "StatusNotification(): CREAM_JOB_ID attribute not found, or is not a string, in classad: %1%") % ad_string ) );
    boost::trim_if( m_cream_job_id, boost::is_any_of("\"" ) );

    string job_status_str;
    if ( !classad_safe_ptr->EvaluateAttrString( "JOB_STATUS", job_status_str ) )
        throw ClassadSyntax_ex( boost::str( boost::format( "StatusNotification(): JOB_STATUS attribute not found, or is not a string, in classad: %1%") % ad_string ) );
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

void StatusNotification::apply_to_job( CreamJob& j ) const
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


iceCommandUpdateStatus::iceCommandUpdateStatus( const monitortypes__Event& ev, const string& cemonDN ) :
  m_ev( ev ),
  m_cemondn( cemonDN )
{

}

//____________________________________________________________________________
void iceCommandUpdateStatus::execute( ) throw( )
{    
    log4cpp::Category *m_log_dev( api::util::creamApiLogger::instance()->getLogger() );
    
    try { // FIXME: temporary fix

    if( m_ev.Message.empty() ) {

        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "iceCommandUpdateStatus::execute() - "
                        << "got empty notification, skipping"
                        << log4cpp::CategoryStream::ENDLINE);

        return;
    }

    CREAM_SAFE_LOG( m_log_dev->infoStream()
                    << "iceCommandUpdateStatus::execute() - "
                    << "processing notification"
                    << log4cpp::CategoryStream::ENDLINE);

    iceLBLogger *m_lb_logger( iceLBLogger::instance() );
    jobCache *m_cache( jobCache::getInstance() );
    glite::wms::ice::Ice* m_ice_manager( glite::wms::ice::Ice::instance() );

    string cream_job_id;

    // First, we need to get the jobID for which this notification 
    // refers. In order to do so, we need to parse at least the first
    // notification in the event.
    try {
        StatusNotification first_notification( *(m_ev.Message.begin()) );
        cream_job_id = first_notification.get_cream_job_id();
    } catch( ClassadSyntax_ex& ex ) {
        CREAM_SAFE_LOG( m_log_dev->errorStream()
                        << "iceCommandUpdateStatus::execute() - "
                        << "Cannot parse the first notification "
                        << *(m_ev.Message.begin())
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
                           << "iceCommandUpdateStatus::execute() - "
                           << "creamjobid ["
                           << cream_job_id
                           << "] was not found in the cache. "
                           << "Ignoring the whole notification..."
                           << log4cpp::CategoryStream::ENDLINE);
        return;
    }

    
    {
      string cemonurl, proxy, creamurl, cemondn;

      {
	boost::recursive_mutex::scoped_lock M( DNProxyManager::mutex );
	proxy = DNProxyManager::getInstance()->getBetterProxyByDN(jc_it->getUserDN());
      }

      creamurl = jc_it->getCreamURL();

      {
	boost::recursive_mutex::scoped_lock M( subscriptionManager::mutex );
	subscriptionManager::getInstance()->getCEMonURL( proxy, creamurl, cemonurl);
	subscriptionManager::getInstance()->getCEMonDN( proxy, cemonurl, cemondn);
      }
      
      if( cemondn != m_cemondn ) {
	CREAM_SAFE_LOG(m_log_dev->warnStream()
		       << "iceCommandUpdateStatus::execute() - "
		       << "the CEMon that sent this notification "
		       << "apparently didn't receive the submission of current job ["
		       << jc_it->describe()
		       << "]. Ignoring the whole notification..."
		       << log4cpp::CategoryStream::ENDLINE);
      }
    }

    // Now, for each status change notification, check if it has to be logged
    // vector<StatusNotification>::const_iterator it;
    int count;
    vector<string>::const_iterator msg_it;
    for ( msg_it = m_ev.Message.begin(), count = 1;
          msg_it != m_ev.Message.end(); ++msg_it, ++count ) {

        // setLastSeen must be called ONLY if the job IS NOT in a
        // TERMINAL state (that means that more states are coming...),
        // like DONE-OK; otherwise the eventStatusPoller will never
        // purge it...
        if( !api::job_statuses::isFinished( jc_it->getStatus() ) ) {
            jc_it->setLastSeen( time(0) );
            jc_it = m_cache->put( *jc_it );
        }
        
        if ( count <= jc_it->get_num_logged_status_changes() ) {
            if (!getenv("NO_LISTENER_MESS")) {
                CREAM_SAFE_LOG(m_log_dev->debugStream()
                               << "iceCommandUpdateStatus::execute() - "
                               << "Skipping current notification because contains old states"
                               << log4cpp::CategoryStream::ENDLINE);
            }
            continue; // skip to the next job
        }

        boost::scoped_ptr< StatusNotification > notif_ptr;
        try {
            StatusNotification* n = new StatusNotification( *msg_it );
            notif_ptr.reset( n );
        } catch( ClassadSyntax_ex ex ) {
            if (!getenv("NO_LISTENER_MESS"))
                CREAM_SAFE_LOG(m_log_dev->errorStream()
                               << "iceCommandUpdateStatus::execute() - "
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
                               << "iceCommandUpdateStatus::execute() - "
                               << jc_it->describe()
                               << " is reported as PURGED. Removing from cache"
                               << log4cpp::CategoryStream::ENDLINE); 
            m_cache->erase( jc_it );
            return;
        }
        
        if (!getenv("NO_LISTENER_MESS"))
            CREAM_SAFE_LOG(m_log_dev->debugStream() 
                           << "iceCommandUpdateStatus::execute() - "
                           << "Checking job [" << notif_ptr->get_cream_job_id()
                           << "] with status [" 
                           << api::job_statuses::job_status_str[ notif_ptr->get_status() ] << "]"
                           << " This is CEMON notification notification count=" << count
                           << " num already logged=" 
                           << jc_it->get_num_logged_status_changes()
                           << log4cpp::CategoryStream::ENDLINE);
        
        CreamJob tmp_job( *jc_it );
        notif_ptr->apply_to_job( tmp_job ); // apply status change to job
        tmp_job.set_num_logged_status_changes( count );
        iceLBEvent* ev = iceLBEventFactory::mkEvent( tmp_job );
        if ( ev ) {
            tmp_job = m_lb_logger->logEvent( ev );
        }
        jc_it = m_cache->put( tmp_job );
        m_ice_manager->resubmit_or_purge_job( jc_it ); // FIXME!! May invalidate the jc_it iterator
    }

    //
    // FIXME: Temporary fix
    // 
    } catch( std::exception& ex ) {
        CREAM_SAFE_LOG(m_log_dev->errorStream() 
                       << "iceCommandUpdateStatus::execute() - "
                       << "Got exception: "
                       << ex.what()
                       << ". Going on and hoping for the best"
                       << log4cpp::CategoryStream::ENDLINE);        
    } catch( ... ) {
        CREAM_SAFE_LOG(m_log_dev->errorStream() 
                       << "iceCommandUpdateStatus::execute() - "
                       << "Got unknown exception. "
                       << "Going on and hoping for the best"
                       << log4cpp::CategoryStream::ENDLINE);        
    }
}
