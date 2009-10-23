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
 * Jobdir request class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#ifndef GLITE_WMS_ICE_REQUEST_JOBDIR_H
#define GLITE_WMS_ICE_REQUEST_JOBDIR_H

#include "ice_timer.h"
#include "Request.h"
#include <string>
#include <boost/filesystem/operations.hpp>

namespace glite {
namespace wms {
namespace ice {
namespace util {

    class Request_jobdir : public Request {
    protected:
        boost::filesystem::path m_old_path; 
        std::string m_request;
    public:
        /**
         * 
         */
        Request_jobdir( boost::filesystem::path old_path );

        virtual ~Request_jobdir( ) { };

        const std::string& to_string( void ) {
#ifdef ICE_PROFILE
	  ice_timer timer("Request_jobdir::to_string");
#endif
            return m_request;
        };

        boost::filesystem::path get_path( void ) const {
#ifdef ICE_PROFILE
  ice_timer timer("Request_jobdir::get_path");
#endif
            return m_old_path;
        }

    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
