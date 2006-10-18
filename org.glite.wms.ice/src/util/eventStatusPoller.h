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
 * Event status poller
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_EVENTSTATUSPOLLER_H
#define GLITE_WMS_ICE_UTIL_EVENTSTATUSPOLLER_H

#undef soapStub_H
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "eventStatusPoller_ex.h"
#include "iceThread.h"
#include "boost/scoped_ptr.hpp"
#include "boost/thread/recursive_mutex.hpp"

#include <list>

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
          class CreamJob;

	//! A job status poller
	/*! \class eventStatusPoller 
	  This class is conceived to run as a boost::thread (this is the
	  motivation of the implementation of the operator()() ).
	  Its main purpose is to get all status of all jobs ICE has submitted (and that are not finished yet) and whose status notification has not been received since long by the eventStatusListener.
	  When a job is finished the poller purges that job on the remote cream host.
	  The poller also resubmit failed or aborted job by calling the call back ice::doOnJobFailure
	  \sa ice
	*/
	class eventStatusPoller : public iceThread {

	  int m_delay;
	  Ice* m_iceManager;
	  boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_creamClient;
	  log4cpp::Category* m_log_dev;
          iceLBLogger* m_lb_logger;
          jobCache* m_cache;
          time_t m_threshold;

	  void purgeJobs(const std::vector< std::string >& );

          /**
           * Gets the list of jobs to poll. The list contains all jobs
           * in the cache whose "oldness" is greater than the threshold
           * defined in ICE configuration file.
           */ 
          std::list< CreamJob > get_jobs_to_poll( void );

          /**
           * Actually send JobInfo requests for jobs in the list
           * passed as parameter. If, for some job(s), the CREAM
           * service reports a "job unknown" exception, that job is
           * also removed from the job cache.
           *
           * @param job_list the list of CreamJobs whose status has to
           * be checked;
           *
           * @result the list of soap_proxy::JobInfo structures for the
           * jobs which were polled succesfully.
           */
	  std::list< glite::ce::cream_client_api::soap_proxy::JobInfo > check_jobs( const std::list< CreamJob > & job_list );


          /**
           * Handle a job which was not accessible through the
           * "jobInfo" method. We try a number of times before giving
           * up and removing the job from the jobCache.
           *
           * @param job the Job which ICE was unable to access. If the
           * job is not found in the job cache, this method does
           * nothing
           */
          void handle_unreachable_job( const std::string& cream_job_id );

          /**
           * Updates the status informations for all jobs in the list
           * l.
           *
           * @param l the list of job status informations (this list is
           * typically the resout of the scanJobs() method call).
           */
	  void updateJobCache( const std::list< glite::ce::cream_client_api::soap_proxy::JobInfo >& l );

          /**
           * Updates the cache with the job status changes (for a single job)
           * contained in s
           *
           * @param s the StatusInfo object from which job informations are updated
           */
          void update_single_job( const glite::ce::cream_client_api::soap_proxy::JobInfo& s );

          /**
           * Prevents copying
           */
	  eventStatusPoller( const eventStatusPoller& ) { };

	public:

	  static boost::recursive_mutex mutexJobStatusPoll;

	  //! eventStatusPoller constructor
	  /*!
	    Creates a eventStatusPoller object
	    \param iceManager is the ICE main object (see the ice class) that creates the poller thread. Ownership of this pointer is not transferred
	    \param D is the delay (default 10 seconds) between two polls
	    \throw eventStatusPoller_ex& if the creation of the internal cream communication client failed
	    \sa ice
	  */
	  eventStatusPoller( Ice* iceManager, int d=10 ) 
	    throw(glite::wms::ice::util::eventStatusPoller_ex&, glite::wms::ice::util::ConfigurationManager_ex&);
	  
	  virtual ~eventStatusPoller();

	  //! Main poller loop (inherited from iceThread)
          virtual void body( void );

	};

      }
    }
  }
}

#endif
