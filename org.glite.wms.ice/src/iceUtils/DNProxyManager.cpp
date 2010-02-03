/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */
#include "DNProxyManager.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/security/proxyrenewal/renewal.h"

#include "iceDb/GetProxyInfoByDN.h"
#include "iceDb/GetProxyInfoByDN_MYProxy.h"
#include "iceDb/RemoveProxyByDN.h"
#include "iceDb/UpdateProxyFieldsByDN.h"
#include "iceDb/UpdateProxyCounterByDN.h"
#include "iceDb/Transaction.h"
#include "iceDb/CreateProxyField.h"
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

namespace iceUtil = glite::wms::ice::util;
namespace cream_api = glite::ce::cream_client_api;
using namespace std;

iceUtil::DNProxyManager* iceUtil::DNProxyManager::s_instance = NULL;
boost::recursive_mutex iceUtil::DNProxyManager::s_mutex;

//______________________________________________________________________________
iceUtil::DNProxyManager* iceUtil::DNProxyManager::getInstance() throw()
{
  boost::recursive_mutex::scoped_lock M( s_mutex );

  if( !s_instance ) 
    s_instance = new DNProxyManager();
  return s_instance;
}

//______________________________________________________________________________
iceUtil::DNProxyManager::DNProxyManager( void ) throw()
{
  m_log_dev = cream_api::util::creamApiLogger::instance()->getLogger();
  
}

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
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
  cream_api::soap_proxy::VOMSWrapper V( prx,  !::getenv("GLITE_WMS_ICE_DISABLE_ACVER") );
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
      db::CreateProxyField creator( dn, "", localProxy, exptime, 0, "DNProxyManager::setUserProxyIfLonger_Legacy" );
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
      db::CreateProxyField creator( dn, "", localProxy, exptime, 0,"DNProxyManager::setUserProxyIfLonger_Legacy" );
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
      db::CreateProxyField creator( dn, "",localProxy, exptime, 0, "DNProxyManager::setUserProxyIfLonger_Legacy" );
      db::Transaction tnx(false, false);
      tnx.execute( &creator );
    }
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
  boost::recursive_mutex::scoped_lock M( s_mutex );

  CREAM_SAFE_LOG(m_log_dev->debugStream() 
		 << "DNProxyManager::removeBetterProxy() - "
		 << "Removing from DB better proxy for userdn ["
		 << userdn << "] MyProxy server ["
		 << myproxyname << "]"
		 );

  {
    //db::RemoveProxyByDN remover( this->composite(userdn, myproxyname), "DNProxyManager::removeBetterProxy" );
    db::RemoveProxyByDN remover( userdn, myproxyname, "DNProxyManager::removeBetterProxy" );
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
  boost::recursive_mutex::scoped_lock M( s_mutex );
  //string mapKey = this->composite( userDN, myproxyname );
  
  string localProxy = this->make_betterproxy_path(userDN, myproxyname );//iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/" + compressed_string( this->composite( userDN, myproxyname ) ) + ".betterproxy";
  


  try {
    
    this->copyProxy( newEntry.get<0>(), localProxy + ".tmp" );
    
  } catch(SourceProxyNotFoundException& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "DNProxyManager::updateBetterProxy() - Error copying proxy ["
		   << newEntry.get<0>() << "] to ["
		   << localProxy << ".tmp]."
		   );
    
    return;
  }

  int rc = ::rename( (localProxy + ".tmp").c_str(), localProxy.c_str() );
  if( rc < 0 ) {
    string errmex = strerror(errno);
    CREAM_SAFE_LOG(m_log_dev->fatalStream() 
		   << "DNProxyManager::updateBetterProxy() - Error renaming ["
		   << localProxy+".tmp" << "] to ["
		   << localProxy << "]: " << errmex << ". Skipping Better Proxy update"
		   );
    return;
  }

  {
    db::UpdateProxyFieldsByDN updater( userDN, myproxyname, localProxy, newEntry.get<1>(), "DNProxyManager::updateBetterProxy" );
    db::Transaction tnx(false, false);
    tnx.execute( &updater );
  }
}

