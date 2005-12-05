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
	    WM_Input_FileList, ICE_Input_FileList, CachePersistFile, LogFile;
	  int ListenerPort;
	  
	  static boost::recursive_mutex mutex;
	  
	protected:
	  iceConfManager()
	    throw (glite::wms::ice::util::ConfigurationManager_ex&);
	  
	public:
	  virtual ~iceConfManager() { delete(instance); }
	  static iceConfManager* getInstance() 
	    throw (glite::wms::ice::util::ConfigurationManager_ex&);
	  static void init(const std::string&);

	  std::string getHostProxyFile( void ) { return HostProxyFile; }
	  std::string getWMInputFile( void ) { return WM_Input_FileList; }
	  std::string getICEInputFile( void ) { return ICE_Input_FileList; }
	  std::string getCachePersistFile( void ) { return CachePersistFile; }
	  std::string getLogFile( void ) { return LogFile; }
	  int         getListenerPort( void ) { return ListenerPort; } 
	};
      }
    }
  }
}

#endif
