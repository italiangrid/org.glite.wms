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
// bool check_lease_expired( const CreamJob& job ) throw()
// {
//   if ( job.getEndLease() && job.getEndLease() <= time(0) ) {

//     CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->warnStream()
// 		   << "iceCommandLeaseUpdater::check_lease_expired() - "
// 		   << "Removing from cache lease-expired job "
// 		   << job.describe()
// 		   << log4cpp::CategoryStream::ENDLINE);
    
//     CreamJob tmp_job( job );
    
//     tmp_job.set_failure_reason( "Lease expired" );
//     iceLBLogger::instance()->logEvent( new job_done_failed_event( tmp_job ) );
//     glite::wms::ice::Ice::instance()->resubmit_job( tmp_job, "Lease expired" );

//     // note: locking on jobcache is not needed because this method is called (and must be) from a block (in the execute method)
//     // that has already locked the cache.

//     jobCache::iterator tmp( jobCache::getInstance()->lookupByGridJobID( tmp_job.getGridJobID() ) );
//     jobCache::getInstance()->erase( tmp );
//     return true;
//   }

//   return false;
// }

//____________________________________________________________________________
namespace {

  //____________________________________________________________________________
  bool onlyupdate( const CreamJob& J ) { return true; }

  //____________________________________________________________________________
  bool check_lease_expired( const CreamJob& job ) throw()
  {
    if( !job.is_active() || job.can_be_purged() ) return false;

    if ( job.getEndLease() && job.getEndLease() <= time(0) ) {
      
      CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->warnStream()
		     << "iceCommandLeaseUpdater::check_lease_expired() - "
		     << "Removing from cache lease-expired job "
		     << job.describe()
		     << log4cpp::CategoryStream::ENDLINE);
      
      CreamJob tmp_job( job );
      
      tmp_job.set_failure_reason( "Lease expired" );
      iceLBLogger::instance()->logEvent( new job_done_failed_event( tmp_job ) );
      glite::wms::ice::Ice::instance()->resubmit_job( tmp_job, "Lease expired" );
      
      // note: locking on jobcache is not needed because this method is called (and must be) from a block (in the execute method)
      // that has already locked the cache.
      
      jobCache::iterator tmp( jobCache::getInstance()->lookupByGridJobID( tmp_job.getGridJobID() ) );
      jobCache::getInstance()->erase( tmp );
      return true;
    }
    
    return false;
  }
  
  //____________________________________________________________________________
  bool insert_condition( const CreamJob& J )
  {
    bool false_condition = J.getCreamJobID().empty() || check_lease_expired( J ) || (!J.is_active()) || J.can_be_purged();
    
    if( false_condition )
      return false;
    else
      return true;
  }

}

//____________________________________________________________________________
iceCommandLeaseUpdater::iceCommandLeaseUpdater( bool only_update ) throw() : 
  m_theProxy( CreamProxyFactory::makeCreamProxy( false ) ),
  m_log_dev( api_util::creamApiLogger::instance()->getLogger() ),
  m_lb_logger( iceLBLogger::instance() ),
  m_delta( iceConfManager::getInstance()->getConfiguration()->ice()->lease_delta_time() ),
  m_cache( jobCache::getInstance() ),
  m_only_update( only_update )
{

}

