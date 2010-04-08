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

#include "GetJobs.h"
#include "ice-core.h"

#include <sstream>
#include <cstdlib>

using namespace glite::wms::ice;
using namespace std;
namespace cream_api = glite::ce::cream_client_api;

namespace { // begin local namespace

  // Local helper function: callback for sqlite
  static int fetch_jobs_callback(void *param, int argc, char **argv, char **azColName){
    
    list<util::CreamJob>* jobs = (list<util::CreamJob>*)param;
    
    if( argv && argv[0] ) {
      vector<string> fields;
      for(int i = 0; i<=27; ++i) {// a database record for a CreamJob has 26 fields, as you can see in Transaction.cpp, but we want to exlude the field "complete_creamjobid", as specified in the SELECT sql statement;
	if( argv[i] )
	  fields.push_back( argv[i] );
	else
	  fields.push_back( "" );
      }

      util::CreamJob tmpJob(fields.at(0),
		      fields.at(1),
		      fields.at(2),
		      fields.at(3),
		      fields.at(4),
		      fields.at(5),
		      fields.at(6),
		      fields.at(7),
		      fields.at(8),
		      fields.at(9),
		      fields.at(10),
		      fields.at(11),
		      fields.at(12),
		      fields.at(13),
		      fields.at(14),
		      fields.at(15),
		      fields.at(16),
		      fields.at(17),
		      fields.at(18),
		      fields.at(19),
		      fields.at(20),
		      fields.at(21),
		      fields.at(22),
		      fields.at(23),
		      fields.at(24),
		      fields.at(25),
		      fields.at(26),
		      fields.at(27)
		      );
      
      jobs->push_back( tmpJob );
    }

    return 0;
  }
  
} // end local namespace

void db::GetJobs::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd;

  sqlcmd << "SELECT * FROM jobs";

  if( !m_clause.empty() ) {
    
    sqlcmd << " WHERE ";
    
    for( list< pair<string, string> >::const_iterator it=m_clause.begin();
	 it != m_clause.end();
	 ++it)
      {

	sqlcmd << it->first 
	       << "="
	       << Ice::get_tmp_name() 
	       << it->second 
	       << Ice::get_tmp_name() << " " ;
	
	if(m_use_or)
	  sqlcmd << " OR ";
	else
	  sqlcmd << " AND ";
      }
    
    string tmp = sqlcmd.str();
    if( !tmp.empty() )
      tmp = tmp.substr(0, tmp.length()-4); // remove the trailing ","
    
    sqlcmd.str( "" );
    sqlcmd << tmp << ";";
    
  } else {
    sqlcmd << ";";
  }
  
  do_query( db, sqlcmd.str(), fetch_jobs_callback, m_result );

}
