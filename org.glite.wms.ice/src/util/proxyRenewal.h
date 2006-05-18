#ifndef __GLITE_WMS_ICE_UTIL_PROXYRENEWAL_H__
#define __GLITE_WMS_ICE_UTIL_PROXYRENEWAL_H__

#include "iceThread.h"
#include "creamJob.h"
#include "boost/thread/recursive_mutex.hpp"
#include "boost/scoped_ptr.hpp"

namespace glite {
  namespace ce {
    namespace cream_client_api {
      namespace soap_proxy {
	class CreamProxy;
      }
    }
  }
};

namespace log4cpp {    
  class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

          class jobCache;

          class proxyRenewal : public iceThread {
          protected:

              log4cpp::Category* m_log_dev;
              jobCache *m_cache;
              boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_creamClient;
              time_t m_delay;

              void checkProxies();
              
          public: 
              proxyRenewal();
              virtual void body( void );
          };

      } 
    }
  }
}

#endif
