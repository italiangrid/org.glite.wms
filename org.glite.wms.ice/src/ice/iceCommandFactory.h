#ifndef __GLITE_WMS_ICE_ICECOMMANDFACTORY_H__
#define __GLITE_WMS_ICE_ICECOMMANDFACTORY_H__

#include "ClassadSyntax_ex.h"
#include "JobRequest_ex.h"
#include <string>

namespace glite {
    namespace wms {
        namespace ice {

            class iceAbsCommand;

            /**
             * This class is used to build an iceAbsCommand object
             * corresponding to the command serialized in the request
             * classad. This class implements the classic "factory"
             * design pattern.
             */
            class iceCommandFactory {
            public:
                virtual ~iceCommandFactory( ) { };
               
                /**
                 * Returns an object instance of iceAbsCommand class;
                 * the actual object class depends on the content of
                 * the "request" parameter. The caller owns the
                 * returned object, and is responsible for
                 * deallocating it.
                 *
                 * @param request the classad containing the
                 * serialized version of the command to be built.
                 *
                 * @return a dynamically allocated iceAbsCommand object.
                 */ 
                static iceAbsCommand* mkCommand( const std::string& request ) throw( glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);
                
            protected:
                iceCommandFactory( ) { };
                
            };
        }
    }
}

#endif
