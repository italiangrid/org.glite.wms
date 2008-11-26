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
 * ICE status poller
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// ICE Headers
#include "iceCommandStatusPoller.h"
#include "subscriptionManager.h"
#include "iceLBEventFactory.h"
#include "iceLBEventFactory.h"
#include "CreamProxyMethod.h"
#include "iceConfManager.h"
#include "DNProxyManager.h"
#include "iceLBLogger.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "iceLBEvent.h"
#include "jobCache.h"
#include "ice-core.h"
#include "jobCache.h"
#include "iceUtils.h"

// Cream Client API Headers
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"

// WMS Headers
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/utilities/scope_guard.h"

// BOOST Headers
#include <boost/lexical_cast.hpp>
#include <boost/functional.hpp>

// STL Headers
#include <set>
#include <algorithm>
#include <cstdlib>

namespace cream_api  = glite::ce::cream_client_api;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;
namespace jobstat    = glite::ce::cream_client_api::job_statuses;
namespace wms_utils  = glite::wms::common::utilities;
using namespace glite::wms::ice::util;
using namespace std;

namespace { // begin anonymous namespace
    
    bool insert_condition(const CreamJob& J) {        
        if( J.getCompleteCreamJobID().empty() ) 
            return false;
        else 
            return true;
    }
    
    /**
     * This class is used to remove a bunch of jobs from the job
     * cache.  It should be invoked by a scope_guard object, and is
     * used to remove a bunch of jobs if the polling fails due (i.e.)
     * to an expired user proxy.
     *
     * TODO: Remove this class, as it is not being used right now.
     */
    class remove_bunch_of_jobs {
    protected:
        vector< string > m_cream_job_ids;
        string m_reason;
        iceLBLogger* m_lb_logger;
        jobCache* m_cache;
        log4cpp::Category* m_log_dev;
    public:
        /**
         * jobs must be a vector of COMPLETE cream job ids.
         */
        remove_bunch_of_jobs( const vector< string>& jobs ) :
            m_cream_job_ids( jobs ),
            m_reason( "Removed by ICE status poller" ),
            m_lb_logger( iceLBLogger::instance() ),
            m_cache( jobCache::getInstance() ),
            m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() )
        { };
        void set_reason( const string& new_reason ) {
            m_reason = new_reason;
        };
        void operator()( void ) {
            static const char* method_name = "remove_bunch_of_jobs::operator() - ";
            vector< string >::const_iterator it;
            for ( it=m_cream_job_ids.begin(); it != m_cream_job_ids.end(); ++it ) {
                boost::recursive_mutex::scoped_lock M( jobCache::mutex );
                jobCache::iterator j = m_cache->lookupByCompleteCreamJobID( *it );
                if ( m_cache->end() == j )
                    continue; // nothing to do

                CreamJob job( *j );
                CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
                               << "Removing job " << job.describe()
                               << " from the cache. Reason is: "
                               << m_reason);
                job.set_failure_reason( m_reason );
                m_lb_logger->logEvent( new job_aborted_event( job ) ); // ignore return value, the job will be removed from ICE cache anyway
                m_cache->erase( j );
            }
        }
    };


}; // end anonymous namespace

