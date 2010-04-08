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

#include <sstream>

using namespace glite::wms::ice;
using namespace std;

void db::CreateJob::execute( sqlite3* db ) throw ( DbOperationException& )
{

//   string jdl( m_JDL );
//   boost::replace_all( jdl, "'", "''" );

//   string dn( m_theJob.get_user_dn() );
//   boost::replace_all( dn, "'", "''" );

//   string failreason( m_theJob.get_failure_reason() );
//   boost::replace_all( failreason, "'", "''" );
  
//   string prx( m_theJob.get_user_proxy_certificate() );
//   boost::replace_all( prx, "'", "''" );

//   string sc( m_theJob.get_sequence_code() );
//   boost::replace_all( sc, "'", "''" );

//   string wnsc( m_theJob.get_wn_sequence_code() );
//   boost::replace_all( wnsc, "'", "''" );

//   string lid( m_theJob.get_lease_id() );
//   boost::replace_all( lid, "'", "''" );

//   string did( m_theJob.get_delegation_id() );
//   boost::replace_all( did, "'", "''" );

//   string mjdl( m_theJob.get_modified_jdl() );
//   boost::replace_all( mjdl, "'", "''" );

  ostringstream sqlcmd("");
  sqlcmd << "INSERT OR REPLACE INTO jobs ("
	 << util::CreamJob::get_query_fields()
	 << ", last_poller_visited"
	 << ") VALUES (" 
	 << Ice::get_tmp_name()
	 << m_theJob.get_grid_jobid()
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_cream_jobid() 
	 << Ice::get_tmp_name()
	 << "," 
	 << Ice::get_tmp_name()
	 << m_theJob.get_complete_cream_jobid() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_JDL 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_user_proxy_certificate() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_ceid() 
	 << Ice::get_tmp_name() 
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_endpoint() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_creamurl() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_cream_delegurl() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_user_dn()
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_myproxy_address() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << ( m_theJob.is_proxy_renewable() ? "1" : "0" ) 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_failure_reason()
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_sequence_code()
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_wn_sequence_code()
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_prev_status() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_status() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_num_logged_status_changes() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_lease_id()
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_status_poll_retry_count() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_exit_code() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_worker_node() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << ( m_theJob.is_killed_by_ice() ? "1" : "0" ) 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_delegation_id()
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << time(0) 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << time(0) 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << time(0) 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_isbproxy_time_end() 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_theJob.get_modified_jdl()
	 << Ice::get_tmp_name()
	 << ")";
  
  do_query( db, sqlcmd.str() );
}
