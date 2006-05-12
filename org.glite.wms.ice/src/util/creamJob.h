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

	  std::string cream_jobid;
          std::string grid_jobid;
	  std::string jdl;
	  std::string ceid;
	  std::string endpoint;
	  std::string cream_address;
	  std::string cream_deleg_address;
	  std::string user_proxyfile;
          std::string sequence_code;
          std::string delegation_id;     
          std::string wn_sequence_code; //! The sequence code for the job sent to the worker node      
	  glite::ce::cream_client_api::job_statuses::job_status status;
          int m_num_logged_status_changes; //! Number of status changes which have been logged to L&B
          time_t last_seen; //! The time of the last received notification for the job
          time_t end_lease; //! The time the lease for this job ends
	  time_t proxyCertTimestamp; //! The time of last modification of the user proxy certificate (needed by proxy renewal)
	  int    statusPollRetryCount; //! number of time we tried to get the status of the job
          int m_exit_code; // the job exit code

	public:

          //! Default constructor
          CreamJob( );

          //! Costructor from classad
          CreamJob( const std::string& ad ) throw (ClassadSyntax_ex& );

	  //! Creates a CreamJob object by copying from C
	  CreamJob( const CreamJob& C ) {
              cream_jobid = C.cream_jobid;
              grid_jobid = C.grid_jobid;
              jdl = C.jdl;
              ceid = C.ceid;
              endpoint = C.endpoint;
              cream_address = C.cream_address;
              cream_deleg_address = C.cream_deleg_address;
              status = C.status;
              user_proxyfile = C.user_proxyfile;
              sequence_code = C.sequence_code;
              delegation_id = C.delegation_id;
              wn_sequence_code = C.wn_sequence_code;
              m_num_logged_status_changes = C.m_num_logged_status_changes;
              last_seen = C.last_seen;
              end_lease = C.end_lease;
	      proxyCertTimestamp = C.proxyCertTimestamp;
              m_exit_code = C.m_exit_code;
	      // is the following actually needed ? // FIXME
	      //statusPollRetryCount = C.statusPollRetryCount;
          }

	  //! Sets the status of the CreamJob object
	  //void setStatus( const glite::ce::cream_client_api::job_statuses::job_status& st ) { status = st; }
          void setStatus( const glite::ce::cream_client_api::job_statuses::job_status& st ) { status = st; }
	  //! Sets the cream unique identifier for this job
          void setJobID( const std::string& cid ) { cream_jobid = cid; }
          //! Sets the jdl for this job
          void setJdl( const std::string& j ) throw( ClassadSyntax_ex& );
          //! Sets the sequence code
          void setSequenceCode( const std::string& seq ) { sequence_code = seq; }
          //! Sets the delegation id
          void setDelegationId( const std::string& delid ) { delegation_id = delid; }
          //! Sets the new lease end time
          void setEndLease( const time_t& l ) { end_lease = l; }
          //! Sets the time we got info about this job from CREAM
          void setLastSeen( const time_t& l ) { last_seen = l; }
	  //! Sets the user proxy cert file last modification time
	  void setProxyCertMTime( const time_t& l ) { proxyCertTimestamp = l; }
          //! Sets the job exit code
          void set_exit_code( int c ) { m_exit_code = c; }
          //! Sets the sequence code for the job sent to the WN
          void set_wn_sequence_code( const std::string& wn_seq ) { wn_sequence_code = wn_seq; };
	  //! Gets the unique grid job identifier
          std::string getGridJobID( void ) const { return grid_jobid; }
          //! Gets the job exit code
          int get_exit_code( void ) const { return m_exit_code; }
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
	  //! Gets the number of job status changes which have been already logged to L&B
	  int get_num_logged_status_changes( void ) const { return m_num_logged_status_changes; }
          //! Sets the number of job status changes whcih have been already logged to L&B
          void set_num_logged_status_changes( int l ) { m_num_logged_status_changes = l; }
          //! Gets the time when the lease ends
          time_t getEndLease( void ) const { return end_lease; }
          //! Gets the time we last got information about this job
          time_t getLastSeen( void ) const { return last_seen; }
          //! Gets the WN sequence code
          std::string get_wn_sequence_code( void ) const { return wn_sequence_code; }
          //! Gets the sequence code
          std::string getSequenceCode( void ) const { return sequence_code; }
          //! Gets the delegation id
          std::string getDelegationId( void ) const { return delegation_id; }
	  //! Gets the last modification time of the user proxy cert file
	  time_t getProxyCertLastMTime( void ) const { return proxyCertTimestamp; }
	  int    getStatusPollRetryCount( void ) const { return statusPollRetryCount; }
	  void   incStatusPollRetryCount( void ) { statusPollRetryCount++; }
	  void   resetStatusPollRetryCount( void ) { statusPollRetryCount=0; }
          //! Returns true iff the job is active (i.e., the job is either registered, idle, pending, idle, running or held
          bool is_active( void ) const;

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

	};
      }
    }
  }
}

#endif
