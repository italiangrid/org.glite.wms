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
#include <set>
#include "boost/functional.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"

namespace api_util  = glite::ce::cream_client_api::util;
namespace cream_api = glite::ce::cream_client_api;
namespace cream_exceptions = glite::ce::cream_client_api::cream_exceptions;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;

using namespace glite::wms::ice::util;
using namespace std;

//____________________________________________________________________________
namespace {

    /**
     * This class represents the tuple (user_dn, cream_url, lease_id)
     */    
    struct lease_tuple {
        std::string user_dn;
        std::string cream_url;
        std::string lease_id;

        lease_tuple( const std::string& dn, const std::string& uri, const std::string& id ) :
            user_dn( dn ),
            cream_url( url ),
            lease_id( id ) { };
    };

    /**
     * This class is used to compare two lease_tuple
     */
    struct lease_tuple_cmp {
        bool operator()( const struct lease_tuple& t1, const struct lease_tuple& t2 ) {
            return ( ( t1.user_dn < t2.user_dn ) &&
                     ( t1.cream_url < t2.cream_url ) &&
                     ( t1.lease_id < t2.lease_id ) );
        }
    };

    //________________________________________________________________________
    bool onlyupdate( const CreamJob& J ) 
    { 
        return true; 
    }
    
    //________________________________________________________________________
    bool check_lease_expired( const CreamJob& job ) throw()
    {
        if ( job.is_active() &&
             !job.can_be_purged() && 
             job.getEndLease() && 
             job.getEndLease() <= time(0) ) {
            
            CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->warnStream()
                           << "iceCommandLeaseUpdater::check_lease_expired() - "
                           << "Cancelling and removing from cache the lease-expired job "
                           << job.describe()
                           << log4cpp::CategoryStream::ENDLINE);
            
	    /**
	     * MUST CANCEL the lease-expired JOB
	     * TODO:...
	     */

	    {//-------------------------------------------------------------------
	      boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_theProxy( CreamProxyFactory::makeCreamProxy( false ) );
	      string proxy = DNProxyManager::getInstance()->getBetterProxyByDN( job.getUserDN() ) ;

	      try {
		cream_api::soap_proxy::VOMSWrapper V( m_theJob.getUserProxyCertificate() );
		if( !V.IsValid( ) ) {
		  throw cream_api::soap_proxy::auth_ex( V.getErrorMessage() );
		}

		vector<string> jobVec;
		jobVec.push_back( job.getCreamJobID() );
		CreamProxy_Cancel( job.getCreamURL(), vector<string>() ).execute( m_theProxy.get(), 3 );
		
	      } catch(...) {
		
		// Let's ignore this cancellation error

	      }
	      
	    } //-------------------------------------------------------------------
	    
            CreamJob tmp_job( job );
            
            tmp_job.set_failure_reason( "Lease expired" );
            iceLBLogger::instance()->logEvent( new job_done_failed_event( tmp_job ) );
            glite::wms::ice::Ice::instance()->resubmit_job( tmp_job, "Lease expired" );
            
            // note: locking on jobcache is not needed because this method is called (and must be) from a block (in the execute method)
            // that has already locked the cache.
            
            jobCache::iterator tmp( jobCache::getInstance()->lookupByGridJobID( tmp_job.getGridJobID() ) );
            jobCache::getInstance()->erase( tmp );
            return true;
        } else {
            return false;
        }
    }
    
    //_______________________________________________________________________
    bool insert_condition( const CreamJob& J )
    {
        return ( !J.getCreamJobID().empty() &&
                 !check_lease_expired( J ) &&
                 J.is_active() &&
                 !J.can_be_purged() );

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
    /**
     * Be Careful: the cache removal of lease-expired job is performed
     * by check_lease_expired procedure that is called ONLY by the
     * insert_condition function.  Then: using onlyupdate as "insert
     * condition" for the appender means perform just a lease update
     * of ALL jobs without any removal.
     */
    if( m_only_update )
        appender.reset( new jobMap_appender( jobMap, &onlyupdate ) );
    else
        appender.reset( new jobMap_appender( jobMap, &insert_condition ) );
    
    // [ userDN, CreamURL, lease_id ]
    set< lease_tuple, lease_tuple_cmp > lease_set;
    { // acquire lock on the job cache
        boost::recursive_mutex::scoped_lock M( jobCache::mutex );
        for ( jobCache::const_iterator it = m_cache->begin();
              it != m_cache->end(); ++it ) {
            if ( only_update )
                lease_set.push_back( lease_tuple( it->getUserDN(),
                                                  it->getEndpoint(),
                                                  it->getLeasID() );
        for_each( m_cache->begin(), m_cache->end(), *appender );
        }
    } // releases lock on job cache  
    
    // Now jobMap contains all the job that are not lease-expired and
    // that have a non empty creamJobID also the cache has been purged
    // by lease-expired job (and LB "informed" about them)

    for_each(jobMap.begin(), 
             jobMap.end(), 
             boost::bind1st( boost::mem_fn( &iceCommandLeaseUpdater::handle_jobs ), this ));    
}

//____________________________________________________________________________
void iceCommandLeaseUpdater::update_lease( const std::string user_dn, const std::string& cream_url, const std::string& lease_id )
{
    static const char* method_name = "iceCommandLeaseUpdater::handle_jobs() - ";

    CREAM_SAFE_LOG(m_log_dev->infoStream()
                   << method_name
                   << "Will update lease for all jobs for user [" 
                   << user_dn << "] on CREAM ["
                   << cream_url << "] with lease ID ["
                   << lease_id << "]"
                   << log4cpp::CategoryStream::ENDLINE);
            
    string userproxy( DNProxyManager::getInstance()->getBetterProxyByDN( jobs.first.first ) );
        
    try {
        
        CREAM_SAFE_LOG(m_log_dev->infoStream()
                       << method_name
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
        
        // this happens if the vector job_ids contains only one job
        boost::recursive_mutex::scoped_lock M( jobCache::mutex );
      jobCache::iterator tmp( m_cache->lookupByCreamJobID( *(job_ids.begin())) );

        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
                       << "CREAM doesn't know the current job "
                       //<< *(job_ids.begin())
		       << tmp->describe()
                       <<". Removing from cache."
                       << log4cpp::CategoryStream::ENDLINE);
                
	if(!m_only_update) {
        
	  //boost::recursive_mutex::scoped_lock M( jobCache::mutex );
            
	  //jobCache::iterator tmp = m_cache->lookupByCreamJobID( *(job_ids.begin()) ); // vector job_ids caintained only one job otherwise this kind of exception wasn't thrown
            m_cache->erase( tmp );
	}
        
	return;
        
    } catch( cream_exceptions::GenericException& ex) {
        
        /**
         * TEMPORARY PATCH, untill CREAM will not raise any fault in
         * this situation
         */ 
        
        // This fault should be returned if some of the jobs was not
        // lease-updated in this case ICE will ignore and will consider
        // that all jobs have been lease-updated this string match is
        // ugly and very raw!! but there is not another way to do that
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
                           << "Because of the 'Invalid job ID list' ICE will update all lease for user ["
                           << userproxy
                           << "] to expire to [" << time_t_to_string( newExpiration ) << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            
            // MUST UPDATE THE JOBCACHE WITH THE NEW LEASE FOR ALL JOBS
            for ( vector<string>::const_iterator jobid=job_ids.begin(); jobid != job_ids.end(); ++jobid) {

	      //boost::recursive_mutex::scoped_lock M( jobCache::mutex ); NOT NEEDED, already acquired above
                jobCache::iterator tmpJob = m_cache->lookupByCreamJobID( *jobid );
                if ( tmpJob != m_cache->end() ) {
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
    
   
    // temporary patch before to have a "new" Lease interface in CREAM:
    // Let's update with the only one element in the newLease vector
    // all jobs in the vector job_ids
    for ( vector<string>::const_iterator jobid=job_ids.begin(); 
	  jobid != job_ids.end(); 
	  ++jobid ) 
      {
	boost::recursive_mutex::scoped_lock M( jobCache::mutex );
	
	jobCache::iterator tmpJob = m_cache->lookupByCreamJobID( *jobid );
	if ( tmpJob == m_cache->end() ) {
	  CREAM_SAFE_LOG(m_log_dev->errorStream()
			 << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
			 << "Job with CreamJobID ["
			 << *jobid << "] is not present in the cache!!! "
			 << "Skipping."
			 << log4cpp::CategoryStream::ENDLINE);
	  continue;
        }

	CREAM_SAFE_LOG(m_log_dev->debugStream()
		       << "iceCommandLeaseUpdater::update_lease_for_multiple_jobs() - "
		       << "Updating jobCache's lease for job "
		       << tmpJob->describe()
		       << "; old lease ends " << time_t_to_string( tmpJob->getEndLease() )
		       << " new lease ends " << time_t_to_string( newLease.begin()->second )
		       << log4cpp::CategoryStream::ENDLINE);
	
	tmpJob->setEndLease( newLease.begin()->second );
	m_cache->put( *tmpJob ); // Be Careful!! This should not invalidate any iterator on the job cache, as the job j is guaranteed (in this case) to be already in the cache.    
      }

}
