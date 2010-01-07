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

#include "boost/archive/text_iarchive.hpp"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"

#include <iostream>

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;

//______________________________________________________________________________
namespace { // begin local namespace

  // Local helper function: callback for sqlite
  static int fetch_jobs_callback(void *param, int argc, char **argv, char **azColName){

    //list< vector<string> > *jobs = (list<vector<string> >*)param;

    list<CreamJob>* jobs = (list<CreamJob>*)param;

    
    if( argv && argv[0] ) {
      vector<string> fields;
      for(int i = 0; i<=27; ++i) {// a database record for a CreamJob has 26 fields, as you can see in Transaction.cpp, but we excluded complete_cream_jobid from the query
	if( argv[i] )
	  fields.push_back( argv[i] );
	else
	  fields.push_back( "" );
      }
      
      CreamJob tmpJob(fields.at(0),
		      fields.at(1),
		      fields.at(2),
		      fields.at(3),
		      fields.at(4),
		      fields.at(5),
		      fields.at(6),
		      fields.at(7),
		      fields.at(8),
		      fields.at(9),
		      fields.at(10),
		      fields.at(11),
		      fields.at(12),
		      fields.at(13),
		      fields.at(14),
		      fields.at(15),
		      fields.at(16),
		      fields.at(17),
		      fields.at(18),
		      fields.at(19),
		      fields.at(20),
		      fields.at(21),
		      fields.at(22),
		      fields.at(23),
		      fields.at(24),
		      fields.at(25),
		      fields.at(26),
		      fields.at(27)
		      );
      
      jobs->push_back( tmpJob );
    }
    return 0;
  }
} // end local namespace

void GetAllJobs::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd( "" );
  if(m_only_active)
    sqlcmd << "SELECT " << CreamJob::get_query_fields() << " FROM jobs WHERE status=\'"
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
	   << "\' AND is_killed_byice=\'0\'";
  else
    sqlcmd << "SELECT " << CreamJob::get_query_fields()  << " FROM jobs";

  if( m_limit > 0 ) {
  	sqlcmd << " LIMIT " << m_limit << " OFFSET " << m_offset << ";"; 
  } else {
  	sqlcmd << ";";
  }

  //list< vector<string> > jobs;

  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;

  do_query( db, sqlcmd.str(), fetch_jobs_callback, m_result );

}
