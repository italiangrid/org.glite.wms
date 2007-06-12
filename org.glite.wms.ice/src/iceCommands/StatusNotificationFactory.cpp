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
 * ICE Status Notification factory
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "StatusNotificationFactory.h"
#include "normalStatusNotification.h"
#include "emptyStatusNotification.h"

// CREAM stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

// other gLite stuff
#include "classad_distribution.h"

// boost includes
#include "boost/scoped_ptr.hpp"
#include "boost/algorithm/string.hpp"

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;

absStatusNotification* StatusNotificationFactory::makeStatusNotification( const monitortypes__Event& ev, const string& cemondn )
{
    log4cpp::Category *m_log_dev( api::util::creamApiLogger::instance()->getLogger() );

    if( ev.Message.empty() ) {        
        CREAM_SAFE_LOG( m_log_dev->warnStream()
                        << "StatusNotificationFactory::makeStatusNotification() - "
                        << "got empty notification, skipping"
                        << log4cpp::CategoryStream::ENDLINE);        
        return 0;
    }

    CREAM_SAFE_LOG( m_log_dev->infoStream()
                    << "StatusNotificationFactory::makeStatusNotification() - "
                    << "processing non-empty notification, with "
                    << ev.Message.size() << " events"
                    << log4cpp::CategoryStream::ENDLINE);

    string first_event( *(ev.Message.begin()) );

    classad::ClassAdParser parser;
    classad::ClassAd *ad = parser.ParseClassAd( first_event );
        
    if ( !ad ) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "StatusNotificationFactory::makeStatusNotification() - "
                       << "Cannot parse notification classad "
                       << first_event
                       << log4cpp::CategoryStream::ENDLINE);
        return 0;
    }

    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( ad );
	
    bool keep_alive = false;
    if ( !classad_safe_ptr->EvaluateAttrBool( "KEEP_ALIVE", keep_alive ) &&
         keep_alive ) {
        return new emptyStatusNotification( ev, cemondn );
    } else {
        return new normalStatusNotification( ev, cemondn );
    }

}
