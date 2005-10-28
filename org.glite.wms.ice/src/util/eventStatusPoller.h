#ifndef __STATUSPOLLER_H__
#define __STATUSPOLLER_H__

#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "eventStatusPoller_ex.h"
//#include "jobCache.h"
#include "runnable.h"

//class glite::wms::ice::util::jobCache;
class glite::ce::cream_client_api::soap_proxy::CreamProxy;
class glite::ce::cream_client_api::soap_proxy::JobInfoList;

#define DELAY 10

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class eventStatusPoller : public runnable {
	  //	  glite::wms::ice::util::jobCache* jobs;
/* 	  std::string grid_JOBID, cream_JOBID; */
/* 	  glite::ce::cream_client_api::job_statuses::job_status status; */
	  bool endpolling;
	  int delay;
	  std::string cream_service;
	  glite::ce::cream_client_api::soap_proxy::CreamProxy* creamClient;
	  //	  glite::ce::cream_client_api::soap_proxy::JobInfoList* jobinfolist;
	  std::vector<std::string> empty;
	  glite::ce::cream_client_api::soap_proxy::JobInfoList* _jobinfolist;
	  //	  std::vector<SOAP_JOBINFO*> *jobInfoVector;

	public:
	  eventStatusPoller(const std::string& certfile,
			    const std::string& _cream_service,
			    const int& D=DELAY) 
	    throw(glite::wms::ice::util::eventStatusPoller_ex&);

	  virtual ~eventStatusPoller() { if(creamClient) delete(creamClient); }

	  //void setJobCache(glite::wms::ice::util::jobCache* _jobs) { jobs = _jobs; }
	  bool getStatus(void);
	  void updateJobCache(void);
	  virtual void run(void);
	  virtual void stop() { endpolling=true; }
	};

      }
    }
  }
}

#endif
