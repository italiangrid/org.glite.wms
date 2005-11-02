#ifndef __STATUSPOLLER_H__
#define __STATUSPOLLER_H__

#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "eventStatusPoller_ex.h"
#include "runnable.h"

class glite::ce::cream_client_api::soap_proxy::CreamProxy;
class glite::ce::cream_client_api::soap_proxy::JobInfoList;

#define DELAY 10

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class eventStatusPoller : public runnable {

	  bool endpolling;
	  int delay;
	  std::string cream_service;
	  glite::ce::cream_client_api::soap_proxy::CreamProxy* creamClient;

	  std::vector<std::string> jobs_to_query;

	  std::vector<std::string> empty;
	  glite::ce::cream_client_api::soap_proxy::JobInfoList* _jobinfolist;

	public:
	  eventStatusPoller(const std::string& certfile,
			    const std::string& _cream_service,
			    const int& D=DELAY) 
	    throw(glite::wms::ice::util::eventStatusPoller_ex&);

	  virtual ~eventStatusPoller() { if(creamClient) delete(creamClient); }

	  bool getStatus(void);
	  void updateJobCache(void);
	  void checkJobs(void);
	  virtual void run(void);
	  virtual void stop() { endpolling=true; }
	};

      }
    }
  }
}

#endif
