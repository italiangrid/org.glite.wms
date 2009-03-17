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
 *
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "GetJobByGid.h"

#include "boost/algorithm/string.hpp"
//#include "boost/format.hpp"
#include "boost/archive/text_iarchive.hpp"
//#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include <sstream>

using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;
namespace cream_api = glite::ce::cream_client_api;

GetJobByGid::GetJobByGid( const string& gid ) :
    AbsDbOperation(),
    m_gridjobid( gid ),
    m_theJob(),
    m_found( false )
{

}

namespace { // begin local namespace

    // Local helper function: callback for sqlite
    static int fetch_job_callback(void *param, int argc, char **argv, char **azColName){
        string* jdl = (string*)param;
	if ( argv && argv[0] )
	  *jdl = argv[0];
	return 0;
    }

} // end local namespace

void GetJobByGid::execute( sqlite3* db ) throw ( DbOperationException& )
{
    //static const char* method_name = "GetJobByGid::execute() - ";

//     string sqlcmd = boost::str( boost::format( 
//       "select jdl from jobs" \
//       " where gridjobid = \'%1%\';" ) % m_gridjobid );
//     string serialized_job;
//     do_query( db, sqlcmd, fetch_jdl_callback, &serialized_job );
//     if ( !serialized_job.empty() ) {
//       try {
// 	//             istringstream is;
// 	//             is.str( serialized_job );
// 	//             {
// 	//                 boost::archive::text_iarchive ia(is);
// 	//                 ia >> m_theJob;
// 	//             }
// 	m_theJob.setJdl( serialized_job );
// 	m_found = true;
//       } catch( std::exception& ex ) {
// 	CREAM_SAFE_LOG(cream_api::util::creamApiLogger::instance()->getLogger()->fatalStream() << method_name
// 		       << "Deserialization error of the string "
// 		       << serialized_job
// 		       << ". Exception string is  \"" 
// 		       << ex.what() << "\". Aborting."
// 		       );
// 	abort();
//       }
//     }

    ostringstream sqlcmd("");
    sqlcmd << "SELECT serialized from jobs WHERE gridjobid=\'" << m_gridjobid << "\';";
    do_query( db, sqlcmd.str(), fetch_job_callback, &m_serialized_job );
    if( !m_serialized_job.empty() ) {
    
        try {
	  istringstream is;
	  is.str( m_serialized_job );
	  {
	    boost::archive::text_iarchive ia(is);
	    ia >> m_theJob;
	  }
	} catch( std::exception& ex ) {
	  throw DbOperationException( ex.what() );
	}
  
      m_found = true;
    }
}
