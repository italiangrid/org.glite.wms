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
 * Get jobs to poll
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "GetAllGridJobID.h"
#include "iceUtils/iceConfManager.h"

//#include "boost/algorithm/string.hpp"
//#include "boost/format.hpp"
//#include "boost/archive/text_iarchive.hpp"
//#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

// #include <sstream>
// #include <cstdlib>

using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;
//namespace cream_api = glite::ce::cream_client_api;
//namespace wms_utils  = glite::wms::common::utilities;

GetAllGridJobID::GetAllGridJobID( ) :    
    AbsDbOperation()
{

}

namespace { // begin local namespace

    // Local helper function: callback for sqlite
    static int fetch_grid_job_id_callback(void *param, int argc, char **argv, char **azColName){
        if ( argv && argv[0] ) {
            set< string > *result( (set< string >*)param );
            result->insert( argv[0] );
        }
        return 0;
    }

} // end local namespace

void GetAllGridJobID::execute( sqlite3* db ) throw ( DbOperationException& )
{
  //static const char* method_name = "GetAllGridJobID::execute() - ";

  string sqlcmd = "select gridjobid from jobs;" ;
  
  do_query( db, sqlcmd, fetch_grid_job_id_callback, &m_result );
}
