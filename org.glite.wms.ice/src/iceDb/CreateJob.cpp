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

#include "CreateJob.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"

#include <sstream>

using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;

CreateJob::CreateJob( const CreamJob& j ) :
    m_gridjobid( j.getGridJobID() ),
    m_creamjobid( j.getCompleteCreamJobID() ),
    m_serialized_job()
    
{
    ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << j;
    }
    m_serialized_job = ofs.str();
    // Replace single quotes with double quotes, so that the SQL query
    // will not fail
    boost::replace_all( m_serialized_job, "'", "''" );
}

void CreateJob::execute( sqlite3* db ) throw ( DbOperationException )
{
    string sqlcmd = boost::str( boost::format( 
      "insert into jobs " \
      " (gridjobid, creamjobid, jdl) " \
      " values ( \'%1%\', \'%2%\', \'%3%\')" ) % m_gridjobid % m_creamjobid % m_serialized_job );
    do_query( db, sqlcmd );
}
