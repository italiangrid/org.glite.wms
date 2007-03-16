/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * youw may not use this file except in compliance with the License. 
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
 * Filelist request class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#ifndef GLITE_WMS_ICE_REQUEST_FILELIST_H
#define GLITE_WMS_ICE_REQUEST_FILELIST_H

#include "Request.h"
#include "glite/wms/common/utilities/FLExtractor.h"

namespace glite {
namespace wms {
namespace ice {
namespace util {

    class Request_filelist : public Request {
    protected:
        glite::wms::common::utilities::FLExtractor<std::string>::iterator m_pos;
    public:
        /**
         * 
         */
        Request_filelist( glite::wms::common::utilities::FLExtractor<std::string>::iterator pos ) : 
            m_pos( pos ) 
            { }

        virtual ~Request_filelist( ) { };

        const std::string& to_string( void ) {
            return *m_pos;
        };

        glite::wms::common::utilities::FLExtractor<std::string>::iterator get_iterator( void ) const {
            return m_pos;
        };

    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
