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

#include "CreateProxyField.h"
#include "ice-core.h"

#include <sstream>

using namespace glite::wms::ice;
using namespace std;

void db::CreateProxyField::execute( sqlite3* db ) throw ( DbOperationException& )
{

  ostringstream sqlcmd("");

//   string dn( m_userdn );
//   boost::replace_all( dn, "'", "''" );
//   string prx( m_proxyfile );
//   boost::replace_all( prx, "'", "''" );

  sqlcmd << "INSERT OR REPLACE INTO proxy (" 
         << " userdn,myproxyurl,proxyfile,exptime,counter) VALUES ("
	 << Ice::get_tmp_name()
	 << m_userdn 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_myproxy 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_proxyfile 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_exptime 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_counter 
	 << Ice::get_tmp_name()
	 << ");";
   		      
  do_query( db, sqlcmd.str() );
}
