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
 * ICE subscription proxy
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "subscriptionProxy.h"
#include "glite/ce/monitor-client-api-c/CESubscription.h"
#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "iceConfManager.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"

#include "iceUtils.h"
#include "ice-core.h"
#include <cstring> // for memset
#include <netdb.h>
#include <sstream>
#include <ctime>

#include <boost/algorithm/string.hpp>

//extern int h_errno;

using namespace std;
namespace iceUtil = glite::wms::ice::util;
namespace cemon_api = glite::ce::monitor_client_api::soap_proxy;

iceUtil::subscriptionProxy* iceUtil::subscriptionProxy::s_instance = 0;
boost::recursive_mutex iceUtil::subscriptionProxy::mutex;

//______________________________________________________________________________
iceUtil::subscriptionProxy::subscriptionProxy() throw()
  : m_conf( iceConfManager::getInstance() ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    m_valid(true),
    m_myurl()
{
    CREAM_SAFE_LOG(m_log_dev->debugStream() 
                   << "subscriptionProxy::CTOR - Authenticating..."
                   );

    

    
    try {
      m_myurl = iceUtil::getURL();
    } catch(runtime_error& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() 
                   << "subscriptionProxy::CTOR - "
		   << ex.what()
                   );
      abort();
    } catch(exception& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() 
                   << "subscriptionProxy::CTOR - "
		   << ex.what()
                   );
      abort();
    }
    m_D = new cemon_api::DialectW("CLASSAD");

    m_theIce = glite::wms::ice::Ice::instance();
}

//______________________________________________________________________________
iceUtil::subscriptionProxy::~subscriptionProxy() throw()
{
}

//______________________________________________________________________________
iceUtil::subscriptionProxy* iceUtil::subscriptionProxy::getInstance() throw()
{
  if(!s_instance)
    s_instance = new subscriptionProxy();

  return s_instance;
}

//______________________________________________________________________________
void iceUtil::subscriptionProxy::list(const string& userProxy, 
				      const string& url,
				      vector<cemon_api::Subscription>& vec)
  throw (exception&)
{
  

  CREAM_SAFE_LOG(m_log_dev->infoStream() 
  		 << "subscriptionProxy::list() - retrieving list of "
		 << "subscriptions from [" << url << "] for proxy ["
		 << userProxy << "]"
		 );
  
  cemon_api::CESubscriptionMgr ceSMgr;
  
  try {
    ceSMgr.authenticate(userProxy.c_str(), "/");
    ceSMgr.list(url, vec); // can throw an std::exception
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "subscriptionProxy::list() - retrieving list of "
		   << "subscriptions from [" << url << "] for proxy ["
		   << userProxy << "]: "  << ex.what() 
		   );

    throw(ex);

  } catch(...) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "subscriptionProxy::list() - retrieving list of "
		   << "subscriptions from [" << url << "] for proxy ["
		   << userProxy << "]: Unknown exception catched"
		   );

    return;
  }
  
  for(vector<cemon_api::Subscription>::const_iterator it = vec.begin();
      it != vec.end();
      it++) 
    {
      m_tp = it->getExpirationTime();
      localtime_r( &m_tp, &m_Time );
      memset( (void*) m_aT, 0, 256 );
      strftime(m_aT, 256, "%a %d %b %Y %T", &m_Time);
      CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionProxy::list() - "
		     << "*** Found subscription: Id=["
		     << it->getSubscriptionID()
		     << "] ConsumerURL=[" << it->getConsumerURL() << "]"
		     << " Topic=[" << it->getTopicName()<<"]"
		     << " ExpirationTime=[" << m_aT << "] Rate=["
                     << it->getRate() << "]"
		     );
    }
}

