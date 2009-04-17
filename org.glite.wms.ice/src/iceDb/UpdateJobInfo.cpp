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
 * DB operation used to update a job's sequence code
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "UpdateJobInfo.h"
//#include "boost/algorithm/string.hpp"
#include <iostream>

using namespace glite::wms::ice::db;

using namespace std;

UpdateJobInfo::UpdateJobInfo( const glite::wms::ice::util::CreamJob& aJob ): m_theJob( aJob )
{
}

void UpdateJobInfo::execute( sqlite3* db ) throw ( DbOperationException& )
{

 
    ostringstream sqlcmd("");
 
    sqlcmd << "UPDATE jobs SET "
    	   << "worker_node=\'"
	   <<  m_theJob.get_worker_node() << "\',"
	   << "last_seen=\'"
	   << m_theJob.getLastSeen() << "\',"
	   << "last_empty_notification=\'"
	   << m_theJob.get_last_empty_notification() << "\',"
	   << "status=\'"
	   << m_theJob.getStatus() << "\',"
	   << "exit_code=\'"
	   << m_theJob.get_exit_code() << "\',"
	   << "num_logged_status_changes=\'"
	   << m_theJob.get_num_logged_status_changes() << "\'"
	   << " WHERE gridjobid=\'"
	   << m_theJob.getGridJobID() << "\';";
     
  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;

    do_query( db, sqlcmd.str() );
}
