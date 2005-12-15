
#include "eventStatusListener.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include <unistd.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <sstream>
#include <cstring> // for memset
#include <netdb.h>

extern int h_errno;
extern int errno;

#include <boost/algorithm/string.hpp>

using namespace std;

using namespace glite::ce::cream_client_api::util;
namespace iceUtil = glite::wms::ice::util;

//______________________________________________________________________________
glite::wms::ice::util::eventStatusListener::eventStatusListener(int i,const string& hostcert) 
  : CEConsumer(i), 
    grid_JOBID(""), 
    cream_JOBID(""),
    status(glite::ce::cream_client_api::job_statuses::UNKNOWN),
    endaccept(false),
    subscriber(),
    subManager(),
    proxyfile(hostcert),
    tcpport(i), 
    myname("")
{
  char name[256];
  memset((void*)name, 0, 256);
  
  if(gethostname(name, 256) == -1)
    {
      cerr << "Couldn't resolve local hostname: "<<strerror(errno)<<endl;
      exit(1);
    }
  struct hostent *H=gethostbyname(name);
  if(!H) {
    cerr << "Couldn't resolve local hostname: "<<strerror(h_errno)<<endl;
    exit(1);
  }
  myname = H->h_name;
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::operator()()
{
  //std::cout << "eventStatusListener::run - called run" << std::endl;
  endaccept=false;

  init();

  exit(1);

  while(!endaccept) {
    cout << "eventStatusListener::run - called run" << endl;
    sleep(1);
  }
  cout << "eventStatusListener::()() - ending..." << endl;
}

//______________________________________________________________________________
void iceUtil::eventStatusListener::acceptJobStatus(void)
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
      cout << "eventStatusListener::acceptJobStatus() - CEConsumer::Accept() returned false." << endl;
  }
  
  
  cout << "eventStatusListener::acceptJobStatus() - Connection accepted from [" << this->getClientIP() 
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
void iceUtil::eventStatusListener::updateJobCache(void) 
{
  cout << "eventStatusListener::updateJobCache() - Going to update jobcache with "
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
void iceUtil::eventStatusListener::init(void)
{
  /**
   * This method is executed ONLY at startup of the listener
   * it collects all jobids, extracts the CEUrl they belong to,
   * checks for each CEUrl if the current ICE is subscribed to it.
   */
  map< string , int > tmpMap;
  string ceurl;
  for(jobCache::iterator it=jobCache::getInstance()->begin();
      it != jobCache::getInstance()->end();
      it++) 
    {
      ceurl = it->getCreamURL();
      boost::replace_first(ceurl,
                           iceUtil::iceConfManager::getInstance()->getCreamUrlPostfix(),
                           iceUtil::iceConfManager::getInstance()->getCEMonUrlPostfix());
      tmpMap[ceurl] = 1;
    }

  /**
   * Now we've got a collection of CEMon urls (without duplicates, 
   * thanks to the map's property) we've to check for subscription
   */
  vector<Subscription> vec;
  vec.reserve(100);
  //  struct hostent* H;
  //  char hname[256];
  
  for(map<string, int>::iterator it = tmpMap.begin();
      it!=tmpMap.end();
      it++)
    {
      try {
	subManager.authenticate(proxyfile.c_str(), "/");
      } catch(AuthenticationInitException& ex) {
	cerr << ex.what()<<endl;
	exit(1);
      }
      vec.clear();
      try {
	subManager.list(it->first, vec);
      } catch(exception& ex) {
	cerr << ex.what() << endl;
	exit(1);
      }
      if(!vec.size())
	{
	  /**
	   * there're no subs in that CEMon; so for sure we've to
	   * subscribe to it
	   */
	  subscriber.setServiceURL(it->first);
	  Topic T = Topic("ICE");
	  DialectW *D = new DialectW("");
	  T.addDialect(D);
	  Policy P(5000); // rate in milliseconds
	  
	  subscriber.setSubscribeParam(myname.c_str(), T, P, iceUtil::iceConfManager::getInstance()->getSubscriptionDuration()); 
	  try {
	    subscriber.subscribe();

	  } catch(exception& ex) {
	    cerr << ex.what() << endl;
	    exit(1);
	  }
	}
      vector<string> _tmp; _tmp.reserve(2);
      char name[256];
      bool subscribed = false;
      string consumerUrl;
      for(vector<Subscription>::iterator sit=vec.begin();
	  (sit != vec.end()) && (!subscribed);
	  sit++) 
	{
	  _tmp.clear();
	  consumerUrl = (*sit).getConsumerURL();
	  boost::replace_first(consumerUrl, "https://", "");
	  boost::replace_first(consumerUrl, "http://", "");
	  boost::split(_tmp, consumerUrl, boost::is_any_of(":"));
	  memset((void*)name, 0, 256);
	  if(gethostname(name, 256) == -1)
	    {
	      cerr << "Couldn't resolve CEMon hostname: "<<strerror(errno)<<endl;
	      exit(1);
	    }
	  struct hostent *H = gethostbyname(name);
	  if(!H) {
	    cerr << "Couldn't resolve CEMon hostname: "<<strerror(h_errno)<<endl;
	    exit(1);
	  }
	  consumerUrl = H->h_name;
	  if( (consumerUrl==myname) && ((*sit).getTopicName()==iceUtil::iceConfManager::getInstance()->getICETopic()) ) 
	    {
	      /**
	       * this ICE is subscribed to CEMon
	       */
	      subscribed = true;
	    }
	  
	}
      if(!subscribed) {
	cout << "Must subscribe to ["<<it->first<<"]"<<endl;
	subscriber.setServiceURL(it->first);
	Topic T = Topic(iceUtil::iceConfManager::getInstance()->getICETopic());
	DialectW *D = new DialectW("");
	T.addDialect(D);
	Policy P(5000); // rate in milliseconds
	
	subscriber.setSubscribeParam(myname.c_str(), T, P, iceUtil::iceConfManager::getInstance()->getSubscriptionDuration()); 
	try {
	  subscriber.subscribe();
	} catch(exception& ex) {
	  cerr << ex.what() << endl;

	  exit(1);
	}
      }
    }
}
