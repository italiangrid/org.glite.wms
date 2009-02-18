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
#include "glite/security/proxyrenewal/renewal.h"

#include "jobCache.h"
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

extern int errno;

namespace iceUtil = glite::wms::ice::util;
namespace cream_api = glite::ce::cream_client_api;
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

  CREAM_SAFE_LOG(m_log_dev->debugStream() 
		 << "DNProxyManager::CTOR() - "
		 << "Populating DN -> Proxy cache by scannig the jobCache..."
		 );
  
  /**
   * This lock acquisition should be safe because it is done at the ICE
   * initialization, when no other thread acquire the cache's mutex
   *
   */
  boost::recursive_mutex::scoped_lock M( iceUtil::jobCache::mutex );

  /**
   * Retrieve all distinguished DNs
   */
  map<string, long long int > dnSet_myproxy; // DN -> (myproxy, how many jobs active for this DN having MYPROXYSERVER)
  set<string> dnSet;

  for( jobCache::iterator jit = cache->begin();
       jit != cache->end();
       ++jit) 
    {
      if(jit->is_proxy_renewable()) {

	dnSet_myproxy[ this->composite(jit->getUserDN(), jit->getMyProxyAddress()) ]++;
	
      } else {
	dnSet.insert( jit->getUserDN() );
      }
    }

  /**
   * for each DN check if the better ICE's local proxy is there
   */
  string localProxy;
  string prefix = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/";
  
  iceUtil::jobCache::iterator job_with_better_proxy_from_sandboxDir;
  
  for(set<string>::const_iterator it = dnSet.begin();
      it != dnSet.end();
      ++it)
    {
      if( it->empty() ) continue;

      localProxy = prefix + compressed_string( *it ) + ".proxy";
      boost::filesystem::path thisPath( localProxy, boost::filesystem::native );
      
      pair<iceUtil::jobCache::iterator, time_t> job_with_better_proxy_from_sandboxDir = this->searchBetterProxy( *it );
      
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
			 );

	  if( job_with_better_proxy_from_sandboxDir.first == cache->end() )
	    {
	      continue;
	    }
	    
	  try {
	  
	    this->copyProxy(job_with_better_proxy_from_sandboxDir.first->getUserProxyCertificate(), localProxy);
	    
	  } catch(SourceProxyNotFoundException& ex) {
	    CREAM_SAFE_LOG(m_log_dev->errorStream() 
			   << "DNProxyManager::CTOR() - Error copying proxy ["
			   << job_with_better_proxy_from_sandboxDir.first->getUserProxyCertificate() << "] to ["
			   << localProxy << "] for DN ["
			   << *it << "]. Skipping"
			   );
	    continue;
	  }
	  
	  //m_DNProxyMap[*it] = make_pair( localProxy, job_with_better_proxy_from_sandboxDir.second ) ;
	  m_DNProxyMap[*it] = boost::make_tuple(localProxy, job_with_better_proxy_from_sandboxDir.second, 0 );
	    
	} else {// the local proxy could be there but older than that one owned by the job in the sandbox dir
          if( job_with_better_proxy_from_sandboxDir.first == cache->end() )
	    {
	      CREAM_SAFE_LOG(m_log_dev->warnStream() 
			     << "DNProxyManager::CTOR() - Not found any proxy for DN ["
			     << *it << "] in the sandBoxDirs. Skipping"
			     );
	      continue;
	    }
          this->setUserProxyIfLonger_Legacy(*it, 
					    job_with_better_proxy_from_sandboxDir.first->getUserProxyCertificate(), 
					    job_with_better_proxy_from_sandboxDir.second);
      
        } // if( !boost::filesystem::exists( thisPath ) )
     } // for


  /**
     All "legacy" better proxy have been loaded (those ones related to DN that submitted jobs without MYPROXYSERVER.
     Now let's proceed with loading of all "NEW" better proxies 
  */
  for(map<string, long long int>::const_iterator it = dnSet_myproxy.begin();
      it != dnSet_myproxy.end();
      ++it)
    {
      string localproxy = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/";
      localproxy += compressed_string( it->first ) + "_NEW.proxy";
      boost::filesystem::path thisPath( localProxy, boost::filesystem::native );
      
      if( boost::filesystem::exists( thisPath ) ) {
	cream_api::soap_proxy::VOMSWrapper V( localProxy );
	if( !V.IsValid( ) ) {
	  CREAM_SAFE_LOG(m_log_dev->errorStream() 
			 << "DNProxyManager::CTOR() - "
			 << "Cannot retrieve the Subject for the proxy ["
			 << localProxy << "]. Error is: "
			 << V.getErrorMessage() <<". Skipping..."
			 );
	  continue;
	}
	
	if(V.getProxyTimeEnd()> time(0)) {
	  CREAM_SAFE_LOG(m_log_dev->debugStream() 
			 << "DNProxyManager::CTOR() - "
			 << "Inserting tuple (" << localProxy<<", "
			 << V.getProxyTimeEnd() << ", " << it->second<< ") into m_DNProxyMap for DN ["
			 << it->first << "]..."
			 );

	  m_DNProxyMap[ it->first ] = boost::make_tuple( localProxy, V.getProxyTimeEnd(), it->second);

	}
      }
    }

}

