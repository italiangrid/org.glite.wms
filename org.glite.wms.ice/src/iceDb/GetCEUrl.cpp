#include <sstream>
#include <cstdlib>
#include <iostream>

#include "GetCEUrl.h"

using namespace std;
using namespace glite::wms;

namespace {
  
  static int fetch_jobs_callback(void *param, int argc, char **argv, char **azColName){
    
    if( argv && argv[0] ) {
   
      set<string>* result = (set<string>*)param;
      //*result = atoll(argv[0]);
//      result->push_back( argv[0] );
	result->insert( argv[0] );

    }

    return 0;
  }
  
} // end local namespace

//______________________________________________________________________________
void ice::db::GetCEUrl::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd;
  sqlcmd << "SELECT creamurl FROM delegation;";

//  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
//    cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;
  
  do_query( db, sqlcmd.str(), fetch_jobs_callback, m_celist );

}
