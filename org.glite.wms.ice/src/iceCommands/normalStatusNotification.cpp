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
 * ICE normal status notification handler
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#include "normalStatusNotification.h"

// ICE stuff
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEventFactory.h"
#include "iceUtils.h"
#include "ice-core.h"
#include "subscriptionManager.h"
#include "iceConfManager.h"
#include "ice-core.h"
#include "DNProxyManager.h"

// CREAM and WMS stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

// other gLite stuff
#include "classad_distribution.h"

// boost includes
#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"

// System includes
#include <iostream>
#include <string>

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;

//////////////////////////////////////////////////////////////////////////////
//
// Helper class StatusChange

/**
 * This class represents status change notifications as sent by
 * CEMON to ICE. A Job status change notification is a classad
 * with several fields indicating che cream job id of the job
 * whose status changed, the new status, and the timestamp when
 * the change occurred.
 */
class StatusChange {
public:
    /**
     * Builds a StatusChange object from a classad.
     *
     * @param ad the string representing a classad to build
     * this object from.
     */
    StatusChange( const string& ad ) throw( ClassadSyntax_ex& );
    virtual ~StatusChange( ) { };

    /**
     * Returns the CREAM job ID for this notification
     */
    string get_cream_job_id( void ) const { 

      string completeURL = m_cream_url;
      boost::replace_all( completeURL, 
			  iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_postfix(), "" );

      completeURL += "/" + m_cream_job_id;

      //      return m_cream_job_id; 
      
      return completeURL;
      
    }; // FIXME: Take into account the CREAM URL also!!!

    /**
     * Returns the status for this notification
     */
    api::job_statuses::job_status  get_status( void ) const { return m_job_status; };

    /**
     * Returns the exit code
     */ 
    int get_exit_code( void ) const { return m_exit_code; };


    /**
     * Returns the worker node
     */
    const string& get_worker_node( void ) {return m_worker_node; };

    /**
     * Apply this status change notification to all relevant jobs in
     * cache.  This means that the status of all jobs referenced in
     * this notification (and possibly their exit codes) are changed
     * according to this notification.
     */
    void apply_to_job( CreamJob& j ) const;

    const string& get_failure_reason( void ) const { return m_failure_reason; };
    const string& get_description( void ) const { return m_description; };

protected:

    string m_cream_job_id;
    string m_cream_url;
    api::job_statuses::job_status m_job_status;
    bool m_has_exit_code;
    bool m_has_failure_reason;
    bool m_has_description;
    int m_exit_code;
    string m_failure_reason;
    string m_worker_node;
    string m_description;
};

//
//
//______________________________________________________________________________
StatusChange::StatusChange( const string& ad_string ) throw( ClassadSyntax_ex& ) :
    m_has_exit_code( false ),
    m_has_failure_reason( false ),
    m_has_description( false ),
    m_exit_code( 0 ) // default
{

  { // Classad-mutex protected region
    boost::recursive_mutex::scoped_lock M_classad( glite::wms::ice::Ice::ClassAd_Mutex );
    
    classad::ClassAdParser parser;
    classad::ClassAd *ad = parser.ParseClassAd( ad_string );	
    
    if (!ad)
      throw ClassadSyntax_ex( boost::str( boost::format("StatusChange() got an error while parsing notification classad: %1%" ) % ad_string ) );
    
    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( ad );
    
    if ( !classad_safe_ptr->EvaluateAttrString( "CREAM_JOB_ID", m_cream_job_id ) )
      throw ClassadSyntax_ex( boost::str( boost::format( "StatusChange(): CREAM_JOB_ID attribute not found, or is not a string, in classad: %1%") % ad_string ) );
    boost::trim_if( m_cream_job_id, boost::is_any_of("\"" ) );
    
    if ( !classad_safe_ptr->EvaluateAttrString( "CREAM_URL", m_cream_url ) )
      throw ClassadSyntax_ex( boost::str( boost::format( "StatusChange(): CREAM_URL attribute not found, or is not a string, in classad: %1%") % ad_string ) );
    boost::trim_if( m_cream_url, boost::is_any_of("\"" ) );
    
    string job_status_str;
    if ( !classad_safe_ptr->EvaluateAttrString( "JOB_STATUS", job_status_str ) )
      throw ClassadSyntax_ex( boost::str( boost::format( "StatusChange(): JOB_STATUS attribute not found, or is not a string, in classad: %1%") % ad_string ) );
    boost::trim_if( job_status_str, boost::is_any_of("\"" ) );
    m_job_status = api::job_statuses::getStatusNum( job_status_str );
    
    if ( classad_safe_ptr->EvaluateAttrInt( "EXIT_CODE", m_exit_code ) ) {
      m_has_exit_code = true;
    }
    
    if ( classad_safe_ptr->EvaluateAttrString( "FAILURE_REASON", m_failure_reason ) ) {
      m_has_failure_reason = true;
      boost::trim_if( m_failure_reason, boost::is_any_of("\"") );
    }
    
    if ( classad_safe_ptr->EvaluateAttrString( "DESCRIPTION", m_description ) ) {
      m_has_description = true;
      boost::trim_if( m_description, boost::is_any_of("\"") );
    }
    
    if ( classad_safe_ptr->EvaluateAttrString( "WORKER_NODE", m_worker_node ) ) {
      boost::trim_if( m_failure_reason, boost::is_any_of("\"") );
    }
  
    // "Pretty print" the classad
    string pp_classad;
    classad::ClassAdUnParser unparser;
    unparser.Unparse( pp_classad, classad_safe_ptr.get() );
    if (!getenv("NO_LISTENER_MESS")) {
      CREAM_SAFE_LOG(api::util::creamApiLogger::instance()->getLogger()->debugStream()
		     << "StatusChange::CTOR() - "
		     << "Parsed status change notification "
		     << pp_classad
		     );
    }
  } // end of Classad-mutex protected region
};

