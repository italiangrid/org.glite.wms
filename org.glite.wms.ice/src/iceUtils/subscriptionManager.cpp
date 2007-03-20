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
#include "glite/ce/cream-client-api-c/certUtil.h"

#include "subscriptionManager.h"
#include "jobCache.h"
#include "iceConfManager.h"
#include "subscriptionProxy.h"
#include "CreamProxyFactory.h"
#include <iostream>
#include <boost/scoped_ptr.hpp>
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"

#define MAX_SUBSCRIPTION_OLDNESS 60

using namespace std;
namespace iceUtil = glite::wms::ice::util;
namespace api_util = glite::ce::cream_client_api::util;
namespace api = glite::ce::cream_client_api::soap_proxy;

iceUtil::subscriptionManager* iceUtil::subscriptionManager::s_instance = NULL;
boost::recursive_mutex iceUtil::subscriptionManager::mutex;
string iceUtil::subscriptionManager::s_persist_dir( DEFAULT_PERSIST_DIR );
bool   iceUtil::subscriptionManager::s_recoverable_db = false;

//______________________________________________________________________________
iceUtil::subscriptionManager* iceUtil::subscriptionManager::getInstance() throw()
{
  if( !s_instance ) 
    s_instance = new subscriptionManager();
  return s_instance;
}

//______________________________________________________________________________
iceUtil::subscriptionManager::subscriptionManager() throw() :
  m_conf(iceUtil::iceConfManager::getInstance()),
  m_log_dev(api_util::creamApiLogger::instance()->getLogger()),
  m_cache( iceUtil::jobCache::getInstance() ),
  m_authz( m_conf->getConfiguration()->ice()->listener_enable_authz() ),
  m_authn( m_conf->getConfiguration()->ice()->listener_enable_authn() )
{ 
  
  m_subProxy = iceUtil::subscriptionProxy::getInstance();
  
  if( !m_subProxy->isValid() )
  {
    CREAM_SAFE_LOG(m_log_dev->fatalStream()
    		   << "subscriptionManager::CTOR() - Couldn't create "
		   << "a subscriptionProxy object. STOP!"
		   << log4cpp::CategoryStream::ENDLINE);
		   
    // this is severe, must exit
    abort();
  }
  
  this->init();
  
  /**
   * Updating the internal maps and sets after an ICE restart can be dangerous
   * because iceCommandSubmit at the moment determines if ICE is subscribed to a CEMon
   * by asking subscriptionManager if it got that cemon url in its internal data structures.
   * This is ugly, but working for know and reasonably safe. But in future should change
   * into something better designed.
   */ 
  
//   CEDbManager* db = new CEDbManager(s_persist_dir, s_recoverable_db);
//   if(!db->isValid()) {
//     CREAM_SAFE_LOG( m_log_dev->fatalStream() << db->getInvalidCause() << log4cpp::CategoryStream::ENDLINE );
//     abort();
//   }
//   m_dbMgr.reset( db );
//   
//   // Now load some cached CEMon URL/DN
//   map<string, pair<string, string> > creamCEMonDN;
//   try {
//     m_dbMgr->getAllRecords( creamCEMonDN );
//   } catch(CEDbExceptio& ex) {
//     abort();
//   }
//   
//   for(map<string, pair<string, string> >::const_iterator it = creamCEMonDN.begin();
//   	it != creamCEMonDN.end();
// 	++it)
//   {
//     m_cemonURL.inset( it->second.first );
//     m_DN.insert( it->second.second );
//     m_mappingCemonDN[ it->second.first ] = it->second.second;
//     m_mappingCreamCemon[ it->first ] = it->second.first;
//   }
}

