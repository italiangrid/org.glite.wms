#ifndef __ICECOMMANDFACTORY_H__
#define __ICECOMMANDFACTORY_H__

#include "ClassadSyntax_ex.h"
#include "JobRequest_ex.h"
#include <string>

namespace glite {
    namespace wms {
        namespace ice {

            class iceAbsCommand;

            class iceCommandFactory {
            public:
                virtual ~iceCommandFactory( ) { };
                
                static iceAbsCommand* mkCommand( const std::string& request ) throw( glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);
                
            protected:
                iceCommandFactory( ) { };
                
            };
        }
    }
}

#endif
