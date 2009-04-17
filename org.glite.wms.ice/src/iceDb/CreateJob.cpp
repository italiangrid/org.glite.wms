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

CreateJob::CreateJob( const CreamJob& j ) :
  m_theJob( j ),
  m_JDL( j.getJDL() )
{
}

void CreateJob::execute( sqlite3* db ) throw ( DbOperationException& )
{

  ostringstream sqlcmd("");
  sqlcmd << "INSERT OR REPLACE INTO jobs " 
	 << " (gridjobid, "	     
	 << " creamjobid, "			     
    	 << " complete_cream_jobid, "
	 << " jdl, "
	 << " userproxy, "			     
	 << " ceid, "				     
	 << " endpoint, "			     
	 << " creamurl, "			     
	 << " creamdelegurl, "		     
	 << " userdn, "			     
	 << " myproxyurl, "			     
	 << " proxy_renewable, "		     
	 << " failure_reason, "		     
	 << " sequence_code, "		     
	 << " wn_sequence_code, "		     
	 << " prev_status, "			     
	 << " status, "			     
	 << " num_logged_status_changes, "	     
	 << " leaseid, "			     
	 << " proxycert_timestamp, "		     
	 << " status_poller_retry_count, "	     
	 << " exit_code, "			     
	 << " worker_node, "			     
	 << " is_killed_byice, "		     
	 << " delegationid, "			     
	 << " last_empty_notification, "	     
	 << " last_seen) "			     
	 << " VALUES (" 
	 << "\'"<< m_theJob.getGridJobID() <<"\',"
	 << "\'"<< m_theJob.getCreamJobID() <<"\'," 
	 << "\'"<< m_theJob.getCompleteCreamJobID() <<"\',"
	 << "\'"<< m_JDL <<"\',"
	 << "\'"<< m_theJob.getUserProxyCertificate() <<"\',"
	 << "\'"<< m_theJob.getCEID() <<"\',"
	 << "\'"<< m_theJob.getEndpoint() <<"\',"
	 << "\'"<< m_theJob.getCreamURL() <<"\',"
	 << "\'"<< m_theJob.getCreamDelegURL() <<"\',"
	 << "\'"<< m_theJob.getUserDN() <<"\',"
	 << "\'"<< m_theJob.getMyProxyAddress() <<"\',"
	 << "\'"<< ( m_theJob.is_proxy_renewable() ? "1" : "0" ) <<"\',"
	 << "\'"<< m_theJob.get_failure_reason() <<"\',"
	 << "\'"<< m_theJob.getSequenceCode() <<"\',"
	 << "\'"<< m_theJob.get_wn_sequence_code() <<"\',"
	 << "\'"<< m_theJob.get_prev_status() <<"\',"
	 << "\'"<< m_theJob.getStatus() <<"\',"
	 << "\'"<< m_theJob.get_num_logged_status_changes() <<"\',"
	 << "\'"<< m_theJob.get_lease_id() <<"\',"
	 << "\'"<< m_theJob.getProxyCertLastMTime() <<"\',"
	 << "\'"<< m_theJob.getStatusPollRetryCount() <<"\',"
	 << "\'"<< m_theJob.get_exit_code() <<"\',"
	 << "\'"<< m_theJob.get_worker_node() <<"\',"
	 << "\'"<< ( m_theJob.is_killed_by_ice() ? "1" : "0" ) <<"\',"
	 << "\'"<< m_theJob.getDelegationId() <<"\',"
	 << "\'"<< m_theJob.get_last_empty_notification() <<"\',"
	 << "\'"<< m_theJob.getLastSeen()  <<"\')";
	
  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;
		      
  do_query( db, sqlcmd.str() );
}
