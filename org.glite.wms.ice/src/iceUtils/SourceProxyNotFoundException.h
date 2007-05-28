#ifndef __GLITE_WMS_ICE_UTIL_PROXYNOTFOUND_H__
#define __GLITE_WMS_ICE_UTIL_PROXYNOTFOUND_H__

#include <exception>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
        
        //! An exception for all errors that relate to BDB operation
        class SourceProxyNotFoundException : public std::exception {
          
          std::string cause;
          
        public:
          //! SourceProxyNotFoundException constructor
          /**!
            Creates a SourceProxyNotFoundException object
            \param _cause the cause of the error
          */
          SourceProxyNotFoundException(const std::string& _cause) throw() : cause(_cause) {};
          virtual ~SourceProxyNotFoundException() throw() {}
          //! Gets the cause of the error
          const char* what() const throw() { return cause.c_str(); }
          
        };
      }
    }
  }
}

#endif
