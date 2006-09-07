
#include "subscriptionManager.h"
#include "glite/ce/monitor-client-api-c/CESubscription.h"
#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include <cstring> // for memset
#include <netdb.h>
#include <sstream>
#include <ctime>
#include <boost/format.hpp>

extern int h_errno;

using namespace std;
using namespace glite::wms::ice::util;

subscriptionManager* subscriptionManager::s_instance = NULL;
boost::recursive_mutex subscriptionManager::mutex;

//______________________________________________________________________________
subscriptionManager::subscriptionManager()
  : m_T( iceConfManager::getInstance()->getICETopic() ),
    m_P( iceConfManager::getInstance()->getNotificationFrequency() ),
    m_conf( iceConfManager::getInstance() ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    m_valid(true),
    //m_myname(""),
    m_myurl(""),
    m_lastSubscriptionID("")
{
  CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionManager::CTOR - Authenticating..."
		 << log4cpp::CategoryStream::ENDLINE);
  try {
    //boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
    CESubscription ceS;
    CESubscriptionMgr ceSMgr;
    ceS.authenticate(m_conf->getHostProxyFile().c_str(), "/");
    ceSMgr.authenticate(m_conf->getHostProxyFile().c_str(), "/");
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->fatalStream() << "subscriptionManager::CTOR - Fatal ERROR authenticating: "
		   << ex.what()
		   << log4cpp::CategoryStream::ENDLINE);
    m_valid = false;
    return;
  }

  m_T.addDialect(NULL);
  //m_vec.reserve(100);
  string tmp_myname;
  try {
    tmp_myname = getHostName();
    
  } catch(runtime_error& ex) {
    CREAM_SAFE_LOG(m_log_dev->fatalStream() << "subscriptionManager::CTOR - Fatal ERROR getting local hostname: "
		   << ex.what()<<". Stop!"
		   << log4cpp::CategoryStream::ENDLINE);
    abort();
  }
  string tmp_prefix;
  if( iceConfManager::getInstance()->getListenerEnableAuthN() )
    tmp_prefix = "https";
  else
    tmp_prefix = "http";
    
  m_myurl = boost::str( boost::format("%1%://%2%:%3%") % tmp_prefix % tmp_myname % iceConfManager::getInstance()->getListenerPort() );
    
}

//______________________________________________________________________________
subscriptionManager* subscriptionManager::getInstance()
{
  if(!s_instance)
    s_instance = new subscriptionManager();

  return s_instance;
}

//______________________________________________________________________________
void subscriptionManager::list(const string& url, vector<Subscription>& vec)
  throw (exception&)
{
  CREAM_SAFE_LOG(m_log_dev->infoStream() 
  		 << "subscriptionManager::list() - retrieving list of "
		 << "subscriptions from [" << url << "]"
		 << log4cpp::CategoryStream::ENDLINE);
	
  CESubscriptionMgr ceSMgr;
  CESubscription ceS;
	 
  ceSMgr.authenticate(m_conf->getHostProxyFile().c_str(), "/");
  ceSMgr.list(url, vec); // can throw an std::exception
  
  for(vector<Subscription>::const_iterator it = vec.begin();
      it != vec.end();
      it++) 
    {
      m_tp = it->getExpirationTime();
      localtime_r( &m_tp, &m_Time );
      memset( (void*) m_aT, 0, 256 );
      strftime(m_aT, 256, "%a %d %b %Y %T", &m_Time);
      CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionManager::list() - "
		     << "*** Found subscription: ["
		     << it->getSubscriptionID()
		     << "] [" << it->getConsumerURL() << "]"
		     << " [" << it->getTopicName()<<"]"
		     << " [" << m_aT << "]"
		     << log4cpp::CategoryStream::ENDLINE);
    }
}

//______________________________________________________________________________
bool subscriptionManager::subscribe(const string& url)
{
  CESubscription ceS;
  ceS.setServiceURL(url);

  CREAM_SAFE_LOG(m_log_dev->infoStream() 
  		 << "subscriptionManager::subscribe() - Subscribing to ["
		 << url << "] ["
                 << m_myurl << "]"
		 << log4cpp::CategoryStream::ENDLINE);

  {
    //boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
    ceS.setSubscribeParam( m_myurl.c_str(),
	                     m_T,
			     m_P,
			     m_conf->getSubscriptionDuration()
			    );
  }
  try {
    ceS.authenticate(m_conf->getHostProxyFile().c_str(), "/");
    ceS.subscribe();
    m_lastSubscriptionID = ceS.getSubscriptionID();
    CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionManager::subscribe() - Subscribed with ID ["
		   << m_lastSubscriptionID << "]"
		   << log4cpp::CategoryStream::ENDLINE);
    return true;
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << "subscriptionManager::subscribe() - Subscription Error: "
		   << ex.what() << log4cpp::CategoryStream::ENDLINE);
    return false;
  }
}

//______________________________________________________________________________
bool subscriptionManager::updateSubscription(const string& url,
 					     const string& ID,
					     string& newID)
{
  try {
    //boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
    CESubscriptionMgr ceSMgr;
    ceSMgr.authenticate(m_conf->getHostProxyFile().c_str(), "/");
    newID = ceSMgr.update(url, ID, m_myurl, m_T, m_P,
    		          time(NULL)+m_conf->getSubscriptionDuration());
    //return true;
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << "subscriptionManager::updateSubscription()"
		   << " - SubscriptionUpdate Error: "
		   << ex.what() << log4cpp::CategoryStream::ENDLINE);
    return false;
  }
  return true;
  //return ""; // unreachable code; just to silent a compilation warning
}

//______________________________________________________________________________
bool subscriptionManager::subscribedTo(const string& url,
					vector<Subscription>& vec)
{
  //m_vec.clear();
  try {
    this->list(url, vec);
  }
  catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << "subscriptionManager::subscribedTo() - "
		   << "Error retrieving subscription list: "
		   << ex.what() << log4cpp::CategoryStream::ENDLINE);
  }
  
  
  
  //cout << "\n*** vec.size() = " << vec.size() << endl<<endl;
  
  for(vector<Subscription>::const_iterator it = vec.begin();
      it != vec.end();
      it++) 
  {
    //cout << "\n*** m_myurl = ["<<m_myurl<<"] it->getConsumerURL() = ["<< it->getConsumerURL() << "]" << endl<<endl;
    if( it->getConsumerURL() == m_myurl ) return true;
  }
  
  return false;
}
