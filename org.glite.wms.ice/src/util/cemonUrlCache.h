#ifndef _GLITE_WMS_ICE_UTIL_CEMONURLCACHE_H__
#define _GLITE_WMS_ICE_UTIL_CEMONURLCACHE_H__

#include <map>
#include <string>
#include "boost/thread/recursive_mutex.hpp"

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	class cemonUrlCache {
	  std::map<std::string, std::string> cemon_cream_urlMap;
	  static cemonUrlCache* instance;
	  
	public:
	  static boost::recursive_mutex mutex;
	  static cemonUrlCache* getInstance();
	  void        putCEMonUrl(const std::string& creamURL, const std::string& cemonURL);
	  std::string getCEMonUrl(const std::string& creamURL);
	};
	
      }
    }
  }
}

#endif
