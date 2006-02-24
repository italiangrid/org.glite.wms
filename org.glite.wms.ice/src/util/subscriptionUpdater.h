#ifndef __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__
#define __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__

#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h" // needed to have the Subscription class definition
#include "iceConfManager.h"
#include "iceThread.h"
#include <string>
#include <set>
#include <vector>
#include "boost/scoped_ptr.hpp"

class Topic;
class Policy;

namespace log4cpp {
    class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class subscriptionManager;

	class subscriptionUpdater : public iceThread {

		std::string proxyfile;
		iceConfManager* conf;
		log4cpp::Category *log_dev;
		subscriptionManager *subMgr;

        public:
		subscriptionUpdater(const std::string& cert);

		virtual ~subscriptionUpdater() {}

		virtual void body( void );
        protected:
		void renewSubscriptions(const std::vector<Subscription>&);
		void retrieveCEURLs(std::set<std::string>&);
	};
        
      }
    }
  }
}

#endif
