#ifndef __GLITE_WMS_ICE_UTIL_SEREX_H__
#define __GLITE_WMS_ICE_UTIL_SEREX_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
        
        //! An exception for all errors raised by the boost::i[o]archive
        class SerializeException : public std::exception {
          
          std::string cause;
          
        public:
          //! SerializeException constructor
          /**!
            Creates a SerializeException object
            \param _cause the cause of the error
          */
          SerializeException(const std::string& _cause = "") throw() : cause(_cause) {};
          virtual ~SerializeException() throw() {}
          //! Gets the cause of the error
          const char* what() const throw() { return cause.c_str(); }
          
        };
      }
    }
  }
}

#endif
