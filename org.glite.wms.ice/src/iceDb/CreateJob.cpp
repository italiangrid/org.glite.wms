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

#include "CreateJob.h"
#include "ice-core.h"
#include "iceUtils.h"


using namespace glite::wms::ice;
using namespace std;

void db::CreateJob::execute( sqlite3* db ) throw ( DbOperationException& )
{

  string sqlcmd("");
  sqlcmd = string( "INSERT OR REPLACE INTO jobs (" )
    + util::CreamJob::get_query_fields( )
    + ") VALUES (" 
    + m_theJob.get_query_values( )
    + ")";

 //  + Ice::get_tmp_name()
//     + m_theJob.grid_jobid()
//     + Ice::get_tmp_name()
//     + ","
//     + Ice::get_tmp_name()
//     + m_theJob.cream_jobid() 
//     + Ice::get_tmp_name()
//     + "," 
//     + Ice::get_tmp_name()
//     + m_theJob.complete_cream_jobid() 
//     + Ice::get_tmp_name()
//     + ","
//     + Ice::get_tmp_name()
//     + m_theJob.jdl() 
//     + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.user_proxyfile() 
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.ceid() 
// 	 + Ice::get_tmp_name() 
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.endpoint() 
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.cream_address() 
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.cream_deleg_address() 
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.user_dn()
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.myproxy_address() 
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + ( m_theJob.proxy_renewable() ? "1" : "0" ) 
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.failure_reason()
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.sequence_code()
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.wn_sequence_code()
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + util::utilities::to_string((long int)m_theJob.prev_status() )
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + util::utilities::to_string((long int)m_theJob.status() )
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + util::utilities::to_string((long int)m_theJob.num_logged_status_changes() )
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.lease_id()
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + util::utilities::to_string((long int)m_theJob.status_poll_retry_count() )
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + util::utilities::to_string((long int)m_theJob.exit_code() )
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.worker_node() 
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + ( m_theJob.killed_byice() ? "1" : "0" ) 
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.delegation_id()
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + util::utilities::to_string(time(0) )
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + util::utilities::to_string(time(0) )
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + util::utilities::to_string(time(0) )
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + util::utilities::to_string((long int)m_theJob.isbproxy_time_end() )
// 	 + Ice::get_tmp_name()
// 	 + ","
// 	 + Ice::get_tmp_name()
// 	 + m_theJob.modified_jdl()
// 	 + Ice::get_tmp_name()
// 	 + ")";
  
  do_query( db, sqlcmd );
}
