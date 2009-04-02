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
 * Get jobs to poll
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "GetJobsToPoll.h"
#include "iceUtils/iceConfManager.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
//#include "boost/archive/text_iarchive.hpp"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <sstream>
#include <cstdlib>

using namespace glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;
namespace cream_api = glite::ce::cream_client_api;
//namespace wms_utils  = glite::wms::common::utilities;

GetJobsToPoll::GetJobsToPoll( bool poll_all_jobs ) :    
    AbsDbOperation(),
    m_poll_all_jobs( poll_all_jobs )
{

}

namespace { // begin local namespace

  // Local helper function: callback for sqlite
  static int fetch_jobs_callback(void *param, int argc, char **argv, char **azColName){
    //    string* serialized = (string*)param;
    list< vector<string> > *jobs = (list<vector<string> >*)param;
    if( argv && argv[0] ) {
      vector<string> fields;
      for(int i = 0; i<=26; i++) {// a database record for a CreamJob has 29 fields, as you can see in Transaction.cpp
	if( argv[i] )
	  fields.push_back( argv[i] );
	else
	  fields.push_back( "" );
      }

      jobs->push_back( fields );
    }


    //         if ( argv && argv[0] && argv[1] && argv[2] && argv[3] && argv[4] && argv[5] && argv[6] ) {
    //             list< JobToPoll > *result( (list< JobToPoll >*)param );
    //             result->push_back( boost::make_tuple(argv[0], argv[1], argv[2], argv[3], argv[4], (time_t)atoi(argv[5]), (time_t)atoi(argv[6]) ) );
    //         }
    return 0;
  }
  
} // end local namespace

void GetJobsToPoll::execute( sqlite3* db ) throw ( DbOperationException& )
{
  //    static const char* method_name = "GetJobsToPoll::execute() - ";
    time_t 
        threshold( iceConfManager::getInstance()->getConfiguration()->ice()->poller_status_threshold_time() ),
        empty_threshold( iceConfManager::getInstance()->getConfiguration()->ice()->ice_empty_threshold() );

    // time_t oldness = t_now - t_last_seen;
    // time_t empty_oldness = t_now - t_last_empty_notification;

            //
        // Q: When does a job get polled?
        //
        // A: A job gets polled if one of the following situations are true:
        //
        // 1. ICE starts. When ICE starts, it polls all the jobs it thinks
        // have not been purged yet.
        //
        // 2. ICE received the last non-empty status change
        // notification more than m_threshold seconds ago.
        //
        // 3. ICE received the last empty status change notification
        // more than 10*60 seconds ago (that is, 10 minutes).
        //
	// empty notification are effective to reduce the polling frequency if the m_threshold is much greater than 10x60 seconds

    string sqlcmd;
    if ( m_poll_all_jobs ) {
        sqlcmd = "SELECT * FROM jobs WHERE creamjobid not null;" ;
    } else {
        time_t t_now( time(NULL) );
        sqlcmd = boost::str( boost::format( 
                  "SELECT * FROM jobs" \
                  " WHERE ( creamjobid not null ) AND"\
                  "       ( last_seen > 0 AND ( %1% - last_seen >= %2% ) ) "\
                  "  OR   ( last_empty_notification > 0 AND ( %3% - last_empty_notification > %4% ) )" ) % t_now % threshold % t_now % empty_threshold );
    }

    list< vector<string> > jobs;
    do_query( db, sqlcmd, fetch_jobs_callback, &jobs );

    for( list< vector<string> >::const_iterator it=jobs.begin();
	 it != jobs.end();
	 ++it )
      {
	m_result.push_back( CreamJob( *it ) );
      }

//     for(list<string>::const_iterator it = jobs.begin();
// 	it != jobs.end();
// 	++it)
//       {
// 	if( !it->empty() ) {
// 	  try {
// 	    istringstream is;
// 	    is.str( *it );
// 	    {
// 	      CreamJob aJob;
// 	      boost::archive::text_iarchive ia(is);
// 	      ia >> aJob;
// 	      m_result.push_back( aJob );
// 	    }
// 	  } catch( std::exception& ex ) {
// 	    throw DbOperationException( ex.what() );
// 	  }
	  
// 	}
//       }
}
