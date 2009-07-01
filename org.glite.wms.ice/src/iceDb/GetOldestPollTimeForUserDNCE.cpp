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

#include "GetOldestPollTimeForUserDNCE.h"
#include <sstream>
#include <iostream>

using namespace glite::wms::ice::db;
using namespace std;

namespace { // begin local namespace

  // Local helper function: callback for sqlite
  static int fetch_jobs_callback(void *param, int argc, char **argv, char **azColName){

    time_t* result = (time_t*)param;

    if( argv && argv[0] ) {
      *result = (time_t)atoi(argv[0]);
    } else {
      *result = 0;
    }

    return 0;
  }
} // end local namespace

void GetOldestPollTimeForUserDNCE::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd( "" );
  sqlcmd << "SELECT last_seen_poll FROM dn_ce_polltime WHERE userdn=\'"
	 << m_userdn 
	 << "\' AND creamurl=\'"
	 << m_creamurl 
	 << "\';";
 
    //list< vector<string> > jobs;

  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;

  do_query( db, sqlcmd.str(), fetch_jobs_callback, &m_result );
  
}
