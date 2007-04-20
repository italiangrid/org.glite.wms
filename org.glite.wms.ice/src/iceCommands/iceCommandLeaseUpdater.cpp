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
 * ICE lease updater
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceCommandLeaseUpdater.h"
#include "CreamProxyFactory.h"
#include "CreamProxyMethod.h"
#include "DNProxyManager.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"

#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "jobCache.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "creamJob.h"
#include "ice-core.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/BaseException.h"
#include "glite/ce/cream-client-api-c/JobUnknownException.h"

#include <algorithm>
#include "boost/functional.hpp"

namespace api_util  = glite::ce::cream_client_api::util;
namespace cream_api = glite::ce::cream_client_api;
namespace cream_exceptions = glite::ce::cream_client_api::cream_exceptions;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;

using namespace glite::wms::ice::util;
using namespace std;

//____________________________________________________________________________
iceCommandLeaseUpdater::iceCommandLeaseUpdater( ) throw() : 
  m_theProxy( CreamProxyFactory::makeCreamProxy( false ) ),
  m_log_dev( api_util::creamApiLogger::instance()->getLogger() ),
  m_lb_logger( iceLBLogger::instance() ),
  m_delta( iceConfManager::getInstance()->getConfiguration()->ice()->lease_delta_time() ),
  m_threshold( iceConfManager::getInstance()->getConfiguration()->ice()->lease_threshold_time() ),
  m_cache( jobCache::getInstance() )
{

}

//____________________________________________________________________________
void iceCommandLeaseUpdater::execute( ) throw()
{
    typedef list<CreamJob> cj_list_t;
    map< pair<string, string>, list< CreamJob >, ltstring> jobMap;
    cj_list_t jobs_to_check;

    { 
    	// acquire lock on the job cache
        boost::recursive_mutex::scoped_lock M( jobCache::mutex );

	// First of all let's removes from cache the lease-expired jobs
	for_each(m_cache->begin(), m_cache->end(), boost::bind1st( boost::mem_fn( &iceCommandLeaseUpdater::check_lease_expired ), this ));


	// Populates the mapping ( userDN, CEMonURL) -> array[JobID]
	// with ALL job. We want to renew the lease of ALL non lease-expired jobs
	// (even those which are not expiring)
        for(jobCache::iterator jit = m_cache->begin(); jit != m_cache->end(); ++jit) {

	  if( !jit->getCreamJobID().empty() )
	    jobMap[ make_pair( jit->getUserDN(), jit->getCreamURL()) ].push_back( *jit );
	  
        }

    } // releases lock on job cache
    
    for_each(jobMap.begin(), jobMap.end(), boost::bind1st( boost::mem_fn( &iceCommandLeaseUpdater::handle_jobs ), this ));




//     for ( cj_list_t::iterator it = jobs_to_check.begin(); it != jobs_to_check.end(); ++it) {

//         CREAM_SAFE_LOG(m_log_dev->infoStream() 
//                        << "iceCommandLeaseUpdater::execute() - "
//                        << "Checking LEASE for job "
//                        << describe_job( *it )
//                        << " isActive=" << it->is_active()
//                        << " - remaining=" << (it->getEndLease()-time(0))
//                        << " - threshold=" << m_threshold
//                        << log4cpp::CategoryStream::ENDLINE);        

//         if ( it->getEndLease() && it->getEndLease() <= time(0) ) {
//             // Remove expired job from cache
//             CREAM_SAFE_LOG(m_log_dev->warnStream()
//                            << "iceCommandLeaseUpdater::execute() - "
//                            << "Removing from cache lease-expired job "
//                            << describe_job( *it )
//                            << log4cpp::CategoryStream::ENDLINE);
//             CreamJob tmp_job( *it );

//             tmp_job.set_failure_reason( "Lease expired" );
//             m_lb_logger->logEvent( new job_done_failed_event( tmp_job ) );
//             glite::wms::ice::Ice::instance()->resubmit_job( tmp_job, "Lease expired" );
//             boost::recursive_mutex::scoped_lock M( jobCache::mutex );
//             jobCache::iterator tmp( m_cache->lookupByGridJobID( tmp_job.getGridJobID() ) );
// 	    m_cache->erase( tmp );
//         } else {
//             if( !it->getCreamJobID().empty() &&
//                 it->is_active() && 
//                 ( it->getEndLease() - time(0) <= m_threshold ) ) {
//                 update_lease_for_job( *it );
//             }
//         }
//     } // end for
}

