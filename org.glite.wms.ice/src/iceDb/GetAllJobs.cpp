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
 * Get jobs to poll
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "GetAllJobs.h"
#include "iceUtils/iceConfManager.h"

//#include "boost/algorithm/string.hpp"
//#include "boost/format.hpp"
#include "boost/archive/text_iarchive.hpp"
//#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"

// #include <sstream>
// #include <cstdlib>

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;
//namespace cream_api = glite::ce::cream_client_api;
//namespace wms_utils  = glite::wms::common::utilities;

GetAllJobs::GetAllJobs( const bool only_active) :    
    AbsDbOperation(),
    m_only_active( only_active )
{
}

namespace { // begin local namespace

  // Local helper function: callback for sqlite
  static int fetch_jobs_callback(void *param, int argc, char **argv, char **azColName){

    list< vector<string> > *jobs = (list<vector<string> >*)param;
    if( argv && argv[0] ) {
      vector<string> fields;
      for(int i = 0; i<=25; i++) {// a database record for a CreamJob has 29 fields, as you can see in Transaction.cpp
	if( argv[i] )
	  fields.push_back( argv[i] );
	else
	  fields.push_back( "" );
      }
      
      jobs->push_back( fields );
    }
    return 0;
  }
} // end local namespace

void GetAllJobs::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd( "" );
  if(m_only_active)
    sqlcmd << "SELECT * FROM jobs WHERE status=\'"
	   << api::job_statuses::REGISTERED 
	   << "\' OR status=\'"
	   << api::job_statuses::PENDING 
	   << "\' OR status=\'"
	   << api::job_statuses::IDLE
	   << "\' OR status=\'"
	   << api::job_statuses::RUNNING
	   << "\' OR status=\'"
	   << api::job_statuses::REALLY_RUNNING
	   << "\' OR status=\'"
	   << api::job_statuses::HELD
	   << "\' AND is_killed_byice=\'0\';";
  else
    sqlcmd << "SELECT * FROM jobs;";

  list< vector<string> > jobs;
  do_query( db, sqlcmd.str(), fetch_jobs_callback, &jobs );

  for( list< vector<string> >::const_iterator it=jobs.begin();
       it != jobs.end();
       ++it )
    {
      string cream_jobid                = it->at(0);
      string grid_jobid                 = it->at(1);
      string jdl                        = it->at(2);
      string ceid                       = it->at(3);
      string endpoint                   = it->at(4);
      string cream_address              = it->at(5);
      string cream_deleg_address        = it->at(6);
      string user_proxyfile             = it->at(7);
      string user_dn                    = it->at(8);
      string sequence_code              = it->at(9);
      string delegation_id              = it->at(10);
      string wn_sequence_code           = it->at(11);
      string prev_status                = it->at(12);
      string status                     = it->at(13);
      string num_logged_status_changes  = it->at(14);
      string last_seen                  = it->at(15);
      string lease_id                   = it->at(16);
      string proxyCertTimestamp         = it->at(17);
      string statusPollRetryCount       = it->at(18);
      string exit_code                  = it->at(19);
      string failure_reason             = it->at(20);
      string worker_node                = it->at(21);
      string is_killed_by_ice           = it->at(22);
      string last_empty_notification    = it->at(23);
      string proxy_renew                = it->at(24);
      string myproxy_address            = it->at(25);

      CreamJob tmpJob(
		      cream_jobid,
		      grid_jobid ,
		      jdl,
		      ceid,
		      endpoint,
		      cream_address,
		      cream_deleg_address,
		      user_proxyfile,
		      user_dn,
		      sequence_code,
		      delegation_id ,
		      wn_sequence_code ,
		      prev_status,
		      status,
		      num_logged_status_changes,
		      last_seen,
		      lease_id,
		      proxyCertTimestamp,
		      statusPollRetryCount,
		      exit_code,
		      failure_reason,
		      worker_node,
		      is_killed_by_ice,
		      last_empty_notification,
		      proxy_renew,
		      myproxy_address
		      );
      
      m_result.push_back( tmpJob );
    }
}
