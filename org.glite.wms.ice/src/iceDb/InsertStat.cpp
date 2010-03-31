
#include "InsertStat.h"

#include <sstream>
#include <iostream>

using namespace glite::wms::ice::db;
using namespace std;

void InsertStat::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd("");
  sqlcmd << "INSERT INTO stats (" 
	 << "timestamp,ce_timestamp,status"
         << " ) VALUES ("
	 << "\'" << m_timestamp << "\',"
	 << "\'" << m_ce_timestamp << "\',"
	 << "\'" << m_status << "\'"
	 << ");";

  do_query( db, sqlcmd.str() );
}
