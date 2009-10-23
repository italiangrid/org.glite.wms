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
 * DB operation used to create a new lease entry
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_ICEDB_CREATE_LEASE_H
#define GLITE_WMS_ICE_ICEDB_CREATE_LEASE_H

#include "AbsDbOperation.h"
#include <string>
#include <ctime>

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     * This operation creates a new job into the database. It does not
     * update an existing job with the same gridjobid.
     */
    class CreateLease : public AbsDbOperation { 

    public:
      CreateLease( const std::string& userdn,
		   const std::string& creamurl,
		   const time_t exptime,
		   const std::string& leaseid, 
		   const std::string& caller ) : AbsDbOperation( caller ),
						m_userdn( userdn ),
						m_creamurl( creamurl ),
						m_exptime( exptime ),
						m_leaseid( leaseid ) {}
			  
        virtual void execute( sqlite3* db ) throw( DbOperationException& );

    protected:
      const std::string       m_userdn;
      const std::string       m_creamurl;
      const time_t            m_exptime;
      const std::string       m_leaseid;
    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif
