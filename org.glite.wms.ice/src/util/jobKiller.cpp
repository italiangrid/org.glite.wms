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
 * ICE job killer
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// ICE stuff
#include "jobKiller.h"
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "CreamProxyFactory.h"
#include "CreamProxyMethod.h"
#include "iceCommandJobKill.h"

// GLITE stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/certUtil.h"

// STL and Boost stuff
#include <vector>
#include <boost/format.hpp>

using namespace std;
namespace cream_api = glite::ce::cream_client_api;
using namespace glite::wms::ice::util;

//____________________________________________________________________________
jobKiller::jobKiller() : 
    iceThread( "Job Killer" ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_threshold_time( iceConfManager::getInstance()->getJobKillThresholdTime())
{
    if( m_threshold_time < 60 ) m_threshold_time = 60;
    m_delay = m_threshold_time/4;
}

//____________________________________________________________________________
jobKiller::~jobKiller()
{

}

//____________________________________________________________________________
void jobKiller::body()
{
    jobCache::iterator job_it;
    while( !isStopped() ) {
        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "jobKiller::body() - New iteration..."
                        << log4cpp::CategoryStream::ENDLINE);
        { 
            // FIXME: perhaps this locking can be less rough...
            boost::recursive_mutex::scoped_lock M( jobCache::mutex );
            for( job_it = jobCache::getInstance()->begin(); 
                 job_it != jobCache::getInstance()->end();
                 ++job_it) {
		iceCommandJobKill cmd( util::CreamProxyFactory::makeCreamProxy(true), *job_it );
		cmd.execute();
            }
        }
        sleep( m_delay );
    }
}

