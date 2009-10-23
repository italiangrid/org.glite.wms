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

/**
 *
 * ICE Headers
 *
 */
#include "ClassadSyntax_ex.h"
#include "SerializeException.h"
#include "ice_timer.h"
/**
 *
 * Cream Client API C++ Headers
 *
 */
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/certUtil.h"

#include "boost/thread/recursive_mutex.hpp"
#include "boost/tuple/tuple.hpp"

/**
 *
 * System and STL C++ Headers
 *
 */
#include <exception>
#include <vector>
#include <string>
#include <ctime>
#include <set>

namespace api_util = glite::ce::cream_client_api::util;

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
	  
	  static boost::recursive_mutex serialize_mutex;

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
	  //time_t      m_delegation_exptime;
	  //	  int         m_delegation_duration;
          std::string m_wn_sequence_code; //! The sequence code for the job sent to the worker node      
          glite::ce::cream_client_api::job_statuses::job_status m_prev_status; //! previous status of the job
	  glite::ce::cream_client_api::job_statuses::job_status m_status; //! Current status of the job
          int m_num_logged_status_changes; //! Number of status changes which have been logged to L&B
          time_t m_last_seen; //! The time of the last received notification for the job. For newly created jobs, this value is set to zero.
          std::string m_lease_id; //! The lease ID associated with this job
	  //time_t m_proxyCertTimestamp; //! The time of last modification of the user proxy certificate (needed by proxy renewal)
	  int    m_statusPollRetryCount; //! number of time we tried to get the status of the job
          int    m_exit_code; //! the job exit code
          std::string m_failure_reason; //! The job failure reason (if the job is done_failed or aborted)
          std::string m_worker_node; //! The worker node on which the job is being executed
          bool m_is_killed_byice;
          time_t m_last_empty_notification; //! The timestamp of the last received empty notification
	  bool m_proxy_renew;
	  std::string m_myproxy_address;
	  
	  
	protected:
	  /**
           * Initializes a job object from a classad
           *
           * @param buf the string representation of the classad jolding a job 
           */
          // void unserialize( const std::string& buf ) throw ( ClassadSyntax_ex& );

	public:
	  
	  CreamJob( const std::string& gid,
		    const std::string& cid,
		    const std::string& jdl,
		    const std::string& userproxy,
		    const std::string& ceid,
		    const std::string& endpoint,
		    const std::string& creamurl,
		    const std::string& creamdelegurl,
		    const std::string& userdn,
		    const std::string& myproxyurl,
		    const std::string& proxy_renewable,
		    const std::string& failure_reason,
		    const std::string& sequence_code,
		    const std::string& wn_sequence_code,
		    const std::string& prev_status,
		    const std::string& status,
		    const std::string& num_logged_status_changes,
		    const std::string& leaseid,
		    const std::string& status_poller_retry_count,
		    const std::string& exit_code,
		    const std::string& worker_node,
		    const std::string& is_killed_byice,
		    const std::string& delegationid,
		    const std::string& last_empty_notification,
		    const std::string& last_seen);

	  static std::string get_query_fields() {
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_query_fields");
#endif

	    return "gridjobid,"			\
	      "creamjobid,"			\
	      "jdl,"				\
	      "userproxy,"			\
	      "ceid,"				\
	      "endpoint,"			\
	      "creamurl,"			\
	      "creamdelegurl,"			\
	      "userdn,"				\
	      "myproxyurl,"			\
	      "proxy_renewable,"		\
	      "failure_reason,"			\
	      "sequence_code,"			\
	      "wn_sequence_code,"		\
	      "prev_status,"			\
	      "status,"				\
	      "num_logged_status_changes,"	\
	      "leaseid,"			\
	      "status_poller_retry_count,"	\
	      "exit_code,"			\
	      "worker_node,"			\
	      "is_killed_byice,"		\
	      "delegationid,"			\
	      "last_empty_notification,"	\
	      "last_seen";
	  }

	  static std::string get_query_allfields() {
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_query_allfields");
#endif
	    return "gridjobid,"			\
	      "creamjobid,"			\
	      "complete_cream_jobid,"		\
	      "jdl,"				\
	      "userproxy,"			\
	      "ceid,"				\
	      "endpoint,"			\
	      "creamurl,"			\
	      "creamdelegurl,"			\
	      "userdn,"				\
	      "myproxyurl,"			\
	      "proxy_renewable,"		\
	      "failure_reason,"			\
	      "sequence_code,"			\
	      "wn_sequence_code,"		\
	      "prev_status,"			\
	      "status,"				\
	      "num_logged_status_changes,"	\
	      "leaseid,"			\
	      "status_poller_retry_count,"	\
	      "exit_code,"			\
	      "worker_node,"			\
	      "is_killed_byice,"		\
	      "delegationid,"			\
	      "last_empty_notification,"	\
	      "last_seen";
	  }

	  static std::string get_createdb_query() {
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_createdb_query");
#endif
	    return 
	      " gridjobid text primary key, "			\
	      " creamjobid text, "				\
	      " complete_cream_jobid text,"			\
	      " jdl blob not null, "				\
	      " userproxy text not null, "			\
	      " ceid text, "					\
	      " endpoint text, "				\
	      " creamurl text not null, "			\
	      " creamdelegurl text not null, "			\
	      " userdn text not null, "				\
	      " myproxyurl text not null, "			\
	      " proxy_renewable integer(1) not null,"		\
	      " failure_reason blob,"				\
	      " sequence_code text,"				\
	      " wn_sequence_code text,"				\
	      " prev_status integer(1),"			\
	      " status integer(1),"				\
	      " num_logged_status_changes integer(1),"		\
	      " leaseid text,"					\
	      " status_poller_retry_count integer(1),"		\
	      " exit_code integer(1),"				\
	      " worker_node text,"				\
	      " is_killed_byice integer(1),"			\
	      " delegationid text,"				\
	      " last_empty_notification integer(4),"		\
	      " last_seen integer(4),"				\
	      " last_poller_visited integer(4), "               
	      " dbid integer(8) ";
	  }

	  //	  static boost::recursive_mutex s_GlobalICEMutex;

          //! Default constructor
          CreamJob( );
	  

	  //	  CreamJob( const std::vector< std::string >& );

	  //! Sets the status of the CreamJob object
	  void set_status( const glite::ce::cream_client_api::job_statuses::job_status& st ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_status");
#endif
	    m_prev_status = m_status; m_status = st; 
	  }
	  //! Sets the cream unique identifier for this job
          void set_cream_jobid( const std::string& cid ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_cream_jobid");
#endif
	    m_cream_jobid = cid; 
	  }
          //! Sets the jdl for this job
          void set_jdl( const std::string& j ) throw( ClassadSyntax_ex& );
          //! Sets the sequence code
          void set_sequence_code( const std::string& seq ); // { m_sequence_code = seq; }
          //! Sets the delegation id
          void set_delegation_id( const std::string& delid ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_delegation_id");
#endif
	    m_delegation_id = delid; 
	  }
          //! Sets the time we got info about this job from CREAM
          void set_last_seen( const time_t& l ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_last_seen");
#endif
	    m_last_seen = l; 
	  }
	  //! Sets the user proxy cert file last modification time
	  //void set_proxycert_mtime( const time_t& l ) { m_proxyCertTimestamp = l; }
          //! Sets the job exit code
          void set_exitcode( int c ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_exitcode");
#endif	    
	    m_exit_code = c; 
	  }
          //! Sets the sequence code for the job sent to the WN
          void set_wn_sequencecode( const std::string& wn_seq ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_sequencecode");
#endif
	    m_wn_sequence_code = wn_seq; 
	  };
	  //! Sets the user's distinguished name
	  void set_userdn( const std::string& udn ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_userdn");
#endif
	    m_user_dn = udn; 
	  };

	  //	  void set_delegation_expiration_time( const time_t T ) { m_delegation_exptime = T; }

	  //	  void set_delegation_duration( const int T ) { m_delegation_duration = T; }

          /**
           * Sets the job failure reason. NOTE: the failure reason can
           * be set ONLY ONCE. Attempts to set the failure reason
           * multiple times will result in only the first attempt to
           * succeed, and the other ones to silently be ignored. This
           * would probably need to be fixed. 
           *
           */
          void set_failure_reason( const std::string& f ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_failure_reason");
#endif
	    //if ( m_failure_reason.empty() ) {
	    if( f.empty() )
	      m_failure_reason = " "; 
	    else
	      m_failure_reason = f;
	    //}
          };

	  //	  time_t getDelegationExpirationTime( void ) const { return m_delegation_exptime; }

	  //	  int    getDelegationDuration( void ) const { return m_delegation_duration; }

	  //! Gets the unique grid job identifier
          std::string getGridJobID( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getGridJobID");
#endif
	    return m_grid_jobid; 
	  }

          //! Gets the job exit code
          int get_exit_code( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_exit_code");
#endif
	    return m_exit_code; 
	  }

          //! Gets the job failure reason
          std::string get_failure_reason( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_failure_reason");
#endif
	    return m_failure_reason; 
	  }

	  //! Gets the unique cream job identifier
          std::string getCreamJobID( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getCreamJobID");
#endif
	    return m_cream_jobid; 
	  }
	  
	  //! Gets the COMPLETE unique cream job identifier
          std::string getCompleteCreamJobID( void ) const;// { return m_cream_jobid; }

	  //! Gets the entire JDL of the job
          std::string getJDL( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_JDL");
#endif
	    return m_jdl; 
	  }

	  //! Gets the CE identifier for the job (containing the endpoint)
          std::string getCEID( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getCEID");
#endif
	    return m_ceid; 
	  }

	  //! Gets the endpoint of the cream service the job is submitted to
          std::string getEndpoint( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getEndpoint");
#endif
	    return m_endpoint; 
	  }

	  //! Gets the cream service URL this job is submitted to
          std::string getCreamURL( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getCreamURL");
#endif
	    return m_cream_address; 
	  }

	  //! Gets the cream delegation service URL used to delegate the user proxy certificate of this job
          std::string getCreamDelegURL( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getCreamDelegURL");
#endif
	    return m_cream_deleg_address; 
	  }

	  //! Gets the path and file name of the user proxy certificate
          std::string getUserProxyCertificate( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getUserProxyCertificate");
#endif
	    return m_user_proxyfile; 
	  }

          //! Gets the path and file name of the "better proxy" for the DN of the user owning this job
          //std::string getBetterProxy( void ) const;

	  //! Gets the number of job status changes which have been already logged to L&B
	  int get_num_logged_status_changes( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_num_logged_status_changes");
#endif
	    return m_num_logged_status_changes; 
	  }

          //! Sets the number of job status changes whcih have been already logged to L&B
          void set_numlogged_status_changes( int l ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_numlogged_status_changes");
#endif
	    m_num_logged_status_changes = l; 
	  }

          //! Return the lease ID associated with this job
          std::string get_lease_id( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_lease_id");
#endif
	    return m_lease_id; 
	  };

          //! Set the lease ID associated with this job
          void set_leaseid( const std::string& lease_id ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_leaseid");
#endif
	    m_lease_id = lease_id; 
	  };

          //! Gets the worker node on which the job is being execute (empty string if no worker node has been set)
          std::string get_worker_node( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_worker_node");
#endif
	    return m_worker_node; 
	  }

          //! Sets the worker node on which the job is being executed
          void set_workernode( const std::string& w_node ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_workernode");
#endif
	    m_worker_node = w_node; 
	  }

          //! Gets the time we last got information about this job
          time_t getLastSeen( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getLastSeen");
#endif
	    return m_last_seen; 
	  }

          //! Gets the WN sequence code
          std::string get_wn_sequence_code( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_wn_sequence_code");
#endif
	    return m_wn_sequence_code; 
	  }

          //! Gets the sequence code
          std::string getSequenceCode( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getSequenceCode");
#endif
	    return m_sequence_code; 
	  }

          //! Gets the delegation id
          std::string getDelegationId( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getDelegationId");
#endif
	    return m_delegation_id; 
	  }

	  //! Gets the last modification time of the user proxy cert file
