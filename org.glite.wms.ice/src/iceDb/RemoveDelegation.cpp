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

#include "RemoveDelegation.h"
#include "ice-core.h"

#include <sstream>

using namespace glite::wms::ice;
using namespace std;

//______________________________________________________________________________
void db::RemoveDelegation::execute( sqlite3* db ) throw ( DbOperationException& )
{

  ostringstream sqlcmd( "" );
  sqlcmd << "DELETE FROM delegation WHERE digest="
	 << Ice::get_tmp_name()
	 << m_digest 
	 << Ice::get_tmp_name()
	 << " AND creamurl="
	 << Ice::get_tmp_name()
	 << m_creamurl 
	 << Ice::get_tmp_name()
	 << " AND myproxyurl="
	 << Ice::get_tmp_name()
	 << m_myproxy
	 << Ice::get_tmp_name()
	 << ";";

//   string sqlcmd = boost::str( boost::format( 
// 					    "DELETE FROM delegation "	
// 					    " WHERE digest = \'%1%\' AND creamurl = \'%2%\' AND myproxyurl=\'%3%\'; " ) % m_digest % m_creamurl %  m_myproxy );
  
  do_query( db, sqlcmd.str() );
}
