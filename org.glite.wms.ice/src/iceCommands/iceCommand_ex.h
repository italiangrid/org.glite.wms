#ifndef __GLITE_WMS_ICE_ICECOMMAND_EX_H__
#define __GLITE_WMS_ICE_ICECOMMAND_EX_H__

#include <exception>
#include <string>

namespace glite {
    namespace wms {
        namespace ice {

            class iceCommand_ex : public std::exception {
            public:
                iceCommand_ex( const std::string& c ) throw() : _cause( c ) { }
                virtual ~iceCommand_ex() throw() { }
                const char* what() const throw() { return _cause.c_str(); }
            protected:
                std::string _cause;                
            };

        } // namespace ice
    } // namespace wms
} // namespace glite

#endif