//____________________________________________________________________________
// void iceCommandLeaseUpdater::update_lease_for_job( const CreamJob& j ) throw()
// {
//     map< string, time_t > newLease;
//     vector< string > jobids;
    
//     jobids.push_back( j.getCreamJobID() );

//     // Renew the lease
    
//     CREAM_SAFE_LOG(m_log_dev->infoStream()
// 		   << "iceCommandLeaseUpdater::update_lease_for_job() - "
// 		   << "updating lease for job "
//                    << describe_job( j )
// 		   << " m_delta=" << m_delta
// 		   << log4cpp::CategoryStream::ENDLINE);
    
//     try {

//         m_theProxy->Authenticate( j.getUserProxyCertificate() );
//         util::CreamProxy_Lease( j.getCreamURL(), jobids, m_delta, newLease ).execute( m_theProxy.get(), 3 );

//     } catch(cream_exceptions::JobUnknownException& ex) {
//         CREAM_SAFE_LOG(m_log_dev->errorStream()
//                        << "iceCommandLeaseUpdater::update_lease_for_job() - "
//                        << "CREAM doesn't know the current job "
//                        << describe_job( j ) 
//                        <<". Removing from cache."
//                        << log4cpp::CategoryStream::ENDLINE);
//         boost::recursive_mutex::scoped_lock M( jobCache::mutex );
//         jobCache::iterator tmp = m_cache->lookupByCreamJobID( j.getCreamJobID() );
//         m_cache->erase( tmp );
//         return;
      
//     } catch(cream_exceptions::BaseException& ex) {
//         CREAM_SAFE_LOG(m_log_dev->errorStream()
//                        << "iceCommandLeaseUpdater::update_lease_for_job() - "
//                        << "Error updating lease for job "
//                        << describe_job( j )
//                        << ": "<< ex.what()
//                        << log4cpp::CategoryStream::ENDLINE);
//         return;      
//     } catch ( soap_proxy::soap_ex& ex ) {
//         CREAM_SAFE_LOG(m_log_dev->errorStream()
//                        << "iceCommandLeaseUpdater::update_lease_for_job() - "
//                        << "Error updating lease for job "
//                        << describe_job( j ) << ": " << ex.what()
//                        << log4cpp::CategoryStream::ENDLINE);
//         return;
//     } catch(exception& ex) {
//       CREAM_SAFE_LOG(m_log_dev->errorStream()
// 		     << "iceCommandLeaseUpdater::update_lease_for_job() - "
// 		     << "Error updating lease for job "
// 		     << describe_job( j ) << ": " << ex.what()
// 		     << log4cpp::CategoryStream::ENDLINE);
//       return;
//     } catch(...) {
//       CREAM_SAFE_LOG(m_log_dev->errorStream()
// 		     << "iceCommandLeaseUpdater::update_lease_for_job() - "
// 		     << "Error updating lease for job "
// 		     << describe_job( j ) << ": Unkonwn exception catched"
// 		     << log4cpp::CategoryStream::ENDLINE);
//       return;
//     }

//     if ( newLease.find( j.getCreamJobID() ) != newLease.end() ) {
        
//         // The lease for this job has been updated
//         // So update the job cache as well...

//         CREAM_SAFE_LOG(m_log_dev->infoStream()
//                        << "iceCommandLeaseUpdater::update_lease_for_job() - "
//                        << "updating lease for job "
//                        << describe_job( j )
//                        << "; old lease ends " << time_t_to_string( j.getEndLease() )
//                        << " new lease ends " << time_t_to_string( newLease[ j.getCreamJobID() ] )
//                        << log4cpp::CategoryStream::ENDLINE);
        
