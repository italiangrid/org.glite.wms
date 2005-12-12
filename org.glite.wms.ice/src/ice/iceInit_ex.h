
#ifndef __GLITE_WMS_ICE_ICEINIT_EX_H__
#define __GLITE_WMS_ICE_ICEINIT_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {

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

#endif
