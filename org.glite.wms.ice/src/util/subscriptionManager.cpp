
#include "subscriptionManager.h"
#include "glite/ce/monitor-client-api-c/CESubscription.h"
#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "iceConfManager.h"
#include <cstring> // for memset
#include <netdb.h>
#include <sstream>

extern int h_errno;

using namespace std;
using namespace glite::wms::ice::util;

subscriptionManager* subscriptionManager::instance = NULL;

//______________________________________________________________________________
subscriptionManager::subscriptionManager()
  : log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    conf( iceConfManager::getInstance() ),
    valid(true),
    myname("")
{
  ceS = new CESubscription();
  ceSMgr = new CESubscriptionMgr();
  log_dev->infoStream() << "subscriptionManager::CTOR - Authenticating..."
                        << log4cpp::CategoryStream::ENDLINE;
  try {
    ceS->authenticate(conf->getHostProxyFile().c_str(), "/");
    ceSMgr->authenticate(conf->getHostProxyFile().c_str(), "/");
  } catch(exception& ex) {
    valid = false;
  }
  char name[256];
  memset((void*)name, 0, 256);

  if(gethostname(name, 256) == -1) {
    log_dev->fatalStream() << "eventStatusListener::CTOR - Couldn't resolve local hostname: "
                           << strerror(errno)
                           << log4cpp::CategoryStream::ENDLINE;

    exit(1);
  }
  struct hostent *H=gethostbyname(name);
  if(!H) {
      log_dev->fatalStream() << "eventStatusListener::CTOR - Couldn't resolve local hostname: "
                             << strerror(h_errno)
                             << log4cpp::CategoryStream::ENDLINE;
    exit(1);
  }
  ostringstream os("");
  os << "http://" << H->h_name << ":" << conf->getListenerPort();
  myname = os.str();
  T = new Topic(conf->getICETopic());
  P = new Policy(5000);
}

//______________________________________________________________________________
subscriptionManager* subscriptionManager::getInstance() {
  if(!instance) {
    instance = new subscriptionManager();
    if ( !instance->valid )
      instance = NULL;
  }
  return instance;
}

//______________________________________________________________________________
void subscriptionManager::list(const string& url, vector<Subscription>& vec)
{
  try {
    ceSMgr->list(url, vec);
  } catch(AuthenticationInitException& ex) {
      log_dev->fatalStream() << "subscriptionManager::list() - "
                             << ex.what()
	  		     << log4cpp::CategoryStream::ENDLINE;
      exit(1);
  } catch(exception& ex) {
      log_dev->fatalStream() << "subscriptionManager::list() - "
	                     << ex.what()
	 		     << log4cpp::CategoryStream::ENDLINE;
      exit(1);
  }
}

//______________________________________________________________________________
std::string subscriptionManager::subscribe(const std::string& url,
		            		   const long int& duration)
{
  ceS->setServiceURL(url);
  ceS->setSubscribeParam(myname.c_str(),
	                 *T,
			 *P,
			 duration
			 );
  try {
    ceS->subscribe();
  } catch(exception& ex) {
    log_dev->fatalStream() << "subscriptionManager::subscribe() - Subscription Error: "
	                   << ex.what() << log4cpp::CategoryStream::ENDLINE;
    exit(1);
  }
}

//______________________________________________________________________________