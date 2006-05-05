
#ifndef _GLITE_WMS_ICE_UTIL_SUBSCRIPTIONMGR_H__
#define _GLITE_WMS_ICE_UTIL_SUBSCRIPTIONMGR_H__

#include <string>
#include <vector>
#include <ctime>

#include "glite/ce/monitor-client-api-c/CESubscription.h"
#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
#include "boost/thread/recursive_mutex.hpp"

/*class CESubscription;
class CESubscriptionMgr;
class Topic;
class Policy;*/

namespace log4cpp { class Category; }

namespace glite {
namespace wms {
namespace ice {
namespace util {

  class iceConfManager;

  class subscriptionManager {

    CESubscription m_ceS;
    CESubscriptionMgr m_ceSMgr;
    Topic m_T;
    Policy m_P;
    iceConfManager* m_conf;
    log4cpp::Category *m_log_dev;
    bool m_valid;
    static subscriptionManager* s_instance;
    std::string m_myname;
    std::string m_lastSubscriptionID;
    struct tm m_Time;
    char m_aT[256];
    time_t tp;
    std::vector<Subscription> m_vec;

   protected:
    subscriptionManager();
    virtual ~subscriptionManager() {}


   public:

    static      subscriptionManager* getInstance();

    bool        subscribe(const std::string& url);

    bool 	updateSubscription(const std::string& url,
    				   const std::string& ID,
				   std::string& newID
				   );

    bool        subscribedTo(const std::string& url);

    bool        isValid( void ) const { return m_valid; }
    void        list(const std::string& url, std::vector<Subscription>&)
                  throw(std::exception&);
    std::string getLastSubscriptionID() const { return m_lastSubscriptionID; }

    static boost::recursive_mutex mutex;
  };

}
}
}
};

#endif
