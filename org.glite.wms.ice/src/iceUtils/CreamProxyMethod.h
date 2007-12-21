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

#ifndef ICE_CREAMPROXYMETHOD_H
#define ICE_CREAMPROXYMETHOD_H

#include <string>
#include <vector>
#include <map>

#include "glite/ce/cream-client-api-c/AbsCreamProxy.h"

namespace _cream_ex = glite::ce::cream_client_api::cream_exceptions;
namespace _cream_api= glite::ce::cream_client_api::soap_proxy;

namespace glite {
  namespace ce {
    namespace cream_client_api {
      namespace soap_proxy {
	
	//class RegisterArrayRequest;
	//class RegisterArrayResult;
	class JobFilterWrapper;
	class StatusArrayResult;
	class ResultWrapper;
      }
    }
  }
}

namespace glite {
namespace wms {
namespace ice {
namespace util {

    /**
     * Wrapper class for CreamProxy methods. Subclasses of this class
     * can be used to try to send a given request to CREAM up to a
     * user-defined maximum number of times. This can be useful for
     * deal with transient communication failures.
     */
    class CreamProxyMethod {

      
      //theProxy;

    public:
        virtual ~CreamProxyMethod( ) { };
        /**
         * Executes the CREAM proxy method represented by this class.
         * The method is retried if it raises a GeneralException or an
         * InternalException. All other exceptions are immediately
         * rethrown; after the maximum retry count has been reached,
         * further exceptions are rethrown.
         * 
         * @param ntries the maximum number of times the CREAM method
         * will be executed. 
         */
	void execute( int ntries ); // may throw anything

    protected:
        CreamProxyMethod( ) { };
        /**
         * This method actually calls a CreamProxy method.
         *
         * @param timeout the timeout to set for execution of the
         * operation.  If the timeout expires and the operation has
         * not completed yet, this method should raise an appropriate
         * fault.
         */
        virtual void method_call( int timeout ) throw(_cream_ex::BaseException&,
						      _cream_ex::InvalidArgumentException&,
						      _cream_ex::GridProxyDelegationException&,
						      _cream_ex::JobSubmissionDisabledException&,
						      _cream_ex::JobStatusInvalidException&,
						      _cream_ex::JobUnknownException&,
						      _cream_ex::GenericException&,
						      _cream_ex::AuthenticationException&,
						      _cream_ex::AuthorizationException&,
						      _cream_ex::DelegationException&,
						      _cream_ex::InternalException&,
						      _cream_ex::ConnectionTimeoutException&,
						      _cream_api::auth_ex&) = 0;
    };


    /**
     * This class is a wrapper around the CreamProxy::Register
     * method. The operator() calls the method, raising the
     * exact same exceptions as CreamProxy::Register
     */
    class CreamProxy_Register : public CreamProxyMethod {
    public:
        CreamProxy_Register( const std::string& service_uri,
                             const std::string& certfile,
                             const glite::ce::cream_client_api::soap_proxy::AbsCreamProxy::RegisterArrayRequest* req, 
                             glite::ce::cream_client_api::soap_proxy::AbsCreamProxy::RegisterArrayResult* res );

	//	int retrieveNewLeaseTime( void ) { return m_lease_time; }

    protected:
        virtual void method_call(int timeout  ) throw(_cream_ex::BaseException&,
						      _cream_ex::InvalidArgumentException&,
						      _cream_ex::GridProxyDelegationException&,
						      _cream_ex::JobSubmissionDisabledException&,
						      _cream_ex::JobStatusInvalidException&,
						      _cream_ex::JobUnknownException&,
						      _cream_ex::GenericException&,
						      _cream_ex::AuthenticationException&,
						      _cream_ex::AuthorizationException&,
						      _cream_ex::DelegationException&,
						      _cream_ex::InternalException&,
						      _cream_ex::ConnectionTimeoutException&,
						      _cream_api::auth_ex&);


        const std::string& m_service_uri;        
        const std::string& m_certfile;
        const glite::ce::cream_client_api::soap_proxy::AbsCreamProxy::RegisterArrayRequest* m_req;
        glite::ce::cream_client_api::soap_proxy::AbsCreamProxy::RegisterArrayResult* m_res;
    };

