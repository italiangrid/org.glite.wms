/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *    http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 *
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "CheckGridJobID.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include <iostream>
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

using namespace glite::wms::ice::db;

using namespace std;
namespace cream_api = glite::ce::cream_client_api;

// CheckGridJobID::CheckGridJobID( const string& gid, const std::string& caller  ) :
//     AbsDbOperation(caller),
//     m_gridjobid( gid ),
//     m_found( false )
// {

// }

namespace { // begin local namespace

    // Local helper function: callback for sqlite
    static int fetch_jdl_callback(void *param, int argc, char **argv, char **azColName){
        string* jdl = (string*)param;
        *jdl = argv[0];
        return 0;
    }

} // end local namespace

void CheckGridJobID::execute( sqlite3* db ) throw ( DbOperationException& )
{
  string sqlcmd = boost::str( boost::format( 
					    "SELECT gridjobid FROM jobs" \
					    " WHERE gridjobid = \'%1%\';" ) % m_gridjobid );
  string gid;

  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd<<"]"<<endl;

  do_query( db, sqlcmd, fetch_jdl_callback, &gid );

  if ( !gid.empty() ) {
    
    m_found = true;
    
  } 
}

