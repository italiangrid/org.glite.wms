#include <sstream>
#include <cstdlib>
#include <iostream>

#include "GetEventID.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
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
void ice::db::GetEventID::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd;

  string dn( m_userdn );

  boost::replace_all( dn, "'", "''" );

  sqlcmd << "SELECT eventid FROM event_id WHERE userdn='"
	 << dn << "' AND ceurl='"
	 << m_creamurl << "';";

  do_query( db, sqlcmd.str(), fetch_jobs_callback, &m_result );
  
  if(m_result>-1)
    m_found = true;
}
