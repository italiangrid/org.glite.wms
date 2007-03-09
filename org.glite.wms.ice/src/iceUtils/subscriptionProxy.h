/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE subscription proxy
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
 
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

namespace log4cpp { class Category; }

namespace glite {
namespace wms {
namespace ice {
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

    DialectW          *m_D;

    //CESubscription     m_ceS;

   protected:
    subscriptionProxy() throw();
    virtual ~subscriptionProxy() throw();// {}


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
		  
    void	list(const std::string& userProxy, const std::string& cemonUrl, std::vector<Subscription>&) throw(std::exception&);
		  
    //std::string getLastSubscriptionID() const { return m_lastSubscriptionID; }

    static boost::recursive_mutex mutex;
    
  };

}
}
}
};

#endif
