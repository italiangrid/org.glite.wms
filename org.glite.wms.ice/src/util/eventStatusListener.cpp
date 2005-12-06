
#include "eventStatusListener.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include <unistd.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <sstream>

extern int errno;

#include <boost/algorithm/string.hpp>

using namespace std;

using namespace glite::ce::cream_client_api::util;
namespace iceUtil = glite::wms::ice::util;

//______________________________________________________________________________
void glite::wms::ice::util::eventStatusListener::operator()()
{
  //std::cout << "eventStatusListener::run - called run" << std::endl;
  endaccept=false;

  init();

  while(!endaccept) {
    //cout << "eventStatusListener::run - called run" << endl;
    sleep(1);
  }
  cout << "eventStatusListener::()() - ending..." << endl;
}

//______________________________________________________________________________
void glite::wms::ice::util::eventStatusListener::acceptJobStatus(void)
{
  
  /**
   * Waits for an incoming connection
   */
  if(!this->accept() && !endaccept) {
    if(endaccept) {
      cout << "eventStatusListener is ending"
	   << endl;
      return;
    } else
      cout << "CEConsumer::Accept returned false." << endl;
  }
  
  
  cout << "Connection accepted from [" << this->getClientIP() 
       << "]" << endl; 
  
  /**
   * acquires the event from the client
   * and deserializes the data structures
   */
  if(!this->serve()) {
    cout << "ErrorCode=[" << this->getErrorCode() 
	 << "]" << endl;

    cout << "ErrorMessage=["
	 << this->getErrorMessage() << "]" << endl;

  }
  const char *c;
  while((c = this->getNextEventMessage())!=NULL)
    cout << "message=["
	 << c << "]" << endl;
  this->reset();
}

//______________________________________________________________________________
void glite::wms::ice::util::eventStatusListener::updateJobCache(void) 
{
  cout << "Going to update jobcache with "
       << "[grid_jobid=" << grid_JOBID << ", cream_jobid=" 
       << cream_JOBID << ", status=" << status << "]" << endl;

  try {
    //    jobCache::getInstance()->put(grid_JOBID, cream_JOBID, status);
  } catch(std::exception& ex) {
    cerr << ex.what() << endl;
    exit(1);
  }
}

//______________________________________________________________________________
void glite::wms::ice::util::eventStatusListener::init(void)
{
  /**
     This method is executed ONLY at startup of the listener
  */

  jobCache::iterator it;
  map< string, vector<string> > tmpMap;
  string url;
  bool already_subscribed = true;

  ostringstream consumer_url("");

  for(it  = jobCache::getInstance()->begin();
      it != jobCache::getInstance()->end();
      it++)
    {
      string subid = it->getSubscriptionID();
      url = it->getCreamURL();
      //boost::replace_first(url, "ce-cream", "ce-monitor");
      boost::replace_first(url, 
			   iceUtil::iceConfManager::getInstance()->getCreamUrlPostfix(),
			   iceUtil::iceConfManager::getInstance()->getCEMonUrlPostfix());
//       boost::replace_first(url, 
// 			   "CREAM", "CEMonitor");

      cout << "Checking subscription status @"
	   << url << endl;

      already_subscribed = true;
      
      if( subid == "" )
	already_subscribed = false;
      else {
	try {
	  subscriber.authenticate( it->getUserProxyCertificate().c_str(), "/" );
	} catch(AuthenticationInitException& ex) {
	  cerr << "Error authenticating: " << ex.what() << endl;
	  exit(1);
	}
	subscriber.setServiceURL( url );
	try {
	  subscriber.pause( it->getSubscriptionID() );
	} catch(SubscriptionNotFoundException& ex) {
	  already_subscribed = false;
	}
      }

      if(already_subscribed) {
	try{subscriber.resume( it->getSubscriptionID() );}
	catch(exception& ex) {
	  cerr << "Error resuming paused subscription: "
	       << ex.what() << endl;
	  exit(1);
	}
      } else {
	cout << "MUST subscribe @"
	     << url << endl;
	Topic T(iceUtil::iceConfManager::getInstance()->getICETopic());
	Policy P(5*1000);
	char hostname[1024];
	memset((void*)hostname, 0, 1024);
	if(-1==gethostname(hostname, 1024)) {
	  cerr << "Unable to get actual name of localhost: " 
	       << strerror(errno) << endl;
	  exit(1);
	}
	consumer_url.str("");
	consumer_url << "http://"<< hostname <<":"<<tcpport;
	cout << "Subscribing consumer @"<<consumer_url.str()<<endl;
	subscriber.setSubscribeParam(consumer_url.str(), T, P, 86400*30); // subscribes for ~1 month
	try{subscriber.subscribe();}
	catch(exception& ex) {
	  cerr << "Error subscribing: "<<ex.what()<<endl;
	  exit(1);
	}
	it->setSubscriptionID( subscriber.getSubscriptionID() );
	activeSubscriptions.push_back( subscriber.getSubscriptionID() );
      }
    }
}
