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
 * CREAM proxy methods for ICE
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceConfManager.h"
#include "CreamProxyMethod.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;
namespace cream_ex = glite::ce::cream_client_api::cream_exceptions;
namespace api_util = glite::ce::cream_client_api::util;
using namespace glite::wms::ice::util;
using namespace std;

void CreamProxyMethod::execute( soap_proxy::CreamProxy* p, int ntries ) 
{
    log4cpp::Category* m_log_dev( api_util::creamApiLogger::instance()->getLogger() );
    bool do_retry = true;
    int retry_count = 1;
    for ( retry_count = 1; do_retry; ++retry_count ) {
        try {
            this->method_call( p );            
            do_retry = false; // if everything goes well, do not retry
        } catch( cream_ex::ConnectionTimeoutException& ex ) {
            if ( retry_count < ntries ) {
                CREAM_SAFE_LOG( m_log_dev->warnStream()
                                << "Connection timed out to CREAM: \""
                                << ex.what()
                                << "\" on try " << retry_count << "/" << ntries
                                << ". Trying again in 1 sec..."
                                << log4cpp::CategoryStream::ENDLINE );
                do_retry = true; // superfluous
                sleep( 1 );
            } else {
                CREAM_SAFE_LOG( m_log_dev->errorStream()
                                << "Connection timed out to CREAM: \""
                                << ex.what()
                                << "\" on try " << retry_count << "/" << ntries
                                << ". Giving up."
                                << log4cpp::CategoryStream::ENDLINE );
                throw; // rethrow
            }            
        } catch( ... ) {
            throw; // rethrow anything else
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Register
//
//////////////////////////////////////////////////////////////////////////////

CreamProxy_Register::CreamProxy_Register( const string& service,
                                          const string& deleg_service,
                                          string& DelegID,
                                          const string& JDL,
                                          const string& certfile,
                                          vector<string>& result,
                                          const int lease_time,
                                          const bool autostart ) :
    m_service( service ),
    m_deleg_service( deleg_service ),
    m_DelegID( DelegID ),
    m_JDL( JDL ),
    m_certfile( certfile ),
    m_result( result ),
    m_lease_time( lease_time ),
    m_autostart( autostart )
{

}

void CreamProxy_Register::method_call( soap_proxy::CreamProxy* p )  
    throw(cream_ex::BaseException&,
          cream_ex::InvalidArgumentException&,
          cream_ex::ConnectionTimeoutException&,
          cream_ex::GridProxyDelegationException&,
          cream_ex::JobSubmissionDisabledException&,
          cream_ex::GenericException&,
          cream_ex::AuthenticationException&,
          cream_ex::AuthorizationException&,
          cream_ex::DelegationException&,
          cream_ex::InternalException&,
          soap_proxy::auth_ex& )
{
    p->Register( m_service.c_str(), m_deleg_service.c_str(), m_DelegID, m_JDL, m_certfile, m_result, m_lease_time, m_autostart );
}

//////////////////////////////////////////////////////////////////////////////
//
// Cancel
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_Cancel::CreamProxy_Cancel( const std::string& service, const std::vector< std::string>& jobids ) :
    m_service( service ),
    m_jobids( jobids )
{

}

void CreamProxy_Cancel::method_call( soap_proxy::CreamProxy* p )  
    throw(cream_ex::BaseException&,
          cream_ex::JobUnknownException&,
          cream_ex::ConnectionTimeoutException&,
          cream_ex::InvalidArgumentException&,
          cream_ex::JobStatusInvalidException&,
          cream_ex::GenericException&,
          cream_ex::AuthenticationException&,
          cream_ex::AuthorizationException&,
          cream_ex::InternalException&,
          soap_proxy::auth_ex&)
{
    p->Cancel( m_service.c_str(), m_jobids );
}
    
//////////////////////////////////////////////////////////////////////////////
//
// Lease
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_Lease::CreamProxy_Lease( const string& service,
                                    const vector<string>& jobIDs,
                                    int leaseTimeIncrement, 
                                    map<string, time_t>& leaseTimes ) :
    m_service( service ),
    m_jobids( jobIDs ),
    m_increment( leaseTimeIncrement ),
    m_leaseTimes( leaseTimes )
{

}

void CreamProxy_Lease::method_call( soap_proxy::CreamProxy* p ) 
            throw(cream_ex::BaseException&,
		  cream_ex::GenericException&,
                  cream_ex::ConnectionTimeoutException&,
		  cream_ex::AuthenticationException&,
		  cream_ex::AuthorizationException&,
		  cream_ex::InternalException&,
		  cream_ex::JobUnknownException&,
		  cream_ex::JobStatusInvalidException&,
		  soap_proxy::auth_ex&)
{
    p->Lease( m_service.c_str(), m_jobids, m_increment, m_leaseTimes );
}

//////////////////////////////////////////////////////////////////////////////
//
// Info
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_Info::CreamProxy_Info( const string& service,
                                  const vector<string>& JID,
                                  const vector<string>& STATES,
                                  vector<soap_proxy::JobInfo>& target,
                                  int since,
                                  int to ) :
    m_service( service ),
    m_jid( JID ),
    m_states( STATES ),
    m_target( target ),
    m_since( since ),
    m_to( to )
{

}

void CreamProxy_Info::method_call( soap_proxy::CreamProxy* p ) 
    throw( cream_ex::BaseException&,
           cream_ex::JobUnknownException&,
           cream_ex::InvalidArgumentException&,
           cream_ex::ConnectionTimeoutException&,
           cream_ex::GenericException&,
           cream_ex::AuthenticationException&,
           cream_ex::AuthorizationException&,
           cream_ex::InternalException&,
           soap_proxy::invalidTimestamp_ex&,
           soap_proxy::auth_ex&)
{    
    p->Info( m_service.c_str(), m_jid, m_states, m_target, m_since, m_to );
}

//////////////////////////////////////////////////////////////////////////////
//
// Purge
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_Purge::CreamProxy_Purge( const string& service,
                                   const vector<string>& jobids ) :
    m_service( service ),
    m_jid( jobids )
{

}

void CreamProxy_Purge::method_call( soap_proxy::CreamProxy* p ) 
    throw(cream_ex::BaseException&,                  
          cream_ex::JobUnknownException&,
          cream_ex::InvalidArgumentException&,
          cream_ex::GenericException&,
          cream_ex::AuthenticationException&,
          cream_ex::AuthorizationException&,
          cream_ex::InternalException&,
          cream_ex::ConnectionTimeoutException&,
          soap_proxy::auth_ex&)
{    
    p->Purge( m_service.c_str(), m_jid );
}

