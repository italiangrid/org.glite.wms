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
 * ICE proxy renewal command
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceCommandProxyRenewal.h"
#include "jobCache.h"
//#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "CreamProxyMethod.h"
#include "iceUtils.h"
#include "DNProxyManager.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace cream_api = glite::ce::cream_client_api;

using namespace std;
using namespace glite::wms::ice::util;

//______________________________________________________________________________
iceCommandProxyRenewal::iceCommandProxyRenewal( ) :
  m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
  m_cache( jobCache::getInstance() )
{
  
}

//______________________________________________________________________________
bool iceCommandProxyRenewal::renewProxy( const pair<string, string>& deleg, 
					 const string& proxy) throw()
{
  
  string cream_deleg_url = deleg.second;
  string delegation_ID   = deleg.first;

  try {

    CreamProxy_ProxyRenew( cream_deleg_url,
			   proxy,
			   delegation_ID).execute( 3 );

//     cream_api::soap_proxy::AbsCreamProxy* creamClient = 
//       cream_api::soap_proxy::CreamProxyFactory::make_CreamProxy_ProxyRenew( delegation_ID, 30 );
//     if(!creamClient)
//       {
//         CREAM_SAFE_LOG( m_log_dev->errorStream()
// 			<< "iceCommandProxyRenewal::renewProxy() - "
// 			<< "CreamProxy creation FAILED!! Stop!"
// 			<< log4cpp::CategoryStream::ENDLINE);
//         return false;
//       }
    
//     boost::scoped_ptr< cream_api::soap_proxy::AbsCreamProxy > tmpClient;
//     tmpClient.reset( creamClient );
    
//     creamClient->setCredential( proxy );
//     creamClient->execute( cream_deleg_url );


  } catch( exception& ex ) {
    // FIXME: what to do? for now let's continue with an error message
    CREAM_SAFE_LOG( m_log_dev->errorStream() 
		    << "iceCommandProxyRenewal::execute() - "
		    << "Proxy renew for delegation ID ["
		    << delegation_ID
		    << "] failed: "
		    << ex.what() 
		    << log4cpp::CategoryStream::ENDLINE);
    
    return false;
    // Let's ignore; probably another proxy renewal will be done
  }
  
  return true;
}

//______________________________________________________________________________
// bool iceCommandProxyRenewal::renewProxy( const list<CreamJob>& jobs) throw()
// {
//   // job with the same delegation ID also have the same proxy
//   string proxy           = jobs.begin()->getUserProxyCertificate();
//   string cream_url       = jobs.begin()->getCreamURL();
//   string cream_deleg_url = jobs.begin()->getCreamDelegURL();
//   string delegation_ID   = jobs.begin()->getDelegationId();
  
//   try {
//     //m_theProxy->Authenticate( proxy );
    
//     cream_api::soap_proxy::VOMSWrapper V( m_theJob.getUserProxyCertificate() );
//     if( !V.IsValid( ) ) {

//       CREAM_SAFE_LOG( m_log_dev->errorStream() 
// 		      << "iceCommandProxyRenewal::execute() - "
// 		      << "Proxy renew for delegation ID ["
// 		      << delegation_ID
// 		      << "] failed: "
// 		      << V.getErrorMessage()
// 		      << log4cpp::CategoryStream::ENDLINE);
      
//       return false; 

//       //      throw cream_api::soap_proxy::auth_ex( V.getErrorMessage() );
//     }
    
//     vector< string > theJob;
    
//     transform( jobs.begin(), jobs.end(), 
// 	       inserter(theJob, theJob.begin()), 
// 	       mem_fun_ref(&CreamJob::getCreamJobID));
    
// //     m_theProxy->renewProxy( delegation_ID,
// // 			    cream_url,
// // 			    cream_deleg_url,
// // 			    proxy,
// // 			    theJob );
    
    
//     //FIXME: must use new APIs

    
//   } catch( exception& ex ) {
//     // FIXME: what to do? for now let's continue with an error message
//     CREAM_SAFE_LOG( m_log_dev->errorStream() 
// 		    << "iceCommandProxyRenewal::execute() - "
// 		    << "Proxy renew for delegation ID ["
// 		    << delegation_ID
// 		    << "] failed: "
// 		    << ex.what() 
// 		    << log4cpp::CategoryStream::ENDLINE);
    