//         boost::recursive_mutex::scoped_lock M( jobCache::mutex );
      
//         // re-read the current job from the cache in order
//         // to get modifications (if any) made by other threads
//         jobCache::iterator tmpJob = m_cache->lookupByCreamJobID( j.getCreamJobID() );
//         if(tmpJob != m_cache->end() ) {
//             tmpJob->setEndLease( newLease[ j.getCreamJobID() ] );
//             m_cache->put( *tmpJob ); // Be Careful!! This should not invalidate any iterator on the job cache, as the job j is guaranteed (in this case) to be already in the cache.            
//         }

//     } else {
//         // there was an error updating the lease
//         CREAM_SAFE_LOG(m_log_dev->errorStream()
//                        << "iceCommandLeaseUpdater::update_lease_for_job() - "
//                        << "unable to update lease for job "
//                        << describe_job( j )
//                        << "; old lease ends " << time_t_to_string( j.getEndLease() )
//                        << log4cpp::CategoryStream::ENDLINE);
//     }
// }



//____________________________________________________________________________
void iceCommandLeaseUpdater::handle_jobs(const pair< pair<string, string>, list< CreamJob > >& jobs) throw()
{
  boost::recursive_mutex::scoped_lock M( jobCache::mutex );

  CREAM_SAFE_LOG(m_log_dev->infoStream()
		 << "iceCommandLeaseUpdater::handle_jobs() - "
		 << "Updating lease for all " 
		 << jobs.second.size() 
		 << " jobs of the user ["
		 << jobs.first.first << "] to CREAM ["
		 << jobs.first.second << "]"
		 << log4cpp::CategoryStream::ENDLINE);
  
  list< CreamJob >::const_iterator it = jobs.second.begin();
  list< CreamJob >::const_iterator list_end = jobs.second.end();
  vector< string > jobs_to_update;

 
  string proxy;
  {
    boost::recursive_mutex::scoped_lock M( DNProxyManager::mutex );
    proxy = DNProxyManager::getInstance()->getBetterProxyByDN( jobs.first.first ) ;
  }

  while ( it != list_end ) {
    jobs_to_update.clear();
    
    // prepare job_to_update as a chunk of all jobs
    it = transform_n_elements( it, list_end, 100, back_inserter( jobs_to_update ), mem_fun_ref( &CreamJob::getCreamJobID ) );
    
    update_lease_for_multiple_jobs( jobs_to_update, proxy, jobs.first.second );

  }      
}

