#ifndef __ICEABSCOMMAND_H__
#define __ICEABSCOMMAND_H__

#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "ClassadSyntax_ex.h"
#include "classad_distribution.h"
#include "JobRequest_ex.h"

namespace glite {
    namespace wms {
        namespace ice {

            class iceAbsCommand {
	
            public:

                virtual ~iceAbsCommand() { };

                virtual void execute( glite::ce::cream_client_api::soap_proxy::CreamProxy* c ) = 0;
          
            protected:

                iceAbsCommand( ) throw(glite::wms::ice::util::ClassadSyntax_ex&, util::JobRequest_ex&) {};

            };
        }
    }
}

#endif
