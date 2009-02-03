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
 * Get all cream job ids of jobs to poll
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_GET_JOBS_TO_POLL_H
#define GLITE_WMS_ICE_GET_JOBS_TO_POLL_H

#include "AbsDbOperation.h"
#include "iceUtils/creamJob.h"
#include <list>
#include <string>

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     *
     */
    class GetJobsToPoll : public AbsDbOperation {
    protected:
        bool m_poll_all_jobs;
        std::list< std::string > m_result;
    public:
        GetJobsToPoll( bool poll_all_jobs );
        virtual void execute( sqlite3* db ) throw( DbOperationException );

        /**
         * Return the list of jobs to poll
         */ 
        std::list< std::string > get_jobs( void ) const {
            return m_result;
        };

    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif
