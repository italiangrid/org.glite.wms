
#include "GetJobs.h"
#include "iceUtils/iceConfManager.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;
namespace cream_api = glite::ce::cream_client_api;

namespace { // begin local namespace

  // Local helper function: callback for sqlite
  static int fetch_jobs_callback(void *param, int argc, char **argv, char **azColName){
    
    list<CreamJob>* jobs = (list<CreamJob>*)param;
    
    if( argv && argv[0] ) {
      vector<string> fields;
      for(int i = 0; i<=27; ++i) {// a database record for a CreamJob has 26 fields, as you can see in Transaction.cpp, but we want to exlude the field "complete_creamjobid", as specified in the SELECT sql statement;
	if( argv[i] )
	  fields.push_back( argv[i] );
	else
	  fields.push_back( "" );
      }

      CreamJob tmpJob(fields.at(0),
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

void GetJobs::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd;

  sqlcmd << "SELECT * FROM jobs";

  if( !m_clause.empty() ) {
    
    sqlcmd << " WHERE ";
    
    for( list< pair<string, string> >::const_iterator it=m_clause.begin();
	 it != m_clause.end();
	 ++it)
      {
	sqlcmd << it->first << "=\'" << it->second << "\' " ;
	if(m_use_or)
	  sqlcmd << "OR ";
	else
	  sqlcmd << "AND ";
      }
    
    string tmp = sqlcmd.str();
    if( !tmp.empty() )
      tmp = tmp.substr(0, tmp.length()-4); // remove the trailing ","
    
    sqlcmd.str( "" );
    sqlcmd << tmp << ";";
    
  } else {
    sqlcmd << ";";
  }
  
  //   sqlcmd << "SELECT " << CreamJob::get_query_fields() 
  // 	 << " FROM jobs WHERE "
    
  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;

  do_query( db, sqlcmd.str(), fetch_jobs_callback, m_result );

}
