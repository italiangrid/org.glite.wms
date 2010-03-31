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
 * DB operation used to create a new job
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "CreateProxyField.h"

#include <sstream>
#include <iostream>
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
using namespace glite::wms::ice::db;
using namespace std;

void CreateProxyField::execute( sqlite3* db ) throw ( DbOperationException& )
{

  ostringstream sqlcmd("");

  string dn( m_userdn );
  boost::replace_all( dn, "'", "''" );
  string prx( m_proxyfile );
  boost::replace_all( prx, "'", "''" );

  sqlcmd << "INSERT OR REPLACE INTO proxy (" 
         << " userdn,myproxyurl,proxyfile,exptime,counter) VALUES ("
	 << "\'" << dn << "\',"
	 << "\'" << m_myproxy << "\',"
	 << "\'" << prx << "\',"
	 << "\'" << m_exptime << "\',"
	 << "\'" << m_counter << "\'"
	 << ");";
   		      
  do_query( db, sqlcmd.str() );
}
