#include <sstream>
#include <cstdlib>
#include <iostream>

#include "GetDbID.h"

using namespace std;
using namespace glite::wms;

namespace {
  
  static int fetch_jobs_callback(void *param, int argc, char **argv, char **azColName){
    
    if( argv && argv[0] ) {
   
      long long* result = (long long*)param;
      *result = atoll(argv[0]);

    }

    return 0;
  }
  
} // end local namespace

//______________________________________________________________________________
void ice::db::GetDbID::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd;
  sqlcmd << "SELECT db_id FROM ce_dbid WHERE ceurl='"
	 << m_creamurl << "';";

//  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
//    cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;
  
  do_query( db, sqlcmd.str(), fetch_jobs_callback, &m_result );

  if(m_result>-1)
    m_found = true;
}
