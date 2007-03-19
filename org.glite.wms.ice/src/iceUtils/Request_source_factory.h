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
 * Request Factory class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#ifndef GLITE_WMS_ICE_REQUEST_SOURCE_FACTORY_H
#define GLITE_WMS_ICE_REQUEST_SOURCE_FACTORY_H

#include <string>

namespace glite {
namespace wms {
namespace ice {
namespace util {

    class Request_source;

    class Request_source_factory {
    public:
        ~Request_source_factory( ) { };
        
        /**
         * Builds a Request_source object for the Workload Manager
         * (WM), depending on the input type (filelist or jobdir). The
         * caller OWNS the returned object, and is responsible for
         * freeing it. This method looks for the "dispatcher_type"
         * configuration parameter for the WMS, and builds the
         * appropriate request source accordingly.
         *
         * @result The request_source object, or null if the
         * input_type is invalid or the object cannot be constructed
         * (due to invalid path names, for example).
         */
        static Request_source* make_source_input_wm( void );

        /**
         * Builds a Request_source object for ICE depending on the
         * input type (filelist or jobdir). The caller OWNS the
         * returned object, and is responsible for freeing it. This
         * method looks for the "InputType" configuration parameter
         * for ICE, and builds the appropriate request source
         * accordingly.
         *
         * @result The request_source object, or null if the
         * input_type is invalid or the object cannot be constructed
         * (due to invalid path names, for example).
         */
        static Request_source* make_source_input_ice( void );

    protected:
        Request_source_factory( ) { };
    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
