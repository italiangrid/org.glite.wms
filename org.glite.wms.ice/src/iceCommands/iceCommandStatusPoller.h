/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *    http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * ICE status poller
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_ICECOMMANDSTATUSPOLLER_H
#define GLITE_WMS_ICE_ICECOMMANDSTATUSPOLLER_H

#undef soapStub_H

#include "iceAbsCommand.h"
#include "creamJob.h"
#include "glite/ce/cream-client-api-c/JobInfo.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"

#include <boost/scoped_ptr.hpp>

#include<vector>
#include<string>
#include<list>

namespace log4cpp {
  class Category;
};

namespace glite {
  namespace wms {
    namespace ice {

      class Ice;

      namespace util {
        
        class iceLBLogger;
	class jobCache;

	class iceCommandStatusPoller : public iceAbsCommand {
	  boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_theProxy;
          log4cpp::Category *m_log_dev;
          glite::wms::ice::util::iceLBLogger* m_lb_logger;
          Ice* m_iceManager;
	  jobCache* m_cache;
	  time_t m_threshold;
          const unsigned int m_max_chunk_size; ///< maximum number of jobs which will be used in a cumulative request to CREAM
          time_t m_empty_threshold; ///< Threshold for empty notifications
	  bool m_poll_all_jobs;
	  
	  //void purgeJobs( const std::vector< std::string >& );

	  /**
           * Gets the list of jobs to poll. The list contains all jobs
           * in the cache whose "oldness" is greater than the threshold
           * defined in ICE configuration file.
           *
           * @return the list of Cream Job IDs for jobs to poll.
           */ 
	  void get_jobs_to_poll( std::list< glite::wms::ice::util::CreamJob >& ) throw();

          /**
           * Breaks the list of jobs into sublists of size at most max_size
           *
           * @param jobs the list of jobs
           *
           * @param max_size the max size of the returned sublists
           *
           * @return the list of chunks
           */
          //std::list< std::list< glite::wms::ice::util::CreamJob > > create_chunks( const std::list< glite::wms::ice::util::CreamJob >& jobs, unsigned int max_size );

	  /**
           * Updates the status informations for all jobs in the list
           * l.
           *
           * @param l the list of job status informations (this list is
           * typically the resout of the scanJobs() method call).
           */
          void updateJobCache( const std::list< glite::ce::cream_client_api::soap_proxy::JobInfo >& ) throw();

	  /**
           * Updates the cache with the job status changes (for a single job)
           * contained in s
           *
           * @param s the StatusInfo object from which job informations are updated
           */
          void update_single_job( const glite::ce::cream_client_api::soap_proxy::JobInfo& ) throw();

	  //void remove_unknown_jobs_from_cache(std::vector< const glite::ce::cream_client_api::soap_proxy::JobInfo >&) throw();

	  /**
           * Prevents copying
           */
          iceCommandStatusPoller( const iceCommandStatusPoller& ) : m_max_chunk_size( 0 ) { } // keep the compiler happy

          /**
           *
           */
	  std::list< glite::ce::cream_client_api::soap_proxy::JobInfo > check_multiple_jobs( const std::string& user_dn, const std::string& cream_url, const std::vector< std::string >& cream_job_ids ) throw();

          /**
           * This method removes jobs which have been reported as not found by
           * CREAM from the job cache. 
           *
           * @param all_jobs the vector of all CREAM job IDs which
           * were passed to the cumulative CREAM call. Upon succesful
           * completion (if no job is missing), we expect that
           * jobs_found contains exactly JobInfo structures for the
           * exact same job IDs in all_jobs.
           *
           * @param jobs_found the vector of JobInfo structures
           * returned by the cumulative CREAM call. This vector
           * contains *at most* one element for each CREAM job ID
           * in the all_jobs vector.
           */
	  void remove_unknown_jobs_from_cache(const std::vector<std::string>& all_jobs, const std::vector< glite::ce::cream_client_api::soap_proxy::JobInfo >& jobs_found ) throw();

	public:
	  //static boost::recursive_mutex mutexJobStatusPoll;
	  
          //! eventStatusPoller constructor
          /*!
            Creates a eventStatusPoller object
            \param iceManager is the ICE main object (see the ice class) that creates the poller thread. Ownership of this pointer is not trans
	    ferred
            \param D is the delay (default 10 seconds) between two polls
            \throw eventStatusPoller_ex& if the creation of the internal cream communication client failed
            \sa ice
          */
          iceCommandStatusPoller( Ice*, bool poll_all_jobs = false ); //throw(glite::wms::ice::util::eventStatusPoller_ex&, glite::wms::ice::util::ConfigurationManager_ex&);
          
          virtual ~iceCommandStatusPoller() throw() { } 

	  void execute( ) throw();
	  
	  std::string get_grid_job_id() const { return std::string(); }
	};
      }
    }
  }
}

#endif