//
//
//______________________________________________________________________________
void StatusChange::apply_to_job( CreamJob& j ) const
{
    j.setStatus( get_status() );
    j.set_worker_node( m_worker_node );
    if ( m_has_exit_code ) {
        j.set_exit_code( m_exit_code );
    }    
    // 
    // Discussion with Gi, 2008/02/11: The failure reason field should
    // be taken only for jobs in aborted/done_failed state.  If the
    // job is in cancelled state, the cancel reason is located into
    // the description field. The same is done in the
    // iceCommandStatusPoller
    //
    if ( get_status() == api::job_statuses::CANCELLED ) {
        j.set_failure_reason( m_description );
    } else {
        j.set_failure_reason( m_failure_reason );
    }
}


//////////////////////////////////////////////////////////////////////////////
//
// normalStatusNotification class
normalStatusNotification::normalStatusNotification( const monitortypes__Event& ev, const string& cemondn ) : // FIXME can throw anything!!
    absStatusNotification( ),
    m_ev( ev ),
    m_cemondn( cemondn )
{
    log4cpp::Category *m_log_dev( api::util::creamApiLogger::instance()->getLogger() );
    static const char *method_name = "normalStatusNotification::normalStatusNotification() - ";
    
    if( m_ev.Message.empty() ) {        
        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "got a CEMon notification with no messages. Skipping"
                        );
        throw runtime_error( "got empty notification" );
    }
    
    CREAM_SAFE_LOG( m_log_dev->debugStream()
                    << method_name
                    << "processing normal status change notification"
                    );
    
    string cream_job_id;
    
    // First, we need to get the jobID for which this notification 
    // refers. In order to do so, we need to parse at least the first
    // notification in the event.
    try {

        StatusChange first_notification( *(m_ev.Message.begin()) );
        m_complete_cream_jobid = first_notification.get_cream_job_id();

    } catch( ClassadSyntax_ex& ex ) {
        CREAM_SAFE_LOG( m_log_dev->errorStream()
                        << method_name
                        << "Cannot parse the first notification "
                        << *(m_ev.Message.begin())
                        << " due to error: "
                        << ex.what() << ". "
                        << "Skipping the whole monitor event and hoping for the best..." 
                        );
        throw; // FIXME!!!
    }

}

