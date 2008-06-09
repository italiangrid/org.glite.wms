/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE Proxy renewal thread
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

/**
 *
 * ICE Headers
 *
 */
#include "ice-core.h"
#include "iceUtils.h"
#include "proxyRenewal.h"
#include "iceConfManager.h"
#include "iceCommandProxyRenewal.h"


/**
 *
 * CE and WMS Headers
 *
 */
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

// boost includes
#include <boost/thread/thread.hpp>

// std includes
#include <vector>

/**
 *
 * System OS Headers
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace glite::wms::ice::util;
using namespace std;

//______________________________________________________________________________
proxyRenewal::proxyRenewal() :
    iceThread( "ICE Proxy Renewer" ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{
  m_delay = iceConfManager::getInstance()->getConfiguration()->ice()->proxy_renewal_frequency();
}


//______________________________________________________________________________
void proxyRenewal::body( void )
{
    while( !isStopped() ) {        
        CREAM_SAFE_LOG(m_log_dev->infoStream()
                       << "proxyRenewal::body() - new iteration"
                       );
        
	iceCommandProxyRenewal().execute();

	if(m_delay<=10) 
	  sleep( m_delay );
	else {
	  
	  for(int i=0; i<=m_delay; i++) {
	    if( isStopped() ) return;
	    sleep(1);
	  }
	  
	}
	
        //sleep( m_delay );
    }
}

// //______________________________________________________________________________
// void proxyRenewal::checkProxies()
// {
//     boost::recursive_mutex::scoped_lock M( jobCache::mutex );

//     for(jobCache::iterator jobIt = m_cache->begin(); jobIt != m_cache->end(); ++jobIt) {
//         if ( ! jobIt->is_active() ) 
//             continue; // skip terminated jobs

//         struct stat buf;
//         if( ::stat( jobIt->getUserProxyCertificate().c_str(), &buf) == 1 ) {
//             CREAM_SAFE_LOG(m_log_dev->errorStream() 
//                            << "proxyRenewal::checkProxies() - Cannot stat proxy file ["
//                            << jobIt->getUserProxyCertificate() << "] for job "
//                            << jobIt->describe() << ". Cannot check if it needs to be renewed."
//                            );
//             return;
//             // FIXME: what to do?
//         }

// 	iceCommandProxyRenewal PR;
// 	PR.execute();

//         CREAM_SAFE_LOG(m_log_dev->infoStream()
//                        << "proxyRenewal::checkProxies() - "
//                        << "Proxy file ["
//                        << jobIt->getUserProxyCertificate()
//                        << "] for job "
//                        << jobIt->describe() << " was modified on " 
//                        << time_t_to_string( buf.st_mtime )
//                        << ", the last proxy file modification time recorded by ICE is "
//                        << time_t_to_string( jobIt->getProxyCertLastMTime() )
//                        );

//         if( buf.st_mtime > jobIt->getProxyCertLastMTime() ) {
            
//             try {
//                 m_creamClient->Authenticate( jobIt->getUserProxyCertificate() );
                
//                 vector< string > theJob;
//                 theJob.push_back( jobIt->getCreamJobID() );
                
//                 m_creamClient->renewProxy( jobIt->getDelegationId(),
//                                            jobIt->getCreamURL(),
//                                            jobIt->getCreamDelegURL(),
//                                            jobIt->getUserProxyCertificate(),
//                                            theJob );
//             } catch( soap_proxy::soap_ex& ex ) {
//                 // FIXME: what to do? for now let's continue with an error message
//                 CREAM_SAFE_LOG( m_log_dev->errorStream() 
//                                 << "proxyRenewal::checkProxies() - Proxy renew failed for job "
//                                 << jobIt->describe() << ". Reason is: ["
//                                 << ex.what() << "]"
//                                 );
//             }
            
//             jobIt->setProxyCertMTime( buf.st_mtime );
//             m_cache->put( *jobIt );
            
//             // update of lastmodification time of proxy file
//             // put in cache
//         }
//     }
// }
