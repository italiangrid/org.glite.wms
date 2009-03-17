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
 * Get all grid job ids
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_GET_ALLGID_H
#define GLITE_WMS_ICE_GET_ALLGID_H

#include "AbsDbOperation.h"
#include <list>
#include <string>
#include "iceUtils/creamJob.h"

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     *
     */
    class GetAllJobs : public AbsDbOperation {
    protected:
        std::list< glite::wms::ice::util::CreamJob > m_result;
	const bool                                   m_only_active;

    public:
        GetAllJobs( const bool only_active = false );
        virtual void execute( sqlite3* db ) throw( DbOperationException& );

        /**
         * Return the list of jobs to poll
         */ 
        std::list< glite::wms::ice::util::CreamJob > get_jobs( void ) const {
            return m_result;
        };

    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif
