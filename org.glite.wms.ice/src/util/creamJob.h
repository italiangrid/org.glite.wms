
#ifndef _CREAMJOB_H__
#define _CREAMJOB_H__

#include "ClassadSyntax_ex.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include <string>

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
	  std::string user_proxyfile;
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
            user_proxyfile = C.user_proxyfile;
	  }

	  inline void setStatus(const glite::ce::cream_client_api::job_statuses::job_status& st) 
	  { 
	    status = st;
	  }
          void setJobID(const std::string& cid) { jobid = cid; }
          std::string getGridJobID(void) const { return edg_jobid; }
          std::string getJobID(void) const { return jobid; }
          std::string getJDL(void) const { return jdl; }
          std::string getCEID(void) const { return ceid; }
          std::string getEndpoint(void) const { return endpoint; }
          std::string getCreamURL(void) const { return cream_address; }
          std::string getCreamDelegURL(void) const { return cream_deleg_address; }
          std::string getUserProxyCertificate( void ) const { return user_proxyfile; }
          glite::ce::cream_client_api::job_statuses::job_status getStatus(void) const { return status; }
          friend bool operator==( const CreamJob& j1, const CreamJob& j2 ) { return ( j1.getGridJobID() == j2.getGridJobID() ); }
	};
      }
    }
  }
}

#endif
