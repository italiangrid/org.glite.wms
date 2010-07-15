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

#include "UpdateDelegationTimesByID.h"
#include "iceUtils/iceUtils.h"
#include "ice/IceCore.h"

using namespace glite::wms::ice;
using namespace std;

void db::UpdateDelegationTimesByID::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd("UPDATE delegation SET exptime=");
  sqlcmd += util::utilities::withSQLDelimiters( util::utilities::to_string( m_exptime ) );
  sqlcmd += ",";
  sqlcmd += "duration=";
  sqlcmd += util::utilities::withSQLDelimiters( util::utilities::to_string( (unsigned long int)m_duration ) );
  sqlcmd += " WHERE delegationid=";
  sqlcmd += util::utilities::withSQLDelimiters( m_delegid );
  sqlcmd += ";";

  do_query( db, sqlcmd );
}
