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

#include "iceDb/GetAllProxyByDN.h"
#include "iceDb/RemoveJobByProxy.h"
#include "iceDb/GetAllUserDN.h"
#include "iceDb/GetAllUserDN_MyProxy.h"
#include "iceDb/GetProxyInfoByDN.h"
#include "iceDb/RemoveProxyByDN.h"
#include "iceDb/UpdateProxyFieldsByDN.h"
#include "iceDb/Transaction.h"
#include "iceDb/CreateProxyField.h"
#include "iceDb/UpdateProxyFieldsByDN.h"
#include "iceDb/GetAllProxyInfo.h"

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
#include <map>
#include <string>

#include <boost/filesystem/operations.hpp>
#include <boost/tuple/tuple.hpp>

extern int errno;

namespace iceUtil = glite::wms::ice::util;
namespace cream_api = glite::ce::cream_client_api;
using namespace std;

iceUtil::DNProxyManager* iceUtil::DNProxyManager::s_instance = NULL;
boost::recursive_mutex iceUtil::DNProxyManager::s_mutex;

//______________________________________________________________________________
iceUtil::DNProxyManager* iceUtil::DNProxyManager::getInstance() throw()
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::getInstance");
#endif
  boost::recursive_mutex::scoped_lock M( s_mutex );

  if( !s_instance ) 
    s_instance = new DNProxyManager();
  return s_instance;
}

//______________________________________________________________________________
iceUtil::DNProxyManager::DNProxyManager( void ) throw()
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::DNProxyManager");
#endif
  m_log_dev = cream_api::util::creamApiLogger::instance()->getLogger();
  
}

//______________________________________________________________________________
// void
// iceUtil::DNProxyManager::unregisterUserProxy( const string& dn,
// 					      const string& server,
// 					      const string& _proxyfile) throw()
// {
//   boost::recursive_mutex::scoped_lock M( s_mutex );
  
//   string mapKey = this->composite( dn, server );
  
//   string reg_id = compressed_string( mapKey );

//   string proxyfile = _proxyfile;

//   int err = glite_renewal_UnregisterProxy( reg_id.c_str(), NULL );
      
//   if ( err && (err != EDG_WLPR_PROXY_NOT_REGISTERED) ) {
//     CREAM_SAFE_LOG(
// 		   m_log_dev->errorStream()
// 		   << "DNProxyManager::unregisterUserProxy() - "
// 		   << "Couldn't unregister the proxy registered with ID ["
// 		   << reg_id << "]. Error is: "
// 		   << edg_wlpr_GetErrorText(err) << ". Ignoring..."
// 		   );
//   }
  
//   CREAM_SAFE_LOG(
// 		 m_log_dev->debugStream()
// 		 << "DNProxyManager::unregisterUserProxy() - "
// 		 << "Unlinking file ["
// 		 << proxyfile << "]... "
// 		 );
  
//   if( proxyfile.empty() ) {
//     db::GetProxyInfoByDN getter( mapKey );
//     db::Transaction tnx;
//     tnx.execute( &getter );
//     proxyfile = getter.get_info().get<0>();
//   }

//   if(::unlink( proxyfile.c_str()) < 0 )
//     {
//       int saveerr = errno;
//       CREAM_SAFE_LOG(
// 		     m_log_dev->errorStream()
// 		     << "DNProxyManager::unregisterUserProxy() - "
// 		     << "Unlink of file ["
// 		     << proxyfile << "] is failed. Error is: "
// 		     << strerror(saveerr);
// 		     );
//     }
  
// }



//________________________________________________________________________
// void iceUtil::DNProxyManager::changeRegisteredUserProxy( const string& userDN, 
// 							 const string& userProxy,
// 							 const string& my_proxy_server,
// 							 const time_t proxy_time_end)
//   throw()
// {
//   boost::recursive_mutex::scoped_lock M( s_mutex );
//   string mapKey = this->composite( userDN, my_proxy_server );
//   db::Transaction tnx;
//   tnx.Begin_exclusive();

//   char *renewal_proxy_path = NULL;

//   string regID = compressed_string( mapKey );

//   CREAM_SAFE_LOG(m_log_dev->debugStream() 
// 		 << "DNProxyManager::changeRegisteredUserProxy() - Going to register proxy ["
// 		 << userProxy << "] to MyProxyServer ["
// 		 << my_proxy_server << "] with ID ["
// 		 << regID << "]."
// 		 );

