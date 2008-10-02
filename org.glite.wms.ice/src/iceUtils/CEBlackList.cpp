/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * youw may not use this file except in compliance with the License. 
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
 * Authors: Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "CEBlackList.h"
#include "iceUtils.h"
#include "algorithm"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"

namespace api_util = glite::ce::cream_client_api::util;
using namespace glite::wms::ice::util;
using namespace std;

CEBlackList* CEBlackList::s_instance = 0;
boost::recursive_mutex CEBlackList::m_mutex;

//////////////////////////////////////////////////////////////////////////////
CEBlackList::CEBlackList( ) :
    m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
    m_operation_count( 0 ),
    m_operation_count_max( 20 ), // FIXME: hardcoded default
    m_max_blacklist_time( 30*60 ) // FIXME: hardcoded default
{

}

CEBlackList* CEBlackList::instance( ) 
{
    boost::recursive_mutex::scoped_lock L( m_mutex );
    if ( 0 == s_instance ) 
        s_instance = new CEBlackList( );
    return s_instance;
}

void CEBlackList::blacklist_ce( const std::string& ce )
{
    boost::recursive_mutex::scoped_lock L( m_mutex );

    const time_t curtime = time(0);
    static char* method_name = "CEBlackList::blacklist_ce() - ";

    cleanup_blacklist( ); // "lazy" purging

    if ( m_blacklist.end() == m_blacklist.find( ce ) ||
         m_blacklist[ ce ] < curtime ) {
        CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                        << "Blacklisting CE " << ce
                        << " until " << time_t_to_string( curtime + m_max_blacklist_time )
                        );
        m_blacklist[ce] = curtime + m_max_blacklist_time;

    }
}

bool CEBlackList::is_blacklisted( const std::string& ce )
{
    boost::recursive_mutex::scoped_lock L( m_mutex );

    const time_t curtime = time(0);
    static char* method_name = "CEBlackList::is_blacklisted() - ";

    cleanup_blacklist( ); // "lazy" purging

    if ( m_blacklist.end() == m_blacklist.find( ce ) ||
         m_blacklist[ ce ] < curtime ) {
        return false;
    } else {
        CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                        << "CE " << ce
                        << " is blacklisted until " 
                        << time_t_to_string( m_blacklist[ce]) );
        return true;        
    }
}

void CEBlackList::cleanup_blacklist( bool force )
{
    if ( ++m_operation_count > m_operation_count_max || force ) {
        m_operation_count = 0;
        
        // We cannot use remove_if on STL maps
        // http://www.tech-archive.net/Archive/VC/microsoft.public.vc.stl/2005-07/msg00036.html
        map<string,time_t>::iterator it;
        for ( it = m_blacklist.begin(); it != m_blacklist.end(); ) {
            if ( it->second < time(0) ) {
                map<string,time_t>::iterator to_remove = it++;
                m_blacklist.erase( to_remove );                
            } else {
                ++it;
            }
        }
    }
}
