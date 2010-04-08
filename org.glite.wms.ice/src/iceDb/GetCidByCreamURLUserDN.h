/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */

#ifndef GLITE_WMS_ICE_GET_GETCIDCREAMURLUSERDN_H
#define GLITE_WMS_ICE_GET_GETCIDCREAMURLUSERDN_H

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
    class GetCidByCreamURLUserDN : public AbsDbOperation {
    protected:
        std::list< std::string > m_result;
	std::pair< std::string, std::string> m_creamurl_userdn;

    public:
        GetCidByCreamURLUserDN( const std::pair<std::string, std::string>& src, const std::string& caller )
	: AbsDbOperation( caller ), m_result(), m_creamurl_userdn( src ) {}

        virtual void execute( sqlite3* db ) throw( DbOperationException& );

        /**
         * Return the list of jobs to poll
         */ 
        std::list< std::string > get( void ) const {
            return m_result;
        };

    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif
