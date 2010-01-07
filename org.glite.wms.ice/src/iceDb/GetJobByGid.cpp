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

#include "GetJobByGid.h"

#include <iostream>
#include <sstream>

using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;
namespace cream_api = glite::ce::cream_client_api;

namespace { // begin local namespace

    // Local helper function: callback for sqlite
  static int fetch_job_callback(void *param, int argc, char **argv, char **azColName){
    vector<string> *fields = (vector<string>*)param;
    if ( argv && argv[0] ) 
      {
	for(int i = 0; i<=27; ++i) {// a database record for a CreamJob has 26 fields, as you can see in Transaction.cpp, but we excluded the complete_cream_jobid from query
	  if( argv[i] )
	    fields->push_back( argv[i] );
	  else
	    fields->push_back( "" );
	}
      }
    return 0;
  }
  
} // end local namespace
  
void GetJobByGid::execute( sqlite3* db ) throw ( DbOperationException& )
{


    ostringstream sqlcmd("");
    sqlcmd << "SELECT " << CreamJob::get_query_fields() << " FROM jobs WHERE gridjobid=\'" << m_gridjobid << "\';";
    
    vector<string> field_list;
    
    if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
      cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;
    
    do_query( db, sqlcmd.str(), fetch_job_callback, &field_list );
    
    if( !field_list.empty() ) {
      m_found = true;

      m_theJob = CreamJob (
			   field_list.at(0),
			   field_list.at(1),
			   field_list.at(2),
			   field_list.at(3),
			   field_list.at(4),
			   field_list.at(5),
			   field_list.at(6),
			   field_list.at(7),
			   field_list.at(8),
			   field_list.at(9),
			   field_list.at(10),
			   field_list.at(11),
			   field_list.at(12),
			   field_list.at(13),
			   field_list.at(14),
			   field_list.at(15),
			   field_list.at(16),
			   field_list.at(17),
			   field_list.at(18),
			   field_list.at(19),
			   field_list.at(20),
			   field_list.at(21),
			   field_list.at(22),
			   field_list.at(23),
			   field_list.at(24),
			   field_list.at(25),
			   field_list.at(26),
			   field_list.at(27)
			   );
    }
}
