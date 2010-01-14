
#include "DNHasJobs.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"

#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace glite::wms::ice::db;
using namespace std;

namespace { // begin local namespace

  // Local helper function: callback for sqlite
  static int fetch_jobs_callback(void *param, int argc, char **argv, char **azColName){
    
    bool* found = (bool*)param;
    
    if( argv && argv[0] ) {
    
      (*found) = true;
    
    }

    return 0;
  }
  
} // end local namespace

void DNHasJobs::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd;

  sqlcmd << "SELECT gridjobid FROM jobs WHERE userdn=\'" << m_dn << " \' LIMIT 1;";

  do_query( db, sqlcmd.str(), fetch_jobs_callback, &m_found );
}
