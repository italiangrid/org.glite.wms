#ifndef __GLITE_WMS_ICE_UTIL_EVENTSTATUSPOLLER_H__
#define __GLITE_WMS_ICE_UTIL_EVENTSTATUSPOLLER_H__

#undef soapStub_H
#include "glite/ce/cream-client-api-c/JobStatusList.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "eventStatusPoller_ex.h"
#include "iceEventLogger.h"
#include "iceThread.h"
#include "boost/scoped_ptr.hpp"

namespace log4cpp {
    class Category;
};

namespace glite {
  namespace wms {
    namespace ice {

      class ice;

      namespace util {

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

	  int delay;
	  std::vector< std::string > empty;
	  std::vector< glite::ce::cream_client_api::soap_proxy::JobStatusList* > _jobstatuslist;
	  ice* iceManager;
	  void purgeJobs(const std::vector<std::string>&);

	  boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > creamClient;
	  std::vector<std::string> oneJobToQuery;
	  std::vector<std::string> oneJobToPurge;

	  bool getStatus(void);
	  void updateJobCache(void);
	  void checkJobs(void);

	  log4cpp::Category* log_dev;
          iceEventLogger* _ev_logger;
          jobCache* cache;

	  //protected:

	  eventStatusPoller( const eventStatusPoller& ) {}

	public:
	  //! eventStatusPoller constructor
	  /*!
	    Creates a eventStatusPoller object
	    \param iceManager is the ICE main object (see the ice class) that creates the poller thread
	    \param D is the delay (default 10 seconds) between two polls
	    \throw eventStatusPoller_ex& if the creation of the internal cream communication client failed
	    \sa ice
	  */
	  eventStatusPoller(
			    ice* iceManager, //! ownership of this pointer is not transferred
			    const int D=10
			    ) 
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
