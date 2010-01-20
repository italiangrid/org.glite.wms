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
 * DB operation used to update a job's sequence code
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "UpdateProxyCounterByDN.h"
#include <sstream>
#include <iostream>

#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"

using namespace glite::wms::ice::db;

using namespace std;

// UpdateProxyFieldsByDN::UpdateProxyFieldsByDN( const string& dn,
// 			        const list< pair<string, string> >& nameval_list)
//  : m_dn( dn ),
//    m_nameval_list( nameval_list )
// {
// }

void UpdateProxyCounterByDN::execute( sqlite3* db ) throw ( DbOperationException& )
{
    ostringstream sqlcmd("");
 
    sqlcmd << "UPDATE proxy SET counter=\'" << m_counter << "\'"
	   << " WHERE userdn=\'" << m_dn << "\' AND myproxyurl=\'" << m_myproxy << "\';";
    
    do_query( db, sqlcmd.str() );
}
