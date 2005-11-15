#ifndef __STATUSPOLLER_H__
#define __STATUSPOLLER_H__

#include "glite/ce/cream-client-api-c/JobInfoList.h"
//#include "ice-core.h"
#include "eventStatusPoller_ex.h"
#include "runnable.h"

#define DELAY 10

namespace glite {
  namespace wms {
    namespace ice {

      class absice;

      namespace util {

	class eventStatusPoller : public runnable {

	  bool endpolling;
	  int delay;
	  std::vector<std::string> jobs_to_query;
	  std::vector<std::string> empty;
	  std::vector<glite::ce::cream_client_api::soap_proxy::JobInfoList*> _jobinfolist;
	  std::vector<std::string> url_pieces;
	  absice* iceManager;
	  void purgeJobs(const std::vector<std::string>&);

	public:
	  eventStatusPoller(const std::string& certfile,
			    const int& D=DELAY,
			    absice* _im = NULL) 
	    throw(glite::wms::ice::util::eventStatusPoller_ex&);

	  virtual ~eventStatusPoller() { }

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
