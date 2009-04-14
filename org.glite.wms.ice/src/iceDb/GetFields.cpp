/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *    http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * Get all user proxies
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "GetFields.h"
#include <sstream>

using namespace glite::wms::ice::db;
using namespace std;

int fields_count;

GetFields::GetFields( const std::list<std::string> fields_to_retrieve, const std::list<std::pair<std::string, std::string> > clause, const bool distinct ) :    
  AbsDbOperation(),
  m_fields_to_retrieve( fields_to_retrieve ),
  m_clause( clause ),
  m_distinct( distinct )
{
}

namespace { // begin local namespace

    // Local helper function: callback for sqlite
    static int fetch_fields_callback(void *param, int argc, char **argv, char **azColName){
      list< vector< string > >* result = (list< vector< string > >*)param;
        if ( argv && argv[0] ) {
	  vector<string> tmp;
	  for( int i = 0; i<fields_count; i++ ) {
	    if(argv[i])
	      tmp.push_back( argv[i] );
	    else
	      tmp.push_back( "" );
	  }
	  result->push_back( tmp );
	}
	  
        return 0;
    }

} // end local namespace

void GetFields::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd;
 
  sqlcmd << "SELECT ";

  if(m_distinct)
    sqlcmd << "DISTINCT ";

  fields_count = m_fields_to_retrieve.size();

  for(list< string>::const_iterator it=m_fields_to_retrieve.begin();
      it!=m_fields_to_retrieve.end();
      ++it)
    {
      sqlcmd << *it << ",";
    }
  
  string tmp = sqlcmd.str();
  if( !tmp.empty() )
    tmp = tmp.substr(0, tmp.length()-1); // remove the trailing ","
  
  sqlcmd.str( "" );
  sqlcmd << tmp << " FROM jobs";

  if( !m_clause.empty() )
    {

      sqlcmd << " WHERE ";

      for( list< pair<string, string> >::const_iterator it=m_clause.begin();
	   it != m_clause.end();
	   ++it)
	{
	  sqlcmd << it->first << "=\'" << it->second << "\' AND ";
	}

      string tmp = sqlcmd.str();
      if( !tmp.empty() )
	tmp = tmp.substr(0, tmp.length()-4); // remove the trailing ","
      
      sqlcmd.str( "" );
      sqlcmd << tmp << ";";

    } else {
    sqlcmd << ";";
  }

  do_query( db, sqlcmd.str(), fetch_fields_callback, &m_result );
}
