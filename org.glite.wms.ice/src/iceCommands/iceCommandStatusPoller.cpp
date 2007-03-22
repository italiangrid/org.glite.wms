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

#include "iceCommandStatusPoller.h"
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "iceLBEventFactory.h"
#include "iceConfManager.h"
#include "ice-core.h"
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "iceLBEventFactory.h"
#include "CreamProxyFactory.h"
#include "CreamProxyMethod.h"

#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <boost/lexical_cast.hpp>

#define STATUS_POLL_RETRY_COUNT 4

namespace cream_api  = glite::ce::cream_client_api;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;
namespace jobstat    = glite::ce::cream_client_api::job_statuses;
namespace ice_util   = glite::wms::ice::util;

using namespace std;

//____________________________________________________________________________
ice_util::iceCommandStatusPoller::iceCommandStatusPoller( glite::wms::ice::Ice* theIce, bool poll_all_jobs ) :
  m_theProxy( CreamProxyFactory::makeCreamProxy( false ) ),
  m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
  m_lb_logger( iceLBLogger::instance() ),
  m_iceManager( theIce ),
  m_cache( ice_util::jobCache::getInstance() ),
  m_threshold( ice_util::iceConfManager::getInstance()->getConfiguration()->ice()->poller_status_threshold_time() ),
  m_poll_all_jobs( poll_all_jobs )
{

}


//____________________________________________________________________________
list< ice_util::CreamJob > ice_util::iceCommandStatusPoller::get_jobs_to_poll( void ) 
{
    list<ice_util::CreamJob> result;
    boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
    
    for(ice_util::jobCache::iterator jit = m_cache->begin(); jit != m_cache->end(); ++jit) {

        if( jit->getCreamJobID().empty() ) {
            // This job doesn't have yet the CREAM Job ID. Skipping...
            continue;
        }
        
        time_t t_now( time(NULL) );
        time_t t_last_seen( jit->getLastSeen() ); // This can be zero for jobs which are being submitted right now. The value of the last_seen field of creamJob is set only before exiting from the execute() method of iceCommandSubmit.
        time_t oldness = t_now - t_last_seen;

        if ( m_poll_all_jobs ||
	     ( ( t_last_seen > 0 ) && oldness >= m_threshold ) ) {
            CREAM_SAFE_LOG(m_log_dev->debugStream() 
                           << "eventStatusPoller::get_jobs_to_poll() - "
                           << "GridJobID=[" << jit->getGridJobID() << "] "
                           << "CreamJobID=[" << jit->getCreamJobID() << "]"
                           << " t_now=" << t_now
                           << " t_last_seen=" << t_last_seen
                           << " oldness=" << oldness 
                           << " threshold=" << m_threshold
                           << log4cpp::CategoryStream::ENDLINE);

            result.push_back( *jit );
        }
    }
    return result;
}

//____________________________________________________________________________
 list< soap_proxy::JobInfo > ice_util::iceCommandStatusPoller::check_jobs( const list< ice_util::CreamJob > & job_list ) 
{
    list< soap_proxy::JobInfo > result;

    for ( list< ice_util::CreamJob >::const_iterator jit = job_list.begin(); jit != job_list.end(); ++jit ) {

        vector< string > job_to_query;
        vector< soap_proxy::JobInfo > the_job_status;

        job_to_query.push_back( jit->getCreamJobID() );

        CREAM_SAFE_LOG(m_log_dev->debugStream() 
                       << "eventStatusPoller::check_jobs() - "
                       << "Polling job with "
                       << "GridJobID=[" << jit->getGridJobID() << "] "
                       << "CreamJobID=[" << jit->getCreamJobID() << "]"
                       << log4cpp::CategoryStream::ENDLINE);
	
        try {
            m_theProxy->Authenticate( jit->getUserProxyCertificate() );
            CreamProxy_Info( jit->getCreamURL(), job_to_query, vector<string>(), the_job_status, -1, -1).execute( m_theProxy.get(), 3 );

        } catch( cream_api::cream_exceptions::JobUnknownException& ex) {
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::check_jobs() - "
                           << "CREAM responded JobUnknown for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" << ex.what() << "]. Removing it from the cache"
                           << log4cpp::CategoryStream::ENDLINE);
            
            boost::recursive_mutex::scoped_lock M( jobCache::mutex );
            ice_util::jobCache::iterator thisJobToRemove = m_cache->lookupByCreamJobID( jit->getCreamJobID() );
            m_cache->erase( thisJobToRemove );
            
            continue;
	  
        } catch(soap_proxy::auth_ex& ex) {

            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::check_jobs() - "
                           << "Cannot query status job for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" << ex.what() << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            continue;
            
        } catch(soap_proxy::soap_ex& ex) {

            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::check_jobs() - "
                           << "Cannot query status job for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" 
                           << ex.what() << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            continue;	  

        } catch(cream_api::cream_exceptions::BaseException& ex) {

            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::check_jobs() - "
                           << "Cannot query status job for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" 
                           << ex.what() << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            continue;

        } catch(cream_api::cream_exceptions::InternalException& ex) {
            
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::check_jobs() - "
                           << "Cannot query status job for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" 
                           << ex.what() << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            continue;

            // this ex can be raised if the remote service is not
            // reachable and scanJobs is called again
            // immediately. Untill the service is down this could
            // overload the cpu and the logfile. So let's wait for a
            // while before returning...
	  
        } catch(cream_api::cream_exceptions::DelegationException& ex) {
            
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::check_jobs() - "
                           << "Cannot query status job for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" 
                           << ex.what() << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            continue;
        }

        result.push_back( the_job_status.front() );

    }
    return result;
}


