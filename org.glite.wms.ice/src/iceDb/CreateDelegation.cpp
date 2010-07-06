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
#include "iceUtils.h"

using namespace glite::wms::ice;
using namespace std;

void db::CreateDelegation::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd = string("INSERT INTO delegation (");
  sqlcmd	 += "digest,creamurl,exptime,duration,delegationid,userdn,renewable,myproxyurl";
   sqlcmd      += " ) VALUES (";
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += m_digest ;
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += ",";
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += m_creamurl;
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += ",";
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += util::utilities::to_string((long int)m_exptime) ;
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += ",";
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += util::utilities::to_string((long int)m_duration );
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += ",";
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += m_delegid ;
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += ",";
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += m_userdn ;
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += ",";
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += ( m_renewable ? "1" : "0" ) ;
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += ",";
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += m_myproxyurl ;
sqlcmd	 += Ice::get_tmp_name();
sqlcmd	 += ");";

  do_query( db, sqlcmd );
}
