#ifndef __eventlistener_h__
#define __eventlistener_h__

#include "glite/ce/monitor-client-api-c/CEConsumer.h"

#ifndef soapStub_H
#error gSOAP version is changed and the critical label 'soapStub_H' is not define anymore. Stop.
#endif

#undef soapStub_H

#include "glite/ce/monitor-client-api-c/GeneralException.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class eventStatusListener : public CEConsumer {
	  std::string grid_JOBID, cream_JOBID;
	  glite::ce::cream_client_api::job_statuses::job_status status;
	  bool endaccept;

	protected:
	  eventStatusListener(const eventStatusListener&) : CEConsumer(9999) {}

	public:
	  eventStatusListener(int i, const std::string& cert, const std::string& key) 
	    : CEConsumer(i), 
	    grid_JOBID(""), 
	    cream_JOBID(""),
	    status(glite::ce::cream_client_api::job_statuses::UNKNOWN),
	    endaccept(false) { }
	  
	  virtual ~eventStatusListener() {}

	  void acceptJobStatus(void);
	  void updateJobCache(void);
	  virtual void operator()();
	  virtual void stop() { endaccept=true; }
	};

      }
    }
  }
}

#endif
