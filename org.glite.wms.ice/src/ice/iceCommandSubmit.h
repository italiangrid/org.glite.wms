#ifndef __ICECOMMANDSUBMIT_H__
#define __ICECOMMANDSUBMIT_H__

#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "iceAbsCommand.h"
#include "ClassadSyntax_ex.h"
#include "classad_distribution.h"

namespace glite {
    namespace wms {
        namespace ice {

            class iceCommandSubmit : public iceAbsCommand {
	
            public:
                iceCommandSubmit( const std::string& request ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);

                virtual ~iceCommandSubmit() {};

                virtual void execute( glite::ce::cream_client_api::soap_proxy::CreamProxy* c );          

                std::string _jdl;
                std::string _certfile;
                std::string _gridJobId;
            };
        }
    }
}

#endif
