
#ifndef __GLITE_WMS_ICE_UTIL_CLASSAD_EX_H__
#define __GLITE_WMS_ICE_UTIL_CLASSAD_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	//! \class ClassadSyntax_ex An exception for classad parsing errors
	class ClassadSyntax_ex : public std::exception {
	  
	  std::string cause;
	  
	public:
	  //! ClassadSyntax_ex constructor
	  /**!
	    Creates a ClassadSyntax_ex object
	    \param _cause the cause of the error
	  */
	  ClassadSyntax_ex(const std::string& _cause) throw() : cause(_cause) {};
	  virtual ~ClassadSyntax_ex() throw() {}
	  //! Gets the cause of the error
	  const char* what() const throw() { return cause.c_str(); }
	  
	};
      }
    }
  }
}

#endif
