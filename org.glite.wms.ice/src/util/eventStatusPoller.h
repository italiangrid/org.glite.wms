#ifndef __GLITE_WMS_ICE_UTIL_EVENTSTATUSPOLLER_H__
#define __GLITE_WMS_ICE_UTIL_EVENTSTATUSPOLLER_H__

#include "glite/ce/cream-client-api-c/JobInfoList.h"
#include "eventStatusPoller_ex.h"

#define DELAY 10

namespace glite {
  namespace ce {
    namespace cream_client_api {
      namespace soap_proxy {
	class CreamProxy;
      }
    }
  }
};

namespace log4cpp {
  class Category;
};

namespace glite {
  namespace wms {
    namespace ice {

      class absice;

      namespace util {

	//! A job status poller
	/*! \class eventStatusPoller 
	  This class is conceived to run as a boost::thread (this is the
	  motivation of the implementation of the operator()() ).
	  Its main purpose is to get all status of all jobs ICE has submitted (and that are not finished yet) and whose status notification has not been received since long by the eventStatusListener.
	  When a job is finished the poller purges that job on the remote cream host.
	  The poller also resubmit failed or aborted job by calling the call back ice::doOnJobFailure
	  \sa ice
	*/
	class eventStatusPoller {

	  bool endpolling;
	  int delay;
	  std::vector<std::string> jobs_to_query;
	  std::vector<std::string> empty;
	  std::vector<glite::ce::cream_client_api::soap_proxy::JobInfoList*> 
	    _jobinfolist;
	  std::vector<std::string> url_pieces;
	  absice* iceManager;
	  void purgeJobs(const std::vector<std::string>&);

	  glite::ce::cream_client_api::soap_proxy::CreamProxy* creamClient;
	  std::vector<std::string> oneJobToQuery;
	  std::vector<std::string> oneJobToPurge;

	  bool getStatus(void);
	  void updateJobCache(void);
	  void checkJobs(void);

	  log4cpp::Category* log_dev;

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
			    absice* iceManager,
			    const int& D=DELAY
			    ) 
	    throw(glite::wms::ice::util::eventStatusPoller_ex&);

	  
	  virtual ~eventStatusPoller();

	  //! Main poller loop (needed by boost:thread)
	  virtual void operator()();

	  //! Stop the main poller loop and end the poller thread
	  virtual void stop() { endpolling=true; }
	};

      }
    }
  }
}

#endif
