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

#include "iceUtils/IceUtils.h"
#include "SetDbID.h"
#include <boost/lexical_cast.hpp>
using namespace std;
using namespace glite::wms::ice;

//______________________________________________________________________________
void db::SetDbID::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd( "INSERT OR REPLACE INTO ce_dbid (ceurl,db_id) VALUES (" );
  sqlcmd += util::IceUtils::withSQLDelimiters( m_creamurl );
  sqlcmd += ", " ;
  sqlcmd += util::IceUtils::withSQLDelimiters( boost::lexical_cast<string>( (unsigned long long int)m_new_dbid ) );
  sqlcmd += ");";

  do_query( db, sqlcmd );
}
