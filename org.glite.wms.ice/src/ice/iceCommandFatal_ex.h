#ifndef __ICECOMMANDFATAL_EX_H__
#define __ICECOMMANDFATAL_EX_H__

#include "iceCommand_ex.h"

namespace glite {
    namespace wms{
        namespace ice {

            class iceCommandFatal_ex : public iceCommand_ex {
            public:
                iceCommandFatal_ex( const std::string& c ) throw() : 
                    iceCommand_ex( c ) { }
                virtual ~iceCommandFatal_ex() throw() { }
            };


        } // namespace ice
    } // namespace wms
} // namespace glite

#endif