//	  time_t getProxyCertLastMTime( void ) const { return m_proxyCertTimestamp; }

	  int    getStatusPollRetryCount( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getStatusPollRetryCount");
#endif
	    return m_statusPollRetryCount; 
	  }

	  void   inc_status_pollretry_count( void ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::inc_status_pollretry_count");
#endif
	    m_statusPollRetryCount++; 
	  }

	  void   reset_status_pollretry_count( void ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::reset_status_pollretry_count");
#endif
	    m_statusPollRetryCount=0; 
	  }

	  std::string getUserDN( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getUserDN");
#endif 
	    return m_user_dn; 
	  }

          //! Returns true iff the job is active (i.e., the job is either registered, idle, pending, running, reslly_running or held
          bool is_active( void ) const;

          /**
           * Returns true iff the job was killed by ICE for some
           * reason (e.g., because the proxy expired). This is
           * important as a job which has been killed by ICE should be
           * reported to be in status ABORTED.
           */
          bool is_killed_by_ice( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::is_killed_by_ice");
#endif
	    return m_is_killed_byice; 
	  }


          /**
           * This method is used to indicate that this job has been
           * killed by ICE, instead of by the user. This is used by
           * logging the appropriate event when the job terminates.
           */
          void set_killed_byice( void ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_killed_byice");
#endif
	    m_is_killed_byice = true; 
	  }

          time_t get_last_empty_notification( void ) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_last_empty_notification");
