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
 * 
 * Get the oldest poll_timestamp from a couple userdn,creamurl
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "UpdateOldestPollTimeForUserDNCE.h"
//#include "glite/ce/cream-client-api-c/job_statuses.h"
#include <sstream>
#include <iostream>

using namespace glite::wms::ice::db;
using namespace std;


void UpdateOldestPollTimeForUserDNCE::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd( "" );
  sqlcmd << "UPDATE dn_ce_polltime SET last_seen_poll=\'" 
         << m_last_poll_time << "\' WHERE userdn=\'"
	 << m_userdn 
	 << "\' AND creamurl=\'"
	 << m_creamurl 
	 << "\';";
 

  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;

  do_query( db, sqlcmd.str() );
  
}
