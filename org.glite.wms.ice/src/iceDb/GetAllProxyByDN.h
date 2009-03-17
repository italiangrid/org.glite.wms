/* Licensed under the Apache License, Version 2.0 (the "License"); 
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
 * Get all User Proxies
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_GET_ALLPRXBYDN_H
#define GLITE_WMS_ICE_GET_ALLPRXBYDN_H

#include "AbsDbOperation.h"
#include <set>
#include <string>

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     *
     */
    class GetAllProxyByDN : public AbsDbOperation {
    protected:
        std::set< std::string > m_result;
	bool                    m_proxy_renewable;
	std::string             m_userdn;
	bool                    m_all;

    public:
        GetAllProxyByDN( const std::string& userdn, const bool proxy_renewable );
	GetAllProxyByDN( const std::string& userdn );
        virtual void execute( sqlite3* db ) throw( DbOperationException& );

        /**
         * Return the list of jobs to poll
         */ 
        std::set< std::string > get_proxies( void ) const {
            return m_result;
        };

    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif
