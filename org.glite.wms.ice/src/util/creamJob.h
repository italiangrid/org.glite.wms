
#ifndef _GLITE_WMS_ICE_UTIL_CREAMJOB_H__
#define _GLITE_WMS_ICE_UTIL_CREAMJOB_H__

#include "ClassadSyntax_ex.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include <string>
#include <ctime>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	//______________________________________________________________________

	//! A class for a ICE's job
	/*!\class CreamJob 
	  CreamJob is a class describing what a job is for ICE:
	  - a cream job identifier string as returned by the CREAM service after a JDL has been registered
	  - a edg job id string that represents the unique identifier of the user's job in the entire grid
	  - a jdl string that describe the user job
	  - the address of the CREAM service where the job has been submitted
	  - the address of the CREAMDelegation service that is the URL of the service that perform user proxy delegation
	  - the path of the user proxy certificate
	  - the status of the job (PENDING, IDLE, RUNNING, DONE-OK, DONE-FAILED, ABORTED etc.)
	  - last time of update that is the time of the last status update
	*/
	class CreamJob {

	  std::string cream_jobid;
          std::string grid_jobid;
	  std::string jdl;
	  std::string ceid;
	  std::string endpoint;
	  std::string cream_address;
	  std::string cream_deleg_address;
	  std::string user_proxyfile;
	  glite::ce::cream_client_api::job_statuses::job_status status;
	  time_t      lastUpdate;
	  //std::string subscriptionID;

	public:

	  //! CreamJob constructor
	  /*!
	    Creates a CreamJob object
	    \param _jdl is the job description string describing the job
	    \param _cream_jobid is the CREAM unique identifier of the registered (and submitted) job
	    \param st the current status of the job
	    \param ts the time of the last status update of the job
	    \param subID the subscription identifier that is used by the listener to check if it is receiving notification about the status changes of the job
	    \throw ClassadSyntax_ex& if the parsing of the _jdl string failed
	  */
	  CreamJob( const std::string& _jdl,
		    const std::string& _cream_jobid, //< Cream Job ID 
		    const glite::ce::cream_client_api::job_statuses::job_status& st,
		    time_t ts) 
	    throw(ClassadSyntax_ex&);

	  //! Creates a CreamJob object by copying from C
	  CreamJob(const CreamJob& C) {
	    cream_jobid         = C.cream_jobid;
	    grid_jobid          = C.grid_jobid;
	    jdl                 = C.jdl;
	    ceid                = C.ceid;
	    endpoint            = C.endpoint;
	    cream_address       = C.cream_address;
	    cream_deleg_address = C.cream_deleg_address;
	    status              = C.status;
            user_proxyfile      = C.user_proxyfile;
	    lastUpdate          = C.lastUpdate;
	    //subscriptionID      = C.subscriptionID;
	  }

	  //! Sets the status of the CreamJob object
	  inline void setStatus(const glite::ce::cream_client_api::job_statuses::job_status& st)
	  {
	    status = st;
	  }
	  //! Sets the last time of status update
	  void setLastUpdate(const time_t& t) { lastUpdate = t; }
/*	  //! Sets the subscription identifier of this job (is used by the eventStatusListener)
	  void setSubscriptionID(const std::string& subID) { subscriptionID = subID; }*/
	  //! Sets the cream unique identifier for this job
          void setJobID(const std::string& cid) { cream_jobid = cid; }
	  //! Gets the unique grid job identifier
          std::string getGridJobID(void) const { return grid_jobid; }
	  //! Gets the unique cream job identifier
          std::string getJobID(void) const { return cream_jobid; }
	  //! Gets the entire JDL of the job
          std::string getJDL(void) const { return jdl; }
	  //! Gets the CE identifier for the job (containing the endpoint)
          std::string getCEID(void) const { return ceid; }
	  //! Gets the endpoint of the cream service the job is submitted to
          std::string getEndpoint(void) const { return endpoint; }
	  //! Gets the cream service URL this job is submitted to
          std::string getCreamURL(void) const { return cream_address; }
	  //! Gets the cream delegation service URL used to delegate the user proxy certificate of this job
          std::string getCreamDelegURL(void) const { return cream_deleg_address; }
	  //! Gets the path and file name of the user proxy certificate
          std::string getUserProxyCertificate( void ) const { return user_proxyfile; }
	  //! Gets the last time of status update of the job
	  time_t      getLastUpdate( void ) const { return lastUpdate; }
/*	  //! Gets the subscription identifier of this job (used by eventStatusListener)
	  std::string getSubscriptionID(void) const { return subscriptionID; }*/
	  //! Gets the status of the job
          glite::ce::cream_client_api::job_statuses::job_status getStatus(void) const {
	    return status;
	  }
	  //! Return true if j1 and j2 have the same grid job identifier
          friend bool operator==( const CreamJob& j1, const CreamJob& j2 ) {
	    return ( j1.getGridJobID() == j2.getGridJobID() );
	  }
	};
      }
    }
  }
}

#endif
