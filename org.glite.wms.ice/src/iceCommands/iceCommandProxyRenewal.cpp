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

/**
 *
 * ICE Headers
 *
 */
#include "iceCommandProxyRenewal.h"
#include "CreamProxyMethod.h"
#include "DNProxyManager.h"
#include "jobCache.h"
#include "iceUtils.h"

/**
 *
 * Cream Client API C++ Headers
 *
 */
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

/**
 *
 * OS Headers
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace cream_api = glite::ce::cream_client_api;

using namespace std;
using namespace glite::wms::ice::util;

//______________________________________________________________________________
iceCommandProxyRenewal::iceCommandProxyRenewal( ) :
    iceAbsCommand( "iceCommandProxyRenewal" ),
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

  } catch( exception& ex ) {
    // FIXME: what to do? for now let's continue with an error message
    CREAM_SAFE_LOG( m_log_dev->errorStream() 
		    << "iceCommandProxyRenewal::execute() - "
		    << "Proxy renew for delegation ID ["
		    << delegation_ID
		    << "] failed: "
		    << ex.what() 
		    );
    
    return false;
    // Let's ignore; probably another proxy renewal will be done
  }
  
  return true;
}

//______________________________________________________________________________
void iceCommandProxyRenewal::execute( void ) throw()
{

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
		       );
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
		       );
	
	glite::wms::ice::util::DNProxyManager::getInstance()->setUserProxyIfLonger( jobIt->getUserProxyCertificate() );
	jobMap[ make_pair(jobIt->getDelegationId(), jobIt->getCreamDelegURL()) ].push_back( *jobIt );
	timeMap[ jobIt->getCompleteCreamJobID() ] = buf.st_mtime;
      }
    }
  } // unlock the jobCache
  
  
  // now must loop over the keys of jobMap
  // and renew each proxy.
  map< pair<string, string>, list<CreamJob>, ltstring >::iterator jobMap_it = jobMap.begin();
  map< pair<string, string>, list<CreamJob>, ltstring >::const_iterator end = jobMap.end();

  while( jobMap_it != end ) {

    // jobMap::iterator.first is the couple (delegationID,CREAM_Deleg_URL)
    // jobMap::iterator.second is the list of job

    boost::recursive_mutex::scoped_lock M( jobCache::mutex );

    CREAM_SAFE_LOG(m_log_dev->infoStream() 
		   << "iceCommandProxyRenewal::execute() - "
		   << "Renewing proxy for delegation ID ["
		   << jobMap_it->first.first << "] to CREAMDelegation service ["
		   << jobMap_it->first.second << "] using proxy certificate file ["
		   << jobMap_it->second.begin()->getUserProxyCertificate()
		   << "]"
		   );

    if( this->renewProxy( jobMap_it->first, jobMap_it->second.begin()->getUserProxyCertificate() ))
      {
	// update the cache for all jobs that have this delegationID
	list<CreamJob>::iterator jobit     = jobMap_it->second.begin();
	list<CreamJob>::iterator jobit_end = jobMap_it->second.end();
	while( jobit != jobit_end ) {
	  jobit->setProxyCertMTime( timeMap[jobit->getCompleteCreamJobID()] );
	  m_cache->put( *jobit );
	  ++jobit;
	}
      }
    ++jobMap_it;
  }
}