//______________________________________________________________________________
void iceUtil::subscriptionManager::getCEMonURL(const string& proxy,
					       const string& creamURL, 
					       string& cemonURL) throw()
{
  
  // Try to get the CEMon's address from the memory
  map<string, string>::const_iterator cit = m_mappingCreamCemon.find( creamURL );
  if( cit != m_mappingCreamCemon.end() )
    {
      cemonURL = cit->second;
      return;
    }
  
  
  // try to get the CEMon's address directly from the server
  string _cemonURL;
  try {
    // this method is not called frequently because subscriptionManager
    // keeps in memory information about CEUrl after the first 
    // explicit request to CREAM.
    // So we can create here a CreamProxy object and destroy it when exiting
    // We cannot use a member CreamProxy because CreamProxy contains a 
    // gSOAP runtime env that cannot be shared between threads and subscriptionManager
    // is a singleton and could be shared between threads

    boost::scoped_ptr< api::CreamProxy > creamProxy( iceUtil::CreamProxyFactory::makeCreamProxy( false ) );
    creamProxy->Authenticate( proxy );
    creamProxy->GetCEMonURL( creamURL.c_str(), _cemonURL );
    
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << "subscriptionManager::getCEMonURL() - "
		   << "Couldn't retrieve CEMon URL for CREAM URL ["
		   << creamURL << "]: "
		   << ex.what()<< ". Composing it from configuration file."
		   << log4cpp::CategoryStream::ENDLINE);
    _cemonURL = creamURL;
    boost::replace_first(_cemonURL,
                         m_conf->getConfiguration()->ice()->cream_url_postfix(),
                         m_conf->getConfiguration()->ice()->cemon_url_postfix()
			 );
    
  }
  
  CREAM_SAFE_LOG(m_log_dev->infoStream()
		 << "subscriptionManager::getCEMonURL() - For CREAM URL ["
		 << creamURL << "] got CEMon URL ["
		 << _cemonURL <<"]"
		 << log4cpp::CategoryStream::ENDLINE);
  
  m_mappingCreamCemon[creamURL] = _cemonURL;
  m_cemonURL.insert( _cemonURL );
  
  cemonURL = _cemonURL;
}

//______________________________________________________________________________
bool iceUtil::subscriptionManager::getCEMonDN(
					      const string& proxy,
					      const string& cemonURL,
					      string& DN 
					      ) throw()
{
  // try to get DN from the memory
  map<string, string>::const_iterator it = m_mappingCemonDN.find(cemonURL);
  if( it != m_mappingCemonDN.end() ) {
    DN = it->second;
    return true;
  }
  
  // try to get DN directly from the server
  CEInfo ceInfo(m_conf->getConfiguration()->common()->host_proxy_file(), "/");
  ceInfo.setServiceURL( cemonURL );
  try {
    ceInfo.authenticate( proxy.c_str(), "/");
    ceInfo.getInfo();
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "subscriptionManager::getCEMonDN() - "
		   << "Couldn't get DN for CEMon ["
		   << cemonURL << "]: "
		   << ex.what()
		   << log4cpp::CategoryStream::ENDLINE);
    return false;
  } catch(...) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "subscriptionManager::getCEMonDN() - "
		   << "Unknown exception catched"
		   << log4cpp::CategoryStream::ENDLINE);
    return false;
  }
  
  DN = ceInfo.getDN();
  m_mappingCemonDN[cemonURL] = DN;
  m_DN.insert( DN );
  ceInfo.cleanup();
  return true;
}

//______________________________________________________________________________
void iceUtil::subscriptionManager::getCEMons( vector<string>& target ) throw()
{
  for(set<string>::const_iterator it=m_cemonURL.begin();
      it!=m_cemonURL.end();
      ++it) target.push_back( *it );
}

//______________________________________________________________________________
void iceUtil::subscriptionManager::getCEMons( set<string>& target ) throw()
{
  target = m_cemonURL;
}

//______________________________________________________________________________
void iceUtil::subscriptionManager::init( void ) throw()
{
  /**
   * This method recreates all needed subscriptions basing on
   * the job stored in the jobCache's persistency database.
   */

  map< string, set<string> > UsersCEMons;
  string cemon;

  this->getUserCEMonMapping( UsersCEMons, true );// get all subscriptions only for active jobs, and also populate the mapping DN->BetterProxy
  
  // now loop over all users's proxies and check subscription
  for(map<string, set<string> >::const_iterator mit = UsersCEMons.begin(); mit!=UsersCEMons.end(); ++mit)
  { 
    this->checkSubscription( mit );
  }
} // unlock jobCache

//______________________________________________________________________________
/**
 * Checks subscription for a user proxy: if it finds some ghost recreate it
 */
