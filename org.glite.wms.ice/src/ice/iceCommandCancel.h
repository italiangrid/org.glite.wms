#ifndef __ICECOMMANDCANCEL_H__
#define __ICECOMMANDCANCEL_H__

#include "iceAbsCommand.h"
#include "ClassadSyntax_ex.h"
#include "classad_distribution.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"

namespace glite {
    namespace wms {
        namespace ice {

            class iceCommandCancel : public iceAbsCommand {
	
            public:
                iceCommandCancel( const std::string& request ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);

                virtual ~iceCommandCancel() {};

                void execute( ) throw ( iceCommandFatal_ex&, iceCommandTransient_ex& );          
            protected:
                std::string _gridJobId;
            };
        }
    }
}

#endif
