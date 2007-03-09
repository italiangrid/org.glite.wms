
#ifndef __GLITE_WMS_ICE_UTIL_JNLFILERO_EX_H__
#define __GLITE_WMS_ICE_UTIL_JNLFILERO_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	class jnlFileReadOnly_ex : public std::exception {
	  
	  std::string cause;
	  
	public:
	  jnlFileReadOnly_ex(const std::string& _cause) throw() : cause(_cause) {};
	  virtual ~jnlFileReadOnly_ex() throw() {}
	  const char* what() const throw() { return cause.c_str(); }
	  
	};
      }
    }
  }
}

#endif
