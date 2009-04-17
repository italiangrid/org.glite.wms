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
 * DB operation used to update an existing job
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "UpdateJob.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"

#include <sstream>
#include <iostream>

using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;

UpdateJob::UpdateJob( const CreamJob& j ) :
    m_job ( j )
{

}

void UpdateJob::execute( sqlite3* db ) throw ( DbOperationException& )
{
    //string m_gridjobid( m_job.getGridJobID() );
    //string m_creamjobid( m_job.getCompleteCreamJobID() );
    string m_serialized_job;
    ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << m_job;
    }
    m_serialized_job = ofs.str();
    // Replace single quotes with double quotes, so that the SQL query
    // will not fail
    boost::replace_all( m_serialized_job, "'", "''" );

    string sqlcmd = boost::str( boost::format( 
      "update jobs set " \
      " creamjobid = \'%1%\', " \
      " jdl = \'%2%\' where " \
      " gridjobid = \'%3%\' " \
      " last_seen = %4% " \
      " last_empty_notification = %5%" ) % m_job.getCompleteCreamJobID() % m_serialized_job % m_job.getGridJobID() % m_job.getLastSeen() % m_job.get_last_empty_notification() );
 
  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
    cout << "Executing query ["<<sqlcmd<<"]"<<endl;

    do_query( db, sqlcmd );
}
