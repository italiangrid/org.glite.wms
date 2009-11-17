
#include "RemoveJobsByDbID.h"

#include <iostream>
#include <sstream>

using namespace glite::wms::ice::db;
using namespace std;

void RemoveJobsByDbID::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd;
  sqlcmd << "DELETE FROM jobs "
	 << " WHERE dbid = \'" << m_dbid << "\'; ";
  
  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd<<"]"<<endl;
  
  do_query( db, sqlcmd.str().c_str() );
}
