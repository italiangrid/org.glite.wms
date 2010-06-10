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
#include "GetStats.h"
#include "ice-core.h"
#include "iceUtils.h"

#include <vector>
#include <cstdlib>

using namespace glite::wms::ice;
using namespace std;

namespace { // begin local namespace

    // Local helper function: callback for sqlite
    static int fetch_fields_callback(void *param, int argc, char **argv, char **azColName) {
    
      vector< pair<time_t, int> >* result = (vector< pair<time_t, int> >*) param;
      
      if ( argv && argv[0] ) {
        //result->push_back(argv[0]);
	result->push_back( make_pair((time_t)atoi(argv[0]), atoi(argv[1])) );
      }
	  
      return 0;
      
    }

} // end local namespace

db::GetStats::GetStats( vector< pair< time_t, int > >& target, const time_t datefrom, const time_t dateto,const string& caller )
: AbsDbOperation( caller ), m_target(&target), m_datefrom( datefrom ), m_dateto( dateto )
{
}

void db::GetStats::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd;
 
  sqlcmd += "SELECT timestamp,status FROM stats WHERE timestamp >= "
	 + Ice::get_tmp_name() 
	 + util::utilities::to_string( (unsigned long long int)m_datefrom )
	 + Ice::get_tmp_name()
	 + " AND timestamp <= "
	 + Ice::get_tmp_name() 
	 + util::utilities::to_string( (unsigned long long int)m_dateto  )
	 + Ice::get_tmp_name() + ";";

  do_query( db, sqlcmd, fetch_fields_callback, m_target );
  
}