//   int register_result = glite_renewal_RegisterProxy(userProxy.c_str(), 
// 						    my_proxy_server.c_str(), 
// 						    7512, 
// 						    regID.c_str(), 
// 						    EDG_WLPR_FLAG_UNIQUE, 
// 						    &renewal_proxy_path);

//   if(register_result) {

//     CREAM_SAFE_LOG(m_log_dev->errorStream() 
// 		   << "DNProxyManager::changeRegisteredUserProxy() - glite_renewal_RegisterProxy failed with error code: "
// 		   << register_result << ". Falling back to LEGACY BetterProxy calculation..."
// 		   );
    
//     this->setUserProxyIfLonger_Legacy( userDN, userProxy );
    
//     return;

//   } else {
    
//     /**
//        make a symlink in the persist_dir to the path of the renewal_proxy_path. 
//        Used at ICE's boot to restore the m_DNProxyMap_NEW.
//     */
//     string proxylink = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/";
//     proxylink += regID + "_NEW.proxy";

//     if( symlink( renewal_proxy_path, proxylink.c_str() ) < 0) {
//       CREAM_SAFE_LOG(m_log_dev->errorStream() 
// 		     << "DNProxyManager::changeRegisteredUserProxy() - "
// 		     << "Cannot symlink [" << renewal_proxy_path << "] to ["
// 		     << proxylink << "]. Error is: " << strerror(errno) 
// 		     << ". Falling back to LEGACY BetterProxy calculation..."
// 		     );
//       /**
// 	 UN-register 
//       */
      
//       int err = glite_renewal_UnregisterProxy( regID.c_str(), NULL );

//       if ( err && (err != EDG_WLPR_PROXY_NOT_REGISTERED) ) {
// 	CREAM_SAFE_LOG(
// 		       m_log_dev->errorStream()
// 		       << "DNProxyManager::changeRegisteredUserProxy() - "
// 		       << "Couldn't unregister the proxy registered with ID ["
// 		       << regID << "]. Error is: "
// 		       << edg_wlpr_GetErrorText(err) << ". Ignoring..."
// 		       );
	
	

// 	return;
//       }

//       this->setUserProxyIfLonger_Legacy( userDN, userProxy );
      
//       return;

//     } else {
      
//       /**
// 	 Everything went ok. Now let's save the registered proxy !
//       */
//       {
// 	//db::CreateProxyField creator( mapKey, proxylink, proxy_time_end, 1 );
// 	CREAM_SAFE_LOG(
// 		       m_log_dev->errorStream()
// 		       << "DNProxyManager::changeRegisteredUserProxy() - "
// 		       << "Goingo to change proxyfile to ["
// 		       << proxylink << "] and exptime to ["
// 		       << proxy_time_end << "] for DN ["
// 		       << userDN << "] myproxyserver ["
// 		       << my_proxy_server << "]"
// 		       );

// 	list< pair<string, string> > params;
// 	params.push_back( make_pair( "proxyfile", proxylink) );
// 	params.push_back( make_pair( "exptime", time_t_to_string(proxy_time_end )) );
// 	db::UpdateProxyFieldsByDN updater( mapKey, params );
	
// 	db::Transaction tnx;
// 	tnx.execute( &updater );
//       }
//     }
//   }
// }

//________________________________________________________________________
// void iceUtil::DNProxyManager::registerUserProxy( const string& userDN, 
// 						 const string& userProxy,
// 						 const string& my_proxy_server,
// 						 const time_t proxy_time_end)
//   throw()
// {

//   boost::recursive_mutex::scoped_lock M( s_mutex );

//   string mapKey = this->composite( userDN, my_proxy_server );

//   bool found = false;
//   boost::tuple<string, time_t, long long int> proxy_info;

//   db::Transaction tnx;
//   tnx.Begin_exclusive();

//   {
//     db::GetProxyInfoByDN getter( mapKey );
//     tnx.execute( &getter );
//     found = getter.found();
//     if(found)
//       proxy_info = getter.get_info();
//   }
  
//   if( found ) {

//     CREAM_SAFE_LOG(m_log_dev->debugStream() 
// 		   << "DNProxyManager::registerUserProxy() - Found a proxy for user DN ["
// 		   << mapKey <<"]. Will not register it to MyProxyServer..."
// 		   );

