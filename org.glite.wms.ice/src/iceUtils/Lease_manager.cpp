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
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "Lease_manager.h"
#include "CreamProxyMethod.h"
#include "iceConfManager.h"
#include "iceUtils.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <ctime>
#include <utility> // defines std::pair<>

namespace cream_api = glite::ce::cream_client_api::soap_proxy;
namespace api_util = glite::ce::cream_client_api::util;
namespace cert_util = glite::ce::cream_client_api::certUtil;
using namespace glite::wms::ice::util;
using namespace std;

Lease_manager* Lease_manager::s_instance = 0;
boost::recursive_mutex Lease_manager::m_mutex;

Lease_manager::Lease_manager( ) :
    m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
    m_operation_count( 0 ),
    m_operation_count_max( 20 ), // FIXME: hardcoded default
    m_host_dn( "UNKNOWN_ICE_DN" ),
    m_cert_file( "unknown/cert/file" ),
    m_lease_delta_time( 60*60 )
{
    glite::wms::common::configuration::Configuration* conf = 0;
    static char* method_name = "Lease_manager::Lease_manager() - ";

    try {
        conf = iceConfManager::getInstance()->getConfiguration();
    } catch( ConfigurationManager_ex& ex ) {
        CREAM_SAFE_LOG( m_log_dev->errorStream()
                        << method_name
                        << "Error while accessing ICE configuration file "
                        << "Giving up, but this may cause troubles."
                        << log4cpp::CategoryStream::ENDLINE );
        return;
    }
    
    // lease id not found (or force). Creates a new lease ID
    m_lease_delta_time = conf->ice()->lease_delta_time();
    
    m_cert_file = conf->ice()->ice_host_cert();
    try {
        m_host_dn = cert_util::getDN( m_cert_file );
    } catch( const glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
        CREAM_SAFE_LOG( m_log_dev->errorStream()
                        << method_name
                        << "Could not get DN for the local host." 
                        << "Using hardcoded default of \"UNKNOWN_ICE_DN\""
                        << log4cpp::CategoryStream::ENDLINE );
        m_host_dn = "UNKNOWN_ICE_DN";
    }    
}

Lease_manager* Lease_manager::instance( ) 
{
    boost::recursive_mutex::scoped_lock L( m_mutex );
    if ( 0 == s_instance ) 
        s_instance = new Lease_manager( );
    return s_instance;
}

string Lease_manager::make_lease( const CreamJob& job, bool force )
{
    boost::recursive_mutex::scoped_lock L( m_mutex );

    static char* method_name = "Lease_manager::make_lease() - ";
    string lease_id; // lease ID to return as a result

    // Utility variables
    const string certfile( job.getUserProxyCertificate() );
    const string cream_url( job.getCreamURL() );
    const string user_DN( job.getUserDN() );

    // Check whether the cache should be cleaned up
    if ( ++m_operation_count > m_operation_count_max ) { // FIXME: Hardcoded default
        purge_old_lease_ids( );
        m_operation_count = 0;
    }

    // Lookup the (user_DN,cream_url) pair into the set
    typedef t_lease_set::nth_index<0>::type t_lease_by_key;
    t_lease_by_key& lease_by_key_view( m_lease_set.get<0>() );
    typedef t_lease_set::nth_index<2>::type t_lease_by_seq;
    t_lease_by_seq& lease_by_seq( m_lease_set.get<2>() );

    t_lease_by_key::iterator it = lease_by_key_view.find( boost::make_tuple(user_DN, cream_url));

    if ( force && lease_by_key_view.end() != it ) {
        lease_by_key_view.erase( it );
        it = lease_by_key_view.end();
    }

    if ( lease_by_key_view.end() == it ) {
        // lease id not found (or force). Creates a new lease ID
        time_t expiration_time = time(0) + m_lease_delta_time;

        lease_id = 
            canonizeString( m_host_dn ).
            append("--").
            append(canonizeString( user_DN )).
            append("--").
            append(canonizeString( cream_url ));

        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "Creating new lease "
                        << "with lease ID "
                        << lease_id
                        << " CREAM URL "
                        << cream_url
                        << " user DN "
                        << user_DN
                        << " expiration time "
                        << time_t_to_string( expiration_time )
                        << log4cpp::CategoryStream::ENDLINE );
        
        pair< string, time_t > 
            lease_in = make_pair( lease_id, expiration_time ),
            lease_out;

        try {
	    CreamProxy_Lease( cream_url, m_cert_file, lease_in, &lease_out ).execute( 3 );
        } catch( ... ) {
            // Lease operation failed
            CREAM_SAFE_LOG( m_log_dev->errorStream()
                            << method_name
                            << "Lease operation FAILED for "
                            << "lease ID "
                            << lease_id
                            << " CREAM URL "
                            << cream_url
                            << " user DN "
                            << user_DN
                            << " expiration date "
                            << time_t_to_string( expiration_time )
                            << log4cpp::CategoryStream::ENDLINE );
            // Returns an empty string
            return string();
        }     
        // lease operation succesful. Prints the output
        lease_id = lease_out.first;
        expiration_time = lease_out.second;
        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "Lease operation SUCCESFUL. Returned "
                        << "lease ID "
                        << lease_id
                        << " CREAM URL "
                        << cream_url
                        << " user DN "
                        << user_DN
                        << " new expiration time "
                        << time_t_to_string( expiration_time )
                        << log4cpp::CategoryStream::ENDLINE );        

        // Inserts the lease ID into the lease set
        m_lease_set.insert( Lease_t( user_DN, cream_url, expiration_time, lease_id ) );

        // Inserts into the front of the list; this is guaranteed to
        // be a new element, as it has not been found in this "if"
        // branch
        lease_by_seq.push_front( Lease_t( user_DN, cream_url, expiration_time, lease_id ) );
    } else {
        // Delegation id FOUND. Returns it
        lease_id = it->m_lease_id;

        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "Using existing lease ID "
                        << lease_id
                        << " for CREAM URL "
                        << cream_url
                        << " user DN "
                        << user_DN
                        << " expiration time "
                        << time_t_to_string( it->m_expiration_time )
                        << log4cpp::CategoryStream::ENDLINE );
    }
    return lease_id;
}

