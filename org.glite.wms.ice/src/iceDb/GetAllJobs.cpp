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
#include "GetAllJobs.h"
#include "iceUtils/iceConfManager.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice;
using namespace std;

//______________________________________________________________________________
namespace { // begin local namespace

  // Local helper function: callback for sqlite
  static int fetch_jobs_callback(void *param, int argc, char **argv, char **azColName){

    //list< vector<string> > *jobs = (list<vector<string> >*)param;

    list<util::CreamJob>* jobs = (list<util::CreamJob>*)param;

    
    if( argv && argv[0] ) {
      vector<string> fields;
      for(int i = 0; i<=util::CreamJob::num_of_members()-1; ++i) {// a database record for a CreamJob has 26 fields, as you can see in Transaction.cpp, but we excluded complete_cream_jobid from the query
	if( argv[i] )
	  fields.push_back( argv[i] );
	else
	  fields.push_back( "" );
      }

     util::CreamJob tmpJob(fields.at(0),
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
                      (const api::job_statuses::job_status)atoi(fields.at(13).c_str()),
                      (const api::job_statuses::job_status)atoi(fields.at(14).c_str()),
                      strtoul(fields.at(15).c_str(), 0, 10),
                      (time_t)strtoll(fields.at(16).c_str(), 0, 10),
                      fields.at(17),
                      strtoul(fields.at(18).c_str(), 0, 10),
                      strtoul(fields.at(19).c_str(), 0, 10),
                      fields.at(20),
                      fields.at(21),
                      (fields.at(22)=="1" ? true : false),
                      (time_t)strtoll(fields.at(23).c_str(), 0, 10),
                      (fields.at(24) == "1" ? true : false ),
                      fields.at(25),
                      (time_t)strtoll(fields.at(26).c_str(), 0, 10),
                      fields.at(27),
                      (time_t)strtoll(fields.at(28).c_str(), 0, 10),
                      strtoull(fields.at(29).c_str(), 0, 10)
                      );
      
      tmpJob.set_retrieved_from_db( );      
      jobs->push_back( tmpJob );

    }
    return 0;
  }
} // end local namespace

void db::GetAllJobs::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd = "";
  if(m_only_active)
    sqlcmd += "SELECT " 
	   + util::CreamJob::get_query_fields() 
	   + " FROM jobs WHERE " + util::CreamJob::status_field() + "="
	   + Ice::get_tmp_name()
	   + util::utilities::to_string((unsigned long int)api::job_statuses::REGISTERED )
	   + Ice::get_tmp_name()
	   + " OR " + util::CreamJob::status_field() + "="
	   + Ice::get_tmp_name()
	   +  util::utilities::to_string((unsigned long int)api::job_statuses::PENDING )
	   + Ice::get_tmp_name()
	   + " OR " + util::CreamJob::status_field() + "="
	   + Ice::get_tmp_name()
	   +  util::utilities::to_string((unsigned long int)api::job_statuses::IDLE )
	   + Ice::get_tmp_name()
	   + " OR " + util::CreamJob::status_field() + "="
	   + Ice::get_tmp_name()
	   +  util::utilities::to_string((unsigned long int)api::job_statuses::RUNNING )
	   + Ice::get_tmp_name()
	   + " OR " + util::CreamJob::status_field() + "="
	   + Ice::get_tmp_name()
	   +  util::utilities::to_string((unsigned long int)api::job_statuses::REALLY_RUNNING )
	   + Ice::get_tmp_name()
	   + " OR " + util::CreamJob::status_field() + "="
	   + Ice::get_tmp_name()
	   +  util::utilities::to_string((unsigned long int)api::job_statuses::HELD)
	   + Ice::get_tmp_name()
	   + " AND " + util::CreamJob::killed_byice_field() + "="
	   + Ice::get_tmp_name()
	   + "0"
	   + Ice::get_tmp_name();
  else
    sqlcmd += "SELECT " + util::CreamJob::get_query_fields() + " FROM jobs";

  if( m_limit > 0 ) {
  	sqlcmd += " LIMIT " + util::utilities::to_string((unsigned long int )m_limit) + " OFFSET " + util::utilities::to_string((unsigned long int)m_offset) + ";"; 
  } else {
  	sqlcmd += ";";
  }

  do_query( db, sqlcmd, fetch_jobs_callback, m_result );

}
