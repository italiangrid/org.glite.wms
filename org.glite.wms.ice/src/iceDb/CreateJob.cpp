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
 * DB operation used to create a new job
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "CreateJob.h"

#include <sstream>
#include <iostream>

using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;

void CreateJob::execute( sqlite3* db ) throw ( DbOperationException& )
{

  ostringstream sqlcmd("");
  sqlcmd << "INSERT OR REPLACE INTO jobs ("
	 << CreamJob::get_query_fields()
	 << ", last_poller_visited"
	 << ") VALUES (" 
	 << "\'"<< m_theJob.get_grid_jobid() <<"\',"
	 << "\'"<< m_theJob.get_cream_jobid() <<"\'," 
	 << "\'"<< m_theJob.get_complete_cream_jobid() <<"\',"
	 << "\'"<< m_JDL <<"\',"
	 << "\'"<< m_theJob.get_user_proxy_certificate() <<"\',"
	 << "\'"<< m_theJob.get_ceid() <<"\',"
	 << "\'"<< m_theJob.get_endpoint() <<"\',"
	 << "\'"<< m_theJob.get_creamurl() <<"\',"
	 << "\'"<< m_theJob.get_cream_delegurl() <<"\',"
	 << "\'"<< m_theJob.get_user_dn() <<"\',"
	 << "\'"<< m_theJob.get_myproxy_address() <<"\',"
	 << "\'"<< ( m_theJob.is_proxy_renewable() ? "1" : "0" ) <<"\',"
	 << "\'"<< m_theJob.get_failure_reason() <<"\',"
	 << "\'"<< m_theJob.get_sequence_code() <<"\',"
	 << "\'"<< m_theJob.get_wn_sequence_code() <<"\',"
	 << "\'"<< m_theJob.get_prev_status() <<"\',"
	 << "\'"<< m_theJob.get_status() <<"\',"
	 << "\'"<< m_theJob.get_num_logged_status_changes() <<"\',"
	 << "\'"<< m_theJob.get_lease_id() <<"\',"
	 << "\'"<< m_theJob.get_status_poll_retry_count() <<"\',"
	 << "\'"<< m_theJob.get_exit_code() <<"\',"
	 << "\'"<< m_theJob.get_worker_node() <<"\',"
	 << "\'"<< ( m_theJob.is_killed_by_ice() ? "1" : "0" ) <<"\',"
	 << "\'"<< m_theJob.get_delegation_id() <<"\',"
	 << "\'"<< time(0) <<"\',"
	 << "\'"<< time(0) <<"\',"
	 << "\'"<< time(0)  <<"\')";
  
  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;
		      
  do_query( db, sqlcmd.str() );
}