//
//
//______________________________________________________________________________
void normalStatusNotification::apply( void ) // can throw anything
{    
    log4cpp::Category *m_log_dev( api::util::creamApiLogger::instance()->getLogger() );
    static const char* method_name = "normalStatusNotification::execute() - ";

    // the costructor ensures that the notification is non-empty
    CREAM_SAFE_LOG( m_log_dev->debugStream()
                    << method_name
                    << "processing notification"
                    );

    iceLBLogger *m_lb_logger( iceLBLogger::instance() );
    jobCache *m_cache( jobCache::getInstance() );
    glite::wms::ice::Ice* m_ice_manager( glite::wms::ice::Ice::instance() );

    string cream_job_id;

    // First, we need to get the jobID for which this notification 
    // refers. In order to do so, we need to parse at least the first
    // notification in the event.
    try {

        StatusChange first_notification( *(m_ev.Message.begin()) );
        cream_job_id = first_notification.get_cream_job_id();

    } catch( ClassadSyntax_ex& ex ) {
        CREAM_SAFE_LOG( m_log_dev->errorStream()
                        << method_name
                        << "Cannot parse the first notification "
                        << *(m_ev.Message.begin())
                        << " due to error: "
                        << ex.what() << ". "
                        << "Skipping the whole monitor event and hoping for the best..." 
                        );
        return;
    }

    // Now that we (hopefully) have the jobid, we lock the cache
    // and find the job
    
    boost::recursive_mutex::scoped_lock jc_M( jobCache::mutex );    
    jobCache::iterator jc_it( m_cache->lookupByCompleteCreamJobID( cream_job_id ) );

    // No job found in cache. This is fine, we may be receiving "old"
    // notifications, for jobs which have been already purged.
    if ( jc_it == m_cache->end() ) {
        if(!getenv("NO_LISTENER_MESS"))
	    CREAM_SAFE_LOG(m_log_dev->warnStream()
                           << method_name
                           << "creamjobid ["
                           << cream_job_id
                           << "] was not found in the cache. "
                           << "Ignoring the whole notification..."
                           );
        return;
    }

    string _cemon_url;
    string proxy = DNProxyManager::getInstance()->getAnyBetterProxyByDN( jc_it->getUserDN() ).get<0>();
    subscriptionManager::getInstance()->getCEMonURL(proxy, jc_it->getCreamURL()/*m_cream_address*/, _cemon_url);
    string _cemon_dn;
    subscriptionManager::getInstance()->getCEMonDN( jc_it->getUserDN()/*m_user_dn*/, _cemon_url, _cemon_dn );
    //_cemon_dn = jc_it->get_cemon_dn();

    if( m_cemondn.compare( _cemon_dn /*jc_it->get_cemon_dn()*/ ) ) {
	CREAM_SAFE_LOG(m_log_dev->warnStream()
		       << method_name
		       << "the CEMon ["
		       << m_cemondn << "] that sent this notification "
		       << "apparently didn't receive the submission of current job "
		       << jc_it->describe()
		       << ". Ignoring the whole notification..."
		       );
	return;
    }

    //
    // Now, for each status change notification, check if it has to be logged
    // vector<StatusNotification>::const_iterator it;
    int count;
    vector<string>::const_iterator msg_it;
    for ( msg_it = m_ev.Message.begin(), count = 1;
          msg_it != m_ev.Message.end(); 
          ++msg_it, ++count ) {

        // setLastSeen must be called ONLY if the job IS NOT in a
        // TERMINAL state (that means that more states are coming...),
        // like DONE-OK; otherwise the eventStatusPoller will never
        // purge it...
        if( !api::job_statuses::isFinished( jc_it->getStatus() ) ) {
            jc_it->setLastSeen( time(0) );
            jc_it->set_last_empty_notification( time(0) );
	    
            jc_it = m_cache->put( *jc_it );
        }
        
        if ( count <= jc_it->get_num_logged_status_changes() ) {
            if (!getenv("NO_LISTENER_MESS")) {
                CREAM_SAFE_LOG(m_log_dev->debugStream()
                               << method_name
                               << "Skipping current notification because contains old states"
                               );
            }
            continue; // skip to the next job
        }

        boost::scoped_ptr< StatusChange > notif_ptr;
        try {
            StatusChange* n = new StatusChange( *msg_it );
            notif_ptr.reset( n );
        } catch( ClassadSyntax_ex ex ) {
            if (!getenv("NO_LISTENER_MESS"))
                CREAM_SAFE_LOG(m_log_dev->errorStream()
                               << method_name
                               << "received a notification "
                               << *msg_it << " which could not be understood; error is: "
                               << ex.what() << ". "
                               << "Skipping this notification and hoping for the best..."
                               );
            continue;
        }

        // If the status is "PURGED", remove the job from cache        
        if( notif_ptr->get_status() == api::job_statuses::PURGED ) {
            if (!getenv("NO_LISTENER_MESS"))
                CREAM_SAFE_LOG(m_log_dev->warnStream()
                               << method_name
                               << jc_it->describe()
                               << " is reported as PURGED. Removing from cache"
                               ); 
            m_cache->erase( jc_it );
            return;
        }
        
        if (!getenv("NO_LISTENER_MESS"))
            CREAM_SAFE_LOG(m_log_dev->debugStream() 
                           << method_name
                           << "Checking job [" << notif_ptr->get_cream_job_id()
                           << "] with status [" 
                           << api::job_statuses::job_status_str[ notif_ptr->get_status() ] << "]"
                           << " This is CEMON notification notification count=" << count
                           << " num already logged=" 
                           << jc_it->get_num_logged_status_changes()
                           );
        
        CreamJob tmp_job( *jc_it );
        notif_ptr->apply_to_job( tmp_job ); // apply status change to job
        tmp_job.set_num_logged_status_changes( count );
        iceLBEvent* ev = iceLBEventFactory::mkEvent( tmp_job );
        if ( ev ) {
            tmp_job = m_lb_logger->logEvent( ev );
        }
        jc_it = m_cache->put( tmp_job );
        jobCache::iterator jc_it_new = m_ice_manager->resubmit_or_purge_job( jc_it ); // FIXME!! May invalidate the jc_it iterator
        if ( jc_it_new != jc_it )
            return; // FIXME: Remove this temporary patch
    }
}