void iceUtil::subscriptionManager::checkSubscription( map<string, set<string> >::const_iterator it ) 
 throw()
{
  // it->first is the user DN
  // it->second is the set of cemons to check for subscriptions

  for(set<string>::const_iterator sit=it->second.begin(); sit!=it->second.end(); ++sit)
  {
    
    iceSubscription sub("", 0);
    string cemondn;
    bool subscribed;

    string proxy = this->getBetterProxyByDN(it->first);

    try { 

      subscribed = m_subProxy->subscribedTo( proxy, *sit, sub ); // also sets the sub's internal data members

    } catch(exception& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream()
		     << "subscriptionManager::checkSubscription() - Error checking subscription to ["
		     << *sit << "] for DN [" << it->first << "]: "
		     << ex.what() << ". Will not receive notifications from this CEMon for this user."
		     << log4cpp::CategoryStream::ENDLINE);
      continue;

    } 

    if( subscribed ) {
      
      if( m_authz && m_authn) {
	if( !this->getCEMonDN( proxy, *sit, cemondn ) )
	  {
	    CREAM_SAFE_LOG(m_log_dev->warnStream()
			   << "subscriptionManager::checkSubscription() - Subscription to [" << *sit << "] for proxy ["
			   << proxy << "] is there but cannot get CEMon's DN. If it hasn't "
			   <<"previously retrieved the notification will not be authorized."
			   << log4cpp::CategoryStream::ENDLINE);
	  } else {
	 
	}
	  
      }

      
      
    } else {
    
      CREAM_SAFE_LOG(m_log_dev->errorStream()
      		     << "subscriptionManager::checkSubscription() - "
		     << "Subscription to [" << *sit << "] for proxy ["
		     << it->first << "] DISAPPEARED! Recreating it..."
		     << log4cpp::CategoryStream::ENDLINE);
      

      bool subscribeSuccessful = m_subProxy->subscribe( proxy, *sit, sub );// also sets the sub's internal data members


      if( !subscribeSuccessful )
      {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "subscriptionManager::checkSubscription() - "
		   << "Owner of proxy ["<<it->first<<"] couldn't subscribe to CEMon ["
		   << *sit << "]. Skipping..."
		   << log4cpp::CategoryStream::ENDLINE);
	continue;
      }
    }
   
    
    m_Subs[ make_pair( it->first, *sit)].setSubscriptionID( sub.getSubscriptionID() );
    m_Subs[ make_pair( it->first, *sit)].setExpirationTime( sub.getExpirationTime() );
    
  }
}

//______________________________________________________________________________
/**
 * Remove OLD subscriptions
 */
void iceUtil::subscriptionManager::purgeOldSubscription( map<string, set<string> >::const_iterator it ) 
 throw()
{
  // it->first is the user DN
  // it->second is the set of cemons to check for subscriptions

  for(set<string>::const_iterator sit=it->second.begin(); sit!=it->second.end(); ++sit)
    {
      map< pair<string, string> , iceSubscription, ltstring >::const_iterator subit = m_Subs.find( make_pair(it->first, *sit) );
      if( subit == m_Subs.end() ) return;
      
      CREAM_SAFE_LOG(m_log_dev->infoStream()  
		     << "subscriptionManager::purgeOldSubscription() - Subscription for DN ["
		     << it->first << "] to CEMon [" << *sit << "]: expir time=["
		     << subit->second.getExpirationTime() << "] time(null)+OLDNESS=["
		     << (time(NULL)+MAX_SUBSCRIPTION_OLDNESS) << "]"
		     << log4cpp::CategoryStream::ENDLINE);

      if( subit->second.getExpirationTime() > (time(NULL)+MAX_SUBSCRIPTION_OLDNESS)) {
	CREAM_SAFE_LOG(m_log_dev->infoStream()  
		       << "subscriptionManager::purgeOldSubscription() - Subscription for DN ["
		       << it->first << "] to CEMon [" << *sit << "] is older than "
		       << MAX_SUBSCRIPTION_OLDNESS << " seconds. Removing it from ICE's memory."
		       << log4cpp::CategoryStream::ENDLINE);
      }
    }
}

