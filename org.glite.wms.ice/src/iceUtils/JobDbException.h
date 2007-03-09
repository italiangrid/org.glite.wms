#ifndef __GLITE_WMS_ICE_UTIL_JOBDBEX_H__
#define __GLITE_WMS_ICE_UTIL_JOBDBEX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
        
        //! An exception for all errors that relate to BDB operation
        class JobDbException : public std::exception {
          
          std::string cause;
          
        public:
          //! JobDbException constructor
          /**!
            Creates a JobDbException object
            \param _cause the cause of the error
          */
          JobDbException(const std::string& _cause) throw() : cause(_cause) {};
          virtual ~JobDbException() throw() {}
          //! Gets the cause of the error
          const char* what() const throw() { return cause.c_str(); }
          
        };
      }
    }
  }
}

#endif