//____________________________________________________________________________
void iceCommandLeaseUpdater::execute( ) throw()
{
  //typedef list<CreamJob> cj_list_t;
  map< pair<string, string>, list< CreamJob >, ltstring> jobMap;
  //cj_list_t jobs_to_check;
  
  { 
    // acquire lock on the job cache
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    
    list<CreamJob> check_list;
    copy(m_cache->begin(), m_cache->end(), back_inserter(check_list));
    
    
    // Now the cache is purged from the lease-expired jobs, and we can 
    // populate the map ( userDN, CEMonURL) -> array[JobID]
    // with ALL remaining jobs (not yet expired). 
    // We want to renew the lease of ALL these jobs
    // (even those which are not expiring)
    /**
     * an appender() applied to a CreamJob J, appends J to the array whose key is a 
     * pair<J.getUserDN, J.getCEMonURL> (creating the key pair<J.getUserDN, J.getCEMonURL> if it doensn't already exist.
     */
    jobMap_appender *appender;
    
    /**
     * Be Careful: the cache removal of lease-expired job is performed by check_lease_expired procedure
     * that is called ONLY by the insert_condition function.
     * Then: using onlyupdate as "insert condition" for the appender means 
     * perform just a lease update of ALL jobs without any removal.
     */
    if( m_only_update )
      appender = new jobMap_appender( jobMap, &onlyupdate );
    else
      appender = new jobMap_appender( jobMap, &insert_condition );
  
    for_each( check_list.begin(),
	      check_list.end(),
	      *appender);

    delete appender;

    // Now jobMap contains all the job that are not lease-expired and that have a non empty creamJobID
    // also the cache has been purged by lease-expired job (and LB "informed" about them)
    for_each(jobMap.begin(), 
	     jobMap.end(), 
	     boost::bind1st( boost::mem_fn( &iceCommandLeaseUpdater::handle_jobs ), this ));
    
  }// releases lock on job cache
  
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
  //boost::recursive_mutex::scoped_lock M( jobCache::mutex );

  CREAM_SAFE_LOG(m_log_dev->infoStream()
		 << "iceCommandLeaseUpdater::handle_jobs() - "
		 << "Updating lease for all " 
		 << jobs.second.size() 
		 << " job(s) of the user ["
		 << jobs.first.first << "] to CREAM ["
		 << jobs.first.second << "]"
		 << log4cpp::CategoryStream::ENDLINE);
  
  for(list<CreamJob>::const_iterator it=jobs.second.begin();
      it != jobs.second.end();
      ++it)
    {
      CREAM_SAFE_LOG(m_log_dev->infoStream()
		     << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
		     << "Will update job ["
		     << it->describe() << "]"
		     << log4cpp::CategoryStream::ENDLINE);
    }

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
    it = transform_n_elements( it, 
			       list_end, 
			       iceConfManager::getInstance()->getConfiguration()->ice()->bulk_query_size(), 
			       back_inserter( jobs_to_update ), mem_fun_ref( &CreamJob::getCreamJobID ) );
    
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
		       << "Updating Lease for ["
		       << job_ids.size() << "] job(s). Connecting to CREAM ["
		       << endpoint << "]"
		       << log4cpp::CategoryStream::ENDLINE);

        util::CreamProxy_Lease( endpoint, job_ids, m_delta, newLease ).execute( m_theProxy.get(), 3 );

    } catch(cream_exceptions::JobUnknownException& ex) {

      // this happen if the vector job_ids contains only one job

        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
                       << "CREAM doesn't know the current job "
                       << *(job_ids.begin())
                       <<". Removing from cache."
                       << log4cpp::CategoryStream::ENDLINE);

	// jobCache's mutex is not needed because this private method is called only by handle_jobs that already
	// acquired the jobCache's mutex
        //boost::recursive_mutex::scoped_lock M( jobCache::mutex );

	if(!m_only_update) {
	  
	  jobCache::iterator tmp = m_cache->lookupByCreamJobID( *(job_ids.begin()) ); // vector job_ids caintained only one job otherwise this kind of exception wasn't thrown
	  m_cache->erase( tmp );
	}

	return;
      
    } catch( cream_exceptions::GenericException& ex) {

      /**
       *  TEMPORARY PATCH, untill CREAM will not raise any fault in this situation
       */ 

      // This fault should be returned if some of the jobs was not lease-updated
      // in this case ICE will ignore and will consider that all jobs have been lease-updated
      // this string match is ugly and very raw!! but there is not another way to do that
      if( ex.getFaultCause() != "Invalid job ID list" ) {

	time_t newExpiration = time(NULL) + m_delta;

	CREAM_SAFE_LOG(m_log_dev->errorStream()
		       << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
		       << "Error updating lease for userproxy ["
		       << userproxy
		       << "]: " << ex.what()
		       << log4cpp::CategoryStream::ENDLINE);
	
	CREAM_SAFE_LOG(m_log_dev->errorStream()
		       << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
		       << "Because of the 'Invalid job ID list' ICE will make all lease for user ["
		       << userproxy
		       << "] to expire to [" << time_t_to_string( newExpiration ) << "]"
		       << log4cpp::CategoryStream::ENDLINE);

	// MUST UPDATE THE JOBCACHE WITH THE NEW LEASE FOR ALL JOBS
	for(vector<string>::const_iterator jobid=job_ids.begin();
	    jobid != job_ids.end();
	    ++jobid) {
	  jobCache::iterator tmpJob = m_cache->lookupByCreamJobID( *jobid );
	  if(tmpJob != m_cache->end() ) {
	    tmpJob->setEndLease( newExpiration );
	    m_cache->put( *tmpJob ); // Be Careful!! This should not invalidate any iterator on the job cache, as the job j is guaranteed (in this case) to be already in the cache.            
	  }
	}

	return;

      } else {

	CREAM_SAFE_LOG(m_log_dev->warnStream()
		       << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
		       << "Error updating lease for userproxy ["
		       << userproxy
		       << "] for some jobs: " << ex.what()
		       << log4cpp::CategoryStream::ENDLINE);
	// now proceed updating the lease of all jobs in the jobCache
	// assuming that the lease-update has been successful for all jobs! 
      }
  
    } catch(exception& ex) {

      CREAM_SAFE_LOG(m_log_dev->errorStream()
		     << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
		     << "Error updating lease for userproxy ["
		     << userproxy
		     << "]: " << ex.what()
		     << log4cpp::CategoryStream::ENDLINE);
      return;

    } catch(...) {

      CREAM_SAFE_LOG(m_log_dev->errorStream()
		     << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
		     << "Error updating lease for userproxy ["
		     << userproxy << "]: Unkonwn exception catched"
		     << log4cpp::CategoryStream::ENDLINE);
      return;

    }

    // now have to compare the vector jobs with the returned map newLease to 
    // check if some job's lease has not been updated.

  
    for(vector<string>::const_iterator jobid=job_ids.begin();
	jobid != job_ids.end();
	++jobid)
      {
	//CreamJob j( *(m_cache->lookupByCreamJobID( *jobid )) );
	jobCache::iterator tmpJob = m_cache->lookupByCreamJobID( *jobid );
	if( tmpJob == m_cache->end() )
	  {
	    CREAM_SAFE_LOG(m_log_dev->errorStream()
			   << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
			   << "CreamJobID ["
			   << tmpJob->getCreamJobID() << "] is not present in the cache!!! Skipping."
			   << log4cpp::CategoryStream::ENDLINE);
	    continue;
	  }
	
	if( newLease.find(*jobid) == newLease.end()) {
	  // CREAM didn't return any lease information for the job
	  // *jobid!! So there was an error updating the lease for this job
	  
	  CREAM_SAFE_LOG(m_log_dev->errorStream()
			 << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
			 << "unable to update lease for job "
			 << tmpJob->describe()
			 << log4cpp::CategoryStream::ENDLINE);
	  continue;
	}

        CREAM_SAFE_LOG(m_log_dev->infoStream()
		       << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
		       << "updating lease for job "
		       << tmpJob->describe()
		       << "; old lease ends " << time_t_to_string( tmpJob->getEndLease() )
		       << " new lease ends " << time_t_to_string( newLease[ tmpJob->getCreamJobID() ] )
		       << log4cpp::CategoryStream::ENDLINE);
        
	
	
	// re-read the current job from the cache in order
	// to get modifications (if any) made by other threads
	//jobCache::iterator tmpJob = m_cache->lookupByCreamJobID( *jobid );
	tmpJob->setEndLease( newLease[ *jobid ] );
	m_cache->put( *tmpJob ); // Be Careful!! This should not invalidate any iterator on the job cache, as the job j is guaranteed (in this case) to be already in the cache.            
      }
}
