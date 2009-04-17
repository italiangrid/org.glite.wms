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

#include "UpdateJobByGid.h"
#include <sstream>

#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"

using namespace glite::wms::ice::db;

using namespace std;

UpdateJobByGid::UpdateJobByGid( const string& gid,
			        const list< pair<string, string> >& nameval_list)
 : m_gid( gid ),
   m_nameval_list( nameval_list )
{
}

void UpdateJobByGid::execute( sqlite3* db ) throw ( DbOperationException& )
{

 
    ostringstream sqlcmd("");
 
    sqlcmd << "UPDATE jobs SET ";
    
    for(list< pair<string, string> >::const_iterator it=m_nameval_list.begin();
        it!=m_nameval_list.end();
	++it)
    {
    
      string value = it->second;
      boost::replace_all( value, "'", "`" );
    
      sqlcmd << it->first << "=\'" << value << "\',";
    }
    
    string tmp = sqlcmd.str();
    if( !tmp.empty() )
      tmp = tmp.substr(0, tmp.length()-1); // remove the trailing ","
    
    sqlcmd.str( "" );
    
    sqlcmd << tmp << " WHERE gridjobid=\'" << m_gid << "\';";
    
    do_query( db, sqlcmd.str() );
}
