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

#ifndef ICE_CREAMPROXYMETHOD_H
#define ICE_CREAMPROXYMETHOD_H

#include <string>
#include <vector>
#include <map>

#include "glite/ce/cream-client-api-c/CreamProxy.h"

namespace glite {
namespace wms {
namespace ice {
namespace util {

    class CreamProxyMethod {
    public:
        virtual ~CreamProxyMethod( ) { };
        void execute( glite::ce::cream_client_api::soap_proxy::CreamProxy* p, int ntries );
    protected:
        CreamProxyMethod( ) { };
        virtual void method_call( glite::ce::cream_client_api::soap_proxy::CreamProxy* p ) = 0; // can throw anything
    };


    /**
     * This class is a wrapper around the CreamProxy::Register
     * method. The operator() calls the method, raising the
     * exact same exceptions as CreamProxy::Register
     */
    class CreamProxy_Register : public CreamProxyMethod {
    public:
        CreamProxy_Register( const std::string& service,
                             const std::string& deleg_service,
                             std::string& DelegID,
                             const std::string& JDL,
                             const std::string& certfile,
                             std::vector< std::string >& result,
                             const int lease_time,
                             const bool autostart );
    protected:
        void method_call( glite::ce::cream_client_api::soap_proxy::CreamProxy* p ) 
            throw(glite::ce::cream_client_api::cream_exceptions::BaseException&,
                  glite::ce::cream_client_api::cream_exceptions::InvalidArgumentException&,
                  glite::ce::cream_client_api::cream_exceptions::GridProxyDelegationException&,
                  glite::ce::cream_client_api::cream_exceptions::JobSubmissionDisabledException&,
                  glite::ce::cream_client_api::cream_exceptions::GenericException&,
                  glite::ce::cream_client_api::cream_exceptions::AuthenticationException&,
                  glite::ce::cream_client_api::cream_exceptions::AuthorizationException&,
                  glite::ce::cream_client_api::cream_exceptions::DelegationException&,
                  glite::ce::cream_client_api::cream_exceptions::InternalException&,
                  glite::ce::cream_client_api::soap_proxy::auth_ex&);

        const std::string m_service;
        const std::string m_deleg_service;
        std::string& m_DelegID;
        const std::string m_JDL;
        const std::string m_certfile;
        std::vector< std::string >& m_result;
        const int m_lease_time;
        const bool m_autostart;
    };

    /**
     * This class is a wrapper around the Cancel method of CreamProxy
     */
    class CreamProxy_Cancel : public CreamProxyMethod {
    public:
        CreamProxy_Cancel( const std::string& service, const std::vector<std::string>& jobids);
    protected:
        void method_call( glite::ce::cream_client_api::soap_proxy::CreamProxy* p ) 
            throw(glite::ce::cream_client_api::cream_exceptions::BaseException&,
		  glite::ce::cream_client_api::cream_exceptions::JobUnknownException&,
		  glite::ce::cream_client_api::cream_exceptions::InvalidArgumentException&,
		  glite::ce::cream_client_api::cream_exceptions::JobStatusInvalidException&,
		  glite::ce::cream_client_api::cream_exceptions::GenericException&,
		  glite::ce::cream_client_api::cream_exceptions::AuthenticationException&,
		  glite::ce::cream_client_api::cream_exceptions::AuthorizationException&,
		  glite::ce::cream_client_api::cream_exceptions::InternalException&,
		  glite::ce::cream_client_api::soap_proxy::auth_ex&);

        const std::string m_service;
        const std::vector< std::string> m_jobids;
    };


    /** 
     * Wrapper class around the Lease method of CreamProxy
     */
    class CreamProxy_Lease : public CreamProxyMethod {
    public:
        CreamProxy_Lease( const std::string& service,
                          const std::vector<std::string>& jobIDs,
                          int leaseTimeIncrement, 
                          std::map<std::string, time_t>& leaseTimes );
    protected:
        void method_call( glite::ce::cream_client_api::soap_proxy::CreamProxy* p ) 
            throw(glite::ce::cream_client_api::cream_exceptions::BaseException&,
		  glite::ce::cream_client_api::cream_exceptions::GenericException&,
		  glite::ce::cream_client_api::cream_exceptions::AuthenticationException&,
		  glite::ce::cream_client_api::cream_exceptions::AuthorizationException&,
		  glite::ce::cream_client_api::cream_exceptions::InternalException&,
		  glite::ce::cream_client_api::cream_exceptions::JobUnknownException&,
		  glite::ce::cream_client_api::cream_exceptions::JobStatusInvalidException&,
		  glite::ce::cream_client_api::soap_proxy::auth_ex&);

        const std::string m_service;
        const std::vector< std::string > m_jobids;
        int m_increment;
        std::map< std::string, time_t>& m_leaseTimes;
    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite
#endif
