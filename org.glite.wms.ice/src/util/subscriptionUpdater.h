#ifndef __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__
#define __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__

#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
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

	class subscriptionUpdater : public iceThread {
		CESubscriptionMgr subMgr;
		std::string proxyfile;
		iceConfManager* conf;
		boost::scoped_ptr< Topic > T;
		boost::scoped_ptr< Policy > P;
		log4cpp::Category *log_dev;

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
