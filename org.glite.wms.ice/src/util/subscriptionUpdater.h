#ifndef __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__
#define __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__

#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
#include "iceConfManager.h"
#include <string>
#include <vector>

/*namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

       class iceConfManager;

       }
       }
       }
       };*/

class Topic;
class Policy;

namespace log4cpp {
class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class subscriptionUpdater {
		CESubscriptionMgr subMgr;
		bool end;
		std::string proxyfile;
		iceConfManager* conf;
		Topic* T;
		Policy* P;
		log4cpp::Category *log_dev;

	  public:
		subscriptionUpdater(const std::string& cert);

		virtual ~subscriptionUpdater() {}

		virtual void operator()();
	  	virtual void stop() { end=true; }
		void renewSubscriptions(const std::vector<Subscription>&);
		void retrieveCEURLs(std::vector<std::string>&);
	};

      }
      }
      }
      }

#endif
