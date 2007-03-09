#ifndef __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__
#define __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__

//#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h" // needed to have the Subscription class definition
#include "iceConfManager.h"
#include "iceThread.h"
//#include <string>
//#include <set>
//#include <vector>
//#include "boost/scoped_ptr.hpp"

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

		//std::string m_proxyfile;
		//iceConfManager* m_conf;
		//log4cpp::Category *m_log_dev;
		//subscriptionManager *m_subMgr;
		int m_iteration_delay;
		//std::string m_myname;
		//bool m_valid;

        public:
		subscriptionUpdater( );

		virtual ~subscriptionUpdater() { }
		//bool isValid( void ) const { return m_valid; }
		virtual void body( void );
		
        //protected:
		//void renewSubscriptions(std::vector<Subscription>&);
		//void retrieveCEURLs(std::set<std::string>&);
	};
        
      }
    }
  }
}

#endif
