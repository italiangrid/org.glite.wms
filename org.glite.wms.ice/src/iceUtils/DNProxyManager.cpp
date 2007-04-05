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
 * ICE CEMON URL Cache
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "DNProxyManager.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include <iostream>

namespace iceUtil = glite::wms::ice::util;

using namespace std;

iceUtil::DNProxyManager* iceUtil::DNProxyManager::s_instance = NULL;
boost::recursive_mutex iceUtil::DNProxyManager::mutex;

//______________________________________________________________________________
iceUtil::DNProxyManager* iceUtil::DNProxyManager::getInstance() throw()
{

  boost::recursive_mutex::scoped_lock M( mutex );

  if( !s_instance ) 
    s_instance = new DNProxyManager();
  return s_instance;
}

//______________________________________________________________________________
iceUtil::DNProxyManager::DNProxyManager( void ) throw()
{
  m_log_dev = glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger();
  iceUtil::jobCache *cache = iceUtil::jobCache::getInstance();

  CREAM_SAFE_LOG(m_log_dev->infoStream() 
		 << "DNProxyManager::CTOR() - "
		 << "Populating DN -> Proxy cache by scannig the jobCache..."
		 << log4cpp::CategoryStream::ENDLINE);
  
  for(iceUtil::jobCache::iterator jit = cache->begin(); jit != cache->end(); ++jit) {
    CREAM_SAFE_LOG(m_log_dev->infoStream() 
		   << "DNProxyManager::CTOR() - "
		   << "Found DN ["
		   << jit->getUserDN() << "] -> Proxy ["
		   << jit->getUserProxyCertificate() << "]"
		   << log4cpp::CategoryStream::ENDLINE);
    this->setUserProxyIfLonger( jit->getUserDN(), jit->getUserProxyCertificate());
  }
}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger( const string& prx ) throw()
{ 

  string dn = glite::ce::cream_client_api::certUtil::getDN( prx );

  this->setUserProxyIfLonger( dn, prx );

}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger( const string& dn, 
						    const string& prx 
						    ) throw()
{ 

  //string dn = glite::ce::cream_client_api::certUtil::getDN( prx );

  if( m_DNProxyMap.find( dn ) == m_DNProxyMap.end() ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "DN ["
		   << dn << "] not found. Inserting the proxy ["
		   << prx << "]"
		   << log4cpp::CategoryStream::ENDLINE);
    m_DNProxyMap[ dn ] = prx;
    return;
  }

  if (prx == m_DNProxyMap[ dn ] ) return;

  time_t newT, oldT;

  try {
    newT= glite::ce::cream_client_api::certUtil::getProxyTimeLeft(prx);
  } catch(...) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Cannot retrieve time left for proxy ["
		   << prx << "]"
		   << log4cpp::CategoryStream::ENDLINE);
    //    cout << "subscriptionManager::setUserProxyIfLonger - Cannot retrieve time for ["<<prx<<"]"<<endl;
    return;
  }
  
  try {
    oldT = glite::ce::cream_client_api::certUtil::getProxyTimeLeft( m_DNProxyMap[ dn ] );
  } catch(...) {
//     cout<< "subscriptionManager::setUserProxyIfLonger - Setting user proxy to ["
//  	<<  prx
//  	<< "] because cannot retrieve time for ["<< m_DNProxyMap[ dn ] <<"]" <<endl;
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Cannot retrieve time left for old proxy ["
		   << m_DNProxyMap[ dn ] << "]. Setting user proxy to the new one ["
		   << prx << "]"
		   << log4cpp::CategoryStream::ENDLINE);
    m_DNProxyMap[ dn ] = prx;
    return;
  }

  if(newT > oldT) {
    //cout<< "subscriptionManager::setUserProxyIfLonger - Setting user proxy to ["<<prx<<"]" <<endl;
    CREAM_SAFE_LOG(m_log_dev->infoStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Setting user proxy to [ "
		   << prx
		   << "] because the old one is less long-lived."
		   << log4cpp::CategoryStream::ENDLINE);
    m_DNProxyMap[ dn ] = prx;
  } else {
    //cout<< "subscriptionManager::setUserProxyIfLonger - Leaving current proxy ["<< m_DNProxyMap[ dn ] 
    //	<<"] beacuse will expire later" <<endl;
    CREAM_SAFE_LOG(m_log_dev->infoStream() 
		   << "Leaving current proxy ["
		   << m_DNProxyMap[ dn ] <<"] beacuse it will expire later"
		   << log4cpp::CategoryStream::ENDLINE);
  }
}
