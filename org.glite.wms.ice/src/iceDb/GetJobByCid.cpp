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

#include "GetJobByCid.h"

//#include "boost/algorithm/string.hpp"

#include <sstream>

using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;
namespace cream_api = glite::ce::cream_client_api;

GetJobByCid::GetJobByCid( const string& cid ) :
  AbsDbOperation(),
  m_complete_creamjobid( cid ),
  m_theJob(),
  m_found( false )
  //    m_serialized_job( "" )
{
  
}

// Local helper function: callback for sqlite
static int fetch_job_callback(void *param, int argc, char **argv, char **azColName){
  
  vector<string> *fields = (vector<string>*)param;
  if ( argv && argv[0] ) 
    {
      for(int i = 0; i<=26; i++) {// a database record for a CreamJob has 29 fields, as you can see in Transaction.cpp
	if( argv[i] )
	  fields->push_back( argv[i] );
	else
	  fields->push_back( "" );
      }
    }
  return 0;
}

void GetJobByCid::execute( sqlite3* db ) throw ( DbOperationException& )
{

    ostringstream sqlcmd("");
    sqlcmd << "SELECT * FROM jobs WHERE complete_cream_jobid=\'" << m_complete_creamjobid << "\';";
    
    vector<string> field_list;
    
    do_query( db, sqlcmd.str(), fetch_job_callback, &field_list );

    if( !field_list.empty() ) {
      m_found = true;
      m_theJob = CreamJob( field_list );
			  
    }
}