    /**
     *
     * This class is a wrapper around the Cancel method of CreamProxy
     *
     */
    class CreamProxy_Cancel : public CreamProxyMethod {
    public:
        CreamProxy_Cancel( const std::string& service, 
                           const std::string& certfile, 
                           const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* req, 
                           glite::ce::cream_client_api::soap_proxy::ResultWrapper* res );
    protected:
        virtual void method_call( int timeout )throw(_cream_ex::BaseException&,
						      _cream_ex::InvalidArgumentException&,
						      _cream_ex::GridProxyDelegationException&,
						      _cream_ex::JobSubmissionDisabledException&,
						      _cream_ex::JobStatusInvalidException&,
						      _cream_ex::JobUnknownException&,
						      _cream_ex::GenericException&,
						      _cream_ex::AuthenticationException&,
						      _cream_ex::AuthorizationException&,
						      _cream_ex::DelegationException&,
						      _cream_ex::InternalException&,
						      _cream_ex::ConnectionTimeoutException&,
						      _cream_api::auth_ex&);

        const std::string& m_service;
        const std::string& m_certfile;
        const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* m_req;
	glite::ce::cream_client_api::soap_proxy::ResultWrapper* m_res;
    };


    /** 
     * Wrapper class around the Lease method of CreamProxy
     */
    class CreamProxy_Lease : public CreamProxyMethod {
    public:
        CreamProxy_Lease( const std::string& service,
			  const std::string& certfile,
                          const std::pair<std::string, time_t>& lease_IN,
                          std::pair<std::string, time_t>* lease_OUT );
    protected:
        virtual void method_call( int timeout ) throw(_cream_ex::BaseException&,
						      _cream_ex::InvalidArgumentException&,
						      _cream_ex::GridProxyDelegationException&,
						      _cream_ex::JobSubmissionDisabledException&,
						      _cream_ex::JobStatusInvalidException&,
						      _cream_ex::JobUnknownException&,
						      _cream_ex::GenericException&,
						      _cream_ex::AuthenticationException&,
						      _cream_ex::AuthorizationException&,
						      _cream_ex::DelegationException&,
						      _cream_ex::InternalException&,
						      _cream_ex::ConnectionTimeoutException&,
						      _cream_api::auth_ex&);

        const std::string& m_service;
	const std::string& m_certfile;
        const std::pair< std::string, time_t>& m_lease_IN;
	std::pair< std::string, time_t>* m_lease_OUT;
    };

    /**
     * Wrapper class around the Info method of CreamProxy
     */ 
    class CreamProxy_Status : public CreamProxyMethod {
     public:
      CreamProxy_Status( const std::string& service, 
			 const std::string& certfile, 
			 const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* req, 
			 glite::ce::cream_client_api::soap_proxy::AbsCreamProxy::StatusArrayResult* res );
    protected:        
        virtual void method_call( int timeout ) throw(_cream_ex::BaseException&,
						      _cream_ex::InvalidArgumentException&,
						      _cream_ex::GridProxyDelegationException&,
						      _cream_ex::JobSubmissionDisabledException&,
						      _cream_ex::JobStatusInvalidException&,
						      _cream_ex::JobUnknownException&,
						      _cream_ex::GenericException&,
						      _cream_ex::AuthenticationException&,
						      _cream_ex::AuthorizationException&,
						      _cream_ex::DelegationException&,
						      _cream_ex::InternalException&,
						      _cream_ex::ConnectionTimeoutException&,
						      _cream_api::auth_ex&);

        const std::string& m_service;
	const std::string& m_certfile;
	const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* m_req;
	glite::ce::cream_client_api::soap_proxy::AbsCreamProxy::StatusArrayResult* m_res;
    };


    /**
     * Wrapper class around the Purge method of CreamProxy
     */ 
    class CreamProxy_Purge : public CreamProxyMethod {
    public:
        CreamProxy_Purge( const std::string& service,
			  const std::string& certfile,
                          const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* req, 
			  glite::ce::cream_client_api::soap_proxy::ResultWrapper* res );
    protected:        
        virtual void method_call(int timeout  ) throw(_cream_ex::BaseException&,
						      _cream_ex::InvalidArgumentException&,
						      _cream_ex::GridProxyDelegationException&,
						      _cream_ex::JobSubmissionDisabledException&,
						      _cream_ex::JobStatusInvalidException&,
						      _cream_ex::JobUnknownException&,
						      _cream_ex::GenericException&,
						      _cream_ex::AuthenticationException&,
						      _cream_ex::AuthorizationException&,
						      _cream_ex::DelegationException&,
						      _cream_ex::InternalException&,
						      _cream_ex::ConnectionTimeoutException&,
						      _cream_api::auth_ex&);
        
        const std::string& m_service;
	const std::string& m_certfile;
	const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* m_req;
	glite::ce::cream_client_api::soap_proxy::ResultWrapper* m_res;
    };


    /**
     * Wrapper class around the Start method of CreamProxy
     */ 
    class CreamProxy_Start : public CreamProxyMethod {
    public:
        CreamProxy_Start( const std::string& service,
                          const std::string& certfile,
                          const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* req,
                          glite::ce::cream_client_api::soap_proxy::ResultWrapper* res );
    protected:        
        virtual void method_call( int timeout ) throw(_cream_ex::BaseException&,
						      _cream_ex::InvalidArgumentException&,
						      _cream_ex::GridProxyDelegationException&,
						      _cream_ex::JobSubmissionDisabledException&,
						      _cream_ex::JobStatusInvalidException&,
						      _cream_ex::JobUnknownException&,
						      _cream_ex::GenericException&,
						      _cream_ex::AuthenticationException&,
						      _cream_ex::AuthorizationException&,
						      _cream_ex::DelegationException&,
						      _cream_ex::InternalException&,
						      _cream_ex::ConnectionTimeoutException&,
						      _cream_api::auth_ex&);
        
        const std::string m_service;
        const std::string m_certfile;
        const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* m_req;
        glite::ce::cream_client_api::soap_proxy::ResultWrapper* m_res;
    };

    /**
     * Wrapper class around the Delegate method of CreamProxy
     */ 
    class CreamProxy_Delegate : public CreamProxyMethod {
    public:
      CreamProxy_Delegate( const std::string& service,
			   const std::string& certfile,
			   const std::string& delegation_id);
    protected:        
        virtual void method_call( int timeout ) throw(_cream_ex::BaseException&,
						      _cream_ex::InvalidArgumentException&,
						      _cream_ex::GridProxyDelegationException&,
						      _cream_ex::JobSubmissionDisabledException&,
						      _cream_ex::JobStatusInvalidException&,
						      _cream_ex::JobUnknownException&,
						      _cream_ex::GenericException&,
						      _cream_ex::AuthenticationException&,
						      _cream_ex::AuthorizationException&,
						      _cream_ex::DelegationException&,
						      _cream_ex::InternalException&,
						      _cream_ex::ConnectionTimeoutException&,
						      _cream_api::auth_ex&);

        const std::string m_service;
        const std::string m_certfile;
        const std::string m_delegation_id;
    };


    /**
     * Wrapper class around the ProxyRenew method of CreamProxy
     */ 
    class CreamProxy_ProxyRenew : public CreamProxyMethod {
    public:
      CreamProxy_ProxyRenew( const std::string& service,
			     const std::string& certfile,
			     const std::string& delegation_id);
    protected:        
        virtual void method_call( int timeout ) throw(_cream_ex::BaseException&,
						      _cream_ex::InvalidArgumentException&,
						      _cream_ex::GridProxyDelegationException&,
						      _cream_ex::JobSubmissionDisabledException&,
						      _cream_ex::JobStatusInvalidException&,
						      _cream_ex::JobUnknownException&,
						      _cream_ex::GenericException&,
						      _cream_ex::AuthenticationException&,
						      _cream_ex::AuthorizationException&,
						      _cream_ex::DelegationException&,
						      _cream_ex::InternalException&,
						      _cream_ex::ConnectionTimeoutException&,
						      _cream_api::auth_ex&);

        const std::string m_service;
        const std::string m_certfile;
        const std::string m_delegation_id;
    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite
#endif
