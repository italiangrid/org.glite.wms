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

#include "iceUtils/IceUtils.h"
#include "GetAllJobs.h"

#include "glite/ce/cream-client-api-c/job_statuses.h"

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice;
using namespace std;

void db::GetAllJobs::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd = "";
  if(m_only_active) {
    sqlcmd += "SELECT ";
    sqlcmd += util::CreamJob::get_query_fields() ;
    sqlcmd += " FROM jobs WHERE ";
    sqlcmd += util::CreamJob::status_field();
    sqlcmd += "=";
    sqlcmd += util::IceUtils::withSQLDelimiters( util::IceUtils::to_string((unsigned long int)api::job_statuses::REGISTERED ) );
    sqlcmd += " OR ";
    sqlcmd += util::CreamJob::status_field();
    sqlcmd += "=";
    sqlcmd += util::IceUtils::withSQLDelimiters( util::IceUtils::to_string((unsigned long int)api::job_statuses::PENDING ));
    sqlcmd += " OR " ;
    sqlcmd += util::CreamJob::status_field();
    sqlcmd += "=";
    sqlcmd += util::IceUtils::withSQLDelimiters( util::IceUtils::to_string((unsigned long int)api::job_statuses::IDLE ) );
    sqlcmd += " OR ";
    sqlcmd += util::CreamJob::status_field();
    sqlcmd += "=";
    sqlcmd += util::IceUtils::withSQLDelimiters( util::IceUtils::to_string((unsigned long int)api::job_statuses::RUNNING ) );
    sqlcmd += " OR ";
    sqlcmd += util::CreamJob::status_field();
    sqlcmd += "=";
    sqlcmd += util::IceUtils::withSQLDelimiters( util::IceUtils::to_string((unsigned long int)api::job_statuses::REALLY_RUNNING ) );
    sqlcmd += " OR ";
    sqlcmd += util::CreamJob::status_field();
    sqlcmd += "=";
    sqlcmd += util::IceUtils::withSQLDelimiters( util::IceUtils::to_string((unsigned long int)api::job_statuses::HELD) );
    sqlcmd += " AND ";
    sqlcmd += util::CreamJob::killed_byice_field();
    sqlcmd += "=";
    sqlcmd += util::IceUtils::withSQLDelimiters( "0" );
    }
  else {
    sqlcmd += "SELECT ";
    sqlcmd += util::CreamJob::get_query_fields();
    sqlcmd += " FROM jobs";
  }
  if( m_limit > 0 ) {
  	sqlcmd += " LIMIT ";
	sqlcmd += util::IceUtils::to_string((unsigned long int )m_limit);
	sqlcmd += " OFFSET ";
	sqlcmd += util::IceUtils::to_string((unsigned long int)m_offset);
	sqlcmd += ";"; 
  } else {
  	sqlcmd += ";";
  }

  do_query( db, sqlcmd, glite::wms::ice::util::IceUtils::fetch_jobs_callback, m_result );

}
