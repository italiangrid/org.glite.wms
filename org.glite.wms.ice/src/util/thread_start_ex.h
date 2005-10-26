
#ifndef __THSTART_EX_H__
#define __THSTART_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class thread_start_ex : public std::exception {
	  
	  std::string cause;
	  
	public:
	  thread_start_ex(const std::string& _cause) throw() : cause(_cause) {};
	  virtual ~thread_start_ex() throw() {}
	  const char* what() const throw() { return cause.c_str(); }
	};
      }
    }
  }
}

#endif
