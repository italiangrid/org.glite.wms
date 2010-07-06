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

#include "ice-core.h"
#include "iceUtils.h"
#include "GetJobsByDN.h"

#include <cstdlib>

using namespace glite::wms::ice;
using namespace std;
namespace cream_api = glite::ce::cream_client_api;


void db::GetJobsByDN::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd;
  sqlcmd += "SELECT " + util::CreamJob::get_query_fields() 
	 + " FROM jobs WHERE " + util::CreamJob::user_dn_field() 
         + "="
	 + Ice::get_tmp_name()
	 + m_dn
	 + Ice::get_tmp_name() + ";";
    
  do_query( db, sqlcmd, glite::wms::ice::util::utilities::fetch_jobs_callback, m_result );

}