//----------------------------------------------------------------------------
void ice_util::iceCommandStatusPoller::updateJobCache( const list< soap_proxy::JobInfo >& info_list ) throw()
{
    for ( list< soap_proxy::JobInfo >::const_iterator it = info_list.begin(); it != info_list.end(); ++it ) {

        update_single_job( *it );

    }
}

//____________________________________________________________________________
void ice_util::iceCommandStatusPoller::update_single_job( const soap_proxy::JobInfo& info_obj ) throw()
{
    // Locks the cache
    boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );

    vector< soap_proxy::Status > status_changes;
    info_obj.getStatusList( status_changes );
    string cid( info_obj.getCreamJobID() ); // Cream job id

    int count;
    vector< soap_proxy::Status >::const_iterator it;

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
        ice_util::jobCache::iterator jit( m_cache->lookupByCreamJobID( cid ) );    
        
        if ( m_cache->end() == jit ) {
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::update_single_job() - cream_jobid ["
                           << cid << "] disappeared!"
                           << log4cpp::CategoryStream::ENDLINE);
            return;
        }
        
        // Update the worker node
        jit->set_worker_node( info_obj.getWorkerNode() );
        //
        // END block NOT to be moved outside the 'for' loop
        //

        jit->setLastSeen( time(0) );

        jobstat::job_status stNum( jobstat::getStatusNum( it->getStatusName() ) );
        // before doing anything, check if the job is "purged". If so,
        // remove from the cache and forget about it.
        if ( stNum == jobstat::PURGED ) {
            CREAM_SAFE_LOG(m_log_dev->infoStream()
                           << "eventStatusPoller::update_single_job() - "
                           << "Job with cream_job_id = ["
                           << jit->getCreamJobID()
                           << "], grid_job_id = ["
                           << jit->getGridJobID()
                           << "] is reported as PURGED. Removing from cache"
                           << log4cpp::CategoryStream::ENDLINE); 
            m_cache->erase( jit );
            return;
        }

        string exitCode( it->getExitCode() );

        if ( jit->get_num_logged_status_changes() < count ) {
            
            CREAM_SAFE_LOG(m_log_dev->infoStream()
                           << "eventStatusPoller::update_single_job() - "
                           << "Updating jobcache with "
                           << "grid_jobid = [" << jit->getGridJobID() << "] "
                           << "cream_jobid = [" << cid << "]"
                           << " status = [" << it->getStatusName() << "]"
                           << " exit_code = [" << exitCode << "]"
                           << " failure_reason = [" << it->getFailureReason() << "]"
                           << log4cpp::CategoryStream::ENDLINE);

            // Creates a temporary job
            ice_util::CreamJob tmp_job( *jit );

            tmp_job.setStatus( stNum );
            try {
                tmp_job.set_exit_code( boost::lexical_cast< int >( exitCode ) );
            } catch( boost::bad_lexical_cast & ) {
                tmp_job.set_exit_code( 0 );
            }
            tmp_job.set_failure_reason( it->getFailureReason() );
            tmp_job.set_num_logged_status_changes( count );

            // Log to L&B
            iceLBEvent* ev = iceLBEventFactory::mkEvent( tmp_job );
            if ( ev ) {
                tmp_job = m_lb_logger->logEvent( ev );
            }
            jit = m_cache->put( tmp_job );
            
            m_iceManager->resubmit_or_purge_job( jit );
        }
    }
}

void ice_util::iceCommandStatusPoller::execute( ) throw()
{
  list< ice_util::CreamJob > j_list( get_jobs_to_poll() );
  list< soap_proxy::JobInfo > j_status( check_jobs( j_list ) );
  try {
  
    updateJobCache( j_status );
  
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
			 << "eventStatusPoller::body() - "
			 << "catched std::exception: "
			 << ex.what()
			 << log4cpp::CategoryStream::ENDLINE);
  } catch(...) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
			 << "eventStatusPoller::body() - "
			 << "catched unknown exception"
			 << log4cpp::CategoryStream::ENDLINE);
  }
}
