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
#include "ice-core.h"

#include <sstream>

using namespace glite::wms::ice;
using namespace std;

void db::CreateDelegation::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd("");

//   string dig( m_digest );
//   boost::replace_all( dig, "'", "''" );
  
//   string did( m_delegid );
//   boost::replace_all( did, "'", "''" );
  
//   string dn( m_userdn );
//   boost::replace_all( dn, "'", "''" );

  sqlcmd << "INSERT INTO delegation (" 
	 << "digest,creamurl,exptime,duration,delegationid,userdn,renewable,myproxyurl"
         << " ) VALUES ("
	 << Ice::get_tmp_name()
	 << m_digest 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_creamurl
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_exptime 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_duration 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_delegid 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_userdn 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << ( m_renewable ? "1" : "0" ) 
	 << Ice::get_tmp_name()
	 << ","
	 << Ice::get_tmp_name()
	 << m_myproxyurl 
	 << Ice::get_tmp_name()
	 << ");";

  do_query( db, sqlcmd.str() );
}
