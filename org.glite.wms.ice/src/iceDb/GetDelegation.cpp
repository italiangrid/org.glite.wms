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

#include "GetDelegation.h"
#include "iceUtils/iceUtils.h"

#include <vector>

#include <boost/tuple/tuple.hpp>

using namespace glite::wms::ice;
using namespace std;

namespace { // begin local namespace

    // Local helper function: callback for sqlite
    static int fetch_fields_callback(void *param, int argc, char **argv, char **azColName) {
    
      //vector<string>* result = (vector<string>*) param;
      boost::tuple< string, string, time_t, int, string, string, bool, string>* result = 
	(boost::tuple< string, string, time_t, int, string, string, bool, string>*)param;
      
      if ( argv && argv[0] ) {
	(*result) = boost::make_tuple(
				      argv[0],
				      argv[1],
				      (time_t)atoi(argv[2]),
				      atoi(argv[3]),
				      argv[4],
				      argv[5],
				      ( atoi(argv[6]) == 0 ? false : true ),
				      argv[7]
				      );

      }
	  
      return 0;
      
    }

} // end local namespace

//______________________________________________________________________________
void db::GetDelegation::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd = "SELECT * FROM delegation WHERE digest=";
  sqlcmd += util::utilities::withSQLDelimiters( m_digest );
  sqlcmd += " AND creamurl=";
  sqlcmd += util::utilities::withSQLDelimiters( m_creamurl );
  sqlcmd += " AND myproxyurl=";
  sqlcmd += util::utilities::withSQLDelimiters( m_myproxyurl );
  sqlcmd += ";";

  boost::tuple< string, string, time_t, int, string, string, bool, string> tmp;

  do_query( db, sqlcmd, fetch_fields_callback, &tmp );
  
  if( !tmp.get<0>().empty() ) {
    m_found = true;

    m_result = glite::wms::ice::util::Delegation_manager::table_entry( 
								      tmp.get<0>(), 
								      tmp.get<1>(),
								      tmp.get<2>(), 
								      tmp.get<3>(),
								      tmp.get<4>(),
								      tmp.get<5>(),
								      tmp.get<6>(),
								      tmp.get<7>()
								      );
  }
}
