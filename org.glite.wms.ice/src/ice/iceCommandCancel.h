#ifndef __ICECOMMANDCANCEL_H__
#define __ICECOMMANDCANCEL_H__

#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "iceAbsCommand.h"
#include "ClassadSyntax_ex.h"
#include "classad_distribution.h"

namespace glite {
    namespace wms {
        namespace ice {

            class iceCommandCancel : public iceAbsCommand {
	
            public:
                iceCommandCancel( const std::string& request ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);

                virtual ~iceCommandCancel() {};

                void execute( glite::ce::cream_client_api::soap_proxy::CreamProxy* c );          
            protected:
                std::string _gridJobId;
            };
        }
    }
}

#endif