//______________________________________________________________________________
void iceUtil::subscriptionManager::renewSubscription( const std::string& userProxy,
						      const std::string& cemon) 
 throw()
{
  try {
    CREAM_SAFE_LOG(m_log_dev->infoStream()  
		   << "subscriptionManager::renewSubscription() - Update params: "
		   << "Duration=" << m_conf->getConfiguration()->ice()->subscription_duration()
		   << " secs since now - rate="
		   << m_conf->getConfiguration()->ice()->notification_frequency()
		   << " secs."
		   << log4cpp::CategoryStream::ENDLINE);
    
    string newID;
    iceUtil::iceSubscription localsub("", 0);
    
    string dn = glite::ce::cream_client_api::certUtil::getDN( userProxy );
    
    string id = m_Subs[ make_pair( dn, cemon ) ].getSubscriptionID();

    if( id == "" ) {
      CREAM_SAFE_LOG(m_log_dev->infoStream()  
		     << "subscriptionManager::renewSubscription() - "
		     << "SubscriptionID is EMPTY! Cannot renew a subscription without id"
		     << log4cpp::CategoryStream::ENDLINE);
      return;
    }
    
    if(m_subProxy->updateSubscription( userProxy,
				       cemon, 
				       id, 
				       newID )
       ) 
      {
	CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionManager::renewSubscription() - "
		       << "New subscription ID after renewal is ["
		       << newID << "]" << log4cpp::CategoryStream::ENDLINE);
	
	m_Subs[ make_pair( dn, cemon) ].setSubscriptionID( newID );
	m_Subs[ make_pair( dn, cemon) ].setExpirationTime( time(NULL) + m_conf->getConfiguration()->ice()->subscription_duration());
	
	
      } else {
      // subscription renewal failed. Try make a new one
      if(!m_subProxy->subscribe( userProxy, cemon, localsub )) {// also sets the sub's internal data members
	CREAM_SAFE_LOG(m_log_dev->errorStream() 
		       << "subscriptionManager::renewSubscription() - "
		       << "Failed while making new subscription. "
		       << "Wont receive notifications... "
		       << log4cpp::CategoryStream::ENDLINE);
	// let's proceed without notification. The poller will work for us ;)
      } else {
	
	m_Subs[ make_pair( dn, cemon) ].setExpirationTime( localsub.getExpirationTime() );
	m_Subs[ make_pair( dn, cemon) ].setSubscriptionID( localsub.getSubscriptionID() );
	//m_Subs[ make_pair( dn, cemon) ].setUserProxyIfLonger( localsub.getUserProxy() );

	// We made a new subscription because the update
	// failed. Then the CEMon and it's DN are supposed to be
	// already registered in the cemonUrlCache
	
      }
    } 
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "subscriptionManager::renewSubscription() - "
		   << "Failed while renewing subscription: "
		   << ex.what()
		   << log4cpp::CategoryStream::ENDLINE);
  }//  catch(...) {
//     CREAM_SAFE_LOG(m_log_dev->errorStream() 
// 		   << "subscriptionManager::renewSubscription() - "
// 		   << "Unknown exception catched."
// 		   << log4cpp::CategoryStream::ENDLINE);
//   }
}

//________________________________________________________________________
void iceUtil::subscriptionManager::getUserCEMonMapping( map< string, set<string> >& target,
							const bool only_active_jobs) throw()
{

  /**
   *
   * Creates a map < userDN, set_of_cemons > of only jobs that are active,
   * and updates the internal mapping DN->BetterProxy;
   * where a proxy is better than another one if it is more long-lived.
   */

  boost::recursive_mutex::scoped_lock M( jobCache::mutex );
  string cemon;
  set<string> goodCEMons, toCheck;
  map< string, set<string> > tmpTarget;
  string tmpDN;

  //string dn;

  for(iceUtil::jobCache::iterator jit=iceUtil::jobCache::getInstance()->begin();
      jit != iceUtil::jobCache::getInstance()->end();
      ++jit)
  {
    if(only_active_jobs)
      if( !jit->is_active() ) continue;

    
    this->getCEMonURL( jit->getUserProxyCertificate(), jit->getCreamURL(), cemon );


    //tmpTarget[ jit->getUserProxyCertificate() ].insert( cemon );
    tmpTarget[ jit->getUserDN() ].insert( cemon );
    this->setUserProxyIfLonger( jit->getUserDN(), jit->getUserProxyCertificate() );
   

    if( m_authz && m_authn ) {

      if( this->getCEMonDN( jit->getUserProxyCertificate(), cemon, tmpDN ) ) {

	goodCEMons.insert( cemon );

      } else {
	CREAM_SAFE_LOG(m_log_dev->errorStream()
		       << "subscriptionManager::getUserCEMonMapping() - "
		       << "Cannot retrieve DN for CEMon ["
		       << cemon << "]. Will not subscribe to this CEMon!"
		       << log4cpp::CategoryStream::ENDLINE);
      }

    } else {

      goodCEMons.insert( cemon );

    }
  }
  
  if( m_authz && m_authn )
  {
    
    for(map< string, set<string> >::const_iterator mit=tmpTarget.begin(); mit!=tmpTarget.end(); ++mit)
    {
      set<string> cemons = mit->second;
      string usrDN = mit->first;
      for(set<string>::const_iterator sit=cemons.begin(); sit!=cemons.end(); ++sit)
      {
        if( goodCEMons.find( *sit ) != goodCEMons.end() )
	{
	  target[ usrDN ].insert( *sit );
	}
      }
    }
    
  } else { // if not using authz authn all cemons are OK
    target = tmpTarget;
  }
}

