
#ifndef _CREAMJOB_H__
#define _CREAMJOB_H__

#include "ClassadSyntax_ex.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include <string>
//#include "classad_distribution.h"

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	//______________________________________________________________________
	class CreamJob {

	  std::string jobid;
	  std::string edg_jobid;
	  std::string jdl;
	  std::string ceid;
	  std::string endpoint;
	  std::string cream_address;
	  std::string cream_deleg_address;
	  glite::ce::cream_client_api::job_statuses::job_status status;

	public:

	  CreamJob( const std::string& _jdl,
		    const std::string& _jobid,
		    const std::string& _edg_jobid,
		    const glite::ce::cream_client_api::job_statuses::job_status& st) 
	    throw(ClassadSyntax_ex&);

	  CreamJob(const CreamJob& C) {
	    jobid = C.jobid;
	    edg_jobid = C.edg_jobid;
	    jdl = C.jdl;
	    ceid = C.ceid;
	    endpoint = C.endpoint;
	    cream_address = C.cream_address;
	    cream_deleg_address = C.cream_deleg_address;
	    status = C.status;
	  }

	  inline void setStatus(const glite::ce::cream_client_api::job_statuses::job_status& st) 
	  { 
	    status = st;
	  }
	  inline void setJobID(const std::string& cid) { jobid = cid; }
	  inline std::string getGridJobID(void) const { return edg_jobid; }
	  inline std::string getJobID(void) const { return jobid; }
	  inline std::string getJDL(void) const { return jdl; }
	  inline std::string getCEID(void) const { return ceid; }
	  inline std::string getEndpoint(void) const { return endpoint; }
	  inline std::string getCreamURL(void) const { return cream_address; }
	  inline std::string getCreamDelegURL(void) const { return cream_deleg_address; }
	  inline glite::ce::cream_client_api::job_statuses::job_status getStatus(void) const { return status; }
	};
      }
    }
  }
}

#endif
