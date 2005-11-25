#ifndef __ICEABSCOMMAND_H__
#define __ICEABSCOMMAND_H__

#include "ClassadSyntax_ex.h"
#include "classad_distribution.h"
#include "JobRequest_ex.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"

namespace glite {
    namespace wms {
        namespace ice {

            class iceAbsCommand {
	
            public:

                virtual ~iceAbsCommand() { };

                /**
                 * Executes the command. 
                 *
                 * @throw an iceCommandFatal_ex if the command is to
                 * be considered permanently failed;
                 * @throw an iceCommandTransient_ex if the command failed
                 * but could be tried again and succeed.
                 */
                virtual void execute( void ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& ) = 0;
          
            protected:

                iceAbsCommand( ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&) {};

            };
        }
    }
}

#endif
