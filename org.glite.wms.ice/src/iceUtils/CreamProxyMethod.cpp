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


#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include "CreamProxyMethod.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "iceConfManager.h"

#include <boost/scoped_ptr.hpp>

namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;
namespace cream_ex = glite::ce::cream_client_api::cream_exceptions;
//namespace cream_api = glite::ce::cream_client_api;
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
                                          const soap_proxy::AbsCreamProxy::RegisterArrayRequest* req,
                                          soap_proxy::AbsCreamProxy::RegisterArrayResult* res,
					  const string& iceid ) 
  : m_service_uri( service_uri ),
    m_certfile( certfile ),
    m_req( req ),
    m_res( res ),
    m_iceid( iceid )  
{
  
}

void CreamProxy_Register::method_call( int timeout ) 
  throw(cream_ex::BaseException&,
	cream_ex::InvalidArgumentException&,
	cream_ex::GridProxyDelegationException&,
	cream_ex::JobSubmissionDisabledException&,
	cream_ex::JobStatusInvalidException&,
	cream_ex::JobUnknownException&,
	cream_ex::GenericException&,
	cream_ex::AuthenticationException&,
	cream_ex::AuthorizationException&,
	cream_ex::DelegationException&,
	cream_ex::InternalException&,
	cream_ex::ConnectionTimeoutException&,
	soap_proxy::auth_ex&) 
{
  boost::scoped_ptr< soap_proxy::AbsCreamProxy > theProxy( soap_proxy::CreamProxyFactory::make_CreamProxyRegister( m_req, m_res, timeout ) );   
  theProxy->setCredential( m_certfile );
  theProxy->setSoapHeader( m_iceid );
  theProxy->execute( m_service_uri );
  
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
	cream_ex::JobStatusInvalidException&,
	cream_ex::JobUnknownException&,
	cream_ex::GenericException&,
	cream_ex::AuthenticationException&,
	cream_ex::AuthorizationException&,
	cream_ex::DelegationException&,
	cream_ex::InternalException&,
	cream_ex::ConnectionTimeoutException&,
	soap_proxy::auth_ex&)  
  
  
{
  boost::scoped_ptr< soap_proxy::AbsCreamProxy > theProxy( soap_proxy::CreamProxyFactory::make_CreamProxyStart( m_req, m_res, timeout ) );
  theProxy->setCredential( m_certfile );
  theProxy->execute( m_service );
}


//////////////////////////////////////////////////////////////////////////////
//
// Cancel
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_Cancel::CreamProxy_Cancel( const string& service, 
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
	cream_ex::JobStatusInvalidException&,
	cream_ex::JobUnknownException&,
	cream_ex::GenericException&,
	cream_ex::AuthenticationException&,
	cream_ex::AuthorizationException&,
	cream_ex::DelegationException&,
	cream_ex::InternalException&,
	cream_ex::ConnectionTimeoutException&,
	soap_proxy::auth_ex&)   
                                     
{
  boost::scoped_ptr< soap_proxy::AbsCreamProxy > theProxy(soap_proxy::CreamProxyFactory::make_CreamProxyCancel( m_req, m_res, timeout ));
  theProxy->setCredential( m_certfile );
  theProxy->execute( m_service );
}
    
//////////////////////////////////////////////////////////////////////////////
//
// Lease
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_Lease::CreamProxy_Lease( const std::string& service,
				    const std::string& certfile,
				    const std::pair<std::string, time_t>& lease_IN,
				    std::pair<std::string, time_t>* lease_OUT ) :
  m_service( service ),
  m_certfile( certfile ),
  m_lease_IN( lease_IN ),
  m_lease_OUT( lease_OUT )
{
  
}

void CreamProxy_Lease::method_call( int timeout ) 
  throw(cream_ex::BaseException&,
	cream_ex::InvalidArgumentException&,
	cream_ex::GridProxyDelegationException&,
	cream_ex::JobSubmissionDisabledException&,
	cream_ex::JobStatusInvalidException&,
	cream_ex::JobUnknownException&,
	cream_ex::GenericException&,
	cream_ex::AuthenticationException&,
	cream_ex::AuthorizationException&,
	cream_ex::DelegationException&,
	cream_ex::InternalException&,
	cream_ex::ConnectionTimeoutException&,
	soap_proxy::auth_ex&)   
            
{
  boost::scoped_ptr< soap_proxy::AbsCreamProxy > theProxy(soap_proxy::CreamProxyFactory::make_CreamProxyLease( m_lease_IN, m_lease_OUT, timeout ));
  theProxy->setCredential( m_certfile );
  theProxy->execute( m_service );
}

//////////////////////////////////////////////////////////////////////////////
//
// Info
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_Info::CreamProxy_Info( const std::string& service, 
				  const std::string& certfile, 
				  const soap_proxy::JobFilterWrapper* req, 
				  soap_proxy::AbsCreamProxy::InfoArrayResult* res) :
  m_service( service ),
  m_certfile( certfile ),
  m_req( req ),
  m_res( res )
{
  
}

