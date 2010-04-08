/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */

#ifndef GLITE_WMS_ICE_UTIL_CREAMJOB_H
#define GLITE_WMS_ICE_UTIL_CREAMJOB_H

/**
 *
 * ICE Headers
 *
 */
#include "ClassadSyntax_ex.h"
#include "SerializeException.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "iceConfManager.h"
/**
 *
 * Cream Client API C++ Headers
 *
 */
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/certUtil.h"

//#include "boost/thread/recursive_mutex.hpp"
#include <boost/algorithm/string.hpp>
#include "boost/tuple/tuple.hpp"
#include <boost/regex.hpp>

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
	  
	  //static boost::recursive_mutex serialize_mutex;

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
          glite::ce::cream_client_api::job_statuses::job_status m_prev_status; //! previous status of the job
	  glite::ce::cream_client_api::job_statuses::job_status m_status; //! Current status of the job
          int         m_num_logged_status_changes; //! Number of status changes which have been logged to L&B
          time_t      m_last_seen; //! The time of the last received notification for the job. For newly created jobs, this value is set to zero.
          std::string m_lease_id; //! The lease ID associated with this job
	  int         m_statusPollRetryCount; //! number of time we tried to get the status of the job
          int         m_exit_code; //! the job exit code
          std::string m_failure_reason; //! The job failure reason (if the job is done_failed or aborted)
          std::string m_worker_node; //! The worker node on which the job is being executed
          bool        m_is_killed_byice;
          time_t      m_last_empty_notification; //! The timestamp of the last received empty notification
	  bool        m_proxy_renew;
	  std::string m_myproxy_address;
	  std::string m_complete_cream_jobid;
	  time_t      m_isbproxy_time_end;
	  std::string m_modified_jdl;

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
		    const std::string& ccid,
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
		    const std::string& last_seen,
		    const std::string& isbproxy_time_end,
		    const std::string& modif_jdl);

	  static std::string get_query_fields() {
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
	      "last_seen,"			\
	      "isbproxy_time_end,"		\
	      "modified_jdl";
	  }

	  static std::string get_createdb_query() {
	    return 
	      " gridjobid text primary key not null, "			\
	      " creamjobid text not null, "				\
	      " complete_cream_jobid text not null,"			\
	      " jdl blob not null, "					\
	      " userproxy text not null, "				\
	      " ceid text not null, "					\
	      " endpoint text not null, "				\
	      " creamurl text not null, "				\
	      " creamdelegurl text not null, "				\
	      " userdn text not null not null, "			\
	      " myproxyurl text not null not null, "			\
	      " proxy_renewable integer(1) not null,"			\
	      " failure_reason blob not null,"				\
	      " sequence_code text not null,"				\
	      " wn_sequence_code text not null,"			\
	      " prev_status integer(1) not null,"			\
	      " status integer(1) not null,"				\
	      " num_logged_status_changes integer(1) not null,"		\
	      " leaseid text not null,"					\
	      " status_poller_retry_count integer(1) not null,"		\
	      " exit_code integer(1) not null,"				\
	      " worker_node text not null,"				\
	      " is_killed_byice integer(1) not null,"			\
	      " delegationid text not null,"				\
	      " last_empty_notification integer(8) not null,"		\
	      " last_seen integer(8) not null,"				\
	      " last_poller_visited integer(8) not null, "		\
	      " isbproxy_time_end integer(8) not null,"			\
	      " modified_jdl blob not null,"				\
	      " dbid integer(8) ";
	  }

          //! Default constructor
          CreamJob( );
	  
	  void set_isbproxy_time_end( const time_t t ) {
	    m_isbproxy_time_end = t;
	  }

	  time_t get_isbproxy_time_end( void ) const { return m_isbproxy_time_end; }

	  std::string get_modified_jdl( void ) const { return m_modified_jdl; }

	  //! Sets the status of the CreamJob object
	  void set_status( const glite::ce::cream_client_api::job_statuses::job_status& st ) { 
	    m_prev_status = m_status; m_status = st; 
	  }
	  //! Sets the cream unique identifier for this job
          void set_cream_jobid( const std::string& cid ) { 
	    m_cream_jobid = cid; 
	    if(!m_cream_jobid.empty() && !m_cream_address.empty()) {
	      m_complete_cream_jobid = m_cream_address;
	      boost::replace_all( m_complete_cream_jobid, iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_postfix(), "" );
	      m_complete_cream_jobid += "/" + m_cream_jobid;
	    }

	  }
          //! Sets the jdl for this job
          void set_jdl( const std::string& j, const std::string& cmdtype ) throw( ClassadSyntax_ex& );
          //! Sets the sequence code
          void set_sequence_code( const std::string& seq ); // { m_sequence_code = seq; }
          //! Sets the delegation id
          void set_delegation_id( const std::string& delid ) { 
	    m_delegation_id = delid; 
	  }
          //! Sets the time we got info about this job from CREAM
          void set_last_seen( const time_t& l ) { 
	    m_last_seen = l; 
	  }
	  //! Sets the user proxy cert file last modification time
	  //void set_proxycert_mtime( const time_t& l ) { m_proxyCertTimestamp = l; }
          //! Sets the job exit code
          void set_exitcode( int c ) { 
	    m_exit_code = c; 
	  }
          //! Sets the sequence code for the job sent to the WN
          void set_wn_sequencecode( const std::string& wn_seq ) { 
	    m_wn_sequence_code = wn_seq; 
	  };
	  //! Sets the user's distinguished name
	  void set_userdn( const std::string& udn ) { 
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
	    if( f.empty() )
	      m_failure_reason = " "; 
	    else
	      m_failure_reason = f;
          };

	  //! Gets the unique grid job identifier
          std::string get_grid_jobid( void ) const { 
	    return m_grid_jobid; 
	  }

          //! Gets the job exit code
          int get_exit_code( void ) const { 
	    return m_exit_code; 
	  }

          //! Gets the job failure reason
          std::string get_failure_reason( void ) const { 
	    return m_failure_reason; 
	  }

	  //! Gets the unique cream job identifier
          std::string get_cream_jobid( void ) const { 
	    return m_cream_jobid; 
	  }
	  
	  //! Gets the COMPLETE unique cream job identifier
          std::string get_complete_cream_jobid( void ) const { 
	    return m_complete_cream_jobid; 
	  }

	  //! Gets the entire JDL of the job
          std::string get_jdl( void ) const { 
	    return m_jdl; 
	  }

	  //! Gets the CE identifier for the job (containing the endpoint)
          std::string get_ceid( void ) const { 
	    return m_ceid; 
	  }

	  //! Gets the endpoint of the cream service the job is submitted to
          std::string get_endpoint( void ) const { 
	    return m_endpoint; 
	  }

	  //! Gets the cream service URL this job is submitted to
          std::string get_creamurl( void ) const { 
	    return m_cream_address; 
	  }

	  //! Gets the cream delegation service URL used to delegate the user proxy certificate of this job
          std::string get_cream_delegurl( void ) const { 
	    return m_cream_deleg_address; 
	  }

	  //! Gets the path and file name of the user proxy certificate
          std::string get_user_proxy_certificate( void ) const { 
	    return m_user_proxyfile; 
	  }

          //! Gets the path and file name of the "better proxy" for the DN of the user owning this job
          //std::string getBetterProxy( void ) const;

	  //! Gets the number of job status changes which have been already logged to L&B
	  int get_num_logged_status_changes( void ) const { 
	    return m_num_logged_status_changes; 
	  }

          //! Sets the number of job status changes whcih have been already logged to L&B
          void set_numlogged_status_changes( int l ) { 
	    m_num_logged_status_changes = l; 
	  }

          //! Return the lease ID associated with this job
          std::string get_lease_id( void ) const { 
	    return m_lease_id; 
	  };

          //! Set the lease ID associated with this job
          void set_lease_id( const std::string& lease_id ) { 
	    m_lease_id = lease_id; 
	  };

          //! Gets the worker node on which the job is being execute (empty string if no worker node has been set)
          std::string get_worker_node( void ) const { 
	    return m_worker_node; 
	  }

          //! Sets the worker node on which the job is being executed
          void set_worker_node( const std::string& w_node ) { 
	    m_worker_node = w_node; 
	  }

          //! Gets the time we last got information about this job
          time_t get_last_seen( void ) const { 
	    return m_last_seen; 
	  }

          //! Gets the WN sequence code
          std::string get_wn_sequence_code( void ) const { 
	    return m_wn_sequence_code; 
	  }

          //! Gets the sequence code
          std::string get_sequence_code( void ) const { 
	    return m_sequence_code; 
	  }

          //! Gets the delegation id
          std::string get_delegation_id( void ) const { 
	    return m_delegation_id; 
	  }

	  //! Gets the last modification time of the user proxy cert file

	  int    get_status_poll_retry_count( void ) const { 
	    return m_statusPollRetryCount; 
	  }

	  void   inc_status_poll_retry_count( void ) { 
	    m_statusPollRetryCount++; 
	  }

	  void   reset_status_poll_retry_count( void ) { 
	    m_statusPollRetryCount=0; 
	  }

	  std::string get_user_dn( void ) const { 
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
	    return m_is_killed_byice; 
	  }


          /**
           * This method is used to indicate that this job has been
           * killed by ICE, instead of by the user. This is used by
           * logging the appropriate event when the job terminates.
           */
          void set_killed_by_ice( void ) { 
	    m_is_killed_byice = true; 
	  }

          time_t get_last_empty_notification( void ) const { 
	    return m_last_empty_notification; 
	  };
          void set_last_empty_notification_time( time_t t ) { 
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
          glite::ce::cream_client_api::job_statuses::job_status get_status(void) const { 
	    return m_status; 
	  }

          glite::ce::cream_client_api::job_statuses::job_status get_prev_status(void) const { 
	    return m_prev_status; 
	  }

	  bool is_proxy_renewable() const { 
	    return m_proxy_renew; 
	  }

	  std::string get_myproxy_address() const { 
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
/*
	  template<class Archive> void serialize(Archive & ar, const unsigned int version) throw(SerializeException&)
	  {
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
	  }*/
	};
      }
    }
  }
}

#endif