//     {
//       list<pair<string, string> > params;
//       ostringstream tmp;
//       tmp << (proxy_info.get<2>() + 1);
//       params.push_back( make_pair("counter", tmp.str()) );
//       db::UpdateProxyFieldsByDN updater( mapKey, params );
//       tnx.execute( &updater );
//     }

//     return;
//   }

//   char *renewal_proxy_path = NULL;

//   string regID = compressed_string( mapKey );

//   CREAM_SAFE_LOG(m_log_dev->debugStream() 
// 		 << "DNProxyManager::registerUserProxy() - Going to register proxy ["
// 		 << userProxy << "] to MyProxyServer ["
// 		 << my_proxy_server << "] with ID ["
// 		 << regID << "]."
// 		 );

//   int register_result = glite_renewal_RegisterProxy(userProxy.c_str(), 
// 						    my_proxy_server.c_str(), 
// 						    7512, 
// 						    regID.c_str(), 
// 						    EDG_WLPR_FLAG_UNIQUE, 
// 						    &renewal_proxy_path);

//   if(register_result) {

//     CREAM_SAFE_LOG(m_log_dev->errorStream() 
// 		   << "DNProxyManager::registerUserProxy() - glite_renewal_RegisterProxy failed with error code: "
// 		   << register_result << ". Falling back to LEGACY BetterProxy calculation..."
// 		   );

//     this->setUserProxyIfLonger_Legacy( userDN, userProxy );

//     return;

//   } else {
    
//     /**
//        make a symlink in the persist_dir to the path of the renewal_proxy_path. 
//        Used at ICE's boot to restore the m_DNProxyMap_NEW.
//     */
//     string proxylink = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/";
//     proxylink += regID + "_NEW.proxy";

//     if( symlink( renewal_proxy_path, proxylink.c_str() ) < 0) {
//       CREAM_SAFE_LOG(m_log_dev->errorStream() 
// 		     << "DNProxyManager::registerUserProxy() - "
// 		     << "Cannot symlink [" << renewal_proxy_path << "] to ["
// 		     << proxylink << "]. Error is: " << strerror(errno) 
// 		     << ". Falling back to LEGACY BetterProxy calculation..."
// 		     );
//       /**
// 	 UN-register 
//       */

//       int err = glite_renewal_UnregisterProxy( regID.c_str(), NULL );

//       if ( err && (err != EDG_WLPR_PROXY_NOT_REGISTERED) ) {
// 	CREAM_SAFE_LOG(
// 		       m_log_dev->errorStream()
// 		       << "DNProxyManager::registerUserProxy() - "
// 		       << "Couldn't unregister the proxy registered with ID ["
// 		       << regID << "]. Error is: "
// 		       << edg_wlpr_GetErrorText(err) << ". Ignoring..."
// 		       );
	
	

// 	return;
//       }

//       this->setUserProxyIfLonger_Legacy( userDN, userProxy );

//       return;

//     } else {
      
//       /**
// 	 Everything went ok. Now let's save the registered proxy !
//       */
//       {
// 	db::CreateProxyField creator( mapKey, proxylink, proxy_time_end, 1 );
// 	db::Transaction tnx;
// 	tnx.execute( &creator );
//       }
//     }
//   }
// }

/***************************************************
 *
 *
 *
 *
 *
 *    'NORMAL' Better Proxy Handling methods
 *
 *
 *
 *
 *
 ***************************************************/

