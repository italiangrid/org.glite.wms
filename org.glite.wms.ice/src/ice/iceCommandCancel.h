#ifndef __GLITE_WMS_ICE_ICECOMMANDCANCEL_H__
#define __GLITE_WMS_ICE_ICECOMMANDCANCEL_H__

#include "iceAbsCommand.h"
//#include "ClassadSyntax_ex.h"
//#include "classad_distribution.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
//#include "iceEventLogger.h"


namespace glite {
    namespace wms {
        namespace ice {

            namespace util {
                // Forward declaration
                class iceLBLogger;
            };

            class iceCommandCancel : public iceAbsCommand {
	
            public:
                iceCommandCancel( const std::string& request ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);

                virtual ~iceCommandCancel() {};

                void execute( ice* _ice ) throw ( iceCommandFatal_ex&, iceCommandTransient_ex& );          
            protected:
                std::string _gridJobId;
                std::string _sequence_code;
                log4cpp::Category* log_dev;
                util::iceLBLogger *_lb_logger;
            };
        }
    }
}

#endif
