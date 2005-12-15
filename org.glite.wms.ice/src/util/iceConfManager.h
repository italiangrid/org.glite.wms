#ifndef __GLITE_WMS_ICE_UTIL_ICECONFMANAGER_H__
#define __GLITE_WMS_ICE_UTIL_ICECONFMANAGER_H__

#include <string>
#include "ConfigurationManager_ex.h"
#include "boost/thread/recursive_mutex.hpp"

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class iceConfManager {
	  
	  static iceConfManager* instance;
	  static std::string conf_file;
	  static bool initialized;
	  std::string HostProxyFile, 
	    WM_Input_FileList, ICE_Input_FileList, CachePersistFile, LogFile,
	    creamurlprefix, creamurlpostfix, curldelegprefix,
	    curldelegpostfix, cemonurlprefix, cemonurlpostfix,
	    icetopic;
	  int ListenerPort, pollerdelay, subduration;
	  bool startpoller, startlistener;
	  
	  static boost::recursive_mutex mutex;
	  
	protected:
	  iceConfManager()
	    throw (glite::wms::ice::util::ConfigurationManager_ex&);
	  
	public:
	  virtual ~iceConfManager() { delete(instance); }
	  static iceConfManager* getInstance() 
	    throw (glite::wms::ice::util::ConfigurationManager_ex&);
	  static void init(const std::string&);

	  std::string getHostProxyFile( void ) const { return HostProxyFile; }
	  std::string getWMInputFile( void ) const { return WM_Input_FileList; }
	  std::string getICEInputFile( void ) const { return ICE_Input_FileList; }
	  std::string getCachePersistFile( void ) const { return CachePersistFile; }
	  std::string getLogFile( void ) const { return LogFile; }
	  int         getListenerPort( void ) const { return ListenerPort; }
	  bool        startPoller( void ) const { return startpoller; } 
	  bool        startListener( void ) const { return startlistener; }
	  std::string getCreamUrlPrefix( void ) const { return creamurlprefix; }
	  std::string getCreamUrlPostfix( void ) const { return creamurlpostfix; }
	  std::string getCreamUrlDelegationPrefix( void ) const { return curldelegprefix; }
	  std::string getCreamUrlDelegationPostfix( void ) const { return curldelegpostfix; }
	  std::string getCEMonUrlPrefix( void ) const { return cemonurlprefix; }
	  std::string getCEMonUrlPostfix( void ) const { return cemonurlpostfix; }
	  std::string getICETopic( void ) const { return icetopic; }
	  int         getPollerDelay( void ) const { return pollerdelay; }
	  int         getSubscriptionDuration( void ) const { return subduration; }
	};
      }
    }
  }
}

#endif