//________________________________________________________________________
void 
iceUtil::DNProxyManager::setUserProxyIfLonger_Legacy( const string& prx ) 
  throw()
{ 
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::setUserProxyIfLonger_Legacy_1");
#endif
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
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
void 
iceUtil::DNProxyManager::setUserProxyIfLonger_Legacy( const string& dn, 
						      const string& prx ) 
  throw()
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::setUserProxyIfLonger_Legacy_2");
#endif
  boost::recursive_mutex::scoped_lock M( s_mutex );

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
void 
iceUtil::DNProxyManager::setUserProxyIfLonger_Legacy( 
						     const string& dn, 
						     const string& prx,
						     const time_t exptime)
  throw()
{ 
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::setUserProxyIfLonger_Legacy_3");
#endif
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
  string localProxy = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/" + compressed_string( dn ) + ".proxy";
  
  
  bool found = false;
  boost::tuple<string, time_t, long long int> proxy_info;
  {
    db::GetProxyInfoByDN getter( dn, "DNProxyManager::setUserProxyIfLonger_Legacy" );
    db::Transaction tnx(false, false);
    tnx.execute( &getter );
    found = getter.found();
    if(found)
      proxy_info = getter.get_info();
  }

  if( !found ) {
    
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
    
    {
      db::CreateProxyField creator( dn, localProxy, exptime, 0, "DNProxyManager::setUserProxyIfLonger_Legacy" );
      db::Transaction tnx( false, false );
      tnx.execute( &creator );
    }
    
    return;
  }
  
  time_t newT, oldT;

  newT = exptime;
  oldT = proxy_info.get<1>();

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
    {
      db::CreateProxyField creator( dn, localProxy, exptime, 0,"DNProxyManager::setUserProxyIfLonger_Legacy" );
      db::Transaction tnx(false, false);
      tnx.execute( &creator );
    }

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

    {
      db::CreateProxyField creator( dn, localProxy, exptime, 0, "DNProxyManager::setUserProxyIfLonger_Legacy" );
      db::Transaction tnx(false, false);
      tnx.execute( &creator );
    }
  } 
}

//________________________________________________________________________
void iceUtil::DNProxyManager::copyProxy( const string& source, const string& target ) 
  throw(SourceProxyNotFoundException&)
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::copyProxy");
#endif 

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

  CREAM_SAFE_LOG(m_log_dev->debugStream() 
		 << "DNProxyManager::copyProxy() - Copying proxy ["
		 << source <<"] to ["
		 << target << "] ..."
		 );

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

/**********************************************
 *
 *
 *
 *
 *  'SUPER' Better Proxy Handling methods
 *  
 *
 *
 *
 *
 **********************************************/
//________________________________________________________________________
void 
iceUtil::DNProxyManager::removeBetterProxy( const string& userdn, const string& myproxyname )
  throw()
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::removeBetterProxy");
#endif 
  boost::recursive_mutex::scoped_lock M( s_mutex );

  CREAM_SAFE_LOG(m_log_dev->debugStream() 
		 << "DNProxyManager::removeBetterProxy() - "
		 << "Removing from DB better proxy for userdn ["
		 << userdn << "] MyProxy server ["
		 << myproxyname << "]"
		 );

  {
    db::RemoveProxyByDN remover( this->composite(userdn, myproxyname), "DNProxyManager::removeBetterProxy" );
    db::Transaction tnx(false, false);
    tnx.execute( &remover );
  }
  
  string localProxy = this->make_betterproxy_path(userdn, myproxyname); //iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/" + compressed_string( this->composite( userdn, myproxyname ) ) + ".betterproxy";
  
  CREAM_SAFE_LOG(
		 m_log_dev->debugStream()
		 << "DNProxyManager::removeBetterProxy() - "
		 << "Unlinking proxy file ["
		 << localProxy << "]... "
		 );
  
  if(::unlink( localProxy.c_str() ) < 0 )
    {
      int saveerr = errno;
      CREAM_SAFE_LOG(
		     m_log_dev->errorStream()
		     << "DNProxyManager::removeBetterProxy() - "
		     << "Unlink of file ["
		     << localProxy << "] is failed. Error is: "
		     << strerror(saveerr);
		     );
    }
  //::unlink( localProxy.c_str() );
}

//________________________________________________________________________
void 
iceUtil::DNProxyManager::updateBetterProxy( const string& userDN, 
					    const string& myproxyname,
					    const boost::tuple<string, time_t, long long int>& newEntry )
  throw()
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::updateBetterProxy");
#endif
  boost::recursive_mutex::scoped_lock M( s_mutex );
  string mapKey = this->composite( userDN, myproxyname );
  
  string localProxy = this->make_betterproxy_path(userDN, myproxyname );//iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/" + compressed_string( this->composite( userDN, myproxyname ) ) + ".betterproxy";
  
  int rc = ::unlink( localProxy.c_str() );
  if( rc < 0 ) {
    string errmex = strerror(errno);
    CREAM_SAFE_LOG(m_log_dev->fatalStream() 
		   << "DNProxyManager::updateBetterProxy() - Error unlinking ["
		   << localProxy << "]: " << errmex << ". Skipping Better Proxy update"
		   );
    return;
  }

  try {
    
    this->copyProxy( newEntry.get<0>(), localProxy );
    
  } catch(SourceProxyNotFoundException& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::updateBetterProxy() - Error copying proxy ["
		   << newEntry.get<0>() << "] to ["
		   << localProxy << "]."
		   );
    
    return;
  }

  {
    list<pair<string, string> > params;
    ostringstream tmp1, tmp2;
    tmp1 << newEntry.get<1>();
    if( newEntry.get<2>() > 0 )
      tmp2 << newEntry.get<2>();
    params.push_back( make_pair("proxyfile", localProxy ));
    params.push_back( make_pair("exptime", tmp1.str() ));
    if(newEntry.get<2>() > 0)
      params.push_back( make_pair("counter", tmp2.str() ));
    db::UpdateProxyFieldsByDN updater( mapKey, params, "DNProxyManager::updateBetterProxy" );
    db::Transaction tnx(false, false);
    tnx.execute( &updater );
  }
}

