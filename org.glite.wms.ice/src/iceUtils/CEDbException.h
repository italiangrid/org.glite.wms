#ifndef __GLITE_WMS_ICE_UTIL_CEDBEX_H__
#define __GLITE_WMS_ICE_UTIL_CEDBEX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
        
        //! An exception for all errors that relate to BDB operation
        class CEDbException : public std::exception {
          
          std::string cause;
          
        public:
          //! JobDbException constructor
          /**!
            Creates a JobDbException object
            \param _cause the cause of the error
          */
          CEDbException(const std::string& _cause) throw() : cause(_cause) {};
          virtual ~CEDbException() throw() {}
          //! Gets the cause of the error
          const char* what() const throw() { return cause.c_str(); }
          
        };
      }
    }
  }
}

#endif
