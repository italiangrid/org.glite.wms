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
 * DB operation used to create a new lease entry
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "CreateLease.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include <sstream>
#include <iostream>

using namespace glite::wms::ice::db;
using namespace std;

void CreateLease::execute( sqlite3* db ) throw ( DbOperationException& )
{
  ostringstream sqlcmd("");

  string dn( m_userdn );
  boost::replace_all( dn, "'", "''" );
  string lid( m_leaseid );
  boost::replace_all( lid, "'", "''" );

  sqlcmd << "INSERT OR REPLACE INTO lease (" 
	 << "userdn,creamurl,exptime,leaseid"
         << " ) VALUES ("
	 << "\'" << dn << "\',"
	 << "\'" << m_creamurl << "\',"
	 << "\'" << m_exptime << "\',"	
	 << "\'" << lid << "\');";
 
  do_query( db, sqlcmd.str() );
}
