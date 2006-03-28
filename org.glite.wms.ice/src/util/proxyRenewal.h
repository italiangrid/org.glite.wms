#ifndef __GLITE_WMS_ICE_UTIL_PROXYRENEWAL_H__
#define __GLITE_WMS_ICE_UTIL_PROXYRENEWAL_H__

#include "iceThread.h"

namespace log4cpp {    
  class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class proxyRenewal : public iceThread {

	  void checkProxies();
	  log4cpp::Category* log_dev;

	 public: 
  	  proxyRenewal();
	  virtual void body( void );
	};

      }
     }
   }
}

#endif
