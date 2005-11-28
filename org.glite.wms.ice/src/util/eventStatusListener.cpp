
#include "eventStatusListener.h"
#include "glite/ce/cream-client-api-c/logger.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
#include "jobCache.h"
#include <unistd.h>
#include <string>
#include <iostream>

#include <boost/algorithm/string.hpp>

using namespace std;

using namespace glite::ce::cream_client_api::util;

logger& LOG = logger::instance();

logger::PRINT_DEVICE_CONTROLLER lflags = logger::PRINT_DEVICE_CONTROLLER((int)logger::date | (int)logger::console);

//______________________________________________________________________________
void glite::wms::ice::util::eventStatusListener::operator()() {
  //std::cout << "eventStatusListener::run - called run" << std::endl;
  endaccept=false;

  init();

  while(!endaccept) {
    //this->acceptJobStatus();
    //this->updateJobCache();
    cout << "eventStatusListener::run - called run" << endl;
    sleep(1);
  }
  cout << "eventStatusListener::run - ending..." << endl;
}

//______________________________________________________________________________
void glite::wms::ice::util::eventStatusListener::acceptJobStatus(void)
{
  
  /**
   * Waits for an incoming connection
   */
  if(!this->accept() && !endaccept) {
    if(endaccept) {
      LOG << logger::INFO << lflags << "eventStatusListener is ending"
	  << logger::endlog;
      return;
    } else
      LOG << logger::ERROR << lflags
	  << "CEConsumer::Accept returned false."
	  << logger::endlog;
  }
  
  
  LOG << logger::ERROR << lflags
      << "Connection accepted from ["
      << this->getClientIP() << "]" << logger::endlog; 
  
  /**
   * acquires the event from the client
   * and deserializes the data structures
   */
  if(!this->serve()) {
    LOG << logger::ERROR << lflags << "ErrorCode=["
	<< this->getErrorCode() << "]" << logger::endlog;

    LOG << logger::ERROR << lflags << "ErrorMessage=["
	<< this->getErrorMessage() << "]" << logger::endlog;

  }
  const char *c;
  while((c = this->getNextEventMessage())!=NULL)
    LOG << logger::NOTHING << lflags << "message=["
	<< c << "]" << logger::endlog;
  
  this->reset();
}

//______________________________________________________________________________
void glite::wms::ice::util::eventStatusListener::updateJobCache(void) 
{
//   if(!jobs)
//     {
//       LOG << logger::INFO << lflags << "Cache not initialized. Skipping"
// 	  << logger::endlog;
//       return;
//     }
  LOG << logger::INFO << lflags << "Going to update jobcache with "
      << "[grid_jobid="<<grid_JOBID<<", cream_jobid="
      << cream_JOBID << ", status="<<status<<"]"<<logger::endlog;

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
  jobCache::iterator it;

  map< string, vector<string> > tmpMap;

  for(it  = jobCache::getInstance()->begin();
      it != jobCache::getInstance()->end();
      it++)
    {
      cout << "listener: checking SubscriptionID of ["
	   << it->getSubscriptionID() << "]"<<endl;
      string subid = it->getSubscriptionID();
      //subscriber->pause(subid);
      string url = it->getCreamURL();
      boost::replace_first(url, "ce-cream", "ce-monitor");
      boost::replace_first(url, "CREAM", "CEMonitor");
      cout << "URL="<<url<<endl;
      tmpMap[subid].push_back(url);
      tmpMap[subid].push_back(it->getUserProxyCertificate());
      
    }
  exit(1);
}