//____________________________________________________________________________
void iceCommandLeaseUpdater::update_lease_for_multiple_jobs( const vector<string>& job_ids, const string& userproxy, const string& endpoint ) throw()
{
  map< string, time_t > newLease;

    try {

      CREAM_SAFE_LOG(m_log_dev->infoStream()
		     << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
		     << "Authenticating with user proxy ["
		     << userproxy << "]"
		     << log4cpp::CategoryStream::ENDLINE);

        m_theProxy->Authenticate( userproxy );

	CREAM_SAFE_LOG(m_log_dev->infoStream()
		     << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
		     << "Connecting to CREAM ["
		     << endpoint << "]"
		     << log4cpp::CategoryStream::ENDLINE);

        util::CreamProxy_Lease( endpoint, job_ids, m_delta, newLease ).execute( m_theProxy.get(), 3 );

    } catch(cream_exceptions::JobUnknownException& ex) {

      // this happen if the vector job_ids contains only one job

        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "iceCommandLeaseUpdater::update_lease_for_job() - "
                       << "CREAM doesn't know the current job "
                       << *(job_ids.begin())
                       <<". Removing from cache."
                       << log4cpp::CategoryStream::ENDLINE);

	// jobCache's mutex is not needed because this private method is called only by handle_jobs that already
	// acquired the jobCache's mutex
        //boost::recursive_mutex::scoped_lock M( jobCache::mutex );

        jobCache::iterator tmp = m_cache->lookupByCreamJobID( *(job_ids.begin()) ); // vector job_ids caintained only one job otherwise this kind of exception wasn't catched
        m_cache->erase( tmp );
        return;
      
    } catch(cream_exceptions::BaseException& ex) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "iceCommandLeaseUpdater::update_lease_for_job() - "
                       << "Error updating lease for userproxy ["
                       << userproxy << "]"
                       << ": "<< ex.what()
                       << log4cpp::CategoryStream::ENDLINE);
        return;      
    } catch ( soap_proxy::soap_ex& ex ) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "iceCommandLeaseUpdater::update_lease_for_job() - "
                       << "Error updating lease for userproxy ["
                       << userproxy << "]: " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE);
        return;
    } catch(exception& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream()
		     << "iceCommandLeaseUpdater::update_lease_for_job() - "
		     << "Error updating lease for userproxy ["
		     << userproxy
		     << "]: " << ex.what()
		     << log4cpp::CategoryStream::ENDLINE);
      return;
    } catch(...) {
      CREAM_SAFE_LOG(m_log_dev->errorStream()
		     << "iceCommandLeaseUpdater::update_lease_for_job() - "
		     << "Error updating lease for userproxy ["
		     << userproxy << "]: Unkonwn exception catched"
		     << log4cpp::CategoryStream::ENDLINE);
      return;
    }

    // now have to compare the vector jobs with the returned map newLease to 
    // check if some job's lease has not been updated.

    /**

         TODO

    */
    for(vector<string>::const_iterator jobid=job_ids.begin();
	jobid != job_ids.end();
	++jobid)
      {
	CreamJob j( *(m_cache->lookupByCreamJobID( *jobid )) );

	if( newLease.find(*jobid) == newLease.end()) {
	  // CREAM didn't return any lease information for the job
	  // *jobid!! So there was an error updating the lease for this job
	  
	  CREAM_SAFE_LOG(m_log_dev->errorStream()
			 << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
			 << "unable to update lease for job "
			 << j.describe()
			 << log4cpp::CategoryStream::ENDLINE);
	  continue;
	}

        CREAM_SAFE_LOG(m_log_dev->infoStream()
		       << "iceCommandLeaseUpdater::update_lease_for_job() - "
		       << "updating lease for job "
		       << j.describe()
		       << "; old lease ends " << time_t_to_string( j.getEndLease() )
		       << " new lease ends " << time_t_to_string( newLease[ j.getCreamJobID() ] )
		       << log4cpp::CategoryStream::ENDLINE);
        
	
	
	// re-read the current job from the cache in order
	// to get modifications (if any) made by other threads
	jobCache::iterator tmpJob = m_cache->lookupByCreamJobID( *jobid );
	if(tmpJob != m_cache->end() ) {
	  tmpJob->setEndLease( newLease[ *jobid ] );
	  m_cache->put( *tmpJob ); // Be Careful!! This should not invalidate any iterator on the job cache, as the job j is guaranteed (in this case) to be already in the cache.            
	}
      }
}



//____________________________________________________________________________
void iceCommandLeaseUpdater::check_lease_expired( const CreamJob& job ) throw()
{
  if ( job.getEndLease() && job.getEndLease() <= time(0) ) {

    CREAM_SAFE_LOG(m_log_dev->warnStream()
		   << "iceCommandLeaseUpdater::check_lease_expired() - "
		   << "Removing from cache lease-expired job "
		   << job.describe()
		   << log4cpp::CategoryStream::ENDLINE);
    
    CreamJob tmp_job( job );
    
    tmp_job.set_failure_reason( "Lease expired" );
    m_lb_logger->logEvent( new job_done_failed_event( tmp_job ) );
    glite::wms::ice::Ice::instance()->resubmit_job( tmp_job, "Lease expired" );

    // note: locking on jobcache is not needed because this method is called (and must be) from a block (in the execute method)
    // that has already locked the cache.

    jobCache::iterator tmp( m_cache->lookupByGridJobID( tmp_job.getGridJobID() ) );
    m_cache->erase( tmp );
  }
}
