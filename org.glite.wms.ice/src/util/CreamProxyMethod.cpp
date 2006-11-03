/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * Wrapper classes for CreamProxy methods
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
    for ( int t=1; ; ++t ) {
        try {
            this->method_call( p );            
        } catch( cream_ex::GenericException& ex ) {
            if ( t < ntries ) {
                CREAM_SAFE_LOG( m_log_dev->warnStream()
                                << "Cream operation raised a GenericException "
                                << ex.what()
                                << " on try " << t << "/" << ntries
                                << ". Trying again..."
                                << log4cpp::CategoryStream::ENDLINE );
                sleep( 1 );
            } else {
                CREAM_SAFE_LOG( m_log_dev->errorStream()
                                << "Cream operation raised a GenericException "
                                << ex.what()
                                << " on try " << t << "/" << ntries
                                << ". Giving up."
                                << log4cpp::CategoryStream::ENDLINE );
                throw; // rethrow
            }            
        } catch( cream_ex::InternalException& ex ) {
            if ( t < ntries ) {
                CREAM_SAFE_LOG( m_log_dev->warnStream()
                                << "Cream operation raised an InternalException "
                                << ex.what()
                                << " on try " << t << "/" << ntries
                                << ". Trying again..."
                                << log4cpp::CategoryStream::ENDLINE );
                sleep( 1 );
            } else {
                CREAM_SAFE_LOG( m_log_dev->errorStream()
                                << "Cream operation raised an InternalException "
                                << ex.what()
                                << " on try " << t << "/" << ntries
                                << ". Giving up."
                                << log4cpp::CategoryStream::ENDLINE );
                throw; // rethrow
            }            
        } catch( ... ) {
            throw; // rethrow anything else
        }
        break; // everything went ok, exit the loop now
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
           cream_ex::GenericException&,
           cream_ex::AuthenticationException&,
           cream_ex::AuthorizationException&,
           cream_ex::InternalException&,
           soap_proxy::invalidTimestamp_ex&,
           soap_proxy::auth_ex&)
{    
    p->Info( m_service.c_str(), m_jid, m_states, m_target, m_since, m_to );
}
