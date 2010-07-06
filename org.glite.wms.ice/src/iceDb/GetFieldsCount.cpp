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
 
#include "GetFieldsCount.h"
#include "iceUtils.h"
#include "ice-core.h"
#include <cstdlib>

using namespace glite::wms::ice;
using namespace std;

int fields_count_a;

namespace { // begin local namespace

    // Local helper function: callback for sqlite
    static int fetch_fields_callback(void *param, int argc, char **argv, char **azColName){
      int* result = (int*)param;
        if ( argv && argv[0] ) {
	  (*result) = atoi(argv[0]);
	}
	  
        return 0;
    }

} // end local namespace

void db::GetFieldsCount::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd = "SELECT COUNT(";

  fields_count_a = m_fields_to_retrieve.size();

  sqlcmd = util::utilities::join(m_fields_to_retrieve, "," );


//   for(list< string>::const_iterator it=m_fields_to_retrieve.begin();
//       it!=m_fields_to_retrieve.end();
//       ++it)
//     {
//       sqlcmd += *it + ",";
//     }
//   
//   string tmp = sqlcmd;
//   if( !tmp.empty() )
//     tmp = tmp.substr(0, tmp.length()-1); // remove the trailing ","
//   
//   sqlcmd = "";
  sqlcmd += ") FROM jobs";

  if( !m_clause.empty() )
    {

      sqlcmd += " WHERE ";

      for( list< pair<string, string> >::const_iterator it=m_clause.begin();
	   it != m_clause.end();
	   ++it)
	{
	  sqlcmd += it->first 
		 + "=" 
		 + Ice::get_tmp_name()
		 + it->second 
		 + Ice::get_tmp_name() + " AND ";
	}

      string tmp = sqlcmd;
      if( !tmp.empty() )
	tmp = tmp.substr(0, tmp.length()-4); // remove the trailing "AND"
      
      sqlcmd = "";
      sqlcmd += tmp;
      sqlcmd += ";";

    } else {
    sqlcmd += ";";
  }

  do_query( db, sqlcmd, fetch_fields_callback, &m_fields_count );
}
