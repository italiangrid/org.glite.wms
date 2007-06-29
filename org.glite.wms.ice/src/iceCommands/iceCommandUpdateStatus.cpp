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
#include "normalStatusNotification.h"
#include "emptyStatusNotification.h"

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

// other gLite stuff
#include "classad_distribution.h"

// boost includes
#include "boost/scoped_ptr.hpp"

// System includes
#include <iostream>

// STL stuff
#include <utility> // for std::pair
#include <set>
#include <list>

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;

iceCommandUpdateStatus::iceCommandUpdateStatus( const vector<monitortypes__Event>& ev, const string& cemondn ) :
    m_ev( ev ),
    m_cemondn( cemondn )
{

}

//____________________________________________________________________________
void iceCommandUpdateStatus::execute( ) throw( )
{   
    log4cpp::Category *m_log_dev( api::util::creamApiLogger::instance()->getLogger() );
    jobCache *cache( jobCache::getInstance() );
    static const char* method_name = "iceCommandUpdateStatus::execute()";

    // We define two "sets" of operations which must be performed by 
    // the status change notifications.
 
    set< string > subscription_set; // The set of subscription IDs for
                                    // which an empty status change
                                    // notification has been received

    list< absStatusNotification* > commands; // The set of commands
                                             // which will be executed
                                             // to update the jobs
                                             // according to the
                                             // content of each
                                             // notification received.

    //
    // This method operates in two passes.
    // 
    // During pass 1. we scan the vector of monitortypes__Event. For
    // each event: 
    //
    // 1a: if it is a "normal" (i.e., non-empty status notification),
    // we create an appropriate normalStatusNotification object which
    // will update the status of that specific job, and push that
    // object in the commands list.
    //
    // 1b: if it is an "empty" status notification, then we insert in
    // the subscription_set set the pair (subscription_id, cemondn),
    // so that each pair will appear only once in the set.
    //

    vector< monitortypes__Event >::const_iterator it;
    for ( it = m_ev.begin(); m_ev.end() != it; ++it ) {

        if( it->Message.empty() ) {        
            CREAM_SAFE_LOG( m_log_dev->warnStream()
                            << method_name << " - "
                            << "got empty notification, skipping"
                            << log4cpp::CategoryStream::ENDLINE);        
            continue; // Skip to the next notification
        }

        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << method_name << " - "
                        << "processing non-empty notification, with "
                        << it->Message.size() << " events"
                        << log4cpp::CategoryStream::ENDLINE);
        
        string first_event( *(it->Message.begin()) );
        string printable_first_event( boost::erase_all_copy( first_event, "\n" ) );

        classad::ClassAdParser parser;
        classad::ClassAd *ad = parser.ParseClassAd( first_event );
        
        if ( !ad ) {
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << method_name << " - "
                           << "Cannot parse notification classad "
                           << printable_first_event
                           << log4cpp::CategoryStream::ENDLINE);
            continue; // Skip to the next notification
        }

        // Classad gets destroyed when exiting the scope        
        boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( ad );
	
        bool keep_alive = false;
        string subs_id;
        
        // Check whether the current notification is an empty notification
        if ( classad_safe_ptr->EvaluateAttrBool( "KEEP_ALIVE", keep_alive ) &&
             keep_alive &&
             classad_safe_ptr->EvaluateAttrString( "SUBSCRIPTION_ID", subs_id ) ) {
            CREAM_SAFE_LOG( m_log_dev->debugStream()
                            << method_name << " - "
                            << "Trying to make an empty status notification command for "
                            << printable_first_event
                            << log4cpp::CategoryStream::ENDLINE);
            // Push the subs_id into the set, which will be considered
            // after all the notifications have been examined.
            subscription_set.insert( subs_id );
            
        } else {
            CREAM_SAFE_LOG( m_log_dev->debugStream()
                            << method_name << " - "
                            << "Trying to make a new normal status notification command for "
                            << printable_first_event
                            << log4cpp::CategoryStream::ENDLINE);

            // Push back a normalStatusNotification, which will update
            // the status of the job referenced in the monitor event
            normalStatusNotification* notif = 0;
            try {
                notif = new normalStatusNotification( *it, m_cemondn );
            } catch( ... ) {
                continue; // FIXME
            }
            commands.push_front( notif );

            // Gets the job which is mentioned in the notification
            boost::recursive_mutex::scoped_lock L( jobCache::mutex );    
            jobCache::const_iterator job_it( cache->lookupByCreamJobID( notif->get_cream_job_id() ) );
            
            if ( cache->end() != job_it ) {

                // Gets the subscription ID which is used to get
                // notifications associated with that job.
                string subs_id( job_it->getSubscriptionID() );

                // Then, push the pair (subs_id, cemondn) into the
                // set.  This is necessary, as a normal status
                // notification should also update all the jobs of
                // that subs_id, in the exact same way as empty status
                // notifications do.
                CREAM_SAFE_LOG( m_log_dev->debugStream()
                                << method_name << " - "
                                << "Adding subscription id "
                                << subs_id
                                << " to the set of subscriptions whose jobs "
                                << "will be updated"
                                << log4cpp::CategoryStream::ENDLINE);

                subscription_set.insert( subs_id );
            }
        }
    }

    // At this point, we complete the list of commands to add also the
    // processing of empty status notifications. We do this by
    // checking al the jobs in the cache.
    {
        boost::recursive_mutex::scoped_lock L( jobCache::mutex );    
        for ( jobCache::iterator job_it=cache->begin();
              cache->end() != job_it; ++job_it ) {
            
            if ( subscription_set.end() !=  subscription_set.find( job_it->getSubscriptionID() ) ) {
                CREAM_SAFE_LOG( m_log_dev->debugStream()
                                << method_name 
                                << " - Making empty status notification command for job "
                                << job_it->describe()
                                << " for which a non-empty notification was received"
                                << log4cpp::CategoryStream::ENDLINE);
                
                commands.push_front( new emptyStatusNotification( job_it->getCreamJobID() ) );
            }            
        }
    } // end block

    // Now executes all the commands in the list
    while( ! commands.empty() ) {
        boost::scoped_ptr< absStatusNotification > elem( commands.front() );
        commands.pop_front();
        try {
            elem->apply();
        } catch( ... ) {
            // FIXME
        }
    }
}
