/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */

#ifndef GLITE_WMS_ICE_ICECOMMANDCANCEL_H
#define GLITE_WMS_ICE_ICECOMMANDCANCEL_H

#include <string>

#include "iceAbsCommand.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
//#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include <boost/scoped_ptr.hpp>

namespace glite {
namespace wms {
namespace ice {

    namespace util {
        class iceLBLogger;                 // Forward declaration
        class Request;
    };

    class iceCommandCancel : public iceAbsCommand {
        
//        boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_theProxy;
        
    public:
        //iceCommandCancel( glite::ce::cream_client_api::soap_proxy::CreamProxy*, util::Request* request ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);
	iceCommandCancel( util::Request* request ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);
        
        virtual ~iceCommandCancel() { }
        
        void execute( const std::string& ) throw ( iceCommandFatal_ex&, iceCommandTransient_ex& );          
        std::string get_grid_job_id( void ) const { return m_gridJobId; };
        
    protected:
        
        std::string m_gridJobId;
        std::string m_sequence_code;
        log4cpp::Category* m_log_dev;
        util::iceLBLogger *m_lb_logger;
        util::Request* m_request;
    };

} // namespace ice
} // namespace wms
} // namespace glite

#endif
