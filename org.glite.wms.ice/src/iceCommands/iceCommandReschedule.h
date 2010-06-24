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

#ifndef GLITE_WMS_ICE_ICECOMMANDRESCHEDULE_H
#define GLITE_WMS_ICE_ICECOMMANDRESCHEDULE_H

#include "iceCommandSubmit.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include "CreamJob.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/AbsCreamProxy.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/monitor-client-api-c/CESubscription.h"
#include "glite/ce/monitor-client-api-c/Topic.h"
#include "glite/ce/monitor-client-api-c/Policy.h"

#include "ClassadSyntax_ex.h"
#include "classad_distribution.h"

#include <boost/scoped_ptr.hpp>

namespace glite {
namespace wms {

namespace common {
namespace configuration {
    class Configuration;
} // namespace configuration
} // namespace common

namespace ice {
     
// Forward declarations
namespace util {                
    class iceLBLogger;
    class Request;
}
     
  
 class iceCommandReschedule : public iceCommandSubmit {
     
 private:
     void  doSubscription( const glite::wms::ice::util::CreamJob& );
     
 public:
     iceCommandReschedule( util::Request* request, 
		           const util::CreamJob& aJob ) : iceCommandSubmit( request, aJob ) { };
     
     virtual ~iceCommandReschedule() { }
     
     /**
      * This method is invoked to execute this command.
      */
     virtual void execute( const std::string& ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& );
     
 protected:

};

} // namespace ice
} // namespace wms
} // namespace glite

#endif
