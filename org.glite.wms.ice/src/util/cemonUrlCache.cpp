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

#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/monitor-client-api-c/CEInfo.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "cemonUrlCache.h"
#include "jobCache.h"
#include "iceConfManager.h"
#include "subscriptionManager.h"
#include "iceUtils.h"
#include "CreamProxyFactory.h"
#include <iostream>
#include <boost/scoped_ptr.hpp>

using namespace std;
namespace iceUtil = glite::wms::ice::util;
namespace api_util = glite::ce::cream_client_api::util;
namespace api = glite::ce::cream_client_api::soap_proxy;

iceUtil::cemonUrlCache* iceUtil::cemonUrlCache::s_instance = NULL;
boost::recursive_mutex iceUtil::cemonUrlCache::mutex;

//______________________________________________________________________________
iceUtil::cemonUrlCache* iceUtil::cemonUrlCache::getInstance() {
  boost::recursive_mutex::scoped_lock M( mutex );
  if( !s_instance ) 
    s_instance = new cemonUrlCache();
  return s_instance;
}

//______________________________________________________________________________
iceUtil::cemonUrlCache::cemonUrlCache() throw() :
  m_conf(iceUtil::iceConfManager::getInstance()),
  m_log_dev(api_util::creamApiLogger::instance()->getLogger())
{
//   try {
//     ceInfo = new CEInfo( m_conf->getHostProxyFile(), "/" );
//   } catch(exception& ex) {
//     CREAM_SAFE_LOG(m_log_dev->fatalStream()
//     		   << "cemonUrlCache::CTOR() - Couldn't create a CEInfo object: "
// 		   << ex.what() << ". STOP!"
// 		   << log4cpp::CategoryStream::ENDLINE);
		   
//      // this is severe, must exit
//      abort();
//   }
  
  m_subMgr = iceUtil::subscriptionManager::getInstance();
  if( !m_subMgr->isValid() )
  {
    CREAM_SAFE_LOG(m_log_dev->fatalStream()
    		   << "cemonUrlCache::CTOR() - Couldn't create "
		   << "a subscriptionManager object. STOP!"
		   << log4cpp::CategoryStream::ENDLINE);
		   
    // this is severe, must exit
    abort();
  }
}

//______________________________________________________________________________
bool iceUtil::cemonUrlCache::hasCEMon( const std::string& cemon ) const
{
  return( m_cemonURL.find( cemon ) != m_cemonURL.end() );
}

//______________________________________________________________________________
bool iceUtil::cemonUrlCache::isAuthorized( const string& DN ) const
{
  return ( m_DN.find(DN) != m_DN.end() );
}

//______________________________________________________________________________
string iceUtil::cemonUrlCache::getCEMonURL(const string& creamURL)
{

  // Try to get the CEMon's address from the memory
  map<string, string>::const_iterator cit = mappingCreamCemon.find(creamURL);
  if( cit != mappingCreamCemon.end() )
  {
    return cit->second;
  }


  // try to get the CEMon's address directly from the server
  string cemonURL;
  try {
    // this method is not called frequently because cemonUrlCache
    // keeps in memory information about CEUrl after the first 
    // explicit request to CREAM.
    // So we can create here a CreamProxy object and destroy it when exiting
    // We cannot use a member CreamProxy because CreamProxy contains a 
    // gSOAP runtime env that cannot be shared between threads and cemonUrlCache
    // is a singleton and could be shared between threads
    boost::scoped_ptr< api::CreamProxy > creamProxy( iceUtil::CreamProxyFactory::makeCreamProxy( false ) );
    creamProxy->Authenticate( m_conf->getHostProxyFile() );
    creamProxy->GetCEMonURL( creamURL.c_str(), cemonURL );
	
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << "cemonUrlCache::getCEMonURL() - "
		   << "Couldn't retrieve CEMon URL for CREAM URL ["
		   << creamURL << "]: "
		   << ex.what()<< ". Composing it from configuration file."
		   << log4cpp::CategoryStream::ENDLINE);
    cemonURL = creamURL;
    boost::replace_first(cemonURL,
                         m_conf->getCreamUrlPostfix(),
                         m_conf->getCEMonUrlPostfix()
                        );

  }
      
  CREAM_SAFE_LOG(m_log_dev->infoStream()
		 << "cemonUrlCache::getCEMonURL() - For CREAM URL ["
		 << creamURL << "] got CEMon URL ["
		 << cemonURL <<"]"
		 << log4cpp::CategoryStream::ENDLINE);

  mappingCreamCemon[creamURL] = cemonURL;
		 
  return cemonURL;
}

//______________________________________________________________________________
bool iceUtil::cemonUrlCache::getCEMonDN( const string& cemonURL,
					 string& DN 
				       )
{
  // try to get DN from the memory
  map<string, string>::const_iterator it;
  it = mappingCemonDN.find(cemonURL);
  if( it != mappingCemonDN.end() ) {
    DN = it->second;
    return true;
  }

  // try to get DN directly from the server
  CEInfo ceInfo(m_conf->getHostProxyFile(), "/");
  ceInfo.setServiceURL( cemonURL );
  try {
    ceInfo.authenticate( m_conf->getHostProxyFile().c_str(), "/" );
    ceInfo.getInfo();
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "cemonUrlCache::getCEMonDN() - "
		   << "Couldn't get DN for CEMon ["
		   << cemonURL << "]: "
		   << ex.what()
		   << log4cpp::CategoryStream::ENDLINE);
    return false;
  }
  
  DN = ceInfo.getDN();
  mappingCemonDN[cemonURL] = DN;
  ceInfo.cleanup();
  return true;
}

//______________________________________________________________________________
void iceUtil::cemonUrlCache::getCEMons( vector<string>& target )
{
  for(set<string>::const_iterator it=m_cemonURL.begin();
      it!=m_cemonURL.end();
      ++it) target.push_back( *it );
}

//______________________________________________________________________________
void iceUtil::cemonUrlCache::getCEMons( set<string>& target )
{
  for(set<string>::const_iterator it=m_cemonURL.begin();
      it!=m_cemonURL.end();
      ++it) target.insert( *it );
}
