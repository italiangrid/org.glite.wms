
#ifndef __GLITE_WMS_ICE_UTIL_ENF_EX_H__
#define __GLITE_WMS_ICE_UTIL_ENF_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class elementNotFound_ex : public std::exception {
	  
	  std::string cause;
	  
	public:
	  elementNotFound_ex(const std::string& _cause) throw() : cause(_cause) {};
	  virtual ~elementNotFound_ex() throw() {}
	  const char* what() const throw() { return cause.c_str(); }
	};
      }
    }
  }
}

#endif
