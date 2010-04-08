/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */
 
#ifndef _GLITE_WMS_ICE_UTIL_SUBSCRIPTIONPROXY_H__
#define _GLITE_WMS_ICE_UTIL_SUBSCRIPTIONPROXY_H__

#include <string>
#include <vector>
#include <ctime>
#include <exception>

#include "glite/ce/monitor-client-api-c/CESubscription.h"
#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
#include "glite/ce/monitor-client-api-c/Action.h"
#include "glite/ce/monitor-client-api-c/Query.h"
#include "glite/ce/monitor-client-api-c/Dialect.h"
#include "boost/thread/recursive_mutex.hpp"

#include "iceSubscription.h"

namespace cemon_api = glite::ce::monitor_client_api::soap_proxy;

namespace log4cpp { class Category; }

namespace glite {
namespace wms {
namespace ice {

  class Ice;

namespace util {

  class iceConfManager;

  class subscriptionProxy {

    //ActionW	       m_A;
    //ActionW	       m_A2;
    //QueryW	       m_Q;
    //DialectW	      *m_D;
    //Topic              m_T;
    //Policy             m_P;
    iceConfManager    *m_conf;
    log4cpp::Category *m_log_dev;
    bool               m_valid;

    static subscriptionProxy* s_instance;

    std::string        m_myurl;
    //std::string        m_lastSubscriptionID;
    struct tm          m_Time;
    char               m_aT[256];
    time_t             m_tp;

    cemon_api::DialectW          *m_D;

    glite::wms::ice::Ice* m_theIce;

    //CESubscription     m_ceS;

   protected:
    subscriptionProxy() throw();
    virtual ~subscriptionProxy() throw();// {}
    static boost::recursive_mutex mutex;

   public:

    static      subscriptionProxy* getInstance() throw();

    //bool        subscribe(const std::string& url, Subscription& subs);
    
    bool        subscribe(const std::string& userProxy, const std::string& cemonUrl, iceSubscription& sub) throw();

//     bool 	updateSubscription(const std::string& url,
//     				   const std::string& ID,
// 				   std::string& newID,
// 				   
// 				   ) throw();
				   
    bool 	updateSubscription( const std::string& proxy,
    				    const std::string& url,
	    			    const std::string& ID,
				    std::string& newID
				   ) throw();

//    bool        subscribedTo(const std::string& url, Subscription& target) throw(std::exception&);
    
    bool        subscribedTo(const std::string& userProxy, const std::string& cemonUrl, iceSubscription& sub) throw(std::exception&);

    bool        isValid( void ) const throw() { return m_valid; }
    
//    void        list(const std::string& url, std::vector<Subscription>&)
//                  throw(std::exception&);
		  
    void	list(const std::string& userProxy, const std::string& cemonUrl, std::vector<cemon_api::Subscription>&) throw(std::exception&);
		  
    //std::string getLastSubscriptionID() const { return m_lastSubscriptionID; }

    //static boost::recursive_mutex mutex;
    
  };

}
}
}
};

#endif
