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

#include "GetStatusInfoByCompleteCreamJobID.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/archive/text_iarchive.hpp"

#include <iostream>
#include <sstream>

using namespace glite::wms::ice::db;
using namespace std;

// Local helper function: callback for sqlite
static int fetch_jdl_callback(void *param, int argc, char **argv, char **azColName){
    //string* jdl = (string*)param;
    StatusInfo* info( (StatusInfo*)param );
    //*jdl = argv[0];
    if ( argv && argv[0] && argv[1] && argv[2] && argv[3] )
      (*info) = boost::make_tuple( argv[0], argv[1], atoi(argv[2]), atoi(argv[3]));
    return 0;
}

void GetStatusInfoByCompleteCreamJobID::execute( sqlite3* db ) throw ( DbOperationException& )
{
    string sqlcmd = boost::str( boost::format( 
      "select gridjobid,complete_cream_jobid,num_logged_status_changes,status from jobs" \
      " where complete_cream_jobid = \'%1%\';" ) % m_creamjobid );

    do_query( db, sqlcmd, fetch_jdl_callback, &m_info );
    
    if( !m_info.get<0>().empty() )
      m_found = true;
    
}
