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

namespace log4cpp {
    class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
        
      class Ice;

      namespace util {

          class iceLBLogger; // forward declaration
          class jobCache; // forward declaration

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

	  void purgeJobs(const std::vector< std::string >& );

          /**
           * Fills vector l with job status informations for all
           * currently running jobs which did not send status change
           * notifications for a given threshold.
           *
           * @param l the resulting list of job states          
           */
	  void getStatus( std::vector< glite::ce::cream_client_api::soap_proxy::JobInfo >& l );

          /**
           * Updates the status informations for all jobs in the list
           * l.
           */
	  void updateJobCache( const std::vector< glite::ce::cream_client_api::soap_proxy::JobInfo >& l );

          /**
           * Updates the cache with the job status changes (for a single job)
           * contained in l.
           */
          void update_single_job( const std::vector< glite::ce::cream_client_api::soap_proxy::Status >& l );

          /**
           * Checks whether jobs in the list l need to be purged or
           * resubmitted. This method calls purgeJobs for those who
           * need to be purged.
           */
	  void checkJobs( const std::vector< glite::ce::cream_client_api::soap_proxy::JobInfo >& );

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
	    throw(glite::wms::ice::util::eventStatusPoller_ex&);
	  
	  virtual ~eventStatusPoller();

	  //! Main poller loop (inherited from iceThread)
          virtual void body( void );

	};

      }
    }
  }
}

#endif
