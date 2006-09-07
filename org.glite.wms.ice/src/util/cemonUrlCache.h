#ifndef _GLITE_WMS_ICE_UTIL_CEMONURLCACHE_H__
#define _GLITE_WMS_ICE_UTIL_CEMONURLCACHE_H__

//#include <map>
#include <string>
#include <set>
#include "boost/thread/recursive_mutex.hpp"

namespace log4cpp {
    class Category;
};

//class CEInfo;

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	class iceConfManager;
	class subscriptionManager;
	
	
	class cemonUrlCache {
	  
	  
	  
	  std::set<std::string>               m_cemonURL;
	  std::set<std::string>               m_DN;
	  std::map<std::string, std::string>  mappingCreamCemon;
	  std::map<std::string, std::string>  mappingCemonDN;
	  
	  iceConfManager                           *m_conf;  
	  subscriptionManager                      *m_subMgr;
	  static cemonUrlCache                     *s_instance;
	  log4cpp::Category                        *m_log_dev;
	  //CEInfo			           *ceInfo;
	  //glite::ce::cream_client_api::CreamProxy  *m_creamProxy;
	  
	 protected:
	  cemonUrlCache() throw();
	  ~cemonUrlCache() throw() {}
	  
	 public:
	  static cemonUrlCache* getInstance();
	  
          std::string     getCEMonURL(const std::string& creamURL);
          bool            getCEMonDN( const std::string& cemonURL,std::string& DN);
	  bool            hasCEMon( const std::string& ) const;
	  bool            isAuthorized( const std::string& ) const;
	  void            insertCEMon( const std::string& cemon ) 
	  {
	    m_cemonURL.insert( cemon );
	  }
	  void            insertDN( const std::string& DN)
	  {
	    m_DN.insert( DN );
	  }
	  
	  void            getCEMons( std::vector<std::string>& );
          void            getCEMons( std::set<std::string>& );
	  
	  static boost::recursive_mutex  mutex;
 	};
	
      }
    }
  }
}

#endif
