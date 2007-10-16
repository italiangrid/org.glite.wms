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
 * ICE request source purger class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "Request_source_purger.h"
#include "Request.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include <boost/scoped_ptr.hpp>

namespace api_util = glite::ce::cream_client_api::util;
using namespace glite::wms::ice;

void Request_source_purger::operator()( void )
{
    // This ensures that the requests gets deallocated if it is
    // removed from the input queue
    boost::scoped_ptr< util::Request > m_req_scoped_ptr( m_req );

    log4cpp::Category* log_dev = api_util::creamApiLogger::instance()->getLogger();
    if(!getenv("NO_FL_MESS"))
	CREAM_SAFE_LOG(
                   log_dev->debugStream()
                   << "filelist_request_purger - "
                   << "removing request "
                   << m_req->to_string()
                   << log4cpp::CategoryStream::ENDLINE
                   );
    try { 
        Ice::instance()->removeRequest( m_req );
    } catch(std::exception& ex) {
        if(!getenv("NO_FL_MESS")) 
	  CREAM_SAFE_LOG(
			 log_dev->fatalStream() 
			 << "filelist_request_purger::operator() - "
			 << "Error removing request from FL: "
			 << ex.what()
			 << log4cpp::CategoryStream::ENDLINE
			 );
        abort();
    }
}
