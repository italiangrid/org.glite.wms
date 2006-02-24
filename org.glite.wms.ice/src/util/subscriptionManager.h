
#ifndef _GLITE_WMS_ICE_UTIL_SUBSCRIPTIONMGR_H__
#define _GLITE_WMS_ICE_UTIL_SUBSCRIPTIONMGR_H__

#include <string>
#include <vector>

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

    CESubscription ceS;
    CESubscriptionMgr ceSMgr;
    Topic T;
    Policy P;
    iceConfManager* conf;
    log4cpp::Category *log_dev;
    bool valid;
    static subscriptionManager* instance;
    std::string myname;
    std::string lastSubscriptionID;

   protected:
    subscriptionManager();
    virtual ~subscriptionManager() {}
    void    list(const std::string& url, std::vector<Subscription>&) throw(std::exception&);

   public:

    static      subscriptionManager* getInstance();

    bool        subscribe(const std::string& url);

    bool        updateSubscription(const std::string& url,
    				   const std::string& ID
				   );

    bool        subscribedTo(const std::string& url);

    bool        isValid( void ) const { return valid; }

    std::string getLastSubscriptionID() const { return lastSubscriptionID; }

    static boost::recursive_mutex mutex;
  };

}
}
}
};

#endif