//     return false;
//     // Let's ignore; probably another proxy renewal will be done
//   } 
  
//   return true;
// }

//______________________________________________________________________________
void iceCommandProxyRenewal::execute( void ) throw()
{

  return;    
  
  map< pair<string, string>, list<CreamJob>, ltstring > jobMap;
  map< string, time_t > timeMap;
  
  {
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    for(jobCache::iterator jobIt = m_cache->begin(); jobIt != m_cache->end(); ++jobIt) {
      if( !jobIt->is_active() ) 
	continue; // skip terminated jobs
      if( !jobIt->is_proxy_renewable() )
	continue;
      
      struct stat buf;
      //         
      if( ::stat( jobIt->getUserProxyCertificate().c_str(), &buf) == 1 ) {
	CREAM_SAFE_LOG(m_log_dev->errorStream() 
		       << "iceCommandProxyRenewal::execute() - "
		       << "Cannot stat proxy file ["
		       << jobIt->getUserProxyCertificate() << "] for job "
		       << jobIt->describe()
		       << ". Skipping it..."
		       << log4cpp::CategoryStream::ENDLINE);
	continue; // skip to next job
	// FIXME: what to do?
      }
      //         
      if( buf.st_mtime > jobIt->getProxyCertLastMTime() ) {
	CREAM_SAFE_LOG(m_log_dev->infoStream() 
		       << "iceCommandProxyRenewal::execute() - "
		       << "Proxy file ["
		       << jobIt->getUserProxyCertificate() << "] for job "
		       << jobIt->describe()
		       << " was modified on "
		       << time_t_to_string( buf.st_mtime )
		       << ", the last proxy file modification time "
		       << "recorded by ICE is "
		       << time_t_to_string( jobIt->getProxyCertLastMTime() )
		       << ". Renewing proxy."
		       << log4cpp::CategoryStream::ENDLINE);
	
	glite::wms::ice::util::DNProxyManager::getInstance()->setUserProxyIfLonger( jobIt->getUserProxyCertificate() );
	jobMap[ make_pair(jobIt->getDelegationId(), jobIt->getCreamDelegURL()) ].push_back( *jobIt );
	timeMap[ jobIt->getCreamJobID() ] = buf.st_mtime;
      }
    }
  } // unlock the jobCache
  
  
  // now must loop over the keys of jobMap
  // and renew each proxy.
  map< pair<string, string>, list<CreamJob>, ltstring >::iterator jobMap_it  = jobMap.begin();
  map< pair<string, string>, list<CreamJob>, ltstring >::const_iterator end = jobMap.end();

  while( jobMap_it != end ) {
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    if( this->renewProxy( jobMap_it->first, jobMap_it->second.begin()->getUserProxyCertificate() ))
      {
	// update the cache for all jobs that have this delegationID
	list<CreamJob>::iterator jobit     = jobMap_it->second.begin();
	list<CreamJob>::iterator jobit_end = jobMap_it->second.end();
	while( jobit != jobit_end ) {
	  jobit->setProxyCertMTime( timeMap[jobit->getCreamJobID()] );
	  m_cache->put( *jobit );
	  ++jobit;
	}
      }
    ++jobMap_it;
  }
  

//   map< pair<string, string>, list<CreamJob>, ltstring >::iterator jobMap_it  = jobMap.begin();
//   map< pair<string, string>, list<CreamJob>, ltstring >::const_iterator end = jobMap.end();
  
//   while( jobMap_it != end ) {
    
//     boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    
//     if( this->renewProxy( jobMap_it->second ) )
//       {
//         list<CreamJob>::iterator aJob = jobMap_it->second.begin();
// 	list<CreamJob>::const_iterator end_list = jobMap_it->second.end();
	
//         for(;
// 	    aJob != end_list;
// 	    ++aJob) 
// 	  {
// 	    // FIXME: TODO
// 	    aJob->setProxyCertMTime( timeMap[aJob->getCreamJobID()] );
// 	    m_cache->put( *aJob );
// 	  }
//       }
    
//     jobMap_it++;  
    
//   }
  
}
