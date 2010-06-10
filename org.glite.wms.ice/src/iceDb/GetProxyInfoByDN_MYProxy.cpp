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

#include "GetProxyInfoByDN_MYProxy.h"
#include "ice-core.h"

#include <vector>

using namespace glite::wms::ice;
using namespace std;

namespace { // begin local namespace

    // Local helper function: callback for sqlite
    static int fetch_fields_callback(void *param, int argc, char **argv, char **azColName) {
    
      vector<string>* result = (vector<string>*) param;
      
      if ( argv && argv[0] ) {
        result->push_back(argv[0]);
	result->push_back(argv[1]);
	result->push_back(argv[2]);
      }
	  
      return 0;
      
    }

} // end local namespace

void db::GetProxyInfoByDN_MYProxy::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd;

  sqlcmd += "SELECT proxyfile,exptime,counter FROM proxy WHERE userdn="
	 + Ice::get_tmp_name()
	 + m_userdn
	 + Ice::get_tmp_name()
	 + " AND myproxyurl="
	 + Ice::get_tmp_name()
	 + m_myproxy
	 + Ice::get_tmp_name()
	 + ";";

  //  sqlcmd << dn << "\' AND myproxyurl=\'" << m_myproxy << "\';";

  vector<string> tmp;

  do_query( db, sqlcmd, fetch_fields_callback, &tmp );
  
  if( tmp.size() ) {
    m_found = true;
    m_result = boost::make_tuple( tmp.at(0), (time_t)atoi(tmp.at(1).c_str()), atoll(tmp.at(2).c_str()));  
  }

}
