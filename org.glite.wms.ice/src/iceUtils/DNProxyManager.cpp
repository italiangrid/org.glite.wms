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
//#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
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
#include <openssl/sha.h> // for using SHA1

#include <boost/filesystem/operations.hpp>

//extern int errno;

namespace iceUtil = glite::wms::ice::util;
namespace cream_api = glite::ce::cream_client_api;
using namespace std;

iceUtil::DNProxyManager* iceUtil::DNProxyManager::s_instance = NULL;
boost::recursive_mutex iceUtil::DNProxyManager::mutex;

//______________________________________________________________________________
namespace {

    //
    // Utility function: Computes a SHA1 hash of the input string. The
    // resulting hash is made of 40 printable characters, each
    // character in the range [0-9A-F].
    //
    // @input name an input string; 
    //
    // @return a string of 40 printable characters, each in the range [0-9A-F]
    //
    string compressed_string( const string& name ) {
        string result;
        unsigned char buf[ SHA_DIGEST_LENGTH ]; // output buffer
        const unsigned char idx[ 17 ] = "0123456789ABCDEF"; // must be 17 chars, as the trailing \0 counts
        SHA1( (const unsigned char*)name.c_str(), name.length(), buf ); // stores SHA1 hash in buf
        for ( int i=0; i<SHA_DIGEST_LENGTH; ++i ) {
            unsigned char to_append;
            // left nibble;
            to_append = idx[ ( buf[i] & 0xf0 ) >> 4 ];
            result.push_back( to_append );
            // right nibble
            to_append = idx[ buf[i] & 0x0f ];
            result.push_back( to_append );
        }
        return result;
    }
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

  CREAM_SAFE_LOG(m_log_dev->debugStream() 
		 << "DNProxyManager::CTOR() - "
		 << "Populating DN -> Proxy cache by scannig the jobCache..."
		 << log4cpp::CategoryStream::ENDLINE);
  
  /**
   * This lock acquisition should be safe because it is done at the ICE
   * initialization, when no other thread acquire the cache's mutex
   *
   */
  boost::recursive_mutex::scoped_lock M( iceUtil::jobCache::mutex );

  /**
   * Retrieve all distinguished DNs
   */
  set<string> dnSet;
  transform( cache->begin(), cache->end(), 
	     inserter(dnSet, dnSet.begin()), 
	     mem_fun_ref(&iceUtil::CreamJob::getUserDN));

  /**
   * for each DN check if the better ICE's local proxy is there
   */
  string localProxy;
  string prefix = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/";
  
  //iceUtil::jobCache::iterator job_with_better_proxy_from_sandboxDir;
  
  