//____________________________________________________________________________
iceCommandStatusPoller::iceCommandStatusPoller( glite::wms::ice::Ice* theIce, bool poll_all_jobs ) :
    iceAbsCommand( "iceCommandStatusPoller" ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_lb_logger( iceLBLogger::instance() ),
    m_iceManager( theIce ),
    m_cache( jobCache::getInstance() ),
    m_threshold( iceConfManager::getInstance()->getConfiguration()->ice()->poller_status_threshold_time() ),
    m_max_chunk_size( iceConfManager::getInstance()->getConfiguration()->ice()->bulk_query_size() ), 
    m_empty_threshold( 10*60 ), // 10 minutes
    m_poll_all_jobs( poll_all_jobs ),
    m_conf( iceConfManager::getInstance() )
{
  m_empty_threshold = m_conf->getConfiguration()->ice()->ice_empty_threshold();
}


//____________________________________________________________________________
void iceCommandStatusPoller::get_jobs_to_poll( list< CreamJob >& result ) throw()
{
    static const char* method_name = "iceCommandStatusPoller::get_jobs_to_poll() - ";

    for( jobCache::iterator jit = m_cache->begin(); 
         jit != m_cache->end(); ++jit ) {
        
        if( jit->getCompleteCreamJobID().empty() ) {
            // This job doesn't have yet the CREAM Job ID. Skipping...
            continue;
        }
        
        time_t t_now( time(NULL) );
        time_t t_last_seen( jit->getLastSeen() ); // This can be zero for jobs which are being submitted right now. The value of the last_seen field of creamJob is set only before exiting from the execute() method of iceCommandSubmit.
        time_t t_last_empty_notification( jit->get_last_empty_notification() ); // The time ICE received the last empty notification for this job
        time_t oldness = t_now - t_last_seen;
        time_t empty_oldness = t_now - t_last_empty_notification;

        //
        // Q: When does a job get polled?
        //
        // A: A job gets polled if one of the following situations are true:
        //
        // 1. ICE starts. When ICE starts, it polls all the jobs it thinks
        // have not been purged yet.
        //
        // 2. ICE received the last non-empty status change
        // notification more than m_threshold seconds ago.
        //
        // 3. ICE received the last empty status change notification
        // more than 10*60 seconds ago (that is, 10 minutes).
        //
	// empty notification are effective to reduce the polling frequency if the m_threshold is much greater than 10x60 seconds
	
        if ( m_poll_all_jobs ||
	     ( ( t_last_seen > 0 ) && oldness >= m_threshold ) ||
             ( ( t_last_empty_notification > 0 ) && empty_oldness > m_empty_threshold ) ) { // empty_oldness must be greater than 10 minutes
            CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
                           << "Adding job " << jit->describe()
                           << " t_now=" << time_t_to_string( t_now )
                           << " t_last_nonempty_notification=" 
                           << time_t_to_string(t_last_seen)
                           << " oldness (t_last_nonempty_notification - t_now)="
                           << oldness 
                           << " threshold=" << m_threshold
                           << " t_last_empty_notification=" 
                           << time_t_to_string( t_last_empty_notification )
                           << " empty_oldness (t_last_empty_notification - t_now)=" 
                           << empty_oldness
                           << " empty_threshold=" << m_empty_threshold
                           << " force_polling=" << m_poll_all_jobs
                           );

            result.push_back( *jit );
        }
    }
}

