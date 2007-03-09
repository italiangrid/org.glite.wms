
#ifndef __GLITE_WMS_ICE_UTIL_JOBREQ_EX_H__
#define __GLITE_WMS_ICE_UTIL_JOBREQ_EX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	//! An exception for all errors that relate to parsing the requests that ICE retrieves from the WM output file list (job submission and cancel)
	class JobRequest_ex : public std::exception {
	  
	  std::string cause;
	  
	public:
	  //! JobRequest_ex constructor
	  /**!
	    Creates a JobRequest_ex object
	    \param _cause the cause of the error
	  */
	  JobRequest_ex(const std::string& _cause) throw() : cause(_cause) {};
	  virtual ~JobRequest_ex() throw() {}
	  //! Gets the cause of the error
	  const char* what() const throw() { return cause.c_str(); }
	  
	};
      }
    }
  }
}

#endif
