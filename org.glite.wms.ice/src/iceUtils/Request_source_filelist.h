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
 * 
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#ifndef ICE_REQUEST_SOURCE_FILELIST_H
#define ICE_REQUEST_SOURCE_FILELIST_H

#include "glite/wms/common/utilities/FLExtractor.h"

#include "Request_source.h"

namespace glite {
namespace wms {
namespace ice {
namespace util {

    class Request_source_filelist : public Request_source {
    public:
        Request_source_filelist( const std::string& fl_name );

        ~Request_source_filelist( );

        /**
         *
         */
        std::list<Request*> get_requests( );

        /**
         *
         */
        void remove_request( Request* req );
        
        /**
         *
         */
        void put_request( const std::string& ad );
    protected:
        typedef glite::wms::common::utilities::FLExtractor<std::string>::iterator FLEit;

        const std::string m_fl_name;
        glite::wms::common::utilities::FileList<std::string> m_filelist;
        glite::wms::common::utilities::FLExtractor<std::string> m_filelist_extractor;
    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
