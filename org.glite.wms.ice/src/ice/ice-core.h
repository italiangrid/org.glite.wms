
#ifndef __GLITE_WMS_ICE_ICE_H__
#define __GLITE_WMS_ICE_ICE_H__

#include <string>
#include "abs-ice-core.h"
#include "ClassadSyntax_ex.h"
#include "iceInit_ex.h"
#include "eventStatusListener.h"
#include "subscriptionUpdater.h"
#include "eventStatusPoller.h"
#include "leaseUpdater.h"
#include "glite/wms/common/utilities/FLExtractor.h"

class jobRequest;

typedef glite::wms::common::utilities::FLExtractor<std::string>::iterator FLEit;

namespace boost {
  class thread;
};

namespace log4cpp {
  class Category;
};

namespace glite {
  namespace wms {
    namespace ice {

      enum iceSupportedCommand {
	jobsubmit,
	jobcancel
      };

      struct iceCommand {
	std::string gid_jobid;
	std::string cream_jobid;
	iceSupportedCommand command;
	std::string jdl;
      };

      class ice : public glite::wms::ice::absice {
/* 	glite::wms::ice::util::eventStatusListener* listener; */
/* 	glite::wms::ice::util::eventStatusPoller* poller; */
	bool status_listener_started;
	bool status_poller_started;
	std::string ns_filelist;
	std::string wm_filelist;
	boost::thread* listenerThread;
	boost::thread* pollerThread;
        boost::thread* updaterThread;
        boost::thread* lease_updaterThread;
	boost::shared_ptr<util::eventStatusPoller> poller;
	boost::shared_ptr<util::eventStatusListener> listener;
	boost::shared_ptr<util::subscriptionUpdater> subsUpdater;
        boost::shared_ptr<util::leaseUpdater> lease_updater;

	std::vector<FLEit> requests;
	glite::wms::common::utilities::FLExtractor<std::string> fle;
	glite::wms::common::utilities::FileList<std::string> flns;

	log4cpp::Category* log_dev;

      public:
	ice(const std::string& NS_FL,
	    const std::string& WM_FL)
	  throw(glite::wms::ice::iceInit_ex&);

	virtual ~ice();

	void clearRequests();
	void getNextRequests(std::vector<std::string>&);
	void removeRequest(const unsigned int&);
	void ungetRequest(const unsigned int&);
	void startListener(const int&);
	void startPoller(const int&);
	void startLeaseUpdater( void );
	void stopListener();
	void stopPoller();
/*	bool isSubscribedTo(const std::string& url) {
	  return listener->isSubscribedTo(url);
	}*/
	virtual void doOnJobFailure(const std::string&);

      }; // class ice

    } // namespace ice
  } // namespace ce
} // namespace glite

#endif
