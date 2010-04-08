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

#include "GetCreamURLUserDN.h"

using namespace glite::wms::ice;

using namespace std;

namespace { // begin local namespace

    // Local helper function: callback for sqlite
    static int fetch_creamurluserdn_callback(void *param, int argc, char **argv, char **azColName){
        list< pair<string, string> > *result = (list< pair<string, string> >*)param;
	if ( argv && argv[0] && argv[1] ) {
            //set< string > *result( (set< string >*)param );
            //result->insert( argv[0] );
	    result->push_back( make_pair( argv[0], argv[1] ) );
        }
        return 0;
    }

} // end local namespace

void db::GetCreamURLUserDN::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd = "SELECT DISTINCT creamurl,userdn FROM jobs;" ;
  
  do_query( db, sqlcmd, fetch_creamurluserdn_callback, &m_result );
}
