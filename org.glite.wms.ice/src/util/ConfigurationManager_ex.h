
#ifndef __GLITE_WMS_ICE_UTIL_CONFMGR_EX_H__
#define __GLITE_WMS_ICE_UTIL_CONFMGR_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	//! \class ConfigurationManager_ex An exception for ConfigurationManager errors
	class ConfigurationManager_ex : public std::exception {
	  
	  std::string cause;
	  
	public:
	  //! ConfigurationManager_ex constructor
	  /**!
	    Creates a ConfigurationManager_ex object
	    \param _cause the cause of the error
	  */
	  ConfigurationManager_ex(const std::string& _cause) throw() : cause(_cause) {};
	  virtual ~ConfigurationManager_ex() throw() {}
	  //! Gets the cause of the error
	  const char* what() const throw() { return cause.c_str(); }
	  
	};
      }
    }
  }
}

#endif
