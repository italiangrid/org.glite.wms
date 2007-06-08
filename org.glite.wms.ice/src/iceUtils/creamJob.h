/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE cream job
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_CREAMJOB_H
#define GLITE_WMS_ICE_UTIL_CREAMJOB_H

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

	  std::string m_cream_jobid;
          std::string m_grid_jobid;
	  std::string m_jdl;
	  std::string m_ceid;
	  std::string m_endpoint;
	  std::string m_cream_address;
	  std::string m_cream_deleg_address;
	  std::string m_user_proxyfile;
	  std::string m_user_dn;
          std::string m_sequence_code;
          std::string m_delegation_id;     
          std::string m_wn_sequence_code; //! The sequence code for the job sent to the worker node      
	  glite::ce::cream_client_api::job_statuses::job_status m_status;
          int m_num_logged_status_changes; //! Number of status changes which have been logged to L&B
          time_t m_last_seen; //! The time of the last received notification for the job. For newly created jobs, this value is set to zero.
          time_t m_end_lease; //! The time the lease for this job ends
	  time_t m_proxyCertTimestamp; //! The time of last modification of the user proxy certificate (needed by proxy renewal)
	  int    m_statusPollRetryCount; //! number of time we tried to get the status of the job
          int    m_exit_code; //! the job exit code
          std::string m_failure_reason; //! The job failure reason (if the job is done_failed or aborted)
          std::string m_worker_node; //! The worker node on which the job is being executed
          bool m_is_killed_by_ice;

	protected:
	  
	public:

          //! Default constructor
          CreamJob( );

          //! Costructor from classad
          CreamJob( const std::string& ad ) throw (ClassadSyntax_ex& );

	  //CreamJob( const CreamJob& ) throw();

	  //! Sets the status of the CreamJob object
	  void setStatus( const glite::ce::cream_client_api::job_statuses::job_status& st ) { m_status = st; }
	  //! Sets the cream unique identifier for this job
          void setCreamJobID( const std::string& cid ) { m_cream_jobid = cid; }
          //! Sets the jdl for this job
          void setJdl( const std::string& j ) throw( ClassadSyntax_ex& );
          //! Sets the sequence code
          void setSequenceCode( const std::string& seq ); // { m_sequence_code = seq; }
          //! Sets the delegation id
          void setDelegationId( const std::string& delid ) { m_delegation_id = delid; }
          //! Sets the new lease end time
          void setEndLease( const time_t& l ) { m_end_lease = l; }
          //! Sets the time we got info about this job from CREAM
          void setLastSeen( const time_t& l ) { m_last_seen = l; }
	  //! Sets the user proxy cert file last modification time
	  void setProxyCertMTime( const time_t& l ) { m_proxyCertTimestamp = l; }
          //! Sets the job exit code
          void set_exit_code( int c ) { m_exit_code = c; }
          //! Sets the sequence code for the job sent to the WN
          void set_wn_sequence_code( const std::string& wn_seq ) { m_wn_sequence_code = wn_seq; };
	  //! Sets the user's distinguished name
	  void setUserDN( const std::string& udn ) { m_user_dn = udn; };

          /**
           * Sets the job failure reason. NOTE: the failure reason can
           * be set ONLY ONCE. Attempts to set the failure reason
           * multiple times will result in only the first attempt to
           * succeed, and the other ones to silently be ignored. This
           * would probably need to be fixed. 
           *
           */
          void set_failure_reason( const std::string& f ) { 
	    if ( m_failure_reason.empty() ) {
	      m_failure_reason = f; 
	    }
          };

	  //! Gets the unique grid job identifier
          std::string getGridJobID( void ) const { return m_grid_jobid; }
          //! Gets the job exit code
          int get_exit_code( void ) const { return m_exit_code; }
          //! Gets the job failure reason
          std::string get_failure_reason( void ) const { return m_failure_reason; }
	  //! Gets the unique cream job identifier
          std::string getCreamJobID( void ) const { return m_cream_jobid; }
	  //! Gets the entire JDL of the job
          std::string getJDL( void ) const { return m_jdl; }
	  //! Gets the CE identifier for the job (containing the endpoint)
          std::string getCEID( void ) const { return m_ceid; }
	  //! Gets the endpoint of the cream service the job is submitted to
          std::string getEndpoint( void ) const { return m_endpoint; }
	  //! Gets the cream service URL this job is submitted to
          std::string getCreamURL( void ) const { return m_cream_address; }
	  //! Gets the cream delegation service URL used to delegate the user proxy certificate of this job
          std::string getCreamDelegURL( void ) const { return m_cream_deleg_address; }
	  //! Gets the path and file name of the user proxy certificate
          std::string getUserProxyCertificate( void ) const { return m_user_proxyfile; }
	  //! Gets the number of job status changes which have been already logged to L&B
	  int get_num_logged_status_changes( void ) const { return m_num_logged_status_changes; }
          //! Sets the number of job status changes whcih have been already logged to L&B
          void set_num_logged_status_changes( int l ) { m_num_logged_status_changes = l; }
          //! Gets the time when the lease ends
          time_t getEndLease( void ) const { return m_end_lease; }
          //! Gets the worker node on which the job is being execute (empty string if no worker node has been set)
          std::string get_worker_node( void ) const { return m_worker_node; }
          //! Sets the worker node on which the job is being executed
          void set_worker_node( const std::string& w_node ) { m_worker_node = w_node; }
          //! Gets the time we last got information about this job
          time_t getLastSeen( void ) const { return m_last_seen; }
          //! Gets the WN sequence code
          std::string get_wn_sequence_code( void ) const { return m_wn_sequence_code; }
          //! Gets the sequence code
          std::string getSequenceCode( void ) const { return m_sequence_code; }
          //! Gets the delegation id
          std::string getDelegationId( void ) const { return m_delegation_id; }
	  //! Gets the last modification time of the user proxy cert file
	  time_t getProxyCertLastMTime( void ) const { return m_proxyCertTimestamp; }
	  int    getStatusPollRetryCount( void ) const { return m_statusPollRetryCount; }
	  void   incStatusPollRetryCount( void ) { m_statusPollRetryCount++; }
	  void   resetStatusPollRetryCount( void ) { m_statusPollRetryCount=0; }
	  std::string getUserDN( void ) const { return m_user_dn; }

          //! Returns true iff the job is active (i.e., the job is either registered, idle, pending, running, reslly_running or held
          bool is_active( void ) const;

          /**
           * Returns true iff the job was killed by ICE for some
           * reason (e.g., because the proxy expired). This is
           * important as a job which has been killed by ICE should be
           * reported to be in status ABORTED.
           */
          bool is_killed_by_ice( void ) const { return m_is_killed_by_ice; }


          /**
           * This method is used to indicate that this job has been
           * killed by ICE, instead of by the user. This is used by
           * logging the appropriate event when the job terminates.
           */
          void set_killed_by_ice( void ) { m_is_killed_by_ice = true; }


          /**
           * Checke whether a job can be purged (by issuing a "purge"
           * request to CREAM). Currently, a job can be purged when it
           * is in done_ok, cancelled, done_ok or aborted states.
           *
           * @return true iff the job can be purged; false otherwise.
           */
          bool can_be_purged( void ) const;

          /**
           * Checks whether a job should be resubmitted. Currently, a
           * job can be resubmitted when it is in done_failed or
           * aborted state.
           *
           * @return true iff the job can be resubmitted; false otherwise.
           */
          bool can_be_resubmitted( void ) const;

	  //! Gets the status of the job
          glite::ce::cream_client_api::job_statuses::job_status getStatus(void) const { return m_status; }

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
           * This function outputs a string containing the CREAM and
           * grid jobid for this job. This function should be used
           * whenever any information related to a job needs to be
           * logged into the log files.
           *
           * @return the string containing cream and grid job id, in
           * human-readable form
           */
          std::string describe( void ) const;

	  size_t size( void ) const;

	};
      }
    }
  }
}

#endif
