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
 * ICE command for updating job status
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceCommandUpdateStatus.h"
#include "absStatusNotification.h"
#include "StatusNotificationFactory.h"

// ICE stuff
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEventFactory.h"
#include "iceUtils.h"
#include "ice-core.h"
#include "DNProxyManager.h"
#include "subscriptionManager.h"

// CREAM stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

// System includes
#include <iostream>

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;

iceCommandUpdateStatus::iceCommandUpdateStatus( const monitortypes__Event& ev, const string& cemondn ) :
    m_ev( ev ),
    m_cemondn( cemondn )
{

}

//____________________________________________________________________________
void iceCommandUpdateStatus::execute( ) throw( )
{    
    log4cpp::Category *m_log_dev( api::util::creamApiLogger::instance()->getLogger() );
    
    try {

        boost::scoped_ptr< absStatusNotification > notif;
        absStatusNotification* n = StatusNotificationFactory::makeStatusNotification( m_ev, m_cemondn );
        if ( !n ) return;
        notif.reset( n );
        notif->apply( );

    } catch( std::exception& ex ) {
        CREAM_SAFE_LOG(m_log_dev->errorStream() 
                       << "iceCommandUpdateStatus::execute() - "
                       << "Got exception: "
                       << ex.what()
                       << ". Going on and hoping for the best"
                       << log4cpp::CategoryStream::ENDLINE);        
    } catch( ... ) {
        CREAM_SAFE_LOG(m_log_dev->errorStream() 
                       << "iceCommandUpdateStatus::execute() - "
                       << "Got unknown exception. "
                       << "Going on and hoping for the best"
                       << log4cpp::CategoryStream::ENDLINE);        
    }
}
