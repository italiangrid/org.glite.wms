#ifndef __GLITE_WMS_ICE_ICECOMMANDFATAL_EX_H__
#define __GLITE_WMS_ICE_ICECOMMANDFATAL_EX_H__

#include "iceCommand_ex.h"

namespace glite {
    namespace wms{
        namespace ice {

            /**
             * This class represents a fatal exception thrown during
             * the execution of ICE commands (submit or cancel).  A
             * fatal exception means that the command is permanently
             * failed, which means that submitting the command another
             * time will invariably result in another failure.
             */
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
