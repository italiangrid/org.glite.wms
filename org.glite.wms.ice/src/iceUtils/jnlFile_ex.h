
#ifndef __GLITE_WMS_ICE_UTIL_JNLFILE_EX_H__
#define __GLITE_WMS_ICE_UTIL_JNLFILE_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	class jnlFile_ex : public std::exception {
	  
	  std::string _cause;
	  
	public:
	  jnlFile_ex(const std::string& cause) throw() : _cause(cause) {};
	  virtual ~jnlFile_ex() throw() {}
	  const char* what() const throw() { return _cause.c_str(); }
	  
	};
      }
    }
  }
}

#endif
