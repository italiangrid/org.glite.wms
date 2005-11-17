#ifndef __ICEABSCOMMAND_H__
#define __ICEABSCOMMAND_H__

//#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "ClassadSyntax_ex.h"
#include "classad_distribution.h"
#include "JobRequest_ex.h"
#include <string>

namespace glite {
    namespace wms {
        namespace ice {

            class iceAbsCommand {
	
            public:

                virtual ~iceAbsCommand() { };

                /**
                 * Executes the command. 
                 *
                 */
                virtual void execute( void ) = 0;
          
            protected:

                iceAbsCommand( ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&) {};

            };
        }
    }
}

#endif
