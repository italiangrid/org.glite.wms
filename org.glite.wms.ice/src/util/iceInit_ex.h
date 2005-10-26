
#ifndef __ICEINIT_EX_H__
#define __ICEINIT_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class iceInit_ex : public std::exception {
	  
	  std::string cause;
	  
	public:
	  iceInit_ex(const std::string& _cause) throw() : cause(_cause) {};
	  virtual ~iceInit_ex() throw() {}
	  const char* what() const throw() { return cause.c_str(); }
	  
	};
      }
    }
  }
}

#endif
