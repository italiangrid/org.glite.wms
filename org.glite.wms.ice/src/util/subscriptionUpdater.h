#ifndef __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__
#define __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__

#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
#include "iceConfManager.h"
#include <string>
#include <vector>

/*
namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

       class iceConfManager;

       }
       }
       }
       };
       */

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class subscriptionUpdater {
		CESubscriptionMgr subMgr;
		bool end;
		std::string proxyfile;
		iceConfManager* conf;

		public:
		subscriptionUpdater(const std::string& cert) : subMgr(),
			end(false),
			proxyfile(cert),
			conf(glite::wms::ice::util::iceConfManager::getInstance()) {}

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
