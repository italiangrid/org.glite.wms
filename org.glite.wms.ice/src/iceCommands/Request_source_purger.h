/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *    http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * ICE input queue request purger class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_REQUEST_SOURCE_PURGER_H
#define GLITE_WMS_ICE_REQUEST_SOURCE_PURGER_H

#include "ice-core.h"

namespace glite {
namespace wms {
namespace ice {

    namespace util {
        class Request;
    }

    class Request_source_purger {
    protected:
        glite::wms::ice::util::Request* m_req;
    public:
        Request_source_purger( glite::wms::ice::util::Request* req ) :
            m_req( req )
            { };

        /**
         * Actually removes the request from the request source
         */
        void operator()( void );

    };

} // namespace ice
} // namespace wms
} // namespace glite

#endif
