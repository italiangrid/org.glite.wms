#include <sstream>
#include <cstdlib>
#include <iostream>

#include "SetEventID.h"

#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"

using namespace std;
using namespace glite::wms;

//______________________________________________________________________________
void ice::db::SetEventID::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd;

  string dn( m_userdn );

  boost::replace_all( dn, "'", "''" );

  sqlcmd << "INSERT OR REPLACE INTO event_id (userdn,ceurl,eventid) VALUES (\'" 
	 << dn << "\', \'"
	 << m_creamurl << "\',\'"
	 << m_new_eventid <<"\');";

  do_query( db, sqlcmd.str() );
}
