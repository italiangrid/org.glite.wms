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
#include "iceConfManager.h"
#include "iceUtils.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int errno;

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

}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger( const string& prx ) throw()
{ 

  string dn;
  try {
    dn = glite::ce::cream_client_api::certUtil::getDNFQAN( prx );
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
  string localProxy = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() 
    + "/" + iceUtil::canonizeString( dn ) + ".proxy";


  if( m_DNProxyMap.find( dn ) == m_DNProxyMap.end() ) {
    CREAM_SAFE_LOG(m_log_dev->warnStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "DN ["
		   << dn << "] not found. Inserting the proxy ["
		   << prx << "]. Copied into ["
		   << localProxy << "]"
		   << log4cpp::CategoryStream::ENDLINE);

    this->copyProxy( prx, localProxy );
    m_DNProxyMap[ dn ] = make_pair(prx, localProxy);

    return;
  }

  time_t newT, oldT;

  try {
    newT= glite::ce::cream_client_api::certUtil::getProxyTimeLeft( prx );
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Cannot retrieve time left for proxy ["
		   << prx << "]: "
		   << ex.what()
		   << log4cpp::CategoryStream::ENDLINE);

    return;
  }
  
  

  try {
    // check time validity of the ICE's local proxy
    oldT = glite::ce::cream_client_api::certUtil::getProxyTimeLeft( m_DNProxyMap[ dn ].second ); 

  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Cannot retrieve time left for old proxy ["
		   << m_DNProxyMap[ dn ].first << "]. Setting user proxy to the new one ["
		   << prx << "] copied to ["
		   << localProxy <<"]: "
		   << ex.what()
		   << log4cpp::CategoryStream::ENDLINE);

    this->copyProxy( prx, localProxy );
    m_DNProxyMap[ dn ] = make_pair( prx, localProxy);
    return;
  }

  if(newT > oldT) {
    //cout<< "subscriptionManager::setUserProxyIfLonger - Setting user proxy to ["<<prx<<"]" <<endl;
    CREAM_SAFE_LOG(m_log_dev->infoStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Setting user proxy to [ "
		   << prx
		   << "] copied to " << localProxy 
		   << "] because the old one is less long-lived."
		   << log4cpp::CategoryStream::ENDLINE);

    this->copyProxy( prx, localProxy );
    m_DNProxyMap[ dn ] = make_pair( prx, localProxy );

  } else {

    CREAM_SAFE_LOG(m_log_dev->infoStream() 
		   << "DNProxyManager::setUserProxyIfLonger - Leaving current proxy ["
		   << m_DNProxyMap[ dn ].second <<"] beacuse it will expire later"
		   << log4cpp::CategoryStream::ENDLINE);

  }
}

//________________________________________________________________________
void iceUtil::DNProxyManager::copyProxy( const string& source, const string& target ) throw()
{
  string tmpTargetFilename = target;
  tmpTargetFilename = tmpTargetFilename + string(".tmp") ;
  ifstream input( source.c_str() );
  ofstream output( tmpTargetFilename.c_str() );

  if(!input) {
    CREAM_SAFE_LOG(m_log_dev->fatalStream() 
		   << "DNProxyManager::copyProxy - Cannot open"
		   << " proxy file ["
		   << source << "] for input. ABORTING!!"
		   << log4cpp::CategoryStream::ENDLINE);
    abort();
  }

  if(!output) {
    CREAM_SAFE_LOG(m_log_dev->fatalStream() 
		   << "DNProxyManager::copyProxy - Cannot open"
		   << " proxy file ["
		   << tmpTargetFilename << "] for output. ABORTING!!"
		   << log4cpp::CategoryStream::ENDLINE);
    abort();
  }
  char c;
  while( input.get(c) ) 
    {
      if(!input.good()||
	 input.fail() || 
	 input.bad())
	{
	  CREAM_SAFE_LOG(m_log_dev->fatalStream() 
			 << "DNProxyManager::copyProxy - Error copying ["
			 << source << "] to ["
			 << tmpTargetFilename << "]. ABORTING!!"
			 << log4cpp::CategoryStream::ENDLINE);
	  abort();
	}
      //input.tie(&output);
      output.put(c);
    }
  
  int err = ::rename(tmpTargetFilename.c_str(), target.c_str());
  if(err != 0 )
    {
      string errmex = strerror(errno);
      CREAM_SAFE_LOG(m_log_dev->fatalStream() 
		     << "DNProxyManager::copyProxy - Error moving ["
		     << tmpTargetFilename << "] to ["
		     << target << "]: " << errmex << ". ABORTING!!"
		     << log4cpp::CategoryStream::ENDLINE);
      abort();
    }

  ::unlink(tmpTargetFilename.c_str());

  ::chmod( target.c_str(), 00600 );
}