//______________________________________________________________________________
bool iceUtil::subscriptionProxy::subscribe(const string& proxy,
					   const string& endpoint,
					   iceSubscription& sub) throw()
{
  cemon_api::CESubscription ceS;
  ceS.setServiceURL(endpoint);

  cemon_api::Topic T( iceConfManager::getInstance()->getConfiguration()->ice()->ice_topic() );
  T.addDialect( m_D ); // this doesn't copy but put the argument into an array
  cemon_api::Policy P( iceConfManager::getInstance()->getConfiguration()->ice()->notification_frequency() );
  cemon_api::ActionW A1("SendNotification", "", true);
  cemon_api::ActionW A2("DoNotSendNotification", "", false);
  P.addAction( &A1 );
  P.addAction( &A2 );
  
  cemon_api::QueryW Q;
  
  string iceid = m_theIce->getHostDN();//glite::wms::ice::Ice::instance()->getHostDN();

/*  try {
    iceid=  glite::ce::cream_client_api::certUtil::getCertSubj( m_conf->getConfiguration()->ice()->ice_host_cert() );
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "subscriptionProxy::subscribe() - Cannot determine the certificate's Subject to put in the iceid of the subscription: "
		   << ex.what() << ". Won't subscribe"
		   );
    return false;
  } catch(...) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "subscriptionProxy::subscribe() - Cannot determine the certificate's Subject to put in the iceid of the subscription: Unknown exception catched. Won't subscribe"
		   );
    return false;
  }
  */

  boost::trim_if(iceid, boost::is_any_of("/"));
  boost::replace_all( iceid, "/", "_" );
  boost::replace_all( iceid, "=", "_" );

  

  string expr = "ICE_ID == \"";
  expr.append(iceid);
  expr.append("\"");
  Q.setExpression( expr );
  Q.setQueryLanguage("ClassAd");

  P.setQuery( &Q );
    
  CREAM_SAFE_LOG(m_log_dev->infoStream() 
  		 << "subscriptionProxy::subscribe() - Subscribing to ["
		 << endpoint << "] ["
                 << m_myurl << "] notification freq ["
		 << m_conf->getConfiguration()->ice()->notification_frequency() << "] with proxy user ["
		 << proxy << "] with topic ["
		 << iceConfManager::getInstance()->getConfiguration()->ice()->ice_topic()
		 << "]"
		 );

  {
    ceS.setSubscribeParam( m_myurl.c_str(),
	                     T,
			     P,
			     m_conf->getConfiguration()->ice()->subscription_duration()
			    ); // ceS internally makes a copy of T and P using their copy-ctor; the ceS' dtor free its local copies
  }
  try {
    
    ceS.authenticate(proxy.c_str(), "/");
    ceS.subscribe();
    
    CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionProxy::subscribe() - Subscribed with ID ["
		   << ceS.getSubscriptionID() << "]"
		   );
	

    sub.setSubscriptionID( ceS.getSubscriptionID() );
    sub.setExpirationTime( time(NULL) + m_conf->getConfiguration()->ice()->subscription_duration() );

    return true;
  } catch(exception& ex) {
    CREAM_SAFE_LOG( m_log_dev->errorStream() << "subscriptionProxy::subscribe() - Subscription Error: "
		    << ex.what() );
    return false;
  }
}

//______________________________________________________________________________
bool iceUtil::subscriptionProxy::updateSubscription( const string& proxy,
						     const string& endpoint,
 					             const string& ID,
					             string& newID
						    ) throw()
{
  cemon_api::Topic T( iceConfManager::getInstance()->getConfiguration()->ice()->ice_topic() );
  cemon_api::ActionW A1("SendNotification", "", true);
  cemon_api::ActionW A2("DoNotSendNotification", "", false);
  cemon_api::QueryW Q;

  cemon_api::Policy P( iceConfManager::getInstance()->getConfiguration()->ice()->notification_frequency() );
  
  T.addDialect( m_D );
  string iceid;
  try {
    iceid=  glite::ce::cream_client_api::certUtil::getCertSubj( m_conf->getConfiguration()->ice()->ice_host_cert() );
  }  catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "subscriptionProxy::updateSubscription() - Cannot determine the certificate's Subject to put in the iceid of the subscription: "
		   << ex.what() << ". Won't update subscription"
		   );
    return false;
  } catch(...) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "subscriptionProxy::updateSubscription() - Cannot determine the certificate's Subject to put in the iceid of the subscription: Unknown exception catched. Won't update subscription"
		   );
    return false;
  }

  string expr = "ICE_ID == \"";

  boost::trim_if(iceid, boost::is_any_of("/"));
  boost::replace_all( iceid, "/", "_" );
  boost::replace_all( iceid, "=", "_" );

  expr.append(iceid);
  expr.append("\"");
  Q.setExpression(expr /* "<![CDATA[ ICE_ID == \"RUNNING\"]]>" */);
  Q.setQueryLanguage("ClassAd");
  P.addAction( &A1 );
  P.addAction( &A2 );
  P.setQuery( &Q );
  
  try {
    
    cemon_api::CESubscriptionMgr ceSMgr;
    ceSMgr.authenticate(proxy.c_str(), "/");
    newID = ceSMgr.update(endpoint, ID, m_myurl, T, P, time(NULL)+m_conf->getConfiguration()->ice()->subscription_duration());
    
  } catch(exception& ex) {
    CREAM_SAFE_LOG( m_log_dev->errorStream() << "subscriptionProxy::updateSubscription()"
		    << " - SubscriptionUpdate Error: "
		    << ex.what() );
    return false;
  } 

  return true;
}

//______________________________________________________________________________
bool iceUtil::subscriptionProxy::subscribedTo( const string& proxy, 
					       const string& url,
					       iceSubscription& newSub
					       )
  throw(exception&)
{
  vector<cemon_api::Subscription> vec;
  try {

    this->list(proxy, url, vec);

  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "subscriptionProxy::subscribedTo() - "
		   << "Error retrieving subscription list: "
		   << ex.what() );
    throw(ex);
  }
  
  for(vector<cemon_api::Subscription>::const_iterator it = vec.begin(); it != vec.end(); ++it) 
    {
      
      if( it->getConsumerURL() == m_myurl ) {
	
	newSub.setSubscriptionID( it->getSubscriptionID() );
	newSub.setExpirationTime( it->getExpirationTime() );

	return true;
      }
      
    }
  
  return false;
}
