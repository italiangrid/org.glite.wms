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
 * ICE job Db Abstract operation
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "AbsDbOperation.h"
#include "boost/format.hpp"

using namespace glite::wms::ice::db;
using namespace std;

void AbsDbOperation::do_query( sqlite3* db, const string& sqlcmd, sqlite_callback_t callback, void* param ) throw( DbOperationException& )
{
    char* errMsg; 
    string error;                               
    int retry = 0;
    int s = 2;
    
    while(1) {
      int rc = sqlite3_exec(db, sqlcmd.c_str(), callback, param, &errMsg);
      switch ( rc ) {
      case SQLITE_OK:
	return;
        break; // normal termination        
      case SQLITE_BUSY:
      case SQLITE_LOCKED:
        error = boost::str( boost::format( "Query [%1%] failed due to error [%2%]" ) % sqlcmd % errMsg );
        sqlite3_free(errMsg);
        throw DbLockedException( error );
      default:
        error = boost::str( boost::format( "Query [%1%] failed due to error [%2%]" ) % sqlcmd % errMsg );
        sqlite3_free(errMsg);
	sleep(s);
	s = s*2;
	retry++;
	if(retry > 5)
          throw DbOperationException( error );
	
      }
    }
}