//________________________________________________________________________
void iceUtil::subscriptionManager::removeSubscription( const std::string& userProxy, 
						       const std::string& cemon) throw()
{
  string dn = glite::ce::cream_client_api::certUtil::getDN( userProxy );
  
  m_Subs.erase( make_pair( dn, cemon) );
}

//________________________________________________________________________
void iceUtil::subscriptionManager::insertSubscription( const std::string& userProxy,
						       const std::string& cemonURL,
						       const iceSubscription& S ) throw()
{
  string dn = glite::ce::cream_client_api::certUtil::getDN( userProxy );
  m_Subs[ make_pair( dn, cemonURL) ] = S;
  
}

//________________________________________________________________________
// void iceUtil::subscriptionManager::updateSubscriptionProxy( const std::string& userProxy,
// 							    const std::string& cemonURL)
// {
//   string dn = glite::ce::cream_client_api::certUtil::getDN( userProxy );
//   //m_Subs[ make_pair( dn, cemonURL) ].setUserProxyIfLonger( userProxy );
// }

//________________________________________________________________________
bool iceUtil::subscriptionManager::hasSubscription( const std::string& userProxy, 
						    const std::string& cemon ) const throw()
{
  
//   cout << "subscriptionManager::hasSubscription - method called with userProxy="
//        << userProxy << "] - cemon = ["
//        << cemon <<"]" << endl;
  
  //return ( m_Subs.find(s) != m_Subs.end() );  

  string dn = glite::ce::cream_client_api::certUtil::getDN( userProxy );

  std::map< std::pair<std::string, std::string>, iceSubscription>::const_iterator it = m_Subs.find( make_pair( dn, cemon));
  if( it != m_Subs.end() ) {
    
//     cout << "subscriptionManager::hasSubscription - userProxy=[" 
// 	 << it->first.first << "] - CEMonURL = ["
// 	 << it->first.second << "]"
// 	 << endl;
    
    return true;
  } else {
    return false;
  }
  
}

//________________________________________________________________________
void iceUtil::subscriptionManager::setUserProxyIfLonger( const string& prx ) 
{ 

  string dn = glite::ce::cream_client_api::certUtil::getDN( prx );

  this->setUserProxyIfLonger( dn, prx );

}

//________________________________________________________________________
void iceUtil::subscriptionManager::setUserProxyIfLonger( const string& dn, 
							 const string& prx 
							 ) 
{ 

  //string dn = glite::ce::cream_client_api::certUtil::getDN( prx );

  if( m_DNProxyMap.find( dn ) == m_DNProxyMap.end() ) {
    m_DNProxyMap[ dn ] = prx;
    return;
  }

  if (prx == m_DNProxyMap[ dn ] ) return;

  time_t newT, oldT;

  try {
    newT= glite::ce::cream_client_api::certUtil::getProxyTimeLeft(prx);
  } catch(...) {
    //cout << "subscriptionManager::setUserProxyIfLonger - Cannot retrieve time for ["<<prx<<"]"<<endl;
    return;
  }
  
  try {
    oldT = glite::ce::cream_client_api::certUtil::getProxyTimeLeft( m_DNProxyMap[ dn ] );
  } catch(...) {
    // cout<< "subscriptionManager::setUserProxyIfLonger - Setting user proxy to ["
// 	<<  prx
// 	<< "] because cannot retrieve time for ["<< m_DNProxyMap[ dn ] <<"]" <<endl;
    m_DNProxyMap[ dn ] = prx;
    return;
  }

  if(newT > oldT) {
    //cout<< "subscriptionManager::setUserProxyIfLonger - Setting user proxy to ["<<prx<<"]" <<endl;
    m_DNProxyMap[ dn ] = prx;
  } else {
    // cout<< "subscriptionManager::setUserProxyIfLonger - Leaving current proxy ["<< m_DNProxyMap[ dn ] 
// 	<<"] beacuse will expire later" <<endl;
  }
}