time_t Lease_manager::renew_lease( const string& lease_id )
{
    static char* method_name = "Lease_manager::renew_lease() - ";

    typedef t_lease_set::index<idx_lease_id>::type t_lease_by_id;
    t_lease_by_id& lease_by_id_view( m_lease_set.get<idx_lease_id>() );

    t_lease_by_id::iterator it = lease_by_id_view.find( lease_id );

    if ( lease_by_id_view.end() == it ) 
        return 0; // lease id not found
    
    struct Lease_t entry = *it; // this is the (old) entry
    time_t expiration_time = time(0) + m_lease_delta_time;

    pair< string, time_t > 
        lease_in = make_pair( entry.m_lease_id, expiration_time ),
        lease_out;
    
    try {
        CreamProxy_Lease( entry.m_cream_url, m_cert_file, lease_in, &lease_out ).execute( 3 );
    } catch( ... ) {
        // Lease operation failed
        CREAM_SAFE_LOG( m_log_dev->errorStream()
                        << method_name
                        << "Lease operation FAILED for lease ID "
                        << lease_id
                        << " CREAM URL "
                        << entry.m_cream_url
                        << " desired expiration date "
                        << time_t_to_string( expiration_time )
                        << log4cpp::CategoryStream::ENDLINE );
        // Returns an empty string
        return 0;
    }         
    CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << method_name
                        << "Succesfully updated lease for "
                        << "lease ID "
                        << lease_id
                        << " CREAM URL "
                        << entry.m_cream_url
                        << " new expiration time "
                        << time_t_to_string( lease_out.second )
                        << log4cpp::CategoryStream::ENDLINE );
    entry.m_expiration_time = lease_out.second;
    lease_by_id_view.replace( it, entry );
    return entry.m_expiration_time;
}

void Lease_manager::purge_old_lease_ids( void )
{
    static char* method_name = "Lease_manager::purge_old_lease_ids() - ";

    typedef t_lease_set::nth_index<1>::type t_lease_by_expiration;
    t_lease_by_expiration& lease_time_view( m_lease_set.get<1>() );

    t_lease_by_expiration::iterator it_end = lease_time_view.lower_bound( time(0) );
    size_t size_before = lease_time_view.size();    
    lease_time_view.erase( lease_time_view.begin(), it_end );
    size_t size_after = lease_time_view.size();

    if ( size_before != size_after ) {
        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "Purged "
                        << size_before - size_after
                        << " elements from the lease cache"
                        << log4cpp::CategoryStream::ENDLINE );
    }
}
