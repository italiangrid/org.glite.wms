#ifndef __eventlistener_h__
#define __eventlistener_h__

#include "glite/ce/monitor-client-api-c/CEConsumer.h"

#ifndef soapStub_H
#error gSOAP version is changed and the critical label 'soapStub_H' is not define anymore. Stop.
#endif

#undef soapStub_H

#include "glite/ce/monitor-client-api-c/GeneralException.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "runnable.h"


//class glite::wms::ice::util::jobCache;

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class eventStatusListener : public CEConsumer, public runnable {
	  //	  glite::wms::ice::util::jobCache* jobs;
	  std::string grid_JOBID, cream_JOBID;
	  glite::ce::cream_client_api::job_statuses::job_status status;
	  bool endaccept;

	public:
	  eventStatusListener(int i) : CEConsumer(i), 
	    grid_JOBID(""), cream_JOBID(""),
	    status(glite::ce::cream_client_api::job_statuses::UNKNOWN),
	    endaccept(false) { }

	  virtual ~eventStatusListener() {}

	  //	  void setJobCache(glite::wms::ice::util::jobCache* _jobs) { jobs = _jobs; }
	  void acceptJobStatus(void);
	  void updateJobCache(void);
	  virtual void run(void);
	  virtual void stop() { endaccept=true; }
	};

      }
    }
  }
}

#endif