//____________________________________________________________________________
list< soap_proxy::JobInfoWrapper > 
iceCommandStatusPoller::check_multiple_jobs( const string& user_dn, 
                                             const string& cream_url, 
                                             const vector< CreamJob >& cream_job_ids ) 
  throw()
{
    static const char* method_name = "iceCommandStatusPoller::check_multiple_jobs() - ";
    CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
                   << "Will poll " << cream_job_ids.size() 
                   << " job(s) of the user [" << user_dn << "] to CREAM ["
                   << cream_url << "]"
                   );

    vector< string > job_id_vector;

    for( vector< CreamJob >::const_iterator thisJob = cream_job_ids.begin(); 
         thisJob != cream_job_ids.end(); 
         ++thisJob) {
        job_id_vector.push_back( thisJob->getCompleteCreamJobID() );
        CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
                       << "Will poll job with CREAM job id = ["
                       << thisJob->getCompleteCreamJobID() << "]"
                       );
    }    

    // Following a discussion on 2008-11-12, we decided not to remove
    // jobs which cannot be polled. The reason being that errors may
    // be transient (so that the jobs could be polled in future), and
    // that the lease mechanism will take care of unreachable jobs
    // anyway.

    // Build the scope_guard object which will remove all jobs in the
    // vector from the job cache, if necessary.
    // remove_bunch_of_jobs remove_f( job_id_vector );
    // The following is needed because we want to pass to
    // remove_job_guard a reference to function remove_f, instead of a
    // copy of it.  Passing a reference here is needed because we can
    // later modify the object remove_f by changing the failure
    // reason, and we want the scope_guard to invoke the (modified)
    // function object.
    // boost::function<void()> remove_f_ref = boost::ref( remove_f );
    // wms_utils::scope_guard remove_jobs_guard( remove_f_ref );
    
    string proxy( DNProxyManager::getInstance()->getBetterProxyByDN( user_dn ).first );
    
    if ( proxy.empty() ) {
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                       << "A Proxy file for DN [" << user_dn
                       << "] is not available"
                       );
        // Cannot process the list of jobs submitted by this user;
        // the scope_guard will remove the jobs.
        return list< soap_proxy::JobInfoWrapper >();
    }
    
    CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
                   << "Authenticating with proxy [" << proxy << "]" 
                   );

    list<soap_proxy::JobInfoWrapper> the_job_status;
    
    try {
        
        soap_proxy::VOMSWrapper V( proxy );
        if( !V.IsValid( ) ) {
            // remove_f.set_reason( V.getErrorMessage() );
            throw cream_api::soap_proxy::auth_ex( V.getErrorMessage() );
        }
        
        if( V.getProxyTimeEnd() <= time(NULL) ) {
            // remove_f.set_reason( string("Proxy [")+proxy+"] is expired!" );
            throw cream_api::soap_proxy::auth_ex( string("Proxy [")+proxy+"] is expired!" );
        }
        
        CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
                       << "Connecting to [" << cream_url << "]"
                       );
        
        soap_proxy::AbsCreamProxy::InfoArrayResult res;        
        {
            vector<CreamJob>::const_iterator jobit;
            vector< soap_proxy::JobIdWrapper > jobVec;
            
            for(jobit = cream_job_ids.begin(); jobit != cream_job_ids.end(); ++jobit) {
                soap_proxy::JobIdWrapper J( jobit->getCreamJobID(), 
                                            cream_url, // we trust cream_url is the same for all jobs!!!
                                            vector<soap_proxy::JobPropertyWrapper>());
                jobVec.push_back( J );
            }
            
            soap_proxy::JobFilterWrapper req( jobVec, 
                                              vector<string>(), 
                                              -1, -1, 
                                              "", 
                                              "");
            
            
            
            CreamProxy_Info( cream_url, 
                             proxy,
                             &req,
                             &res).execute( 3 );
            // remove_jobs_guard.dismiss(); // dismiss the guard, we should be safe here...
            
        } // free some array



        map<string, boost::tuple<soap_proxy::JobInfoWrapper::RESULT, soap_proxy::JobInfoWrapper, string> >::const_iterator infoIt;
        
        for( infoIt = res.begin(); infoIt != res.end(); ++infoIt ) {
            boost::tuple<soap_proxy::JobInfoWrapper::RESULT, 
                soap_proxy::JobInfoWrapper, 
                string> thisInfo = infoIt->second;
            
            if ( thisInfo.get<0>() == soap_proxy::JobInfoWrapper::OK ) {
                
                the_job_status.push_back( thisInfo.get<1>() );
                
            } else {
                
                CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                               << "CREAM didn't return information for the Job=["
                               << infoIt->first << "] - DN=[" << user_dn
                               << "] - ProxyFile=[" << proxy
                               << "]. Error is: " << thisInfo.get<2>() 
                               << ". Removing this job from the cache"
                               );
                {
                    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
                    jobCache::iterator it = m_cache->lookupByCompleteCreamJobID( infoIt->first );
                    m_cache->erase( it );
                }
                
            }
            
        } // for

    } catch(soap_proxy::auth_ex& ex) {
      
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                       << "Cannot query status job for DN=["
                       << user_dn << "]. Exception is [" << ex.what() << "]"
                       );
        // remove_f.set_reason( ex.what() );
        sleep(1);

    } catch(soap_proxy::soap_ex& ex) {
        
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                       << "Cannot query status job for DN=["
                       << user_dn << "]. Exception is ["  << ex.what() << "]"
                       );
        // remove_f.set_reason( ex.what() );
        sleep(1);

    } catch(cream_api::cream_exceptions::InternalException& ex) {
      
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                       << "Cannot query status job for DN=["
                       << user_dn << "]. Exception is [" << ex.what() << "]"
                       );
        // remove_f.set_reason( ex.what() );
        
        // this ex can be raised if the remote service is not
        // reachable and scanJobs is called again immediately. Until
        // the service is down this could overload the cpu and the
        // logfile. So let's wait for a while before returning...
        sleep(1);
        
    } catch(exception& ex) {
        
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                       << "Cannot query status job for DN=["
                       << user_dn << "]. Exception is [" << ex.what() << "]"
                       );
        // remove_f.set_reason( ex.what() );

    } catch(...) {
        
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                       << "Cannot query status job for DN=["
                       << user_dn << "]. Unknown exception catched"
                       );
        
    }
    return the_job_status;
}

