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
 * Get all User DN
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_GET_CREAMURLUSERDN_H
#define GLITE_WMS_ICE_GET_CREAMURLUSERDN_H

#include "AbsDbOperation.h"
#include <list>
#include <utility>
#include <string>

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     *
     */
    class GetCreamURLUserDN : public AbsDbOperation {
    protected:
        std::list< std::pair<std::string, std::string> > m_result;

    public:
        GetCreamURLUserDN( const std::string& caller ) : AbsDbOperation( caller ) {}

        virtual void execute( sqlite3* db ) throw( DbOperationException& );

        /**
         * Return the list of jobs to poll
         */ 
        std::list< std::pair<std::string, std::string> > get( void ) const {
            return m_result;
        };

    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif
