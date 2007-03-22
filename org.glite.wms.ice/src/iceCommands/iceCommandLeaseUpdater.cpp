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

    cj_list_t jobs_to_check;

    { 
    	// acquire lock on job cache
        boost::recursive_mutex::scoped_lock M( jobCache::mutex );
        for(jobCache::iterator jit = m_cache->begin(); jit != m_cache->end(); ++jit) {
            jobs_to_check.push_back( *jit );
        }
    } // releases lock on job cache
    
    for ( cj_list_t::iterator it = jobs_to_check.begin(); it != jobs_to_check.end(); ++it) {
        if ( it->getEndLease() && it->getEndLease() <= time(0) ) {
            // Remove expired job from cache
            CREAM_SAFE_LOG(m_log_dev->warnStream()
                           << "iceCommandLeaseUpdater::execute() - "
                           << "Removing from cache lease-expired job "
                           << describe_job( *it )
                           << log4cpp::CategoryStream::ENDLINE);
            CreamJob tmp_job( *it );

            tmp_job.set_failure_reason( "Lease expired" );
            m_lb_logger->logEvent( new job_done_failed_event( tmp_job ) );
            glite::wms::ice::Ice::instance()->resubmit_job( tmp_job, "Lease expired" );
            boost::recursive_mutex::scoped_lock M( jobCache::mutex );
            jobCache::iterator tmp( m_cache->lookupByGridJobID( tmp_job.getGridJobID() ) );
	    m_cache->erase( tmp );
        } else {
            if( ! it->getCreamJobID().empty() ) {
                CREAM_SAFE_LOG(m_log_dev->infoStream() 
                               << "iceCommandLeaseUpdater::execute() - "
                               << "Checking LEASE for job "
                               << describe_job( *it )
                               << " isActive=" << it->is_active()
                               << " - remaining=" << (it->getEndLease()-time(0))
                               << " - threshold=" << m_threshold
                               << log4cpp::CategoryStream::ENDLINE);
                
                if ( it->is_active() && 
                     ( it->getEndLease() - time(0) <= m_threshold ) ) {
                    update_lease_for_job( *it );
                }
            }
        }
    } // end for
}

//____________________________________________________________________________
void iceCommandLeaseUpdater::update_lease_for_job( const CreamJob& j ) throw()
{
    map< string, time_t > newLease;
    vector< string > jobids;
    
    jobids.push_back( j.getCreamJobID() );

    // Renew the lease
    
    CREAM_SAFE_LOG(m_log_dev->infoStream()
		   << "iceCommandLeaseUpdater::update_lease_for_job() - "
		   << "updating lease for job "
                   << describe_job( j )
		   << " m_delta=" << m_delta
		   << log4cpp::CategoryStream::ENDLINE);
    
    try {

        m_theProxy->Authenticate( j.getUserProxyCertificate() );
        util::CreamProxy_Lease( j.getCreamURL(), jobids, m_delta, newLease ).execute( m_theProxy.get(), 3 );

    } catch(cream_exceptions::JobUnknownException& ex) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "iceCommandLeaseUpdater::update_lease_for_job() - "
                       << "CREAM doesn't know the current job "
                       << describe_job( j ) 
                       <<". Removing from cache."
                       << log4cpp::CategoryStream::ENDLINE);
        boost::recursive_mutex::scoped_lock M( jobCache::mutex );
        jobCache::iterator tmp = m_cache->lookupByCreamJobID( j.getCreamJobID() );
        m_cache->erase( tmp );
        return;
      
    } catch(cream_exceptions::BaseException& ex) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "iceCommandLeaseUpdater::update_lease_for_job() - "
                       << "Error updating lease for job "
                       << describe_job( j )
                       << ": "<< ex.what()
                       << log4cpp::CategoryStream::ENDLINE);
        return;      
    } catch ( soap_proxy::soap_ex& ex ) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "iceCommandLeaseUpdater::update_lease_for_job() - "
                       << "Error updating lease for job "
                       << describe_job( j ) << ": " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE);
        return;
    } catch(exception& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream()
		     << "iceCommandLeaseUpdater::update_lease_for_job() - "
		     << "Error updating lease for job "
		     << describe_job( j ) << ": " << ex.what()
		     << log4cpp::CategoryStream::ENDLINE);
      return;
    } catch(...) {
      CREAM_SAFE_LOG(m_log_dev->errorStream()
		     << "iceCommandLeaseUpdater::update_lease_for_job() - "
		     << "Error updating lease for job "
		     << describe_job( j ) << ": Unkonwn exception catched"
		     << log4cpp::CategoryStream::ENDLINE);
      return;
    }

    if ( newLease.find( j.getCreamJobID() ) != newLease.end() ) {
        
        // The lease for this job has been updated
        // So update the job cache as well...

        CREAM_SAFE_LOG(m_log_dev->infoStream()
                       << "iceCommandLeaseUpdater::update_lease_for_job() - "
                       << "updating lease for job "
                       << describe_job( j )
                       << "; old lease ends " << time_t_to_string( j.getEndLease() )
                       << " new lease ends " << time_t_to_string( newLease[ j.getCreamJobID() ] )
                       << log4cpp::CategoryStream::ENDLINE);
        
        boost::recursive_mutex::scoped_lock M( jobCache::mutex );
      
        // re-read the current job from the cache in order
        // to get modifications (if any) made by other threads
        jobCache::iterator tmpJob = m_cache->lookupByCreamJobID( j.getCreamJobID() );
        if(tmpJob != m_cache->end() ) {
            tmpJob->setEndLease( newLease[ j.getCreamJobID() ] );
            m_cache->put( *tmpJob ); // Be Careful!! This should not invalidate any iterator on the job cache, as the job j is guaranteed (in this case) to be already in the cache.            
        }

    } else {
        // there was an error updating the lease
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "iceCommandLeaseUpdater::update_lease_for_job() - "
                       << "unable to update lease for job "
                       << describe_job( j )
                       << "; old lease ends " << time_t_to_string( j.getEndLease() )
                       << log4cpp::CategoryStream::ENDLINE);
    }
}