//________________________________________________________________________
void iceUtil::DNProxyManager::decrementUserProxyCounter( const string& userDN, const string& myproxy_name ) 
  throw()
{
  boost::recursive_mutex::scoped_lock M( mutex );
  
  string mapKey = this->composite( userDN, myproxy_name );
  
  string regID = compressed_string( mapKey );
  
  CREAM_SAFE_LOG(
		   m_log_dev->debugStream()
		   << "DNProxyManager::decrementUserProxyCounter() - "
		   << "Looking for for DN ["
		   << userDN << "] MyProxy server ["
		   << myproxy_name << "] in the m_DNProxyMap..."
		   );
  
  map<string, boost::tuple<string, time_t, long long int> >::iterator it = m_DNProxyMap.find( mapKey );
  
  if( it != m_DNProxyMap.end() ) {

    CREAM_SAFE_LOG(
		   m_log_dev->debugStream()
		   << "DNProxyManager::decrementUserProxyCounter() - "
		   << "Decrementing proxy counter for DN ["
		   << userDN << "] MyProxy server ["
		   << myproxy_name <<"]"
		   );

    m_DNProxyMap[ mapKey ] = boost::make_tuple( it->second.get<0>(), it->second.get<1>(), it->second.get<2>() - 1);

    if(it->second.get<2>() == 1) {

      CREAM_SAFE_LOG(
		   m_log_dev->debugStream()
		   << "DNProxyManager::decrementUserProxyCounter() - "
		   << "Proxy Counter is ZERO for DN ["
		   << userDN << "] MyProxy server ["
		   << myproxy_name<<"]. Unregistering it and removing symlink ["
		   << m_DNProxyMap[ mapKey ].get<0>()
		   << "] from persist_dir..."
		   );
      /**
	 If the counter is 0 deregister the current proxy and clear the map and delete the file.
      */
      int err = glite_renewal_UnregisterProxy( regID.c_str(), NULL );
      
      if ( err && (err != EDG_WLPR_PROXY_NOT_REGISTERED) ) {
	CREAM_SAFE_LOG(
		       m_log_dev->errorStream()
		       << "DNProxyManager::decrementUserProxyCounter() - "
		       << "Couldn't unregister the proxy registered with ID ["
		       << regID << "]. Error is: "
		       << edg_wlpr_GetErrorText(err) << ". Ignoring..."
		       );
      }

      

      if(::unlink( m_DNProxyMap[ mapKey ].get<0>().c_str() ) < 0)
	{
	  int saveerr = errno;
	  CREAM_SAFE_LOG(
			 m_log_dev->errorStream()
			 << "DNProxyManager::decrementUserProxyCounter() - "
			 << "Unlink of file ["
			 << m_DNProxyMap[ mapKey ].get<0>() << "] is failed. Error is: "
			 << strerror(saveerr);
			 );
	}
      
      m_DNProxyMap.erase( mapKey );
      return;
    
    }

  }
}