void CreamProxy_Info::method_call( int timeout )  
  throw(cream_ex::BaseException&,
	cream_ex::InvalidArgumentException&,
	cream_ex::GridProxyDelegationException&,
	cream_ex::JobSubmissionDisabledException&,
	cream_ex::JobStatusInvalidException&,
	cream_ex::JobUnknownException&,
	cream_ex::GenericException&,
	cream_ex::AuthenticationException&,
	cream_ex::AuthorizationException&,
	cream_ex::DelegationException&,
	cream_ex::InternalException&,
	cream_ex::ConnectionTimeoutException&,
	soap_proxy::auth_ex&) 
{   
  boost::scoped_ptr< soap_proxy::AbsCreamProxy > theProxy( soap_proxy::CreamProxyFactory::make_CreamProxyInfo( m_req, m_res, timeout ) );   
  theProxy->setCredential( m_certfile ); 
  theProxy->execute( m_service );
}

//////////////////////////////////////////////////////////////////////////////
//
// Purge
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_Purge::CreamProxy_Purge( const std::string& service,
			  const std::string& certfile,
                          const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* req, 
			  glite::ce::cream_client_api::soap_proxy::ResultWrapper* res ) :
    m_service( service ),
    m_certfile( certfile ),
    m_req( req ),
    m_res( res )
{
  
}

void CreamProxy_Purge::method_call( int timeout ) 
  throw(cream_ex::BaseException&,
	cream_ex::InvalidArgumentException&,
	cream_ex::GridProxyDelegationException&,
	cream_ex::JobSubmissionDisabledException&,
	cream_ex::JobStatusInvalidException&,
	cream_ex::JobUnknownException&,
	cream_ex::GenericException&,
	cream_ex::AuthenticationException&,
	cream_ex::AuthorizationException&,
	cream_ex::DelegationException&,
	cream_ex::InternalException&,
	cream_ex::ConnectionTimeoutException&,
	soap_proxy::auth_ex&)
{    
  boost::scoped_ptr< soap_proxy::AbsCreamProxy > theProxy( soap_proxy::CreamProxyFactory::make_CreamProxyPurge( m_req, m_res, timeout ) );   
  theProxy->setCredential( m_certfile );
  theProxy->execute( m_service );
}


//////////////////////////////////////////////////////////////////////////////
//
// Delegate
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_Delegate::CreamProxy_Delegate( const std::string& service,
					  const std::string& certfile,
					  const std::string& delegation_id ) :
  m_service( service ),
  m_certfile( certfile ),
  m_delegation_id( delegation_id )
{
  
}

void CreamProxy_Delegate::method_call( int timeout ) 
  throw(cream_ex::BaseException&,
	cream_ex::InvalidArgumentException&,
	cream_ex::GridProxyDelegationException&,
	cream_ex::JobSubmissionDisabledException&,
	cream_ex::JobStatusInvalidException&,
	cream_ex::JobUnknownException&,
	cream_ex::GenericException&,
	cream_ex::AuthenticationException&,
	cream_ex::AuthorizationException&,
	cream_ex::DelegationException&,
	cream_ex::InternalException&,
	cream_ex::ConnectionTimeoutException&,
	soap_proxy::auth_ex&)
{ 
  boost::scoped_ptr< soap_proxy::AbsCreamProxy > theProxy( soap_proxy::CreamProxyFactory::make_CreamProxyDelegate( m_delegation_id, timeout ) );   
  theProxy->setCredential( m_certfile );   
  theProxy->execute( m_service );
}


//////////////////////////////////////////////////////////////////////////////
//
// ProxyRenew
//
//////////////////////////////////////////////////////////////////////////////
CreamProxy_ProxyRenew::CreamProxy_ProxyRenew( const std::string& service,
					    const std::string& certfile,
					    const std::string& delegation_id ) :
  m_service( service ),
  m_certfile( certfile ),
  m_delegation_id( delegation_id )
{

}

void CreamProxy_ProxyRenew::method_call( int timeout ) 
  throw(cream_ex::BaseException&,
	cream_ex::InvalidArgumentException&,
	cream_ex::GridProxyDelegationException&,
	cream_ex::JobSubmissionDisabledException&,
	cream_ex::JobStatusInvalidException&,
	cream_ex::JobUnknownException&,
	cream_ex::GenericException&,
	cream_ex::AuthenticationException&,
	cream_ex::AuthorizationException&,
	cream_ex::DelegationException&,
	cream_ex::InternalException&,
	cream_ex::ConnectionTimeoutException&,
	soap_proxy::auth_ex&)
{    
    //p->Delegate( m_delegation_id, m_delegation_service, m_certfile );
  boost::scoped_ptr< soap_proxy::AbsCreamProxy > p( soap_proxy::CreamProxyFactory::make_CreamProxy_ProxyRenew( m_delegation_id, timeout ) );   
  p->setCredential( m_certfile );
  p->execute( m_service );
}

