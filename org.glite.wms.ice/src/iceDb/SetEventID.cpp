#include <sstream>
#include <cstdlib>
#include <iostream>

#include "SetEventID.h"

using namespace std;
using namespace glite::wms;


//______________________________________________________________________________
ice::db::SetEventID::SetEventID( const string& userdn, 
				 const string& ceurl,
				 const long long newid)
  : AbsDbOperation(),
    m_userdn( userdn ), 
    m_creamurl( ceurl ),
    m_new_eventid( newid )
{
  
}

//______________________________________________________________________________
void ice::db::SetEventID::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd;
  sqlcmd << "INSERT OR REPLACE INTO event_id (userdn,ceurl,eventid) VALUES (\'" 
	 << m_userdn << "\', \'"
	 << m_creamurl << "\',\'"
	 << m_new_eventid <<"\');";


  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd.str()<<"]"<<endl;
  
  do_query( db, sqlcmd.str() );
}
