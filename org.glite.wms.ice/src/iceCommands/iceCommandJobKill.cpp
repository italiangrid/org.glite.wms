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

/**
 *
 * ICE Headers
 *
 */
#include "iceCommandJobKill.h"
#include "CreamProxyMethod.h"
#include "DNProxyManager.h"
#include "iceConfManager.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "jobCache.h"
#include "iceUtils.h"


/**
 *
 * CE and WMS Headers
 *
 */
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/ResultWrapper.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

/**
 *
 * STL C++ Headers
 *
 */
#include <string>
#include <vector>
#include <algorithm>

/**
 *
 * Boost Headers
 *
 */
#include <boost/format.hpp>
#include <boost/functional.hpp>

namespace api_util  = glite::ce::cream_client_api::util;
namespace cream_api = glite::ce::cream_client_api;
namespace ice_util  = glite::wms::ice::util;

using namespace glite::wms::ice::util;
using namespace std;

//______________________________________________________________________________
namespace {
  bool insert_condition(const CreamJob& J) {
    if( J.getCompleteCreamJobID().empty() ) return false;
    
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
iceCommandJobKill::iceCommandJobKill( ) throw() : 
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

  jobCache::iterator it( ice_util::jobCache::getInstance()->begin() );
  while( it != ice_util::jobCache::getInstance()->end() ) {
    toCheck.push_back( *it );
    ++it;
  }

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
  string proxy;
  proxy = ice_util::DNProxyManager::getInstance()->getBetterProxyByDN( aList.first.first );

  list< CreamJob >::const_iterator it = aList.second.begin();
  list< CreamJob >::const_iterator list_end = aList.second.end();
  vector< string > jobs_to_cancel;
  while ( it != list_end ) {
    
    jobs_to_cancel.clear();
    it = ice_util::transform_n_elements( it, 
					 list_end, 
					 ice_util::iceConfManager::getInstance()->getConfiguration()->ice()->bulk_query_size(), 
					 back_inserter( jobs_to_cancel ), 
					 mem_fun_ref( &CreamJob::getCompleteCreamJobID ) );
    
    this->cancel_jobs(proxy, aList.first.second, jobs_to_cancel);
    
  } 
}

//______________________________________________________________________________
void iceCommandJobKill::cancel_jobs(const string& proxy, const string& endpoint, 
				    const vector<string>& jobIdList) throw()
{
  try {
    
    cream_api::soap_proxy::VOMSWrapper V( proxy );
    if( !V.IsValid( ) ) {
      CREAM_SAFE_LOG(    
		       m_log_dev->errorStream()
		       << "iceCommandJobKill::cancel_jobs() - proxy ["
		       << proxy << "] is EXPIRED! Skipping..."
		       << log4cpp::CategoryStream::ENDLINE
		       );
      return;
    }	

    vector<cream_api::soap_proxy::JobIdWrapper> toCancel;

    for(vector<string>::const_iterator it=jobIdList.begin();
	it!=jobIdList.end();
	++it)
      {
	
	string thisJob = endpoint;
	boost::replace_all( thisJob, iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_postfix(), "" );
	thisJob += *it;

	CREAM_SAFE_LOG(
		       m_log_dev->errorStream()
		       << "iceCommandJobKill::cancel_jobs() - Will cancel JobID ["
		       << thisJob 
		       << "] because its original proxy"
		       << " is going to expire..."
		       << log4cpp::CategoryStream::ENDLINE
		       );

	toCancel.push_back( cream_api::soap_proxy::JobIdWrapper( *it, 
						     endpoint, 
						     std::vector<cream_api::soap_proxy::JobPropertyWrapper>())
			    );
      }

    cream_api::soap_proxy::JobFilterWrapper req( toCancel, vector<string>(), -1, -1, "", "");
    cream_api::soap_proxy::ResultWrapper res;

    

    CreamProxy_Cancel( endpoint, proxy, &req, &res ).execute( 3 );

    list< pair<cream_api::soap_proxy::JobIdWrapper, string> > tmp;
    res.getOkJobs( tmp );
    if( tmp.size() == jobIdList.size() ) { 
      // in the OkJobs array there are all job we requested to cancel.
      // Then we suppose everything went ok.
      return;
    }

    tmp.clear();

    res.getNotExistingJobs( tmp );
    res.getNotMatchingStatusJobs( tmp );
    res.getNotMatchingDateJobs( tmp );
    res.getNotMatchingProxyDelegationIdJobs( tmp );
    res.getNotMatchingLeaseIdJobs( tmp );

    if( tmp.begin() == tmp.end() )
      {
	// Should not be empty. Something went wrong in the server
	CREAM_SAFE_LOG(    
		       m_log_dev->errorStream()
		       << "iceCommandJobKill::cancel_jobs() - Some job to cancel "
		       << " is not in the OK list neither in the "
		       << " failed list. Something went wrong in the server or in the "
		       << "SOAP communication."
		       << log4cpp::CategoryStream::ENDLINE
		       );
	return;
      }
    
    list< pair<cream_api::soap_proxy::JobIdWrapper, string> >::const_iterator it  = tmp.begin();
    list< pair<cream_api::soap_proxy::JobIdWrapper, string> >::const_iterator end = tmp.end();
    while( it != end ) {
      
      string completeJobId = it->first.getCreamURL();// = it->first.getCreamJobID();
      boost::replace_all( completeJobId, 
			  iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_postfix(), "" );

      completeJobId += "/" + it->first.getCreamJobID();

      CREAM_SAFE_LOG(    
		     m_log_dev->errorStream()
		     << "iceCommandJobKill::cancel_jobs() - "
		     << "Cancellation of job ["
		     << completeJobId << "] failed for error: "
		     << it->second
		     << log4cpp::CategoryStream::ENDLINE
		     );
      ++it;
    }

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
    return;

  } catch(...) {
    CREAM_SAFE_LOG (m_log_dev->errorStream() 
		    << "iceCommandJobKill::cancel_jobs() - Error"
		    << " killing job for the proxy ["
		    << proxy 
		    << "]. Unknown exception catched."
		    << log4cpp::CategoryStream::ENDLINE);
    return;

  } 
}

//______________________________________________________________________________
void iceCommandJobKill::updateCacheAndLog( const pair< pair<string, string>, list< CreamJob > >& aList ) throw()
{
  for(list<CreamJob>::const_iterator jit=aList.second.begin(); jit!=aList.second.end(); ++jit) {
    CreamJob theJob( *jit );
    theJob = m_lb_logger->logEvent( new cream_cancel_request_event( theJob, boost::str( boost::format( "Killed by ice's jobKiller, as residual proxy time is less than the threshold=%1%" ) % m_threshold_time ) ) );
    theJob.set_killed_by_ice();
    theJob.set_failure_reason( "The job has been killed because its proxy was expiring" );
    ice_util::jobCache::getInstance()->put( theJob ); 
  }
}

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

