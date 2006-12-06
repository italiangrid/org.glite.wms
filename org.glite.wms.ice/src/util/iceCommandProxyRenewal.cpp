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
 * ICE status poller
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceCommandProxyRenewal.h"
#include "jobCache.h"
#include "CreamProxyFactory.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<string>

namespace ice_util  = glite::wms::ice::util;
namespace cream_api = glite::ce::cream_client_api;

using namespace std;

//______________________________________________________________________________
ice_util::iceCommandProxyRenewal::iceCommandProxyRenewal( cream_api::soap_proxy::CreamProxy* theProxy ) :
  m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
  m_theProxy( theProxy ),
  m_cache( ice_util::jobCache::getInstance() )
{
  
}

//______________________________________________________________________________
void ice_util::iceCommandProxyRenewal::execute( void ) throw() {
  boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );

  for(ice_util::jobCache::iterator jobIt = m_cache->begin(); 
      jobIt != m_cache->end(); ++jobIt) 
    {
      if ( ! jobIt->is_active() ) 
	continue; // skip terminated jobs
      
      struct stat buf;
      if( ::stat( jobIt->getUserProxyCertificate().c_str(), &buf) == 1 )
	{
	  CREAM_SAFE_LOG(m_log_dev->errorStream() 
			 << "proxyRenewal::checkProxies() - Cannot stat proxy file ["
			 << jobIt->getUserProxyCertificate() << "] for job ["
			 << jobIt->getCreamJobID() 
			 << "]. Wont check if it needs to be renewed."
			 << log4cpp::CategoryStream::ENDLINE);
	  // FIXME: what to do?
	} else {
	if( buf.st_mtime > jobIt->getProxyCertLastMTime() ) {
	  CREAM_SAFE_LOG(m_log_dev->infoStream() 
			 << "proxyRenewal::checkProxies() - Need to renew proxy  ["
			 << jobIt->getUserProxyCertificate() << "] for job ["
			 << jobIt->getCreamJobID() << "]"
			 << log4cpp::CategoryStream::ENDLINE);
	  
	  try {
	    m_theProxy->Authenticate( jobIt->getUserProxyCertificate() );
	    
	    vector< string > theJob;
	    theJob.push_back( jobIt->getCreamJobID() );
	    
	    m_theProxy->renewProxy( 
				   jobIt->getDelegationId(),
				   jobIt->getCreamURL(),
				   jobIt->getCreamDelegURL(),
				   jobIt->getUserProxyCertificate(),
				   theJob 
				   );

	  } catch( cream_api::soap_proxy::soap_ex& ex ) {
	    // FIXME: what to do? for now let's continue with an error message
	    CREAM_SAFE_LOG( m_log_dev->errorStream() 
			    << "proxyRenewal::checkProxies() - Proxy renew failed: ["
			    << ex.what() << "]"
			    << log4cpp::CategoryStream::ENDLINE);
	  }
	  
	  jobIt->setProxyCertMTime( buf.st_mtime );
	  m_cache->put( *jobIt );
	  
	  // update of lastmodification time of proxy file
	  // put in cache
	}
      }
    }
}