  for(set<string>::const_iterator it = dnSet.begin();
      it != dnSet.end();
      ++it)
    {
      if( it->empty() ) continue;

      // localProxy = prefix + iceUtil::canonizeString( *it ) + ".proxy";
      localProxy = prefix + compressed_string( *it ) + ".proxy";
      boost::filesystem::path thisPath( localProxy, boost::filesystem::native );
      
      iceUtil::jobCache::iterator job_with_better_proxy_from_sandboxDir = this->searchBetterProxyForUser( *it );
      
      if( !boost::filesystem::exists( thisPath ) )
	{
	  CREAM_SAFE_LOG(m_log_dev->infoStream() 
			 << "DNProxyManager::CTOR() - "
			 << "Proxy file ["
			 << localProxy << "] not found for DN ["
			 << *it 
			 << "] in ICE's persistency dir. "
			 << "Trying to find the most long-lived"
			 << " one in the job cache for the current DN..."
			 << log4cpp::CategoryStream::ENDLINE);

	  //iceUtil::jobCache::const_iterator better = this->searchBetterProxyForUser( *it );
	  //iceUtil::jobCache::iterator better = this->searchBetterProxyForUser( *it );

	  if( job_with_better_proxy_from_sandboxDir == cache->end() )
	    {
/*	      CREAM_SAFE_LOG(m_log_dev->warnStream() 
			     << "DNProxyManager::CTOR() - Not found any proxy for DN ["
			     << *it << "]. Skipping"
			     << log4cpp::CategoryStream::ENDLINE);*/
	      continue;
	    }
	    
	  try {
	  
	    this->copyProxy(job_with_better_proxy_from_sandboxDir->getUserProxyCertificate(), localProxy);
	    
	  } catch(SourceProxyNotFoundException& ex) {
	    CREAM_SAFE_LOG(m_log_dev->errorStream() 
			   << "DNProxyManager::CTOR() - Error copying proxy ["
			   << job_with_better_proxy_from_sandboxDir->getUserProxyCertificate() << "] to ["
			   << localProxy << "] for DN ["
			   << *it << "]. Skipping"
			   << log4cpp::CategoryStream::ENDLINE);
	    continue;
	  }
	  
	  m_DNProxyMap[*it] = localProxy;
	    
	} else {// the local proxy could be there but older than that one owned by the job in the sandbox dir
          if( job_with_better_proxy_from_sandboxDir == cache->end() )
	    {
	      CREAM_SAFE_LOG(m_log_dev->warnStream() 
			     << "DNProxyManager::CTOR() - Not found any proxy for DN ["
			     << *it << "] in the sandBoxDirs. Skipping"
			     << log4cpp::CategoryStream::ENDLINE);
	      continue;
	    }
          this->setUserProxyIfLonger(*it, job_with_better_proxy_from_sandboxDir->getUserProxyCertificate());
      
        } // if( !boost::filesystem::exists( thisPath ) )
     } // for
}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger( const string& prx ) throw()
{ 
  boost::recursive_mutex::scoped_lock M( mutex );
  //string dn;
  //  try {
  //dn = glite::ce::cream_client_api::certUtil::getDNFQAN( prx );
  
  cream_api::soap_proxy::VOMSWrapper V( prx );
  if( !V.IsValid( ) ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Cannot retrieve the Subject for the proxy ["
		   << prx << "]. ICE will continue to use the old proxy.Error is: "
		   << V.getErrorMessage()
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  }
  
  //string dn = V.getDNFQAN();
  
  this->setUserProxyIfLonger( V.getDNFQAN(), prx );
  
}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger( const string& dn, 
						    const string& prx
						    ) throw()
{ 
  boost::recursive_mutex::scoped_lock M( mutex );
    // string localProxy = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/" + iceUtil::canonizeString( dn ) + ".proxy";
    string localProxy = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/" + compressed_string( dn ) + ".proxy";


  if( m_DNProxyMap.find( dn ) == m_DNProxyMap.end() ) {
    CREAM_SAFE_LOG(m_log_dev->warnStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "DN ["
		   << dn << "] not found. Inserting the proxy ["
		   << prx << "]. Copied into ["
		   << localProxy << "]"
		   << log4cpp::CategoryStream::ENDLINE);

    try {
      this->copyProxy( prx, localProxy );
    } catch(SourceProxyNotFoundException& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() 
		     << "DNProxyManager::setUserProxyIfLonger - Error copying proxy ["
		     << prx << "] to ["
		     << localProxy << "]."
		     << log4cpp::CategoryStream::ENDLINE);
      
      return;
      
    }
    m_DNProxyMap[ dn ] = localProxy;

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
    oldT = glite::ce::cream_client_api::certUtil::getProxyTimeLeft( m_DNProxyMap[ dn ] ); 

  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Cannot retrieve time left for old proxy ["
		   << m_DNProxyMap[ dn ] << "]. Setting user proxy to the new one ["
		   << prx << "] copied to ["
		   << localProxy <<"]: "
		   << ex.what()
		   << log4cpp::CategoryStream::ENDLINE);

    try {
      this->copyProxy( prx, localProxy );
    } catch(SourceProxyNotFoundException& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() 
		     << "DNProxyManager::setUserProxyIfLonger - Error copying proxy ["
		     << prx << "] to ["
		     << localProxy << "]."
		     << log4cpp::CategoryStream::ENDLINE);
      return;
    }
    m_DNProxyMap[ dn ] = localProxy;
    return;
  }

  if(newT > oldT) {
    CREAM_SAFE_LOG(m_log_dev->infoStream() 
		   << "DNProxyManager::setUserProxyIfLonger - "
		   << "Setting user proxy to [ "
		   << prx
		   << "] copied to " << localProxy 
		   << "] because the old one is less long-lived."
		   << log4cpp::CategoryStream::ENDLINE);

    try {
      this->copyProxy( prx, localProxy );
    } catch(SourceProxyNotFoundException& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() 
		     << "DNProxyManager::setUserProxyIfLonger - Error copying proxy ["
		     << prx << "] to ["
		     << localProxy << "]."
		     << log4cpp::CategoryStream::ENDLINE);
    }
    m_DNProxyMap[ dn ] = localProxy;

  } else {

    CREAM_SAFE_LOG(m_log_dev->debugStream() 
		   << "DNProxyManager::setUserProxyIfLonger - Leaving current proxy ["
		   << m_DNProxyMap[ dn ] <<"] beacuse it will expire later"
		   << log4cpp::CategoryStream::ENDLINE);

  }
}

