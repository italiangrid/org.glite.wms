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
 * ICE filelist request purger class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_FILELIST_REQUEST_PURGER_H
#define GLITE_WMS_ICE_FILELIST_REQUEST_PURGER_H

#include "filelist_request.h"
#include "ice-core.h"

namespace glite {
namespace wms {
namespace ice {

    class filelist_request_purger {
    protected:
        filelist_request m_req;
    public:
        filelist_request_purger( const filelist_request& req ) :
            m_req( req )
            { };

        /**
         * Actually removes the request from the filelist
         */
        void operator()( void );

    };

} // namespace ice
} // namespace wms
} // namespace glite

#endif
