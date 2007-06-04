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
#include "iceUtils.h"
#include "DNProxyManager.h"
#include "CreamProxyMethod.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <string>
#include <vector>
#include <algorithm>

#include <boost/format.hpp>
#include "boost/functional.hpp"

namespace api_util  = glite::ce::cream_client_api::util;
namespace cream_api = glite::ce::cream_client_api;
namespace ice_util  = glite::wms::ice::util;

using namespace glite::wms::ice::util;
using namespace std;

namespace {
  bool insert_condition(const CreamJob& J) {
    if( J.getCreamJobID().empty() ) return false;
    
    if( J.is_killed_by_ice() ) {
      CREAM_SAFE_LOG( api_util::creamApiLogger::instance()->getLogger()->debugStream() 
		      << "iceCommandJobKill::insert_condition() - Job "
		      << J.describe()
		      << " is reported as already been killed by ICE. "
		      << "Skipping..."
		      << log4cpp::CategoryStream::ENDLINE);     
      return false; 
    }
    
    return true;
  }
}

//______________________________________________________________________________
iceCommandJobKill::iceCommandJobKill( /*const ice_util::CreamJob& theJob*/ ) throw() : 
  m_theProxy( CreamProxyFactory::makeCreamProxy( false ) ),
  m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
  m_threshold_time( ice_util::iceConfManager::getInstance()->getConfiguration()->ice()->job_cancellation_threshold_time() ),
  m_lb_logger( ice_util::iceLBLogger::instance() )
{

}

//______________________________________________________________________________
void iceCommandJobKill::execute() throw()
{
  boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
  map< pair<string, string>, list< CreamJob >, ltstring> jobMap;

  list<CreamJob> toCheck;
  copy(ice_util::jobCache::getInstance()->begin(),
       ice_util::jobCache::getInstance()->end(),
       back_inserter( toCheck ));

  // Remove from toCheck list the jobs that have NOT the proxy expiring (and that have a non empty CreamJobID)
  this->checkExpiring( toCheck );

  
  jobMap_appender appender( jobMap, &insert_condition );
  for_each( toCheck.begin(),
	    toCheck.end(),
	    appender);
  
  // execute killJob DN-CEMon by DN-CEMon (then, on multiple jobs) on all elements
  // of jobMap (that contains all jobs with expired proxy)
  for_each( jobMap.begin(), 
	    jobMap.end(), 
	    boost::bind1st( boost::mem_fn( &iceCommandJobKill::killJob ), this ));
  
  for_each( jobMap.begin(), 
	    jobMap.end(), 
	    boost::bind1st( boost::mem_fn( &iceCommandJobKill::updateCacheAndLog ), this ));
}

//______________________________________________________________________________
void iceCommandJobKill::killJob( const pair< pair<string, string>, list< CreamJob > >& aList ) throw()
{

  // proxy = getBetterProxy();
  string proxy;
  //  {
  //boost::recursive_mutex::scoped_lock M( ice_util::DNProxyManager::mutex );
  proxy = ice_util::DNProxyManager::getInstance()->getBetterProxyByDN( aList.first.first );
  //}

    list< CreamJob >::const_iterator it = aList.second.begin();
    list< CreamJob >::const_iterator list_end = aList.second.end();
    vector< string > jobs_to_cancel;
    while ( it != list_end ) {
      
      jobs_to_cancel.clear();
      it = ice_util::transform_n_elements( it, 
					   list_end, 
					   ice_util::iceConfManager::getInstance()->getConfiguration()->ice()->bulk_query_size(), 
					   back_inserter( jobs_to_cancel ), 
					   mem_fun_ref( &CreamJob::getCreamJobID ) );
      
      this->cancel_jobs(proxy, aList.first.second, jobs_to_cancel);
      
    } 
}