#endif
	    return m_last_empty_notification; 
	  };
          void set_last_empty_notification_time( time_t t ) { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::set_last_empty_notification");
#endif
	    m_last_empty_notification = t; 
	  };

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
          glite::ce::cream_client_api::job_statuses::job_status getStatus(void) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getStatus");
#endif
	    return m_status; 
	  }

          glite::ce::cream_client_api::job_statuses::job_status get_prev_status(void) const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::get_prev_status");
#endif
	    return m_prev_status; 
	  }

	  //! Return true if j1 and j2 have the same grid job identifier
          friend bool operator==( const CreamJob& j1, const CreamJob& j2 ) {
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::operator==");
#endif
	    return ( j1.getGridJobID() == j2.getGridJobID() );
	  }

	  bool is_proxy_renewable() const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::is_proxy_renewable");
#endif
	    return m_proxy_renew; 
	  }

	  std::string getMyProxyAddress() const { 
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::getMyProxyAddress");
#endif
	    return m_myproxy_address; 
	  }


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


          /**
           * Gets the subscription ID whioch is used to receive status
           * change notifications related to this job.
           *
           * @return the CEMon Subscription ID 
           */
          //std::string _getSubscriptionID( void ) const;

          /**
           * Gets the CEMonitor URL for the CEMon which sends
           * notifications about status changes for this job.
           *
           * @return the CEMon URL for the CEMon which sends
           * notifications for this job
           */
	 protected:
          //std::string _getCEMonURL( void ) const;

         public:
          /**
           * Returns the CEMon DN for the CEMon which is sending 
           * status change notifications for this job.
           *
           * @return the CEMon DN for the CEMon which sends
           * notifications for this job.
           */
          //std::string _get_cemon_dn( void ) const;

	  template<class Archive> void serialize(Archive & ar, const unsigned int version) throw(SerializeException&)
	  {
#ifdef ICE_PROFILE
	    ice_timer timer("CreamJob::serialize");
#endif
	    boost::recursive_mutex::scoped_lock L( serialize_mutex );

	    try {

	      ar & m_cream_jobid;


	      ar & m_grid_jobid;
	      

	      ar & m_jdl;
	     

	      ar & m_ceid;
	      

	      ar & m_endpoint;
	      

	      ar & m_cream_address;
	      

	      ar & m_cream_deleg_address;
	      

	      ar & m_user_proxyfile;
	      

	      ar & m_user_dn;
	      

	      ar & m_sequence_code;
	      

	      ar & m_delegation_id;


	      ar & m_wn_sequence_code;
	      

              ar & m_prev_status;
	      

	      ar & m_status;
	      

	      ar & m_num_logged_status_changes;
	      

	      ar & m_last_seen;
	      

	      ar & m_lease_id;
	      

	      //ar & m_proxyCertTimestamp;
	      

	      ar & m_statusPollRetryCount;
	      

	      ar & m_exit_code;
	      

	      ar & m_failure_reason;
	      

	      ar & m_worker_node;
	      

	      ar & m_is_killed_byice;
	      

	      ar & m_last_empty_notification;
	      

	      ar & m_proxy_renew;

	      ar & m_myproxy_address;

	    } catch(std::exception& ex) {
	      CREAM_SAFE_LOG(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
		       << "creamJob::serialize() - [De]Serialization error: ["
		       << ex.what() << "]"
		       );
	      throw SerializeException( ex.what() );
	    }
	  }

	  //	  void get_fields( std::vector<std::string>& ) const;

	};
      }
    }
  }
}

#endif