//________________________________________________________________________
void iceUtil::DNProxyManager::registerUserProxy( const string& userDN, 
						 const string& userProxy,
						 const string& my_proxy_server,
						 const time_t proxy_time_end)
  throw()
{

  boost::recursive_mutex::scoped_lock M( mutex );

  string mapKey = this->composite( userDN, my_proxy_server );

  map<string, boost::tuple<string, time_t, long long int> >::iterator it = m_DNProxyMap.find( mapKey );
  if( it != m_DNProxyMap.end() ) {

    CREAM_SAFE_LOG(m_log_dev->debugStream() 
		   << "DNProxyManager::registerUserProxy() - Found a proxy for user DN ["
		   << mapKey <<"]. Will not register it to MyProxyServer..."
		   );

    m_DNProxyMap[ mapKey ] = boost::make_tuple( it->second.get<0>(), it->second.get<1>(), it->second.get<2>() + 1);
    return;
  }

  char *renewal_proxy_path = NULL;

  string regID = compressed_string( mapKey );

  CREAM_SAFE_LOG(m_log_dev->debugStream() 
		 << "DNProxyManager::registerUserProxy() - Going to register proxy ["
		 << userProxy << "] to MyProxyServer ["
		 << my_proxy_server << "] with ID ["
		 << regID << "]."
		 );

  int register_result = glite_renewal_RegisterProxy(userProxy.c_str(), 
						    my_proxy_server.c_str(), 
						    7512, 
						    regID.c_str(), 
						    EDG_WLPR_FLAG_UNIQUE, 
						    &renewal_proxy_path);

  if(register_result) {

    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::registerUserProxy() - glite_renewal_RegisterProxy failed with error code: "
		   << register_result << ". Falling back to LEGACY BetterProxy calculation..."
		   );

    this->setUserProxyIfLonger_Legacy( userDN, userProxy );

    return;

  } else {
    
    /**
       make a symlink in the persist_dir to the path of the renewal_proxy_path. 
       Used at ICE's boot to restore the m_DNProxyMap_NEW.
    */
    string proxylink = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/";
    proxylink += regID + "_NEW.proxy";

    if( symlink( renewal_proxy_path, proxylink.c_str() ) < 0) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() 
		     << "DNProxyManager::registerUserProxy() - "
		     << "Cannot symlink [" << renewal_proxy_path << "] to ["
		     << proxylink << "]. Error is: " << strerror(errno) 
		     << ". Falling back to LEGACY BetterProxy calculation..."
		     );
      /**
	 UN-register 
      */

      int err = glite_renewal_UnregisterProxy( regID.c_str(), NULL );

      if ( err && (err != EDG_WLPR_PROXY_NOT_REGISTERED) ) {
	CREAM_SAFE_LOG(
		       m_log_dev->errorStream()
		       << "DNProxyManager::registerUserProxy() - "
		       << "Couldn't unregister the proxy registered with ID ["
		       << regID << "]. Error is: "
		       << edg_wlpr_GetErrorText(err) << ". Ignoring..."
		       );
	
	

	return;
      }

      this->setUserProxyIfLonger_Legacy( userDN, userProxy );

      return;

    }

    /**
       Everything went ok. Now let's save the registered proxy !
    */
    m_DNProxyMap[ mapKey ] = boost::make_tuple( proxylink, proxy_time_end, 1);

  }
}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger_Legacy( const string& prx ) throw()
{ 
  boost::recursive_mutex::scoped_lock M( mutex );
  //string dn;
  //  try {
  //dn = glite::ce::cream_client_api::certUtil::getDNFQAN( prx );
  
  cream_api::soap_proxy::VOMSWrapper V( prx );
  if( !V.IsValid( ) ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger_Legacy() - "
		   << "Cannot read the proxy ["
		   << prx << "]. ICE will continue to use the old better proxy. Error is: "
		   << V.getErrorMessage()
		   );
    return;
  }
  
  this->setUserProxyIfLonger_Legacy( V.getDNFQAN(), prx, V.getProxyTimeEnd() );
  
}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger_Legacy( const string& dn, 
						    const string& prx
						    ) throw()
{
  boost::recursive_mutex::scoped_lock M( mutex );

  time_t newT;

  try {

    newT= glite::ce::cream_client_api::certUtil::getProxyTimeLeft( prx );

  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setUserProxyIfLonger_Legacy() - "
		   << "Cannot retrieve time left for proxy ["
		   << prx << "]. ICE will continue to use the old better proxy. Error is: "
		   << ex.what()
		   );
    
    return;
  }

  this->setUserProxyIfLonger_Legacy( dn, prx, newT );
}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger_Legacy( const string& dn, 
							   const string& prx,
							   const time_t exptime
							   ) throw()
{ 
  boost::recursive_mutex::scoped_lock M( mutex );

    string localProxy = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/" + compressed_string( dn ) + ".proxy";

  if( m_DNProxyMap.find( dn ) == m_DNProxyMap.end() ) {

    try {

      this->copyProxy( prx, localProxy );

    } catch(SourceProxyNotFoundException& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() 
		     << "DNProxyManager::setUserProxyIfLonger_Legacy() - Error copying proxy ["
		     << prx << "] to ["
		     << localProxy << "]."
		     );
      
      return;
      
    }
    
    CREAM_SAFE_LOG(m_log_dev->debugStream() 
		   << "DNProxyManager::setUserProxyIfLonger_Legacy() - "
		   << "DN ["
		   << dn << "] not found. Inserting the new proxy ["
		   << prx << "]. Will be Copied into ["
		   << localProxy << "] - New Expiration Time is ["
		   << time_t_to_string(exptime) << "]"
		   );

    //m_DNProxyMap_Legacy[ dn ] = make_pair(localProxy, exptime);
    m_DNProxyMap[ dn ] = boost::make_tuple( localProxy, exptime, 0);

    return;
  }

  time_t newT, oldT;

  newT = exptime;
  oldT = m_DNProxyMap[ dn ].get<1>();

  if( !oldT ) { // couldn't get the proxy time for some reason
    try {
      this->copyProxy( prx, localProxy );
    } catch(SourceProxyNotFoundException& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() 
     		     << "DNProxyManager::setUserProxyIfLonger_Legacy() - Error copying proxy ["
		     << prx << "] to ["
		     << localProxy << "]."
     		     );
      return;
    }

    CREAM_SAFE_LOG(m_log_dev->debugStream() 
		   << "DNProxyManager::setUserProxyIfLonger_Legacy() - "
		   << "New proxy ["
		   << prx << "] has been copied into ["
		   << localProxy << "] - New Expiration Time is ["
		   << time_t_to_string(exptime) << "]"
		   );
    m_DNProxyMap[ dn ] = boost::make_tuple(localProxy, exptime, 0);//make_pair(localProxy, exptime);

    return;
  }
  
  if(newT > oldT) {
    CREAM_SAFE_LOG(m_log_dev->infoStream() 
		   << "DNProxyManager::setUserProxyIfLonger_Legacy() - "
		   << "Setting user proxy to [ "
		   << prx
		   << "] copied to " << localProxy 
		   << "] because the old one is less long-lived."
		   );

    try {

      this->copyProxy( prx, localProxy );

    } catch(SourceProxyNotFoundException& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() 
		     << "DNProxyManager::setUserProxyIfLonger_Legacy() - Error copying proxy ["
		     << prx << "] to ["
		     << localProxy << "]."
		     );
    }

    CREAM_SAFE_LOG(m_log_dev->debugStream() 
		   << "DNProxyManager::setUserProxyIfLonger_Legacy() - "
		   << "New proxy ["
		   << prx << "] has been copied into ["
		   << localProxy << "] - New Expiration Time is ["
		   << time_t_to_string(newT) << "]"
		   );

    m_DNProxyMap[ dn ] = boost::make_tuple(localProxy, newT, 0);//make_pair(localProxy, newT);

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
		   << "DNProxyManager::copyProxy() - Cannot open"
		   << " proxy file ["
		   << source << "] for input. ABORTING!!"
		   );
    abort();
  }

  if(!output) {
    CREAM_SAFE_LOG(m_log_dev->fatalStream() 
		   << "DNProxyManager::copyProxy() - Cannot open"
		   << " proxy file ["
		   << tmpTargetFilename << "] for output. ABORTING!!"
		   );
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
			 );
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
		     << "DNProxyManager::copyProxy() - Error moving ["
		     << tmpTargetFilename << "] to ["
		     << target << "]: " << errmex << ". ABORTING!!"
		     );
      abort();
    }

  ::unlink(tmpTargetFilename.c_str());

  ::chmod( target.c_str(), 00600 );
}

