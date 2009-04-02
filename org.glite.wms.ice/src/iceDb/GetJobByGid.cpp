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

//#include "boost/algorithm/string.hpp"
//#include "boost/format.hpp"
//#include "boost/archive/text_iarchive.hpp"
//#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include <sstream>

using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;
namespace cream_api = glite::ce::cream_client_api;

GetJobByGid::GetJobByGid( const string& gid ) :
    AbsDbOperation(),
    m_gridjobid( gid ),
    m_theJob(),
    m_found( false )
{

}

namespace { // begin local namespace

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
  
} // end local namespace
  
void GetJobByGid::execute( sqlite3* db ) throw ( DbOperationException& )
{


    ostringstream sqlcmd("");
    sqlcmd << "SELECT * FROM jobs WHERE gridjobid=\'" << m_gridjobid << "\';";
    
    vector<string> field_list;
    
    do_query( db, sqlcmd.str(), fetch_job_callback, &field_list );

    if( !field_list.empty() ) {
      m_found = true;
      m_theJob = CreamJob( field_list );
 //      m_theJob = CreamJob(
// 			  (const string&)field_list.at(0),
// 			  (const string&)field_list.at(1),
// 			  (const string&)field_list.at(3),
// 			  (const string&)field_list.at(4),
// 			  (const string&)field_list.at(5),
// 			  (const string&)field_list.at(6),
// 			  (const string&)field_list.at(7),
// 			  (const string&)field_list.at(8),
// 			  (const string&)field_list.at(9),
// 			  (const string&)field_list.at(11),
// 			  (const string&)field_list.at(12),
// 			  (const string&)field_list.at(13),
// 			  (const string&)field_list.at(14),
// 			  (const string&)field_list.at(15),
// 			  (const string&)field_list.at(16),
// 			  (const string&)field_list.at(17),
// 			  (const string&)field_list.at(18),
// 			  (const string&)field_list.at(19),
// 			  (const string&)field_list.at(20),
// 			  (const string&)field_list.at(21),
// 			  (const string&)field_list.at(22),
// 			  (const string&)field_list.at(23),
// 			  (const string&)field_list.at(24),
// 			  (const string&)field_list.at(25),
// 			  (const string&)field_list.at(26),
// 			  (const string&)field_list.at(27),
// 			  (const string&)field_list.at(28)
// 			  );
			  
    }
}
