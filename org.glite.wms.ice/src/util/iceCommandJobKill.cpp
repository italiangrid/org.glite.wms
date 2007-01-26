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
 * ICE job killer
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceCommandJobKill.h"
#include "CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "iceConfManager.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "jobCache.h"
#include "CreamProxyMethod.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <string>
#include <vector>

#include <boost/format.hpp>

namespace api_util  = glite::ce::cream_client_api::util;
namespace cream_api = glite::ce::cream_client_api;
namespace ice_util  = glite::wms::ice::util;

using namespace glite::wms::ice::util;
using namespace std;

//______________________________________________________________________________
iceCommandJobKill::iceCommandJobKill( const ice_util::CreamJob& theJob ) throw() : 
    m_theProxy( CreamProxyFactory::makeCreamProxy( false ) ),
    m_theJob( theJob ),
    m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
    m_threshold_time( ice_util::iceConfManager::getInstance()->getConfiguration()->ice()->job_cancellation_threshold_time() ),
    m_lb_logger( ice_util::iceLBLogger::instance() )
{
    //     m_theProxy.reset( theProxy );
    
    if ( m_threshold_time < 60 ) 
        m_threshold_time = 60;
}

//______________________________________________________________________________
void iceCommandJobKill::execute( ) throw()
{
    boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
    time_t proxyTimeLeft = cream_api::certUtil::getProxyTimeLeft( m_theJob.getUserProxyCertificate() );
    if( proxyTimeLeft < m_threshold_time && proxyTimeLeft > 5 ) {
        CREAM_SAFE_LOG( m_log_dev->infoStream() 
                        << "iceCommandJobKill::execute() - Job ["
                        << m_theJob.getCreamJobID() << "]"
                        << " has proxy expiring in "
                        << proxyTimeLeft 
                        << " seconds, which is less than "
                        << "the threshold ("
                        << m_threshold_time << " seconds). "
                        << "Going to cancel it..."
                        << log4cpp::CategoryStream::ENDLINE);
        if( !m_theJob.is_killed_by_ice() )
	  killJob( proxyTimeLeft );
    }
}

//______________________________________________________________________________
void iceCommandJobKill::killJob( const time_t residual_proxy_time )
{
  try {
      m_theProxy->Authenticate( m_theJob.getUserProxyCertificate() );
      vector<string> url_jid(1);   
      url_jid[0] = m_theJob.getCreamJobID();
   
      m_theJob = m_lb_logger->logEvent( new cream_cancel_request_event( m_theJob, boost::str( boost::format( "Killed by ice's jobKiller, as residual proxy time=%1%, which is less than the threshold=%2%" ) % residual_proxy_time % m_threshold_time ) ) );

      m_theJob.set_killed_by_ice();
      m_theJob.set_failure_reason( "The job has been killed because its proxy was expiring" );

      
      CreamProxy_Cancel( m_theJob.getCreamURL(), url_jid ).execute( m_theProxy.get(), 3 );

      // The corresponding "cancel done event" will be notified by the
      // poller/listener, so it is not logged here

      // The poller takes care of purging jobs
     
      CREAM_SAFE_LOG(
                     m_log_dev->infoStream() << "jobKiller::killJob() - "
                     << " Cancellation SUCCESFUL for job ["
                     <<  m_theJob.getCreamJobID() << "]"
		     << log4cpp::CategoryStream::ENDLINE);
		     
  } catch(std::exception& ex) {
      m_theJob = m_lb_logger->logEvent( new cream_cancel_refuse_event( m_theJob, ex.what() ) );
      // The job will not be removed from the job cache. We keep
      // trying to cancel it until the residual proxy time is less
      // than a minimum threshold. After that, the statusPoller will
      // eventually take care of removing it from the cache.
      CREAM_SAFE_LOG (
                      m_log_dev->errorStream() 
                      << "jobKiller::killJob() - Error"
                      << " killing job [" << m_theJob.getCreamJobID() << "]: "
                      << ex.what()
                      << log4cpp::CategoryStream::ENDLINE);
  }

  // The cache is already locked
  jobCache::getInstance()->put( m_theJob ); 

}
