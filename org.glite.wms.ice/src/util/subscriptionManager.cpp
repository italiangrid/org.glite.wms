
#include "subscriptionManager.h"
#include "glite/ce/monitor-client-api-c/CESubscription.h"
#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "iceConfManager.h"
#include <cstring> // for memset
#include <netdb.h>
#include <sstream>
#include <ctime>

extern int h_errno;

using namespace std;
using namespace glite::wms::ice::util;

subscriptionManager* subscriptionManager::s_instance = NULL;
boost::recursive_mutex subscriptionManager::mutex;

//______________________________________________________________________________
subscriptionManager::subscriptionManager()
  : m_ceS(),
    m_ceSMgr(),
    m_T( iceConfManager::getInstance()->getICETopic() ),
    m_P( iceConfManager::getInstance()->getNotificationFrequency() ),
    m_conf( iceConfManager::getInstance() ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    m_valid(true),
    m_myname(""),
    m_lastSubscriptionID(""),
    m_vec()
{
  CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionManager::CTOR - Authenticating..."
		 << log4cpp::CategoryStream::ENDLINE);
  try {
    //boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
    m_ceS.authenticate(m_conf->getHostProxyFile().c_str(), "/");
    m_ceSMgr.authenticate(m_conf->getHostProxyFile().c_str(), "/");
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->fatalStream() << "subscriptionManager::CTOR - Fatal ERROR authenticating: "
		   << ex.what()
		   << log4cpp::CategoryStream::ENDLINE);
    m_valid = false;
    return;
  }

  m_T.addDialect(NULL);
  m_vec.reserve(100);
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
  CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionManager::list() - retrieving list of "
		 << "subscriptions from [" << url << "]"
		 << log4cpp::CategoryStream::ENDLINE);
  
  m_ceSMgr.list(url, vec); // can throw an std::exception
  
  for(vector<Subscription>::const_iterator it = m_vec.begin();
      it != m_vec.end();
      it++) 
    {
      tp = it->getExpirationTime();
      localtime_r( &tp, &m_Time );
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
  m_ceS.setServiceURL(url);

  CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionManager::subscribe() - Subscribing to ["
		 << url << "]"
		 << log4cpp::CategoryStream::ENDLINE);

  {
    //boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
    m_ceS.setSubscribeParam( m_myname.c_str(),
	                     m_T,
			     m_P,
			     m_conf->getSubscriptionDuration()
			    );
  }
  try {
    m_ceS.subscribe();
    m_lastSubscriptionID = m_ceS.getSubscriptionID();
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
    newID = m_ceSMgr.update(url, ID, m_myname, m_T, m_P,
    		          time(NULL)+m_conf->getSubscriptionDuration());
    return true;
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << "subscriptionManager::updateSubscription()"
		   << " - SubscriptionUpdate Error: "
		   << ex.what() << log4cpp::CategoryStream::ENDLINE);
    return false;
  }
  //return ""; // unreachable code; just to silent a compilation warning
}

//______________________________________________________________________________
bool subscriptionManager::subscribedTo(const string& url)
{
  m_vec.clear();
  try {this->list(url, m_vec);}
  catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << "subscriptionManager::subscribedTo() - "
		   << "Error retrieving subscription list: "
		   << ex.what() << log4cpp::CategoryStream::ENDLINE);
  }
  for(vector<Subscription>::const_iterator it = m_vec.begin();
      it != m_vec.end();
      it++) if( it->getConsumerURL() == m_myname ) return true;

  return false;
}
