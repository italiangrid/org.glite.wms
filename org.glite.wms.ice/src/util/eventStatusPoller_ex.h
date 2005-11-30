
#ifndef __GLITE_WMS_ICE_UTIL_ESP_EX_H__
#define __GLITE_WMS_ICE_UTIL_ESP_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class eventStatusPoller_ex : public std::exception {
	  
	  std::string cause;
	  
	public:
	  eventStatusPoller_ex(const std::string& _cause) throw() : cause(_cause) {};
	  virtual ~eventStatusPoller_ex() throw() {}
	  const char* what() const throw() { return cause.c_str(); }
	  
	};
      }
    }
  }
}

#endif