      if( cit->getCompleteCreamJobID().empty() ) continue;

      //      try {
	
	//timeleft = cream_api::certUtil::getProxyTimeLeft( cit->getUserProxyCertificate() );
	cream_api::soap_proxy::VOMSWrapper V( cit->getUserProxyCertificate() );
	if( !V.IsValid( ) ) {
	  CREAM_SAFE_LOG( m_log_dev->errorStream()
			  << "iceCommandJobKill::checkExpiring() - ERROR: "
			  << V.getErrorMessage()
			  << log4cpp::CategoryStream::ENDLINE);
	  stillgood.push_back( *cit );
	  continue;
	}
	
	timeleft = V.getProxyTimeEnd() - time(NULL);

//       } catch(exception& ex) {
// 	CREAM_SAFE_LOG( m_log_dev->errorStream()
// 			<< "iceCommandJobKill::checkExpiring() - ERROR: "
// 			<< ex.what()
// 			<< log4cpp::CategoryStream::ENDLINE);

// 	// we cannot retrieve the expiration time, so we do not want to cancel the job
// 	// i.e. the job is still good
// 	stillgood.push_back( *cit );
// 	continue;
//       } catch(...) {
// 	CREAM_SAFE_LOG( m_log_dev->errorStream()
// 			<< "iceCommandJobKill::checkExpiring() - ERROR: Unknown exception catched"
// 			<< log4cpp::CategoryStream::ENDLINE);
 
// 	// we cannot retrieve the expiration time, so we do not want to cancel the job
// 	// i.e. the job is still good
// 	stillgood.push_back( *cit );
// 	continue;
//       }
      
      if( timeleft < m_threshold_time && timeleft > 5 ) {
	CREAM_SAFE_LOG( m_log_dev->warnStream()
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
}
