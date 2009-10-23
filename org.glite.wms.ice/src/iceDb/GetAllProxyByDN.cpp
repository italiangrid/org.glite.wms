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
 * Get all user proxies
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "GetAllProxyByDN.h"
//#include "iceUtils/iceConfManager.h"

#include <iostream>

// #include "glite/wms/common/configuration/Configuration.h"
// #include "glite/wms/common/configuration/ICEConfiguration.h"

using namespace glite::wms::ice::db;
//using namespace glite::wms::ice::util;
using namespace std;
//namespace cream_api = glite::ce::cream_client_api;
//namespace wms_utils  = glite::wms::common::utilities;

// GetAllProxyByDN::GetAllProxyByDN( const string& userdn, const bool proxy_renewable ) :    
//   AbsDbOperation(),
//   m_proxy_renewable( proxy_renewable ),
//   m_userdn( userdn ),
//   m_all( false )
// {

// }

// GetAllProxyByDN::GetAllProxyByDN( const string& userdn ) :    
//   AbsDbOperation(),
//   m_userdn( userdn ),
//   m_all( true )
// {

// }

namespace { // begin local namespace

    // Local helper function: callback for sqlite
    static int fetch_proxy_job_id_callback(void *param, int argc, char **argv, char **azColName){
        if ( argv && argv[0] ) {
            set< string > *result( (set< string >*)param );
            result->insert( argv[0] );
        }
        return 0;
    }

} // end local namespace

void GetAllProxyByDN::execute( sqlite3* db ) throw ( DbOperationException& )
{
  //static const char* method_name = "GetAllGridJobID::execute() - ";

  string sqlcmd;
  
  if( m_all )
    sqlcmd = "select userproxy from jobs;";
  else {
  
    if( m_proxy_renewable )
      sqlcmd = "select userproxy from jobs where proxy_renewable='1';" ;
    else
      sqlcmd = "select userproxy from jobs where proxy_renewable='0';" ;
  }
  
  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd<<"]"<<endl;

  do_query( db, sqlcmd, fetch_proxy_job_id_callback, &m_result );
}
