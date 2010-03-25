/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010.
See http://www.eu-egee.org/partners/ for details on the copyright
holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.
See the License for the specific language governing permissions and
limitations under the License.

END LICENSE */

#include "iceCommandLBLogging.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "iceDb/GetJobByGid.h"
#include "iceDb/Transaction.h"
#include "iceDb/UpdateJobByGid.h"
#include "iceDb/RemoveJobByGid.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "iceLBEventFactory.h"
#include "glite/security/proxyrenewal/renewal.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
/**
 *
 * Cream Client API C++ Headers
 *
 */
#include "glite/ce/cream-client-api-c/scoped_timer.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
//#include "glite/ce/cream-client-api-c/CEUrl.h"
//#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"

/**
 *
 * OS Headers
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <cerrno>

//extern int errno;

namespace cream_api = glite::ce::cream_client_api;

using namespace std;
using namespace glite::wms::ice::util;

//______________________________________________________________________________
iceCommandLBLogging::iceCommandLBLogging( const list<CreamJob>& jobs) :
    iceAbsCommand( "iceCommandLBLogging", "" ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_jobs_to_remove( jobs ),
    m_lb_logger( ice::util::iceLBLogger::instance() )
{
}

//______________________________________________________________________________
iceCommandLBLogging::~iceCommandLBLogging( )
{
}

//______________________________________________________________________________
string iceCommandLBLogging::get_grid_job_id( ) const
{
  ostringstream randid( "" );
  struct timeval T;
  gettimeofday( &T, 0 );
  randid << T.tv_sec << "." << T.tv_usec;
  return randid.str();
}

//______________________________________________________________________________
void iceCommandLBLogging::execute( const std::string& tid ) throw()
{  

  // m_job_to_remove
  list<CreamJob>::iterator jobit = m_jobs_to_remove.begin();
  
  while( jobit != m_jobs_to_remove.end() ) {  

    iceLBEvent* ev = iceLBEventFactory::mkEvent( *jobit );
    if ( ev ) {
      api_util::scoped_timer lbtimer( string("iceCommandLBLogging::execute - TID=[") + getThreadID() + "] LOG TO LB" );
      m_lb_logger->logEvent( ev );
    }
    
    if( cream_api::job_statuses::DONE_OK == jobit->get_status() ||
        cream_api::job_statuses::DONE_FAILED == jobit->get_status() ||
	cream_api::job_statuses::CANCELLED == jobit->get_status() ||
	cream_api::job_statuses::ABORTED == jobit->get_status() )
    {	
      CREAM_SAFE_LOG(m_log_dev->debugStream() << "iceCommandLBLogging::execute - TID=[" << getThreadID() << "] "
	  	     << "Removing job [" << jobit->get_grid_jobid( )
		     << "] because proxy is expired "
		     );
    
      {
        db::RemoveJobByGid remover( jobit->get_grid_jobid(), "iceCommandLBLogging::execute" );
        db::Transaction tnx( false, false );
        tnx.execute( &remover );
      }
    
    }

}
