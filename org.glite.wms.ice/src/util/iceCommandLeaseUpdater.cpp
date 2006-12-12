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
#include "CreamProxyMethod.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "jobCache.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/BaseException.h"
#include "glite/ce/cream-client-api-c/JobUnknownException.h"

namespace api_util  = glite::ce::cream_client_api::util;
namespace cream_api = glite::ce::cream_client_api;
namespace ice_util  = glite::wms::ice::util;
namespace cream_exceptions = glite::ce::cream_client_api::cream_exceptions;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;

//using namespace glite::wms::ice::util;
using namespace std;

//____________________________________________________________________________
ice_util::iceCommandLeaseUpdater::iceCommandLeaseUpdater(cream_api::soap_proxy::CreamProxy* theProxy, const ice_util::CreamJob& theJob ) throw() : 
  m_theProxy( theProxy ),
  m_log_dev( api_util::creamApiLogger::instance()->getLogger() ),
  m_lb_logger( ice_util::iceLBLogger::instance() ),
  m_theJob( theJob ),
  m_delta( ice_util::iceConfManager::getInstance()->getConfiguration()->ice()->lease_delta_time() ),
  m_threshold( ice_util::iceConfManager::getInstance()->getConfiguration()->ice()->lease_threshold_time() )
{

}

//____________________________________________________________________________
void ice_util::iceCommandLeaseUpdater::execute( ) throw()
{
  if( m_theJob.getCreamJobID().empty() ) {
    return;
  }
  
  CREAM_SAFE_LOG(m_log_dev->infoStream() 
		 << "leaseUpdater::update_lease() - "
		 << "Checking LEASE for Job ["
		 << m_theJob.getCreamJobID() << "] - " 
		 << " isActive=" << m_theJob.is_active()
		 << " - remaining=" << (m_theJob.getEndLease()-time(0))
		 << " - threshold=" << m_threshold
		 << log4cpp::CategoryStream::ENDLINE);
  
  if ( (!m_theJob.is_active()) || ( m_theJob.getEndLease() - time(0) > m_threshold ) ) {
    return;
  }
  
  map< string, time_t > newLease;
  vector< string > jobids;
  
  jobids.push_back( m_theJob.getCreamJobID() );
  
  // Renew the lease
  
  CREAM_SAFE_LOG(m_log_dev->infoStream()
		 << "iceCommandLeaseUpdater::execute() - "
		 << "updating lease for cream jobid=["
		 << m_theJob.getCreamJobID()
		 << "] m_delta=" << m_delta
		 << log4cpp::CategoryStream::ENDLINE);
  
  try {
    
    m_theProxy->Authenticate( m_theJob.getUserProxyCertificate() );
    ice_util::CreamProxy_Lease( m_theJob.getCreamURL(), jobids, m_delta, newLease ).execute( m_theProxy.get(), 3 );
    
  } catch( cream_exceptions::JobUnknownException& ex ) {

    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "iceCommandLeaseUpdater::execute() - "
		   << "CREAM doesn't know the current Job ["
		   << m_theJob.getCreamJobID()<<"]. Removing from cache."
		   << log4cpp::CategoryStream::ENDLINE);

    boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
    ice_util::jobCache::iterator tmp = ice_util::jobCache::getInstance()->lookupByCreamJobID( m_theJob.getCreamJobID() );
    ice_util::jobCache::getInstance()->erase( tmp );
    return;
    
  } catch( cream_exceptions::BaseException& ex ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "iceCommandLeaseUpdater::execute() - "
		   << "Error updating lease for job ["
		   << m_theJob.getCreamJobID()<<"]: "<<ex.what()
		   << log4cpp::CategoryStream::ENDLINE);
    return;      
  } catch ( soap_proxy::soap_ex& ex ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "iceCommandLeaseUpdater::execute() - "
		   << "Error updating lease for job ["
		   << m_theJob.getCreamJobID()<<"]: "<<ex.what()
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  } catch( exception& ex ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "iceCommandLeaseUpdater::execute() - "
		   << "Error updating lease for job ["
		   << m_theJob.getCreamJobID()<<"]: "<<ex.what()
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  } catch(...) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "iceCommandLeaseUpdater::execute() - "
		   << "Error updating lease for job ["
		   << m_theJob.getCreamJobID()<<"]: Unknown catched exception."
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  }
  
  if ( newLease.find( m_theJob.getCreamJobID() ) != newLease.end() ) {
    
    // The lease for this job has been updated
    // So update the job cache as well...
    
    CREAM_SAFE_LOG(m_log_dev->infoStream()
		   << "iceCommandLeaseUpdater::execute() - "
		   << "updating lease for cream jobid=["
		   << m_theJob.getCreamJobID()
		   << "]; old lease ends " << ice_util::time_t_to_string( m_theJob.getEndLease() )
		   << " new lease ends " << ice_util::time_t_to_string( newLease[ m_theJob.getCreamJobID() ] )
		   << log4cpp::CategoryStream::ENDLINE);
    
    boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
    
    // re-read the current job from the cache in order
    // to get modifications (if any) made by other threads
    ice_util::jobCache::iterator tmpJob = ice_util::jobCache::getInstance()->lookupByCreamJobID( m_theJob.getCreamJobID() );
    if(tmpJob != ice_util::jobCache::getInstance()->end() ) {
      tmpJob->setEndLease( newLease[ m_theJob.getCreamJobID() ] );
      ice_util::jobCache::getInstance()->put( *tmpJob ); // Be Careful!! This should not invalidate any iterator on the job cache, as the job j is guaranteed (in this case) to be already in the cache.            
    }
    
  } else {
    // there was an error updating the lease
    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "iceCommandLeaseUpdater::execute() - "
		   << "unable to update lease for cream jobid=["
		   << m_theJob.getCreamJobID()
		   << "]; old lease ends " << time_t_to_string( m_theJob.getEndLease() )
		   << log4cpp::CategoryStream::ENDLINE);
  }
}
