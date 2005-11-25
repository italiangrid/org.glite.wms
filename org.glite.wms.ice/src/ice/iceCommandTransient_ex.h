#ifndef __ICECOMMANDTRANSIENT_EX_H__
#define __ICECOMMANDTRANSIENT_EX_H__

#include "iceCommand_ex.h"

namespace glite {
    namespace wms {
        namespace ice {

            /**
             * This class represents a transient exception thrown
             * during the execution of ICE commands (submit or
             * cancel).  A transient exception means that the command
             * failed, but resubmitting it another time <em>may</em>
             * result in a command success or failure.
             */
            class iceCommandTransient_ex : public iceCommand_ex {
            public:
                iceCommandTransient_ex( const std::string& c ) throw() : 
                    iceCommand_ex( c ) { }
                virtual ~iceCommandTransient_ex() throw() { }
            };


        } // namespace ice
    } // namespace wms
} // namespace glite

#endif
