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
 * Get information needed to delegation manager
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "GetDelegationInformation.h"
#include "iceUtils/iceConfManager.h"
#include "iceUtils/iceUtils.h"



#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;

GetDelegationInformation::GetDelegationInformation( const bool proxy_renewable ) :    
    AbsDbOperation(),
    m_proxy_renewable( proxy_renewable )
{

}

namespace { // begin local namespace

    // Local helper function: callback for sqlite
    static int fetch_info_delegation_callback(void *param, int argc, char **argv, char **azColName){
        if ( argv && argv[0] && argv[1] && argv[2] && 
	     argv[3] && argv[4] && argv[5] ) 
	{
            //set< string > *result( (set< string >*)param );
	    DelegInfo *result( (DelegInfo*)param );
            //result->insert( argv[0] );
	    (*result)[ argv[0] ] = boost::make_tuple( argv[1], 
	    					      argv[2],
						      (time_t)atol(argv[3]),
						      atoi(argv[4]),
						      argv[0],
						      true,
						      argv[5]);
        }
        return 0;
    }

    // Local helper function: callback for sqlite
    static int fetch_info_delegation_callback2(void *param, int argc, char **argv, char **azColName){
        if ( argv && argv[0] && argv[1] && argv[2] && 
	     argv[3] && argv[4] && argv[5] ) 
	{
            //set< string > *result( (set< string >*)param );
	    DelegInfo *result( (DelegInfo*)param );
            //result->insert( argv[0] );
	    (*result)[ computeSHA1Digest(argv[0]) ] = boost::make_tuple( argv[1], 
									 argv[2],
									 (time_t)atol(argv[3]),
									 atoi(argv[4]),
									 argv[0],
									 false,
									 argv[5]);
        }
        return 0;
    }
} // end local namespace



void GetDelegationInformation::execute( sqlite3* db ) throw ( DbOperationException& )
{
  //static const char* method_name = "GetAllGridJobID::execute() - ";
  string sqlcmd;
  if(m_proxy_renewable) {
    sqlcmd = "SELECT userdn,delegationid,creamurl,delegation_exptime,delegation_duration,myproxyurl FROM jobs WHERE proxy_renewable='1';" ;
    do_query( db, sqlcmd, fetch_info_delegation_callback, &m_result );
  }
  else {
    sqlcmd = "SELECT userproxy,delegationid,creamurl,delegation_exptime,delegation_duration,myproxyurl FROM jobs WHERE proxy_renewable='0';" ;
    do_query( db, sqlcmd, fetch_info_delegation_callback2, &m_result );
  }
  
  
}
