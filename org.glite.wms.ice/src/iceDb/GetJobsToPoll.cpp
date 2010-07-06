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
#include "GetJobsToPoll.h"
//#include "iceUtils/iceConfManager.h"

//#include "glite/ce/cream-client-api-c/creamApiLogger.h"
//#include "glite/wms/common/configuration/Configuration.h"
//#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <cstdlib>

using namespace glite::wms::ice;
using namespace std;
namespace cream_api = glite::ce::cream_client_api;

void db::GetJobsToPoll::execute( sqlite3* db ) throw ( DbOperationException& )
{
    time_t 
      threshold( util::iceConfManager::getInstance()->getConfiguration()->ice()->poller_status_threshold_time() ),
      empty_threshold( util::iceConfManager::getInstance()->getConfiguration()->ice()->ice_empty_threshold() );


        //
        // Q: When does a job get polled?
        //
        // A: A job gets polled if one of the following situations are true:
        //
        // 1. ICE starts. When ICE starts, it polls all the jobs it thinks
        // have not been purged yet.
        //
        // 2. ICE received the last non-empty status change
        // notification more than m_threshold seconds ago.
        //
        // 3. ICE received the last empty status change notification
        // more than 10*60 seconds ago (that is, 10 minutes).
        //
	// empty notification are effective to reduce the polling frequency if the m_threshold is much greater than 10x60 seconds

    string sqlcmd;

    if ( m_poll_all_jobs ) {
      sqlcmd += "SELECT " + util::CreamJob::get_query_fields() 
	     + " FROM jobs WHERE (" + util::CreamJob::cream_jobid_field() + " not null) "
	     + " AND ( " + util::CreamJob::cream_jobid_field() + " != "
	     + Ice::get_tmp_name()
	     + Ice::get_tmp_name()
	     + " ) AND ( " + util::CreamJob::last_poller_visited_field() + " not null) " 
	     + " AND " + util::CreamJob::cream_address_field() + "="
	     + Ice::get_tmp_name()
	     + m_creamurl 
	     + Ice::get_tmp_name() 
	     + " AND " + util::CreamJob::user_dn_field() + "=" 
	     + Ice::get_tmp_name()
	     + m_userdn 
	     + Ice::get_tmp_name() 
	     + " ORDER BY " + util::CreamJob::last_poller_visited_field() + " ASC";
      
      if( m_limit ) {
	sqlcmd += " LIMIT " + util::utilities::to_string((unsigned long int)m_limit) + ";";
      } else {
	sqlcmd += ";";
      }
      
    } else {
      time_t t_now( time(NULL) );
      sqlcmd += "SELECT " + util::CreamJob::get_query_fields() 
	     + " FROM jobs"					
	     + " WHERE ( " + util::CreamJob::cream_jobid_field() + " not null ) "
	     + " AND ( " + util::CreamJob::cream_jobid_field() + " != "
	     + Ice::get_tmp_name()
	     + Ice::get_tmp_name()
	     + " ) AND (" + util::CreamJob::last_poller_visited_field() + " not null)"	
	     + " AND " + util::CreamJob::user_dn_field() + "=" 
	     + Ice::get_tmp_name()
	     + m_userdn 
	     + Ice::get_tmp_name() 
	     + " AND " + util::CreamJob::cream_address_field() + "=" 
	     + Ice::get_tmp_name()
	     + m_creamurl 
	     + Ice::get_tmp_name() 
	     + " AND ("
	     + "       (  ( " 
	     + util::utilities::to_string((unsigned long long int)t_now)
	     + " - " + util::CreamJob::last_seen_field() + " >= "
	     + util::utilities::to_string((unsigned long int)threshold)
	     + " ) ) "
	     + "  OR   (  ( "
	     + util::utilities::to_string((unsigned long long int)t_now)
	     + " - " + util::CreamJob::last_empty_notification_time_field() + " > "
	     + util::utilities::to_string((unsigned long int)empty_threshold)
	     + " ) )"
	     + ") ORDER BY " + util::CreamJob::last_poller_visited_field() + " ASC";
      if( m_limit ) {
	sqlcmd += " LIMIT " + util::utilities::to_string((unsigned long int)m_limit) + ";";
      } else {
	sqlcmd += ";";
      }
    }

    list< vector<string> > jobs;

  do_query( db, sqlcmd, glite::wms::ice::util::utilities::fetch_jobs_callback, m_result );

}