//----------------------------------------------------------------------------
void iceCommandStatusPoller::updateJobCache( const list< soap_proxy::JobInfoWrapper >& info_list ) throw()
{
    
    for_each( info_list.begin(), 
              info_list.end(), 
              boost::bind1st( boost::mem_fn( &iceCommandStatusPoller::update_single_job ), this ));
    
}

//____________________________________________________________________________
void iceCommandStatusPoller::update_single_job( const soap_proxy::JobInfoWrapper& info_obj ) throw()
{
    static const char* method_name = "iceCommandStatusPoller::update_single_job() - ";
    // Lock the cache
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    vector< soap_proxy::JobStatusWrapper > status_changes;
    info_obj.getStatus( status_changes );
    
    // FIXME: must get cream_url from the JobInfoWrapper structure
    string completeJobID = info_obj.getCreamURL();
    boost::replace_all( completeJobID, 
			m_conf->getConfiguration()->ice()->cream_url_postfix(), "" );
    
    completeJobID += "/" + info_obj.getCreamJobID();
    
    CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                    << "Updating status for CREAM Job ID ["
                    << info_obj.getCreamJobID() << "] CREAM URL ["
                    << info_obj.getCreamURL() << "]"
                    );
    
    jobCache::iterator job_pos( m_cache->lookupByCompleteCreamJobID( completeJobID ) );
    if ( m_cache->end() != job_pos ) {
        CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                        << " Managing Job " << job_pos->describe()
                        << " for which I already processed "
                        << job_pos->get_num_logged_status_changes()
                        << " status changes, and JobStatus contains "
                        << status_changes.size() << " status changes"
                        );

        CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name 
                        << "Job " << job_pos->describe()
                        << " has worker_node=" << info_obj.getWorkerNode()
                        );
    }

    int count;
    vector< soap_proxy::JobStatusWrapper >::const_iterator it;

    for ( it = status_changes.begin(), count = 1; it != status_changes.end(); ++it, ++count ) {

        //
        // Warning: the following block of code CANNOT be moved
        // outside the 'for' cycle; the reason is that ICE should be
        // tolerant to receiving inconsistent notifications, such that
        // two "DONE-FAILED" events. After the first DONE-FAILED, ICE
        // would purge the job. This means that the jit iterator would
        // no longer be valid. Hence, the necessity to check each
        // time.
        //
        jobCache::iterator jit( m_cache->lookupByCompleteCreamJobID( completeJobID ) );    
	
        if ( m_cache->end() == jit ) {
            CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name 
                           << "cream_jobid [" << completeJobID 
                           << "] disappeared!"
                           );
            return;
        }
        
        // Creates a temporary job
        CreamJob tmp_job( *jit );
        
        // Update the worker node
        tmp_job.set_worker_node( info_obj.getWorkerNode() );

        //
        // END block NOT to be moved outside the 'for' loop
        //
        tmp_job.setLastSeen( time(0) );
        tmp_job.set_last_empty_notification( time(0) );

        jobstat::job_status stNum( jobstat::getStatusNum( it->getStatusName() ) );
        // before doing anything, check if the job is "purged". If so,
        // remove from the cache and forget about it.
        if ( stNum == jobstat::PURGED ) {
            CREAM_SAFE_LOG(m_log_dev->warnStream() << method_name
                           << "Job " << tmp_job.describe()
                           << " is reported as PURGED. Removing from cache"
                           ); 
            m_cache->erase( jit );
            return;
        }
        
        string exitCode( it->getExitCode() );
        
        if ( tmp_job.get_num_logged_status_changes() < count ) {
            
            CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
                           << "Updating jobcache for " << jit->describe()
                           << " status = [" << it->getStatusName() << "]"
                           << " exit_code = [" << exitCode << "]"
                           << " failure_reason = [" << it->getFailureReason() << "]"
                           << " description = [" << it->getDescription() << "]"
                           );

            tmp_job.setStatus( stNum );
            try {
                tmp_job.set_exit_code( boost::lexical_cast< int >( exitCode ) );
            } catch( boost::bad_lexical_cast & ) {
                tmp_job.set_exit_code( 0 );
            }
            //
            // See comment in normalStatusNotification.cpp
            //
            if ( stNum == jobstat::CANCELLED ) {
                tmp_job.set_failure_reason( it->getDescription() );
            } else {
                tmp_job.set_failure_reason( it->getFailureReason() );
            }
            tmp_job.set_num_logged_status_changes( count );

            // Log to L&B
            iceLBEvent* ev = iceLBEventFactory::mkEvent( tmp_job );
            if ( ev ) {
                tmp_job = m_lb_logger->logEvent( ev );
            }
        }
        jit = m_cache->put( tmp_job );
        
        m_iceManager->resubmit_or_purge_job( jit );
    }
}

