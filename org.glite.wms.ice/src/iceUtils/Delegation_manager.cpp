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
 * 
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "Delegation_manager.h"
#include "CreamProxyMethod.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"

namespace cream_api = glite::ce::cream_client_api;
namespace api_util = glite::ce::cream_client_api::util;
using namespace glite::wms::ice::util;
using namespace std;

Delegation_manager* Delegation_manager::s_instance = 0;
boost::recursive_mutex Delegation_manager::m_mutex;

namespace {

    /**
     * Utility function which converts a binary blob into a string.
     *
     * @param buf The binary data blob to convert
     *
     * @param len The length of the binary data bloc buf
     *
     * @return a string univocally representing the buffer buf. The
     * string contains a list of exactly (len * 2) hex characters, as
     * follows.  If the blob contains the following bytes (in hex):
     *
     * 0xfe 0xa0 0x01 0x90
     *
     * Then the resulting string will be the following:
     *
     * "fea00190"
     */ 
    string bintostring( unsigned char* buf, size_t len ) {
        string result;
        const char alpha[] = "0123456789abcdef";
        
        for ( size_t i=0; i<len; ++i ) {
            result.push_back( alpha[ ( buf[i] >> 4 ) & 0x0f ] );
            result.push_back( alpha[ buf[i] & 0x0f ] );
        }
        return result;
    };

};

Delegation_manager::Delegation_manager( ) :
    m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
    m_operation_count( 0 ),
    m_operation_count_max( 20 ) // FIXME: hardcoded default
{

}

Delegation_manager* Delegation_manager::instance( ) 
{
    boost::recursive_mutex::scoped_lock L( m_mutex );
    if ( 0 == s_instance ) 
        s_instance = new Delegation_manager( );
    return s_instance;
}

string Delegation_manager::delegate( glite::ce::cream_client_api::soap_proxy::CreamProxy* theProxy, const CreamJob& job )
{
    boost::recursive_mutex::scoped_lock L( m_mutex );

    static char* method_name = "Delegation_manager::get_delegation_id() - ";
    unsigned char bin_sha1_digest[SHA_DIGEST_LENGTH];
    char buffer[ 1024 ]; // buffer for file data
    SHA_CTX ctx;
    int fd; // file descriptor
    unsigned long nread = 0; // number of bytes read
    string delegation_id; // delegation ID to return as a result

    // Utility variables
    const string certfile( job.getUserProxyCertificate() );
    const string cream_url( job.getCreamURL() );
    const string cream_deleg_url( job.getCreamDelegURL() );
    string str_sha1_digest;

    // Check whether the cache should be cleaned up
    if ( ++m_operation_count > m_operation_count_max ) { // FIXME: Hardcoded default
        purge_old_delegations( );
        m_operation_count = 0;
    }

    // Computes digest from file
    fd = open( certfile.c_str(), O_RDONLY );

    if ( fd < 0 ) {
        CREAM_SAFE_LOG( m_log_dev->errorStream()
                        << method_name
                        << "Cannot open proxy file "
                        << certfile
                        << log4cpp::CategoryStream::ENDLINE );        
        // Returns empty string, so that the register will do
        // auto-delegation
        return delegation_id;
    }

    SHA1_Init( &ctx );
    while ( ( nread = read( fd, buffer, 1024 ) ) > 0 ) {
        SHA1_Update( &ctx, buffer, nread );
    }
    SHA1_Final( bin_sha1_digest, &ctx );

    close( fd );

    str_sha1_digest = bintostring( bin_sha1_digest, SHA_DIGEST_LENGTH );

    // Lookup the (sha1_digest,cream_url) into the set
    typedef t_delegation_set::nth_index<0>::type t_delegation_by_key;
    t_delegation_by_key& delegation_by_key_view( m_delegation_set.get<0>() );

    t_delegation_by_key::iterator it = delegation_by_key_view.find( boost::make_tuple(str_sha1_digest,cream_url));

    if ( delegation_by_key_view.end() == it ) {
        // Delegation id not found. Performs a new delegation   
        time_t expiration_time;

        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "Creating new delegation for CREAM URL "
                        << cream_url
                        << " Delegation URL "
                        << cream_deleg_url
                        << " user DN "
                        << cream_api::certUtil::getDN( certfile )
                        << " proxy hash "
                        << str_sha1_digest
                        << log4cpp::CategoryStream::ENDLINE );
        
        try {
            // Gets the proxy expiration time
            expiration_time = time(0) + cream_api::certUtil::getProxyTimeLeft( certfile );

            // The delegation ID is the GRID job id
            delegation_id = job.getGridJobID();
            CreamProxy_Delegate( delegation_id, cream_deleg_url, certfile ).execute( theProxy, 3 );
        } catch( ... ) {
            // Delegation failed
            CREAM_SAFE_LOG( m_log_dev->errorStream()
                            << method_name
                            << "FAILED creation of new delegation for CREAM URL "
                            << cream_url
                            << " Delegation URL "
                            << cream_deleg_url
                            << " user DN "
                            << cream_api::certUtil::getDN( certfile )
                            << log4cpp::CategoryStream::ENDLINE );
            // Returns an empty string
            return delegation_id;
        }     
        // Inserts the new delegation ID into the delegation set
        m_delegation_set.insert( table_entry( str_sha1_digest, cream_url, expiration_time, delegation_id ) );
    } else {
        // Delegation id FOUND. Returns it
        delegation_id = it->m_delegation_id;

        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "Using existing delegation id "
                        << delegation_id
                        << " for CREAM URL "
                        << cream_url
                        << " Delegation URL "
                        << cream_deleg_url
                        << " user DN "
                        << cream_api::certUtil::getDN( certfile )
                        << log4cpp::CategoryStream::ENDLINE );
    }
    return delegation_id;
}


void Delegation_manager::purge_old_delegations( void )
{
    static char* method_name = "Delegation_manager::purge_old_delegations() - ";

    typedef t_delegation_set::nth_index<1>::type t_delegation_by_expiration;
    t_delegation_by_expiration& deleg_time_view( m_delegation_set.get<1>() );

    t_delegation_by_expiration::iterator it_end = deleg_time_view.lower_bound( time(0) );
    size_t size_before = deleg_time_view.size();    
    deleg_time_view.erase( deleg_time_view.begin(), it_end );
    size_t size_after = deleg_time_view.size();

    CREAM_SAFE_LOG( m_log_dev->debugStream()
                    << method_name
                    << "Purged "
                    << size_before - size_after
                    << " elements from the delegation cache"
                    << log4cpp::CategoryStream::ENDLINE );

}