//________________________________________________________________________
void iceUtil::DNProxyManager::copyProxy( const string& source, const string& target ) 
  throw(SourceProxyNotFoundException&)
{
  

  string tmpTargetFilename = target;
  tmpTargetFilename = tmpTargetFilename + string(".tmp") ;

  boost::filesystem::path thisPath( source, boost::filesystem::native );
  if( !boost::filesystem::exists( thisPath ) ) 
    {
      throw SourceProxyNotFoundException(string("Proxy file [")+source+"] is not there. Skipping.");
    }

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


//________________________________________________________________________
iceUtil::jobCache::iterator 
iceUtil::DNProxyManager::searchBetterProxyForUser( const std::string& dn ) 
  throw() 
{
  
  time_t besttime = 0;
  iceUtil::jobCache::iterator bestProxy = iceUtil::jobCache::getInstance()->end();

  CREAM_SAFE_LOG(
  	m_log_dev->debugStream() 
		<< "DNProxyManager::searchBetterProxyForUser() - Searching better proxy for DN ["
		<< dn << "]" << log4cpp::CategoryStream::ENDLINE
  );

  for(iceUtil::jobCache::iterator jit = iceUtil::jobCache::getInstance()->begin();
      jit != iceUtil::jobCache::getInstance()->end();
      ) 
    {
      if( jit->getUserDN() != dn ) {
        CREAM_SAFE_LOG(
  	m_log_dev->debugStream() 
		<< "DNProxyManager::searchBetterProxyForUser() - Skipping DN ["
		<< jit->getUserDN() << "]" << log4cpp::CategoryStream::ENDLINE );
	++jit;
	continue;
      }
    
      string jobCert = jit->getUserProxyCertificate();
      boost::filesystem::path thisPath( jobCert, boost::filesystem::native );


      // check if sandboxdir proxy is there; if not remove the job from cache
      if( !boost::filesystem::exists( thisPath ) )
	{
	  CREAM_SAFE_LOG(m_log_dev->errorStream() 
			 << "DNProxyManager::searchBetterProxyForUser - "
			 << "Cannot find SandboxDir's proxy file ["
			 << jobCert << "]. Removing job "
			 << jit->describe() << " from cache."
			 << log4cpp::CategoryStream::ENDLINE);
	  jit = iceUtil::jobCache::getInstance()->erase( jit );
	  continue;
	}


      // check if the current job's proxy is the long-lived
      //time_t timeleft;
      //      try {
      
      //timeleft = cream_api::certUtil::getProxyTimeLeft( jobCert );
      cream_api::soap_proxy::VOMSWrapper V( jobCert );
      if( !V.IsValid( ) ) {
	CREAM_SAFE_LOG(m_log_dev->errorStream() 
		       << "DNProxyManager::searchBetterProxyForUser() - "
		       << "Cannot extract proxy time left from proxy file ["
		       << jobCert << "]: "
		       << V.getErrorMessage() << ". Skipping"
		       << log4cpp::CategoryStream::ENDLINE);
	++jit;
	continue;
      }
	
//       } catch(exception& ex) {
// 	CREAM_SAFE_LOG(m_log_dev->errorStream() 
// 		       << "DNProxyManager::searchBetterProxyForUser - "
// 		       << "Cannot extract proxy time left from proxy file "
// 		       << "[" << jobCert << "]: "<< ex.what() << ". Skipping"
// 		       << log4cpp::CategoryStream::ENDLINE);
// 	++jit;
// 	continue;
//       }

      time_t timeleft = V.getProxyTimeEnd() - time(NULL);

      if (timeleft > besttime)
	{
	  bestProxy = jit;
	  besttime = timeleft;
	}

      ++jit;
      continue;
      
    }

  return bestProxy;

}
