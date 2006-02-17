
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
          std::string sequence_code;
	  glite::ce::cream_client_api::job_statuses::job_status status;
	  time_t      lastUpdate;
          time_t      endLease; //! The time the lease for this job ends

	public:

          //! Default constructor
          CreamJob( );

          //! Costructor from classad
          CreamJob( const std::string& ad ) throw (ClassadSyntax_ex& );

	  //! Creates a CreamJob object by copying from C
	  CreamJob( const CreamJob& C ) {
              cream_jobid         = C.cream_jobid;
              grid_jobid          = C.grid_jobid;
              jdl                 = C.jdl;
              ceid                = C.ceid;
              endpoint            = C.endpoint;
              cream_address       = C.cream_address;
              cream_deleg_address = C.cream_deleg_address;
              status              = C.status;
              user_proxyfile      = C.user_proxyfile;
              sequence_code       = C.sequence_code;
              lastUpdate          = C.lastUpdate;
              endLease            = C.endLease;
          }

	  //! Sets the status of the CreamJob object
	  //void setStatus(const glite::ce::cream_client_api::job_statuses::job_status& st) { status = st; }
          void setStatus( glite::ce::cream_client_api::job_statuses::job_status st, const time_t& tstamp ) { status = st; lastUpdate = tstamp; }
	  //! Sets the cream unique identifier for this job
          void setJobID( const std::string& cid ) { cream_jobid = cid; }
          //! Sets the jdl for this job
          void setJdl( const std::string& j ) throw( ClassadSyntax_ex& );
          //! Sets the sequence code
          void setSequenceCode( const std::string& seq ) { sequence_code = seq; }
          //! Sets the new lease end time
          void setEndLease( time_t l ) { endLease = l; }
          //! Sets the Grid JobID. FIXME: This is dangerous, as the Grid jobID is in the jdl, but here we may set the Grid JobID without giving a valid JDL!!!
          void setGridJobID( const std::string& id ) { grid_jobid = id; }

	  //! Gets the unique grid job identifier
          std::string getGridJobID( void ) const { return grid_jobid; }
	  //! Gets the unique cream job identifier
          std::string getJobID( void ) const { return cream_jobid; }
	  //! Gets the entire JDL of the job
          std::string getJDL( void ) const { return jdl; }
	  //! Gets the CE identifier for the job (containing the endpoint)
          std::string getCEID( void ) const { return ceid; }
	  //! Gets the endpoint of the cream service the job is submitted to
          std::string getEndpoint( void ) const { return endpoint; }
	  //! Gets the cream service URL this job is submitted to
          std::string getCreamURL( void ) const { return cream_address; }
	  //! Gets the cream delegation service URL used to delegate the user proxy certificate of this job
          std::string getCreamDelegURL( void ) const { return cream_deleg_address; }
	  //! Gets the path and file name of the user proxy certificate
          std::string getUserProxyCertificate( void ) const { return user_proxyfile; }
	  //! Gets the last time of status update of the job
	  time_t getLastUpdate( void ) const { return lastUpdate; }
          //! Gets the time when the lease ends
          time_t getEndLease( void ) const { return endLease; }

          //! Gets the sequence code
          std::string getSequenceCode( void ) const { return sequence_code; }

	  //! Gets the status of the job
          glite::ce::cream_client_api::job_statuses::job_status getStatus(void) const { return status; }

	  //! Return true if j1 and j2 have the same grid job identifier
          friend bool operator==( const CreamJob& j1, const CreamJob& j2 ) {
	    return ( j1.getGridJobID() == j2.getGridJobID() );
	  }

          /**
           * Converts this job into a classad
           *
           * @return the string representation of the classad for this job
           */
          std::string serialize( void ) const;

          /**
           * Initializes a job object from a classad
           *
           * @param buf the string representation of the classad jolding a job 
           */
          void unserialize( const std::string& buf ) throw ( ClassadSyntax_ex& );

          /**
           * This method is used to create a "fake" CreamJob with a
           * given cream job ID. The content of the other fields are
           * completely dummy. This "fake" CreamJob is to be used to
           * log Cancel events to L&B; if ICE receives a request to
           * cancel a nonexisting job, this event has to be logged but
           * no "real" CreamJob object could be found in the jobCache.
           *
           * @param grid_job_id the Grid Job id for this job
           * @return a CreamJob with given Grid job ID
           */
          static CreamJob mkFakeCreamJob( const std::string& grid_job_id );
	};
      }
    }
  }
}

#endif