//______________________________________________________________________________
bool iceCommandJobKill::cancel_jobs(const string& proxy, const string& endpoint, 
				    const vector<string>& jobIdList) throw()
{
  try {
    
    m_theProxy->Authenticate( proxy );
    CreamProxy_Cancel( endpoint, jobIdList ).execute( m_theProxy.get(), 3 );
    
  } catch(std::exception& ex) {
    //m_theJob = m_lb_logger->logEvent( new cream_cancel_refuse_event( m_theJob, ex.what() ) );
    
    // The job will not be removed from the job cache. We keep
    // trying to cancel it until the residual proxy time is less
    // than a minimum threshold. After that, the statusPoller will
    // eventually take care of removing it from the cache.
    CREAM_SAFE_LOG (m_log_dev->errorStream() 
		    << "iceCommandJobKill::cancel_jobs() - Error"
		    << " killing job for the proxy ["
		    << proxy 
		    << "]: "
		    << ex.what()
		    << log4cpp::CategoryStream::ENDLINE);
    return false;
  } catch(...) {
    CREAM_SAFE_LOG (m_log_dev->errorStream() 
		    << "iceCommandJobKill::cancel_jobs() - Error"
		    << " killing job for the proxy ["
		    << proxy 
		    << "]. Unknown exception catched."
		    << log4cpp::CategoryStream::ENDLINE);
    return false;
  } 
  return true;
}

//______________________________________________________________________________
void iceCommandJobKill::updateCacheAndLog( const pair< pair<string, string>, list< CreamJob > >& aList ) throw()
{
//   string proxy;
//   {
//     boost::recursive_mutex::scoped_lock M( ice_util::DNProxyManager::mutex );
//     proxy = ice_util::DNProxyManager::getInstance()->getBetterProxyByDN( aList.first.first );
//   }

//   time_t residual_proxy_time = 0;
  
//   try {
//     residual_proxy_time = cream_api::certUtil::getProxyTimeLeft( proxy );
//   } catch(exception& ex) {
//     CREAM_SAFE_LOG (m_log_dev->errorStream() 
// 		    << "iceCommandJobKill::updateCacheAndLog() - certUtil::getProxyTimeLeft"
// 		    << " raised an exception: "
// 		    << ex.what()
// 		    << "."
// 		    << log4cpp::CategoryStream::ENDLINE);
//   }
  
  for(list<CreamJob>::const_iterator jit=aList.second.begin(); jit!=aList.second.end(); ++jit) {
    CreamJob theJob( *jit );
    theJob = m_lb_logger->logEvent( new cream_cancel_request_event( theJob, boost::str( boost::format( "Killed by ice's jobKiller, as residual proxy time is less than the threshold=%1%" ) % m_threshold_time ) ) );
    theJob.set_killed_by_ice();
    theJob.set_failure_reason( "The job has been killed because its proxy was expiring" );
    ice_util::jobCache::getInstance()->put( theJob ); 
  }
}

//______________________________________________________________________________
// void iceCommandJobKill::execute( ) throw()
// {
//     boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
//     time_t residual_proxy_time = cream_api::certUtil::getProxyTimeLeft( m_theJob.getUserProxyCertificate() );

//     if( m_theJob.is_killed_by_ice() ) {
//         CREAM_SAFE_LOG( m_log_dev->debugStream() 
//                         << "iceCommandJobKill::execute() - Job "
//                         << m_theJob.describe()
//                         << " is reported as already been killed by ICE. "
//                         << "Skipping..."
//                         << log4cpp::CategoryStream::ENDLINE);     
//         return; // nothing to do
//     }

//     CREAM_SAFE_LOG( m_log_dev->debugStream() 
//                     << "iceCommandJobKill::execute() - Job "
//                     << m_theJob.describe()
//                     << " is being cancelled "
//                     << log4cpp::CategoryStream::ENDLINE);     

//     try {
//         m_theProxy->Authenticate( m_theJob.getUserProxyCertificate() );
//         vector<string> url_jid(1);   
//         url_jid[0] = m_theJob.getCreamJobID();
        
//         m_theJob = m_lb_logger->logEvent( new cream_cancel_request_event( m_theJob, boost::str( boost::format( "Killed by ice's jobKiller, as residual proxy time=%1%, which is less than the threshold=%2%" ) % residual_proxy_time % m_threshold_time ) ) );
        
//         m_theJob.set_killed_by_ice();
//         m_theJob.set_failure_reason( "The job has been killed because its proxy was expiring" );
                
