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
 * ICE filelist request class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_FILELIST_REQUEST_H
#define GLITE_WMS_ICE_FILELIST_REQUEST_H

#include <string>
#include "glite/wms/common/utilities/FLExtractor.h"

namespace glite {
namespace wms {
namespace ice {

    class filelist_request {
    protected:
        glite::wms::common::utilities::FLExtractor<std::string>::iterator m_pos;
    public:
        /**
         * Creates a new request. 
         *
         * @param pos the filelist iterator to the request
         */
        filelist_request( glite::wms::common::utilities::FLExtractor<std::string>::iterator pos ) : 
            m_pos( pos ) 
            { }

        virtual ~filelist_request( ) { };

        /**
         * Returns the (string) representation of the request.  NOTE:
         * If the remove_request() method has been called, the
         * behavior of get_request() is undefined.
         */
        const std::string& get_request( void ) const {
            return *m_pos;
        };

        /**
         * Returns the iterator to the request;
         */
        glite::wms::common::utilities::FLExtractor<std::string>::iterator get_iterator( void ) const {
            return m_pos;
        }

    };

} // namespace ice
} // namespace wms
} // namespace glite

#endif
