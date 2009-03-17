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
 * Get job status information by Complete Cream JOB ID
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_ICEDB_GETSTATUSINFOBYCID_H
#define GLITE_WMS_ICE_ICEDB_GETSTATUSINFOBYCID_H

#include "AbsDbOperation.h"
//#include "iceUtils/creamJob.h"
#include <string>
#include <boost/tuple/tuple.hpp>

//                   gridjobid  complete cid  num status changes   status
typedef boost::tuple<std::string,    std::string,       int,                 int>     StatusInfo;

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     *
     */
    class GetStatusInfoByCompleteCreamJobID : public AbsDbOperation { 
    protected:
        const std::string m_creamjobid;
        //glite::wms::ice::util::CreamJob m_theJob;
	StatusInfo m_info;
        bool m_found;
	
    public:
        GetStatusInfoByCompleteCreamJobID( const std::string& cid );
        virtual void execute( sqlite3* db ) throw( DbOperationException& );

        /**
         * Return the job which has been found. Returns a dummy
         * job if none has been found
         */
        StatusInfo get_info( void ) const { return m_info; };

        /**
         * True iff a job has been found
         */
        bool found( void ) const { return m_found; };
    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif
