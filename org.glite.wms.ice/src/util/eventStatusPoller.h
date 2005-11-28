#ifndef __STATUSPOLLER_H__
#define __STATUSPOLLER_H__

#include "glite/ce/cream-client-api-c/JobInfoList.h"
#include "eventStatusPoller_ex.h"
//#include "runnable.h"

#define DELAY 10

namespace glite {
  namespace ce {
    namespace cream_client_api {
      namespace soap_proxy {

	class CreamProxy;
      }
    }
  }
};

namespace glite {
  namespace wms {
    namespace ice {

      class absice;

      namespace util {

	class eventStatusPoller {// : public runnable {

	  bool endpolling;
	  int delay;
	  std::vector<std::string> jobs_to_query;
	  std::vector<std::string> empty;
	  std::vector<glite::ce::cream_client_api::soap_proxy::JobInfoList*> 
	    _jobinfolist;
	  std::vector<std::string> url_pieces;
	  absice* iceManager;
	  void purgeJobs(const std::vector<std::string>&);

	  glite::ce::cream_client_api::soap_proxy::CreamProxy* creamClient;
	  std::vector<std::string> oneJobToQuery;
	  std::vector<std::string> oneJobToPurge;

	  bool getStatus(void);
	  void updateJobCache(void);
	  void checkJobs(void);

	protected:
	  eventStatusPoller( const eventStatusPoller& ) {}

	public:
	  eventStatusPoller(
			    absice* _im,
			    const int& D=DELAY
			    ) 
	    throw(glite::wms::ice::util::eventStatusPoller_ex&);

	  
	  virtual ~eventStatusPoller();// { if(creamClient) delete(creamClient); }

	  virtual void operator()();
	  virtual void stop() { endpolling=true; }
	};

      }
    }
  }
}

#endif