//________________________________________________________________________
pair<iceUtil::jobCache::iterator, time_t>
iceUtil::DNProxyManager::searchBetterProxy( const string& dn ) 
  throw() 
{
  
  /**
     This is a private method. It is only invoked from the CTOR of this class
     that already has taken the joCache's mutex
  */

  time_t besttime = 0;
  iceUtil::jobCache::iterator bestProxy = iceUtil::jobCache::getInstance()->end();

  CREAM_SAFE_LOG(
  	m_log_dev->debugStream() 
		<< "DNProxyManager::searchBetterProxyForUser() - Searching better proxy for DN ["
		<< dn << "]" 
  );

  for(iceUtil::jobCache::iterator jit = iceUtil::jobCache::getInstance()->begin();
      jit != iceUtil::jobCache::getInstance()->end();
      ) 
    {
      if( jit->getUserDN() != dn ) {
	++jit;
	continue;
      }
    
      string jobCert = jit->getUserProxyCertificate();
      boost::filesystem::path thisPath( jobCert, boost::filesystem::native );


      // check if sandboxdir proxy is there; if not remove the job from cache
      if( !boost::filesystem::exists( thisPath ) )
	{
	  CREAM_SAFE_LOG(m_log_dev->errorStream() 
			 << "DNProxyManager::searchBetterProxyForUser() - "
			 << "Cannot find SandboxDir's proxy file ["
			 << jobCert << "]. Removing job "
			 << jit->describe() << " from cache."
			 );
	  jit = iceUtil::jobCache::getInstance()->erase( jit );
	  continue;
	}

      cream_api::soap_proxy::VOMSWrapper V( jobCert );
      if( !V.IsValid( ) ) {
	CREAM_SAFE_LOG(m_log_dev->errorStream() 
		       << "DNProxyManager::searchBetterProxyForUser() - "
		       << "Cannot extract proxy time left from proxy file ["
		       << jobCert << "]: "
		       << V.getErrorMessage() << ". Skipping"
		       );
	++jit;
	continue;
      }
	
      time_t timeleft = V.getProxyTimeEnd() - time(NULL);

      if (timeleft > besttime)
	{
	  bestProxy = jit;
	  besttime = timeleft;
	}

      ++jit;
      continue;
      
    }

  return make_pair(bestProxy, besttime);

}

