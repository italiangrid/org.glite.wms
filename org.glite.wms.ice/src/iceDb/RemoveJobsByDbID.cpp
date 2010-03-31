
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
  
  do_query( db, sqlcmd.str().c_str() );
}
