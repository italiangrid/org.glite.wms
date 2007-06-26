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
 * ICE empty CEMonitor notification class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "emptyStatusNotification.h"

// ICE stuff
#include "subscriptionManager.h"
#include "jobCache.h"
#include "DNProxyManager.h"

// CREAM stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

// other gLite stuff
#include "classad_distribution.h"

// boost
#include "boost/scoped_ptr.hpp"

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;

emptyStatusNotification::emptyStatusNotification( const std::string& subscription_id, const std::string& cemondn ) :
    absStatusNotification( ),
    m_subscription_id( subscription_id ),
    m_cemondn( cemondn )  
{
    
}

void emptyStatusNotification::apply( void )
{
    log4cpp::Category *m_log_dev( api::util::creamApiLogger::instance()->getLogger() );

    pair< string, string > user_cemon( subscriptionManager::getInstance()->getUserCEMonBySubID( m_subscription_id ) );
    
    if ( user_cemon.first.empty() || user_cemon.second.empty() ) {
        CREAM_SAFE_LOG( m_log_dev->warnStream()
                        << "emptyStatusNotification::apply() - "
                        << "Empty user or cemon for subscription ID "
                        << m_subscription_id << ". Ignoring this notification"
                        << log4cpp::CategoryStream::ENDLINE);
        return;
    }

    boost::recursive_mutex::scoped_lock L( jobCache::mutex );
    jobCache* cache( jobCache::getInstance() );
    jobCache::iterator it;
    for ( it = cache->begin(); it != cache->end(); ++it ) {
        if ( !it->getUserDN().compare( user_cemon.first ) ) {
            // The user is the same; now check the CEMonUrl
            string theCemonURL;
            string theProxy( DNProxyManager::getInstance()->getBetterProxyByDN( it->getUserDN() ) );
            subscriptionManager::getInstance()->getCEMonURL( theProxy, it->getCreamURL(), theCemonURL );
            if ( !theCemonURL.compare( user_cemon.second ) ) {
                // The CEMon also matches. We get the job to update
                it->set_last_empty_notification( time(0) );
                CREAM_SAFE_LOG( m_log_dev->debugStream()
                                << "emptyStatusNotification::apply() - "
                                << "timestamp of last empty "
                                << "status notifications for job "
                                << it->describe()
                                << " set to " 
                                << time_t_to_string( it->get_last_empty_notification() )
                                << log4cpp::CategoryStream::ENDLINE);
                cache->put( *it );
            }        
        }
    }
}