//____________________________________________________________________________
void iceCommandStatusPoller::execute( ) throw()
{
    list< CreamJob > j_list;
    {
        // moved mutex here (from the get_jobs_to_poll's body) because
        // of more code readability
        boost::recursive_mutex::scoped_lock M( jobCache::mutex );
        this->get_jobs_to_poll( j_list ); // this method locks the cache
    }
    
    if ( j_list.empty() ) 
        return; // give up if no job to check    
    
    // Step 1. Build the mapping between ( UserDN, CreamURL ) ->
    // list<CreamJobId>
    map< pair<string, string>, list< CreamJob >, ltstring> jobMap;
        
    jobMap_appender appender( jobMap, &insert_condition );
    for_each( j_list.begin(),
 	      j_list.end(),
 	      appender);

    // Step 2. Iterates over the map which has just been constructed.
    for( map< pair<string, string>, list<CreamJob> >::const_iterator jit = jobMap.begin(); 
         jit != jobMap.end(); ++jit ) {
        
        const string user_dn( jit->first.first );
        const string cream_url( jit->first.second );
        
 	
        
        list< CreamJob >::const_iterator it = (jit->second).begin();
        list< CreamJob >::const_iterator list_end = (jit->second).end();
        while ( it != list_end ) {
            vector<CreamJob> jobs_to_poll;
            it = copy_n_elements( it, 
                                  list_end, 
                                  m_max_chunk_size, 
                                  back_inserter( jobs_to_poll )
                                  );
            
            list< soap_proxy::JobInfoWrapper > j_status( check_multiple_jobs( user_dn, cream_url, jobs_to_poll ) ); // doesn't lock the cache
            
            updateJobCache( j_status );// modifies the cache, locks it job by job
        }
    }
}

