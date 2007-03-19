/**
 * This is a wrapper of the wms's configurator. The motivation of this wrapper
 * is that apparently wms configurator doesn't allow to modify conf params at run time
 */

#ifndef GLITE_WMS_ICE_UTIL_ICECONFMANAGER_H
#define GLITE_WMS_ICE_UTIL_ICECONFMANAGER_H

#include <string>
#include <ctime>
#include "ConfigurationManager_ex.h"
#include <boost/scoped_ptr.hpp>
#include "glite/wms/common/configuration/Configuration.h"

// #include "boost/thread/recursive_mutex.hpp"

namespace glite {
  namespace wms {
      namespace common {
          namespace configuration {
              class Configuration;
          }
      }

    namespace ice {
      namespace util {

	class iceConfManager {
	  
	  static iceConfManager* s_instance;
	  static std::string s_conf_file;
	  static bool s_initialized;
          
          boost::scoped_ptr< glite::wms::common::configuration::Configuration > m_configuration;

	protected:
	  iceConfManager()
	    throw (glite::wms::ice::util::ConfigurationManager_ex&);
	  
	public:
	  
	  virtual ~iceConfManager();
          
	  static iceConfManager* getInstance() 
	    throw (glite::wms::ice::util::ConfigurationManager_ex&);
	  static void init(const std::string&);

          glite::wms::common::configuration::Configuration* getConfiguration() { return m_configuration.get(); }
	};
      }
    }
  }
}

#endif
