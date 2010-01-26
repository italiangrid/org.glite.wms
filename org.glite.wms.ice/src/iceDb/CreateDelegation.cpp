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

#include "CreateDelegation.h"

#include <sstream>
#include <iostream>

using namespace glite::wms::ice::db;
using namespace std;

void CreateDelegation::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd("");
  sqlcmd << "INSERT INTO delegation (" 
	 << "digest,creamurl,exptime,duration,delegationid,userdn,renewable,myproxyurl"
         << " ) VALUES ("
	 << "\'" << m_digest << "\',"
	 << "\'" << m_creamurl << "\',"
	 << "\'" << m_exptime << "\',"
	 << "\'" << m_duration << "\',"
	 << "\'" << m_delegid << "\',"
	 << "\'" << m_userdn << "\',"
	 << "\'" << ( m_renewable ? "1" : "0" ) << "\',"
	 << "\'" << m_myproxyurl << "\'"
	 << ");";

//  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
//    cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;

  do_query( db, sqlcmd.str() );
}
