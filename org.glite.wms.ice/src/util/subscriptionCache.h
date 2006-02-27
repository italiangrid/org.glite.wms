
#ifndef _GLITE_WMS_ICE_UTIL_SUBSCRIPTIONCACHE_H__
#define _GLITE_WMS_ICE_UTIL_SUBSCRIPTIONCACHE_H__

//#include <ext/hash_set>
#include <string>
#include <set>
#include "boost/thread/recursive_mutex.hpp"

namespace glite {
namespace wms {
namespace ice {
namespace util {

/*  struct eqstr
  {
    bool operator()(const char* s1, const char* s2) const
    {
      return strcmp(s1, s2) == 0;
    }
  };*/

  class subscriptionCache {
/*    hash_set<const char*, hash<const char*>, eqstr> cemons;
    hash_set<const char*, hash<const char*>, eqstr>::const_iterator it;*/
    std::set<std::string> cemons;
    std::set<std::string>::const_iterator it;
    //static boost::recursive_mutex mutex;
    static subscriptionCache* instance;
    

   protected:
    subscriptionCache() : cemons() {}

   public:
    static boost::recursive_mutex mutex;
    static subscriptionCache* getInstance();
    void insert(const std::string& s) { if(cemons.find(s) == cemons.end()) cemons.insert(s); }
    void remove(const std::string& s);
    bool has(const std::string&);
  };

}
}
}
}

#endif
