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

#include "UpdateJobSequenceCode.h"
#include "boost/algorithm/string.hpp"
#include <sstream>

using namespace glite::wms::ice::db;

using namespace std;

UpdateJobSequenceCode::UpdateJobSequenceCode( const string& gid, const string& seqcode ) :
    m_seqcode( seqcode ),
    m_gid ( gid )
{

}

void UpdateJobSequenceCode::execute( sqlite3* db ) throw ( DbOperationException& )
{

    // Replace single quotes with double quotes, so that the SQL query
    // will not fail
    //boost::replace_all( m_reason, "'", "''" );

//     string sqlcmd = boost::str( boost::format( 
//       "update jobs set " 
//       " failure_reason = \'%1%\' where gridjobid=\'%1%\'" ) % m_reason % m_gid );
    
    ostringstream sqlcmd("");
    sqlcmd <<   "UPDATE jobs SET " << " sequence_code = \'" << m_seqcode << "\' WHERE gridjobid=\'" << m_gid << "\'";
    
    do_query( db, sqlcmd.str() );
}
