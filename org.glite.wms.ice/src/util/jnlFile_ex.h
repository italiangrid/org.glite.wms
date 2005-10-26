
#ifndef __JNLFILE_EX_H__
#define __JNLFILE_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	class jnlFile_ex : public std::exception {
	  
	  std::string cause;
	  
	public:
	  jnlFile_ex(const std::string& _cause) throw() : cause(_cause) {};
	  virtual ~jnlFile_ex() throw() {}
	  const char* what() const throw() { return cause.c_str(); }
	  
	};
      }
    }
  }
}

#endif
