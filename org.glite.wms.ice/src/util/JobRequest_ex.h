
#ifndef __JOBREQ_EX_H__
#define __JOBREQ_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class JobRequest_ex : public std::exception {
	  
	  std::string cause;
	  
	public:
	  JobRequest_ex(const std::string& _cause) throw() : cause(_cause) {};
	  virtual ~JobRequest_ex() throw() {}
	  const char* what() const throw() { return cause.c_str(); }
	  
	};
      }
    }
  }
}

#endif
