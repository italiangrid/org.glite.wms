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
#include <algorithm>

namespace iceUtil = glite::wms::ice::util;

using namespace std;

iceUtil::DNProxyManager* iceUtil::DNProxyManager::s_instance = NULL;
boost::recursive_mutex iceUtil::DNProxyManager::mutex;

//______________________________________________________________________________
namespace {
  class dnprxUpdater {
    iceUtil::DNProxyManager* m_mgr;
    log4cpp::Category *m_log_dev;

  public:
    dnprxUpdater( iceUtil::DNProxyManager* mgr, log4cpp::Category *log_dev ) : m_mgr( mgr ), m_log_dev( log_dev ) {}
    
    void operator()( iceUtil::CreamJob& aJob ) {
      if( m_mgr->getBetterProxyByDN( aJob.getUserDN() ) == aJob.getUserProxyCertificate() )
	return;
      
      CREAM_SAFE_LOG(m_log_dev->infoStream() 
		     << "dnprxUpdater::operator()() - "
		     << "Found DN ["
		     << aJob.getUserDN() << "] -> Proxy ["
		     << aJob.getUserProxyCertificate() << "]"
		     << log4cpp::CategoryStream::ENDLINE);
      
      m_mgr->setUserProxyIfLonger( aJob.getUserDN(), aJob.getUserProxyCertificate());
    }
  };
}

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
  
  boost::recursive_mutex::scoped_lock M( iceUtil::jobCache::mutex );

  
  dnprxUpdater updater( this, m_log_dev );
  for_each(cache->begin(), cache->end(), updater);
  

//   for(iceUtil::jobCache::iterator jit = cache->begin(); jit != cache->end(); ++jit) {
    
//     if( this->getBetterProxyByDN( jit->getUserDN() ) == jit->getUserProxyCertificate() )
//       continue;

//     CREAM_SAFE_LOG(m_log_dev->infoStream() 
// 		   << "DNProxyManager::CTOR() - "
// 		   << "Found DN ["
// 		   << jit->getUserDN() << "] -> Proxy ["
// 		   << jit->getUserProxyCertificate() << "]"
// 		   << log4cpp::CategoryStream::ENDLINE);
    
//     this->setUserProxyIfLonger( jit->getUserDN(), jit->getUserProxyCertificate());
//   }
}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger( const string& prx ) throw()
{ 

  string dn;
  try {
    dn = glite::ce::cream_client_api::certUtil::getCertSubj( prx );
  } catch(exception& ex) {
     CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Cannot retrieve the Subject for the proxy ["
		   << prx << "]. ICE will continue to use the old proxy."
		    << ex.what()
		   << log4cpp::CategoryStream::ENDLINE);
     return;
  }

  this->setUserProxyIfLonger( dn, prx );

}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger( const string& dn, 
						    const string& prx 
						    ) throw()
{ 

  //string dn = glite::ce::cream_client_api::certUtil::getDN( prx );

  if( m_DNProxyMap.find( dn ) == m_DNProxyMap.end() ) {
    CREAM_SAFE_LOG(m_log_dev->warnStream() 
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
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Cannot retrieve time left for proxy ["
		   << prx << "]: "
		   << ex.what()
		   << log4cpp::CategoryStream::ENDLINE);
    //    cout << "subscriptionManager::setUserProxyIfLonger - Cannot retrieve time for ["<<prx<<"]"<<endl;
    return;
  }
  
  try {
    oldT = glite::ce::cream_client_api::certUtil::getProxyTimeLeft( m_DNProxyMap[ dn ] );
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Cannot retrieve time left for old proxy ["
		   << m_DNProxyMap[ dn ] << "]. Setting user proxy to the new one ["
		   << prx << "]: "
		   << ex.what()
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
