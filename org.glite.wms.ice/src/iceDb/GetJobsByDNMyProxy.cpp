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
#include "GetJobsByDNMyProxy.h"

#include <cstdlib>

using namespace glite::wms::ice;
using namespace std;
namespace cream_api = glite::ce::cream_client_api;

// namespace { // begin local namespace
// 
//   // Local helper function: callback for sqlite
//   static int fetch_jobs_callback(void *param, int argc, char **argv, char **azColName){
//     
//     list<util::CreamJob>* jobs = (list<util::CreamJob>*)param;
//     
//     if( argv && argv[0] ) {
//       vector<string> fields;
//       for(int i = 0; i<=util::CreamJob::num_of_members()-1; ++i) {// a database record for a CreamJob has 26 fields, as you can see in Transaction.cpp, but we want to exlude the field "complete_creamjobid", as specified in the SELECT sql statement;
// 	if( argv[i] )
// 	  fields.push_back( argv[i] );
// 	else
// 	  fields.push_back( "" );
//       }
// 
//       CreamJob tmpJob(fields.at(0),
//                       fields.at(1),
//                       fields.at(2),
//                       fields.at(3),
//                       fields.at(4),
//                       fields.at(5),
//                       fields.at(6),
//                       fields.at(7),
//                       fields.at(8),
//                       fields.at(9),
//                       fields.at(10),
//                       fields.at(11),
//                       fields.at(12),
//                       (const api::job_statuses::job_status)atoi(fields.at(13).c_str()),
//                       (const api::job_statuses::job_status)atoi(fields.at(14).c_str()),
//                       strtoul(fields.at(15).c_str(), 0, 10),
//                       (time_t)strtoll(fields.at(16).c_str(), 0, 10),
//                       fields.at(17),
//                       strtoul(fields.at(18).c_str(), 0, 10),
//                       strtoul(fields.at(19).c_str(), 0, 10),
//                       fields.at(20),
//                       fields.at(21),
//                       (fields.at(22)=="1" ? true : false),
//                       (time_t)strtoll(fields.at(23).c_str(), 0, 10),
//                       (fields.at(24) == "1" ? true : false ),
//                       fields.at(25),
//                       (time_t)strtoll(fields.at(26).c_str(), 0, 10),
//                       fields.at(27),
//                       (time_t)strtoll(fields.at(28).c_str(), 0, 10),
//                       strtoull(fields.at(29).c_str(), 0, 10),
// 		      fields.at(30)
//                       );
//       tmpJob.reset_change_flags( );      
//       jobs->push_back( tmpJob );
//     }
// 
//     return 0;
//   }
//   
// } // end local namespace

void db::GetJobsByDNMyProxy::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd;
  sqlcmd += "SELECT " + util::CreamJob::get_query_fields() 
	 + " FROM jobs WHERE " + util::CreamJob::user_dn_field() 
         + "="
	 + Ice::get_tmp_name()
	 + m_dn
	 + Ice::get_tmp_name() 
	 + " AND " + util::CreamJob::myproxy_address_field() 
	 + "="
	 + Ice::get_tmp_name()
	 + m_myproxy
	 + Ice::get_tmp_name()
	 + ";";
    
  do_query( db, sqlcmd, glite::wms::ice::util::utilities::fetch_jobs_callback, m_result );

}
