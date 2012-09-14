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

#ifndef GLITE_WMS_ICE_ICEDB_UPDATEPROXYFIELDSBYDN_H
#define GLITE_WMS_ICE_ICEDB_UPDATEPROXYFIELDSBYDN_H

#include "AbsDbOperation.h"
#include <list>
#include <string>
#include <utility>

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     * This operation updates the information on an existing job.
     */
    class UpdateProxyFieldsByDN : public AbsDbOperation { 
    public:
        UpdateProxyFieldsByDN( const std::string& dn,
			       const std::string& myproxy,
			       const std::string& proxyfile,
			       const time_t       exptime,
			       const std::string& caller )
	  : AbsDbOperation( caller ),
	  m_dn( dn ),
	  m_myproxy( myproxy ),
	  m_proxyfile( proxyfile ),
	  m_exptime( exptime ) {}
	  
        virtual void execute( sqlite3* db ) throw( DbOperationException& );
	
    protected:
        const std::string m_dn;
	const std::string m_myproxy;
   	const std::string m_proxyfile;
	const time_t      m_exptime;
	//const std::list< std::pair<std::string, std::string> > m_nameval_list;
	
    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif
