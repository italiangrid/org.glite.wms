
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

subscriptionManager* subscriptionManager::instance = NULL;
boost::recursive_mutex subscriptionManager::mutex;

//______________________________________________________________________________
subscriptionManager::subscriptionManager()
  : ceS(),
    ceSMgr(),
    T( iceConfManager::getInstance()->getICETopic() ),
    P( iceConfManager::getInstance()->getNotificationFrequency() ),
    conf( iceConfManager::getInstance() ),
    log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    valid(true),
    myname(""),
    lastSubscriptionID(""),
    vec()
{
  log_dev->infoStream() << "subscriptionManager::CTOR - Authenticating..."
                        << log4cpp::CategoryStream::ENDLINE;
  try {
    boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
    ceS.authenticate(conf->getHostProxyFile().c_str(), "/");
    ceSMgr.authenticate(conf->getHostProxyFile().c_str(), "/");
  } catch(exception& ex) {
    log_dev->fatalStream() << "subscriptionManager::CTOR - Fatal ERROR authenticating: "
                           << ex.what()
                           << log4cpp::CategoryStream::ENDLINE;
    valid = false;
    return;
  }
  char name[256];
  memset((void*)name, 0, 256);

  if(gethostname(name, 256) == -1) {
    log_dev->fatalStream() << "subscriptionManager::CTOR - Couldn't resolve local hostname: "
                           << strerror(errno)
                           << log4cpp::CategoryStream::ENDLINE;
    valid = false;
    return;
  }
  struct hostent *H=gethostbyname(name);
  if(!H) {
      log_dev->fatalStream() << "subscriptionManager::CTOR - Couldn't resolve local hostname: "
                             << strerror(h_errno)
                             << log4cpp::CategoryStream::ENDLINE;
    valid = false;
    return;
  }
  ostringstream os("");
  {
    boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
    os << "http://" << H->h_name << ":" << conf->getListenerPort();
  }
  myname = os.str();

  T.addDialect(NULL);
  vec.reserve(100);
}

//______________________________________________________________________________
subscriptionManager* subscriptionManager::getInstance()
{
  //boost::recursive_mutex::scoped_lock M( mutex );
  if(!instance)
    instance = new subscriptionManager();

  return instance;
}

//______________________________________________________________________________
void subscriptionManager::list(const string& url, vector<Subscription>& vec)
  throw (exception&)
{
  log_dev->infoStream() << "subscriptionManager::list() - retrieving list of "
                        << "subscriptions from [" << url << "]"
                        << log4cpp::CategoryStream::ENDLINE;
  try {
    ceSMgr.list(url, vec);
  } catch(AuthenticationInitException& ex) {
      log_dev->errorStream() << "subscriptionManager::list() - "
                             << ex.what()
	  		     << log4cpp::CategoryStream::ENDLINE;
      throw ex;
  } catch(exception& ex) {
      log_dev->errorStream() << "subscriptionManager::list() - "
	                     << ex.what()
	 		     << log4cpp::CategoryStream::ENDLINE;
      throw ex;
  }
  for(vector<Subscription>::const_iterator it = vec.begin();
      it != vec.end();
      it++) {
            //struct tm T;
	    tp = it->getExpirationTime();
	    localtime_r( &tp, &Time );
	    //char aT[256];
	    memset( (void*) aT, 0, 256 );
	    strftime(aT, 256, "%a %d %b %Y %T", &Time);
            log_dev->infoStream() << "subscriptionManager::list() - "
      				  << "*** Found subscription: ["
                                  << it->getSubscriptionID()
				  << "] [" << it->getConsumerURL() << "]"
				  << " [" << it->getTopicName()<<"]"
				  << " [" << aT << "]"
				  << log4cpp::CategoryStream::ENDLINE;
	}
}

//______________________________________________________________________________
bool subscriptionManager::subscribe(const string& url)
{
  ceS.setServiceURL(url);

  log_dev->infoStream() << "subscriptionManager::subscribe() - Subscribing to ["
                        << url << "]"
                        << log4cpp::CategoryStream::ENDLINE;

  {
    boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
    ceS.setSubscribeParam(myname.c_str(),
	                T,
			P,
			conf->getSubscriptionDuration()
			);
  }
  try {
    ceS.subscribe();
    lastSubscriptionID = ceS.getSubscriptionID();
    log_dev->infoStream() << "subscriptionManager::subscribe() - Subscribed with ID ["
                        << lastSubscriptionID << "]"
                        << log4cpp::CategoryStream::ENDLINE;
    return true;
  } catch(exception& ex) {
    log_dev->errorStream() << "subscriptionManager::subscribe() - Subscription Error: "
	                   << ex.what() << log4cpp::CategoryStream::ENDLINE;
    return false;
  }
}

//______________________________________________________________________________
bool subscriptionManager::updateSubscription(const string& url,
 					     const string& ID,
					     string& newID)
{
  try {
    boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
    newID = ceSMgr.update(url, ID, myname, T, P,
    		                 conf->getSubscriptionDuration());
    return true;
  } catch(exception& ex) {
    log_dev->errorStream() << "subscriptionManager::updateSubscription()"
     			   << " - SubscriptionUpdate Error: "
	                   << ex.what() << log4cpp::CategoryStream::ENDLINE;
    return false;
  }
  //return ""; // unreachable code; just to silent a compilation warning
}

//______________________________________________________________________________
bool subscriptionManager::subscribedTo(const string& url)
{
  vec.clear();
  try {this->list(url, vec);}
  catch(exception& ex) {
    log_dev->errorStream() << "subscriptionManager::subscribedTo() - "
    			   << "Error retrieving subscription list: "
	                   << ex.what() << log4cpp::CategoryStream::ENDLINE;
  }
  for(vector<Subscription>::const_iterator it = vec.begin();
      it != vec.end();
      it++) if( it->getConsumerURL() == myname ) return true;

  return false;
}
