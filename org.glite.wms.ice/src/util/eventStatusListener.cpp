
#include "eventStatusListener.h"
#include "glite/ce/cream-client-api-c/logger.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
#include <unistd.h>
#include <string>
#include <iostream>

using namespace std;

using namespace glite::ce::cream_client_api::util;

logger& LOG = logger::instance();

logger::PRINT_DEVICE_CONTROLLER lflags = logger::PRINT_DEVICE_CONTROLLER((int)logger::date | (int)logger::console);

void glite::wms::ice::util::eventStatusListener::run(void) {
  //std::cout << "eventStatusListener::run - called run" << std::endl;
  endaccept=false;
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
void glite::wms::ice::util::eventStatusListener::updateJobCache(void) {
  if(!jobs)
    {
      LOG << logger::INFO << lflags << "Cache not initialized. Skipping"
	  << logger::endlog;
      return;
    }
  LOG << logger::INFO << lflags << "Going to update jobcache with "
      << "[grid_jobid="<<grid_JOBID<<", cream_jobid="
      << cream_JOBID << ", status="<<status<<"]"<<logger::endlog;

  jobs->put(grid_JOBID, cream_JOBID, status);
}