//________________________________________________________________________
void iceUtil::DNProxyManager::setBetterProxy( const string& dn, 
					      const string& proxyfile,
					      const string& myproxyname,
					      const time_t proxy_time_end,
					      const unsigned long long init_counter)
  throw()
{
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
  
  //string mapKey = this->composite( dn, myproxyname );

  CREAM_SAFE_LOG(m_log_dev->debugStream() 
		 << "DNProxyManager::setBetterProxy() - "
		 << "Inserting proxy entry into DB for DN ["
		 << dn 
		 << "] and MyProxy ["
		 << myproxyname
		 << "] Proxy file ["
		 << localProxy << "] counter=["
		 << init_counter << "]"
		 );

  {
    db::CreateProxyField creator( dn, myproxyname, localProxy, proxy_time_end, init_counter, "DNProxyManager::setBetterProxy" );
    db::Transaction tnx(false, false);
    tnx.execute( &creator );    
  }
}

//________________________________________________________________________
boost::tuple<string, time_t, long long int> 
iceUtil::DNProxyManager::getAnyBetterProxyByDN( const string& dn )
const throw() 
{
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
	// FIXME: controllare la validita' prima di tornare il proxy

  boost::tuple<std::string, time_t, long long int> result;
  {
    db::GetProxyInfoByDN getter( dn, "DNProxyManager::getAnyBetterProxyByDN");
    db::Transaction tnx(false, false);
    tnx.execute( &getter );
    if( !getter.found( ) )
      return boost::make_tuple("", 0, 0);
    result = getter.get_info( );
  }
  
  return result;

}

//________________________________________________________________________
boost::tuple<string, time_t, long long int> 
iceUtil::DNProxyManager::getExactBetterProxyByDN( const string& dn,
						  const string& myproxyname)
  const throw() 
{
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
  
  boost::tuple< string, time_t, long long> proxy_info;
  {
    db::GetProxyInfoByDN_MYProxy getter( dn, myproxyname, "DNProxyManager::getExactBetterProxyByDN" );
    db::Transaction tnx(false, false);
    tnx.execute( &getter );
    //    found = getter.found();
    if( !getter.found() )
      return boost::make_tuple("", 0, 0);
    
    proxy_info = getter.get_info();
  }

  return proxy_info;
  
}

//________________________________________________________________________
void 
iceUtil::DNProxyManager::incrementUserProxyCounter( const CreamJob& aJob,
                                                    const time_t proxy_time_end)
  throw()
{
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
  CREAM_SAFE_LOG(
		   m_log_dev->debugStream()
		   << "DNProxyManager::incrementUserProxyCounter() - "
		   << "Looking for DN ["
		   << aJob.get_user_dn() << "] MyProxy server ["
		   << aJob.get_myproxy_address() << "] in the DB..."
		   );
  
  bool found = false;
  boost::tuple<string, time_t, long long int> proxy_info;
  {
    db::GetProxyInfoByDN_MYProxy getter( aJob.get_user_dn(), aJob.get_myproxy_address(), 
					 "DNProxyManager::incrementUserProxyCounter" );
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
		   << aJob.get_user_dn() << "] MyProxy server ["
		   << aJob.get_myproxy_address() <<"] from [" << proxy_info.get<2>()
		   << "] to [" << (proxy_info.get<2>() +1 ) << "]"
		   );

    {
      //list<pair<string, string> > params;
      //ostringstream tmp;
      //tmp << (proxy_info.get<2>() + 1);
      //params.push_back( make_pair("counter", tmp.str()) );
      db::UpdateProxyCounterByDN updater( aJob.get_user_dn(), aJob.get_myproxy_address(), proxy_info.get<2>() + 1, "DNProxyManager::incrementUserProxyCounter" );
      db::Transaction tnx(false, false);
      tnx.execute( &updater );
    }
  } else {
    this->setBetterProxy( aJob.get_user_dn(), 
                          aJob.get_user_proxy_certificate(),
                          aJob.get_myproxy_address(),
                          proxy_time_end,
                          (unsigned long long)1);
  }
}

//________________________________________________________________________
void 
iceUtil::DNProxyManager::decrementUserProxyCounter( const string& userDN, 
						    const string& myproxy_name ) 
  throw()
{
  boost::recursive_mutex::scoped_lock M( s_mutex );
  
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
    db::GetProxyInfoByDN_MYProxy getter( userDN, myproxy_name, "DNProxyManager::decrementUserProxyCounter" );
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
      //list<pair<string, string> > params;
      //ostringstream tmp;
      //tmp << (proxy_info.get<2>() - 1);
      //params.push_back( make_pair("counter", tmp.str()) );
      db::UpdateProxyCounterByDN updater( userDN, myproxy_name, proxy_info.get<2>() - 1, "DNProxyManager::decrementUserProxyCounter" );
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
  return iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() + "/" + compressed_string( this->composite( dn, myproxy ) ) + ".betterproxy";
}
