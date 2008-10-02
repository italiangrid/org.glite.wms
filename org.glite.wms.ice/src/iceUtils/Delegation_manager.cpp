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
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "iceUtils.h"
#include <stdexcept>

namespace cream_api = glite::ce::cream_client_api::soap_proxy;
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
    m_max_size( 1000 ), // FIXME: Hardcoded default
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

string Delegation_manager::delegate( const CreamJob& job, bool force ) throw( std::exception )
{
    boost::recursive_mutex::scoped_lock L( m_mutex );

    static char* method_name = "Delegation_manager::delegate() - ";
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
                         );  
        throw runtime_error( string( "Cannot open proxy file " + certfile ) );
    }

    cream_api::VOMSWrapper V( certfile );
    if( !V.IsValid() ) {
      CREAM_SAFE_LOG( m_log_dev->errorStream()
		      << method_name
		      << "Cannot validate proxy file ["
		      << certfile 
		      << "]. Error is: "
		      << V.getErrorMessage( )
		       );        
      throw runtime_error( V.getErrorMessage() );
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
    typedef t_delegation_set::nth_index<2>::type t_delegation_by_seq;
    t_delegation_by_seq& delegation_by_seq( m_delegation_set.get<2>() );

    t_delegation_by_key::iterator it = delegation_by_key_view.find( boost::make_tuple(str_sha1_digest,cream_url));

    if ( force && delegation_by_key_view.end() != it ) {
        delegation_by_key_view.erase( it );
        it = delegation_by_key_view.end();
    }

    if ( delegation_by_key_view.end() == it ) {
        // Delegation id not found (or force). Performs a new delegation   
        time_t expiration_time;

        // The delegation ID is the "canonized" GRID job id
        delegation_id = canonizeString( job.getGridJobID() );

        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "Creating new delegation "
                        << "with delegation id "
                        << delegation_id
                        << " CREAM URL "
                        << cream_url
                        << " Delegation URL "
                        << cream_deleg_url
                        << " user DN "
                        << V.getDN( )
                        << " proxy hash "
                        << str_sha1_digest
                         );
        
        try {
            // Gets the proxy expiration time
            expiration_time = V.getProxyTimeEnd( );            
	    CreamProxy_Delegate( cream_deleg_url, certfile, delegation_id ).execute( 3 );
        } catch( exception& ex ) {
            // Delegation failed
            CREAM_SAFE_LOG( m_log_dev->errorStream()
                            << method_name
                            << "FAILED Creation of a new delegation "
                            << "with delegation id "
                            << delegation_id
                            << " CREAM URL "
                            << cream_url
                            << " Delegation URL "
                            << cream_deleg_url
                            << " user DN "
                            << V.getDN( )
                            << " proxy hash "
                            << str_sha1_digest
			    << " - ERROR is: ["
			    << ex.what() << "]"
                             );
            throw runtime_error(ex.what());
        }  catch( ... ) {
            // Delegation failed
            CREAM_SAFE_LOG( m_log_dev->errorStream()
                            << method_name
                            << "FAILED Creation of a new delegation "
                            << "with delegation id "
                            << delegation_id
                            << " CREAM URL "
                            << cream_url
                            << " Delegation URL "
                            << cream_deleg_url
                            << " user DN "
                            << V.getDN( )
                            << " proxy hash "
                            << str_sha1_digest
                             );
            throw runtime_error( "Delegation failed" );
        }     
        // Inserts the new delegation ID into the delegation set
        m_delegation_set.insert( table_entry( str_sha1_digest, cream_url, expiration_time, delegation_id ) );

        // Inserts into the front of the list; this is guaranteed to
        // be a new element, as it has not been found in this "if"
        // branch
        delegation_by_seq.push_front( table_entry( str_sha1_digest, cream_url, expiration_time, delegation_id ) );
        if ( m_delegation_set.size() > m_max_size ) {
            if ( m_log_dev->isDebugEnabled() ) {

                // drops least recently used element
                const table_entry last_elem( delegation_by_seq.back() );
                
                CREAM_SAFE_LOG( m_log_dev->debugStream()
                                << method_name
                                << "Dropping entry for delegation id "
                                << last_elem.m_delegation_id
                                << " CREAM URL "
                                << last_elem.m_cream_url
                                << " proxy hash "
                                << last_elem.m_sha1_digest
                                 );
            }
            delegation_by_seq.pop_back();
        }
    } else {
        // Delegation id FOUND. Returns it
        delegation_id = it->m_delegation_id;

        // Project the iterator to the sequencedd index
        t_delegation_by_seq::iterator it_seq( m_delegation_set.project<2>( it ) );

        // Relocates the newly-found element to the front of the list
        delegation_by_seq.relocate( delegation_by_seq.begin(), it_seq );

        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "Using existing delegation id "
                        << delegation_id
                        << " for CREAM URL "
                        << cream_url
                        << " Delegation URL "
                        << cream_deleg_url
                        << " user DN "
                        << V.getDN( )
                         );
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

    if ( size_before != size_after ) {
        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "Purged "
                        << size_before - size_after
                        << " elements from the delegation cache"
                        );
    }
}
