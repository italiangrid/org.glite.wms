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
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include "CreamProxyMethod.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "iceConfManager.h"

namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;
namespace cream_ex = glite::ce::cream_client_api::cream_exceptions;
namespace api_util = glite::ce::cream_client_api::util;
using namespace glite::wms::ice::util;
using namespace std;

void CreamProxyMethod::execute( int ntries ) 
{
    static const char* method_name = "CreamProxyMethod::execute() - ";
    log4cpp::Category* m_log_dev( api_util::creamApiLogger::instance()->getLogger() );
    bool do_retry = true;
    int retry_count = 1;
    int conn_timeout = iceConfManager::getInstance()->getConfiguration()->ice()->soap_timeout(); // Timeout to set for each try
    int conn_timeout_delta = conn_timeout / 2; // Timeout increase for each try
    int delay = 1; // How much to sleep after each try

    for ( retry_count = 1; do_retry; ++retry_count ) {
        try {
            this->method_call( conn_timeout );            
            do_retry = false; // if everything goes well, do not retry
        } catch( cream_ex::ConnectionTimeoutException& ex ) {
            if ( retry_count < ntries ) {
                CREAM_SAFE_LOG( m_log_dev->warnStream()
                                << method_name 
                                << "Connection timed out to CREAM: \""
                                << ex.what()
                                << "\" on try " << retry_count << "/" << ntries
                                << ". Trying again in " << delay << " sec..."
                                << log4cpp::CategoryStream::ENDLINE );
                do_retry = true; // superfluous
                sleep( delay );
                delay += 2; // increments delay for each try
                conn_timeout += conn_timeout_delta;
            } else {
                CREAM_SAFE_LOG( m_log_dev->errorStream()
                                << method_name << "Connection timed out to CREAM: \""
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

CreamProxy_Register::CreamProxy_Register( const string& service_uri,
                                          const string& certfile,
                                          const soap_proxy::RegisterArrayRequest* req,
                                          soap_proxy::RegisterArrayResult* res ) : m_service_url( service_url ),
                                                                                   m_certificate( certfile ),
                                                                                   m_req( req ),
                                                                                   m_res( res )
    
{

}

void CreamProxy_Register::method_call( int timeout )  
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
    boost::scoped_ptr< soap_proxy::AbsCreamProxy > p( soap_rpxoy::CreamProxyFactory::make_CreamProxyRegister( m_req, m_res, timeout ) );   
    //try {
    p->setCredential( m_certfile );
    p->execute( m_service_uri );
    //} catch(cream_ex::ConnectionTimeoutException& ex) {
    //string what = ex.what() + string(" handling GridJobID [") + m_GridJobID + "]";        
    //throw cream_ex::ConnectionTimeoutException(what);
    //}
}


//////////////////////////////////////////////////////////////////////////////
//
// Start
//
//////////////////////////////////////////////////////////////////////////////

CreamProxy_Start::CreamProxy_Start( const string& service,
                                    const string& certfile,
                                    const soap_proxy::JobFilterWrapper* req,
                                    soap_proxy::ResultWrapper* res ):
    m_service( service ),
    m_certfile( certfile ),
    m_req( req ),
    m_res( res )
{

}

void CreamProxy_Start::method_call( int timeout )  
    throw(cream_ex::BaseException&,
          cream_ex::InvalidArgumentException&,
          cream_ex::GridProxyDelegationException&,
          cream_ex::JobSubmissionDisabledException&,
          cream_ex::GenericException&,
          cream_ex::AuthenticationException&,
          cream_ex::AuthorizationException&,
          cream_ex::DelegationException&,
          cream_ex::InternalException&,
          cream_ex::ConnectionTimeoutException&
          soap_proxy::auth_ex&)
{
//     log4cpp::Category* log_dev( api_util::creamApiLogger::instance()->getLogger() );

//     try {
//         p->Start( m_service.c_str(), m_jid );
//     } catch ( cream_ex::JobStatusInvalidException& ex ) {
//         CREAM_SAFE_LOG( log_dev->warnStream()
//                         << "CreamProxy_Start::method_call - JobStatusInvalidException to CREAM: \""
//                         << ex.what()
//                         << "\" for CREAM job id=\"" 
//                         << m_jid << "\". Assuming the job started."
//                         << log4cpp::CategoryStream::ENDLINE );
//     } catch( ... ) {
//         throw; // Rethrow everything else
//     }
}


//////////////////////////////////////////////////////////////////////////////
//
// Cancel
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_Cancel::CreamProxy_Cancel(( const string& service, 
                                       const string& certfile, 
                                       const soap_proxy::JobFilterWrapper* req,
                                       soap_proxy::ResultWrapper* res ) :
                                     m_service( service ),
                                     m_certfile( certfile ),
                                     m_req( req ),
                                     m_res( res )
{

}

void CreamProxy_Cancel::method_call( int timeout )  
                                     throw(cream_ex::BaseException&,
                                           cream_ex::InvalidArgumentException&,
                                           cream_ex::GridProxyDelegationException&,
                                           cream_ex::JobSubmissionDisabledException&,
                                           cream_ex::GenericException&,
                                           cream_ex::AuthenticationException&,
                                           cream_ex::AuthorizationException&,
                                           cream_ex::DelegationException&,
                                           cream_ex::InternalException&,
                                           cream_ex::ConnectionTimeoutException&,
                                           soap_proxy::auth_ex&)
{
    boost::scoped_ptr< soap_proxy::AbsCreamProxy > p( soap_rpxoy::CreamProxyFactory::make_CreamProxyCancel( m_req, m_res, timeout ) );   
    p->setCredential( m_certfile );
    p->execute( m_service_uri );
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

void CreamProxy_Lease::method_call( int timeout ) 
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
    //p->Lease( m_service.c_str(), m_jobids, m_increment, m_leaseTimes );
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

void CreamProxy_Info::method_call( int timeout ) 
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
    //p->Info( m_service.c_str(), m_jid, m_states, m_target, m_since, m_to );
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

void CreamProxy_Purge::method_call( int timeout ) 
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
    //p->Purge( m_service.c_str(), m_jid );
}


//////////////////////////////////////////////////////////////////////////////
//
// Delegate
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_Delegate::CreamProxy_Delegate( const string& delegation_id,
                                          const string& delegation_service,
                                          const string& certfile ) :
    m_delegation_id( delegation_id ),
    m_delegation_service( delegation_service ),
    m_certfile( certfile )
{

}

void CreamProxy_Delegate::method_call( int timeout ) 
    throw(cream_ex::DelegationException&,
          cream_ex::InternalException&,
          cream_ex::ConnectionTimeoutException&,
          soap_proxy::auth_ex&)
{    
    //p->Delegate( m_delegation_id, m_delegation_service, m_certfile );
}