//         CreamProxy_Cancel( m_theJob.getCreamURL(), url_jid ).execute( m_theProxy.get(), 3 );
        
//         // The corresponding "cancel done event" will be notified by the
//         // poller/listener, so it is not logged here
        
//         // The poller takes care of purging jobs
        
//         CREAM_SAFE_LOG(m_log_dev->infoStream() 
//                        << "iceCommandJobKill::execute() - "
//                        << " Cancellation SUCCESFUL for job "
//                        << m_theJob.describe()
//                        << log4cpp::CategoryStream::ENDLINE);
        
//     } catch(std::exception& ex) {
//         m_theJob = m_lb_logger->logEvent( new cream_cancel_refuse_event( m_theJob, ex.what() ) );
//         // The job will not be removed from the job cache. We keep
//         // trying to cancel it until the residual proxy time is less
//         // than a minimum threshold. After that, the statusPoller will
//         // eventually take care of removing it from the cache.
//         CREAM_SAFE_LOG (m_log_dev->errorStream() 
//                         << "iceCommandJobKill::execute() - Error"
//                         << " killing job " 
//                         << m_theJob.describe() << ": "
//                         << ex.what()
//                         << log4cpp::CategoryStream::ENDLINE);
//     } catch(...) {
//         CREAM_SAFE_LOG (m_log_dev->errorStream() 
//                         << "iceCommandJobKill::execute() - Error"
//                         << " killing job "
//                         << m_theJob.describe()
//                         << ". Unknown exception catched."
//                         << log4cpp::CategoryStream::ENDLINE);
//     }
    
//     // The cache is already locked
//     jobCache::getInstance()->put( m_theJob ); 
    
// }

//______________________________________________________________________________
void iceCommandJobKill::checkExpiring( list<CreamJob>& all ) throw()
{

  string proxy ="";
  list<CreamJob> stillgood, target;
  
  time_t timeleft = 0;
  
  for(list<CreamJob>::const_iterator cit = all.begin();
      cit != all.end();
      ++cit)
    {
      // Check the proxy validity

      if( cit->getCreamJobID().empty() ) continue;

      try {
	
	timeleft = cream_api::certUtil::getProxyTimeLeft( cit->getUserProxyCertificate() );
	
      } catch(exception& ex) {
	CREAM_SAFE_LOG( m_log_dev->errorStream()
			<< "iceCommandJobKill::checkExpiring() - ERROR: "
			<< ex.what()
			<< log4cpp::CategoryStream::ENDLINE);

	// we cannot retrieve the expiration time, so we do not want to cancel the job
	// i.e. the job is still good
	stillgood.push_back( *cit );
	continue;
      } catch(...) {
	CREAM_SAFE_LOG( m_log_dev->errorStream()
			<< "iceCommandJobKill::checkExpiring() - ERROR: Unknown exception catched"
			<< log4cpp::CategoryStream::ENDLINE);
 
	// we cannot retrieve the expiration time, so we do not want to cancel the job
	// i.e. the job is still good
	stillgood.push_back( *cit );
	continue;
      }
      
      if( timeleft < m_threshold_time && timeleft > 5 ) {
	CREAM_SAFE_LOG( m_log_dev->debugStream()
			<< "iceCommandJobKill::checkExpiring() - Proxy ["
			<< cit->getUserProxyCertificate() << "] of user ["
			<< cit->getUserDN() // the user DN
			<< "] is expiring. Will cancel related jobs"
			<< log4cpp::CategoryStream::ENDLINE);

	continue;
      } else {
	// the original job's proxy is not going to expire.
	// i.e. the job is not to be cancelled (still good)
	stillgood.push_back( *cit );

      }
    }

  // Remove from all the jobs that are NOT going to proxy-expire
  for_each( 
	   stillgood.begin(), 
	   stillgood.end(), 
	   boost::bind1st( boost::mem_fn( &list<CreamJob>::remove ), &all )
	   );
  
//   for(list<CreamJob>::const_iterator it = stillgood.begin();
//       it != stillgood.end();
//       ++it) 
//     {
//       all.remove( *it );
//     };


}