//________________________________________________________________________
void iceUtil::DNProxyManager::setBetterProxy( const string& dn, 
					      const string& proxyfile,
					      const string& myproxyname,
					      const time_t proxy_time_end )
  throw()
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::setBetterProxy");
#endif
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
  string localProxy = this->make_betterproxy_path(dn, myproxyname);//iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/" + compressed_string( this->composite( dn, myproxyname ) ) + ".betterproxy";
  
  try {
    
    this->copyProxy( proxyfile, localProxy );
    
  } catch(SourceProxyNotFoundException& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::setBetterProxy() - Error copying proxy ["
		   << proxyfile<< "] to ["
		   << localProxy << "]."
		   );
    
    return;
    
  }
  
  string mapKey = this->composite( dn, myproxyname );

  CREAM_SAFE_LOG(m_log_dev->debugStream() 
		 << "DNProxyManager::setBetterProxy() - "
		 << "Inserting proxy entry into DB for DN ["
		 << dn 
		 << "] and MyProxy ["
		 << myproxyname
		 << "] Proxy file ["
		 << localProxy << "]"
		 );

  {
    db::CreateProxyField creator( mapKey, localProxy, proxy_time_end, 0, "DNProxyManager::setBetterProxy" );
    db::Transaction tnx(false, false);
    tnx.execute( &creator );    
  }
}

//________________________________________________________________________
boost::tuple<string, time_t, long long int> 
iceUtil::DNProxyManager::getAnyBetterProxyByDN( const string& dn )
const throw() 
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::getAnyBetterProxyByDN");
#endif
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
  map<string, boost::tuple<string, time_t, long long int> > all_proxy_info;
  {
    db::GetAllProxyInfo getter("DNProxyManager::getAnyBetterProxyByDN");
    db::Transaction tnx(false, false);
    tnx.execute( &getter );
    all_proxy_info = getter.get_info();
  }

  map<string, boost::tuple<string, time_t, long long int> >::const_iterator it = all_proxy_info.begin();
  

  while( it != all_proxy_info.end() /*m_DNProxyMap.end()*/ ) {
    
    /**
       does a regex match. The DN must be present in the key (that can also contains the myproxy server name)
    */
    
    if( strstr( it->first.c_str(),dn.c_str() ) == 0 ) {
      /**
	 'dn' is not found in the it->first string
      */
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
	return (it->second);
      }
    ++it;
  }
  
  return boost::make_tuple("", 0, 0);
  
}

//________________________________________________________________________
boost::tuple<string, time_t, long long int> 
iceUtil::DNProxyManager::getExactBetterProxyByDN( const string& dn,
						  const string& myproxyname)
  const throw() 
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::getExactBetterProxyByDN");
#endif
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
  
  boost::tuple< string, time_t, long long> proxy_info;
  bool found;
  {
    db::GetProxyInfoByDN getter( this->composite(dn, myproxyname ), "DNProxyManager::getExactBetterProxyByDN" );
    db::Transaction tnx(false, false);
    tnx.execute( &getter );
    found = getter.found();
    if( found )
      proxy_info = getter.get_info();
  }

  if( !found ) {
    return boost::make_tuple("", 0, 0);
  } else {
    return proxy_info;
  }
  
}

