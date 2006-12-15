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
 * ICE lease updater
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "leaseUpdater.h"
#include "jobCache.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "CreamProxyFactory.h"
#include "CreamProxyMethod.h"
#include "iceCommandLeaseUpdater.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h" // this is necessary, because CreamProxyFactory and iceCommandLeaseUpdater header only have a forward declaration of CreamProxy

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <boost/thread/thread.hpp>
#include <boost/functional.hpp>
#include <vector>
#include <string>
#include <list>

namespace ice_util = glite::wms::ice::util;
using namespace std;

//____________________________________________________________________________
ice_util::leaseUpdater::leaseUpdater( ) :
    iceThread( "ICE Lease Updater" ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    m_cache( ice_util::jobCache::getInstance() )
{
    double delta_time_for_lease = ((double)ice_util::iceConfManager::getInstance()->getConfiguration()->ice()->lease_threshold_time())/4.0;
    m_delay = (time_t)(delta_time_for_lease);
}

//____________________________________________________________________________
ice_util::leaseUpdater::~leaseUpdater( )
{

}

//____________________________________________________________________________
void ice_util::leaseUpdater::update_lease( void )
{
    typedef list< ice_util::CreamJob > cj_list_t;

    cj_list_t jobs_to_check;

    { // acquire lock on job cache to load all jobs in cache
        boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
        for(ice_util::jobCache::iterator jit = m_cache->begin(); jit != m_cache->end(); ++jit) {
            jobs_to_check.push_back( *jit );
        }
    } // releases lock on job cache
    
    for ( cj_list_t::iterator it = jobs_to_check.begin(); it != jobs_to_check.end(); ++it) {
        if ( it->getEndLease() <= time(0) ) {
            // Remove expired job from cache
            vector<string> job_to_cancel;
            job_to_cancel.push_back( it->getCreamJobID() );
            try {
                CREAM_SAFE_LOG(m_log_dev->infoStream()
                               << "leaseUpdater::update_lease() - "
                               << "Job with cream jobID=["
                               << it->getCreamJobID() << "], grid jobID=["
                               << it->getGridJobID() << "] got lease expired. "
                               << "Attempting to cancel it"
                               << log4cpp::CategoryStream::ENDLINE);

                ice_util::CreamProxy_Cancel( it->getCreamURL(), job_to_cancel );
                CREAM_SAFE_LOG(m_log_dev->infoStream()
                               << "leaseUpdater::update_lease() - "
                               << "Job with cream jobID=["
                               << it->getCreamJobID() << "], grid jobID=["
                               << it->getGridJobID() << "] got lease expired. "
                               << "Attempting to purge it"
                               << log4cpp::CategoryStream::ENDLINE);

                ice_util::CreamProxy_Purge( it->getCreamURL(), job_to_cancel );
            } catch( exception& ex ) {
                CREAM_SAFE_LOG(m_log_dev->warnStream()
                               << "leaseUpdater::update_lease() - "
                               << "Error trying to Cancel/Purge "
                               << "job with cream jobID=["
                               << it->getCreamJobID() << "], grid jobID=["
                               << it->getGridJobID() << "] got lease expired. "
                               << "Ignoring the error and going on."
                               << log4cpp::CategoryStream::ENDLINE);
                
            }
	    
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "leaseUpdater::update_lease() - "
                           << "Removing from cache lease-expired job [" 
                           << it->getCreamJobID() << "]"
                           << log4cpp::CategoryStream::ENDLINE);

            boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
            ice_util::jobCache::iterator tmp( m_cache->lookupByGridJobID( it->getGridJobID() ) );
            m_cache->erase( tmp );              
        } else { 
            try {
                ice_util::iceCommandLeaseUpdater cmd( ice_util::CreamProxyFactory::makeCreamProxy( false ), *it );
                cmd.execute();
            } catch( exception& ex ) {

            } catch( ... ) {

            }
        }
    } // end for
}

//______________________________________________________________________________
void ice_util::leaseUpdater::body( void )
{
    while ( !isStopped() ) {
        CREAM_SAFE_LOG(m_log_dev->infoStream()
                       << "leaseUpdater::body() - new iteration"
                       << log4cpp::CategoryStream::ENDLINE);
        update_lease( );
        sleep( m_delay );
    }
}