//________________________________________________________________________
void iceUtil::DNProxyManager::removeBetterProxy( const string& userDN, const string& myproxyname )
  throw()
{
  boost::recursive_mutex::scoped_lock M( mutex );

  m_DNProxyMap.erase( this->composite(userDN, myproxyname) );
}

//________________________________________________________________________
void iceUtil::DNProxyManager::updateBetterProxy( const string& userDN, 
						 const string& myproxyname,
						 const boost::tuple<string, time_t, long long int>& newEntry )
  throw()
{
  boost::recursive_mutex::scoped_lock M( mutex );
  string mapKey = this->composite( userDN, myproxyname );

  m_DNProxyMap.erase( mapKey );
  m_DNProxyMap[ mapKey ] = newEntry;
}

//________________________________________________________________________
boost::tuple<string, time_t, long long int> 
iceUtil::DNProxyManager::getAnyBetterProxyByDN( const string& dn )
const throw() 
	  {
	    
	    boost::recursive_mutex::scoped_lock M( mutex );
	    
	    map<string, boost::tuple<string, time_t, long long int> >::const_iterator it = m_DNProxyMap.begin();
	    
	    //printf("*** DEBUG DNProxyManager: Searching a betterproxy for DN [%s]...\n", dn.c_str());

	    while( it != m_DNProxyMap.end() ) {
	      
	      /**
		 does a regex match. The DN must be present in the key (that can also contains the myproxy server name)
	      */
	      


	      if( strstr( it->first.c_str(),dn.c_str() ) == 0 ) {
	       /**
		  'dn' is not found in the it->first string
	       */
	      // printf("*** DEBUG DNProxyManager: [%s] doesn't match...\n", it->first.c_str() );
	       ++it;
	       continue;
	     }


	      /**
		 The better proxy is used for operation about job management like: poll, purge, cancel... all but proxy renewal.
		 Then, it is not important that the better proxy is the most long-living one, but a valid one that is valid at least
		 for 2 times the SOAP_TIMEOUT.
	      */
	     if( it->second.get<1>() > (time(0)+(2*iceConfManager::getInstance()->getConfiguration()->ice()->soap_timeout())) ) 
	       {
		// printf("*** DEBUG DNProxyManager: [%s] MATCH and has a profer lifetime!...\n", it->first.c_str() );
		return (it->second);
	       }
	      
	     //printf("*** DEBUG DNProxyManager: [%s] MATCH but has NOT a profer lifetime [%s]!...\n", it->first.c_str(), time_t_to_string( it->second.get<1>() ).c_str() );
	     //	     cout << "*** DEBUG DNProxyManager: ["<<it->first.c_str()<<"] MATCH but has NOT a profer lifetime [" << it->second.get<1>() << "]" <<endl;

	      ++it;
	    }
	    
	    //printf("*** DEBUG DNProxyManager: No PROXY suitable for DN [%s]...\n", dn.c_str() );
	    return boost::make_tuple("", 0, 0);
	    
	  }

//________________________________________________________________________
boost::tuple<string, time_t, long long int> 
iceUtil::DNProxyManager::getExactBetterProxyByDN( const string& dn,
						  const string& myproxyname)
 const throw() 
	  {
	    
	    boost::recursive_mutex::scoped_lock M( mutex );
	    
	    map<string, boost::tuple<string, time_t, long long int> >::const_iterator it = m_DNProxyMap.find( this->composite(dn, myproxyname ) );
	    
	    if( it == m_DNProxyMap.end() ) {
	      return boost::make_tuple("", 0, 0);
	    } else {
	      return boost::make_tuple(it->second.get<0>(), it->second.get<1>(), it->second.get<2>());
	    }
	    
	  }