//________________________________________________________________________
void iceUtil::DNProxyManager::incrementUserProxyCounter( const string& userDN, const string& myproxy_name ) 
  throw()
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::incrementUserProxyCounter");
#endif
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
  string mapKey = this->composite( userDN, myproxy_name );
  
  string regID = compressed_string( mapKey );
  
  CREAM_SAFE_LOG(
		   m_log_dev->debugStream()
		   << "DNProxyManager::incrementUserProxyCounter() - "
		   << "Looking for DN ["
		   << userDN << "] MyProxy server ["
		   << myproxy_name << "] in the DB..."
		   );
  
  bool found = false;
  boost::tuple<string, time_t, long long int> proxy_info;
  {
    db::GetProxyInfoByDN getter( mapKey, "DNProxyManager::incrementUserProxyCounter" );
    db::Transaction tnx(false, false);
    tnx.execute( &getter );
    found = getter.found();
    if(found)
      proxy_info = getter.get_info();
  }
  if( found ) {

    CREAM_SAFE_LOG(
		   m_log_dev->debugStream()
		   << "DNProxyManager::incrementUserProxyCounter() - "
		   << "Incrementing proxy counter for DN ["
		   << userDN << "] MyProxy server ["
		   << myproxy_name <<"] from [" << proxy_info.get<2>()
		   << "] to [" << (proxy_info.get<2>() +1 ) << "]"
		   );

    {
      list<pair<string, string> > params;
      ostringstream tmp;
      tmp << (proxy_info.get<2>() + 1);
      params.push_back( make_pair("counter", tmp.str()) );
      db::UpdateProxyFieldsByDN updater( mapKey, params, "DNProxyManager::incrementUserProxyCounter" );
      db::Transaction tnx(false, false);
      tnx.execute( &updater );
    }
  }
}

//________________________________________________________________________
void iceUtil::DNProxyManager::decrementUserProxyCounter( const string& userDN, const string& myproxy_name ) 
  throw()
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::decrementUserProxyCounter");
#endif
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
  string mapKey = this->composite( userDN, myproxy_name );
  
  string regID = compressed_string( mapKey );
  
  CREAM_SAFE_LOG(
		   m_log_dev->debugStream()
		   << "DNProxyManager::decrementUserProxyCounter() - "
		   << "Looking for DN ["
		   << userDN << "] MyProxy server ["
		   << myproxy_name << "] in the DB..."
		   );
  
  bool found = false;
  boost::tuple<string, time_t, long long int> proxy_info;
  {
    db::GetProxyInfoByDN getter( mapKey, "DNProxyManager::decrementUserProxyCounter" );
    db::Transaction tnx(false, false);
    tnx.execute( &getter );
    found = getter.found();
    if(found)
      proxy_info = getter.get_info();
  }
  
  if( found ) {

    CREAM_SAFE_LOG(
		   m_log_dev->debugStream()
		   << "DNProxyManager::decrementUserProxyCounter() - "
		   << "Decrementing proxy counter for DN ["
		   << userDN << "] MyProxy server ["
		   << myproxy_name <<"] from [" << proxy_info.get<2>()
		   << "] to [" << (proxy_info.get<2>() - 1) << "]"
		   );

    {
      list<pair<string, string> > params;
      ostringstream tmp;
      tmp << (proxy_info.get<2>() - 1);
      params.push_back( make_pair("counter", tmp.str()) );
      db::UpdateProxyFieldsByDN updater( mapKey, params, "DNProxyManager::decrementUserProxyCounter" );
      db::Transaction tnx(false, false);
      tnx.execute( &updater );
    }
    

    if( proxy_info.get<2>() == 1 ) {

      CREAM_SAFE_LOG(
		   m_log_dev->debugStream()
		   << "DNProxyManager::decrementUserProxyCounter() - "
		   << "Proxy Counter is ZERO for DN ["
		   << userDN << "] MyProxy server ["
		   << myproxy_name<<"]. Removing better proxy ["
		   //		   << m_DNProxyMap[ mapKey ].get<0>()
		   << proxy_info.get<0>()
		   << "] from persist_dir..."
		   );
      /**
	 If the counter is 0 deregister the current proxy and clear the map and delete the file.
      */

      this->removeBetterProxy( userDN, myproxy_name );
    
    }

  }
}

//________________________________________________________________________
string 
iceUtil::DNProxyManager::make_betterproxy_path( const string& dn, 
						const string& myproxy )
  throw()
{
#ifdef ICE_PROFILE
  ice_timer timer("DNProxyManager::make_betterproxy_path");
#endif
  return iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/" + compressed_string( this->composite( dn, myproxy ) ) + ".betterproxy";
}
