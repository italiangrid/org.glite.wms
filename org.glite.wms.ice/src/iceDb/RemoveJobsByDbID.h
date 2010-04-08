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


#ifndef GLITE_WMS_ICE_ICEDB_REMOVE_JOB_BYDBID_H
#define GLITE_WMS_ICE_ICEDB_REMOVE_JOB_BYDBID_H

#include "AbsDbOperation.h"

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     * This operation removes a job from the database.
     */
    class RemoveJobsByDbID : public AbsDbOperation { 
    public:
        RemoveJobsByDbID( const long long dbid, const std::string& caller ) : AbsDbOperation( caller ), m_dbid( dbid ) {}

        virtual void execute( sqlite3* db ) throw( DbOperationException& );
	
    protected:
        const long long  m_dbid;
    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif
