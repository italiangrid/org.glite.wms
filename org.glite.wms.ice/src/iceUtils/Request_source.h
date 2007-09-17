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
#ifndef ICE_REQUEST_SOURCE_H
#define ICE_REQUEST_SOURCE_H

#include <string>
#include <list>

namespace glite {
namespace wms {
namespace ice {
namespace util {

    class Request; // forward declaration

    class Request_source {
    protected:
        const std::string m_name; // the name of this request source
    public:
        virtual ~Request_source( ) { };

        /**
         * Get a list of (new) requests from this source.  The caller
         * OWNS the list of returned pointers to Request objects, so
         * he is responsible for freeing the memory.
         *
         * @return the list of new requests.
         */
        virtual std::list<Request*> get_requests( ) = 0;
	virtual Request* get_single_request( ) = 0;

        /**
         * Removes a request from this source. The callre retains
         * ownership of the parameter, so he _must_ deallocate the
         * request explicitly once it has been removed.
         *
         * @param req the request to be removed. If the request is not
         * valid, this method does nothing.
         */
        virtual void remove_request( Request* req ) = 0;

        /**
         * Adds a new string to this request source.
         *
         * @param ad the request to add to the request source.
         */
        virtual void put_request( const std::string& ad ) = 0;

        /**
         * Gets the name of this request source
         */
        std::string get_name( void ) const { return m_name; };
    protected:
        /**
         * Creates a new request source object.
         */
        Request_source( const std::string& name ) : m_name( name ) { };
    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
