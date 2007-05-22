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

#include "glite/ce/cream-client-api-c/CreamProxy.h"

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
    public:
        virtual ~CreamProxyMethod( ) { };
        /**
         * Executes the CREAM proxy method represented by this class.
         * The method is retried if it raises a GeneralException or an
         * InternalException. All other exceptions are immediately
         * rethrown; after the maximum retry count has been reached,
         * further exceptions are rethrown.
         * 
         * @param p a pointer to the CreamProxy object to use. Caller
         * retains ownership of this pointer, thus the caller is
         * responsible for freeing memory pointed to by p.
         *
         * @param ntries the maximum number of times the CREAM method
         * will be executed. 
         */
        virtual void execute( glite::ce::cream_client_api::soap_proxy::CreamProxy* p, int ntries ); // may throw anything
    protected:
        CreamProxyMethod( ) { };
        /**
         * This method actually calls a CreamProxy method.
         *
         * @param p the CreamProxy object on which the method should
         * be called. Caller retains ownership of p. Caller should
         * ensure that p points to a valid CreamProxy instance (i.e.,
         * p != 0).
         */
        virtual void method_call( glite::ce::cream_client_api::soap_proxy::CreamProxy* p ) = 0; // may throw anything
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
			     const std::string& GID,
                             const std::string& certfile,
                             std::vector< std::string >& result,
                             const int lease_time,
                             const bool autostart );

	int retrieveNewLeaseTime( void ) {return m_lease_time;}

    protected:
        void method_call( glite::ce::cream_client_api::soap_proxy::CreamProxy* p ) 
            throw(glite::ce::cream_client_api::cream_exceptions::BaseException&,
                  glite::ce::cream_client_api::cream_exceptions::ConnectionTimeoutException&,
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
        int m_lease_time;
        const bool m_autostart;
	const std::string m_GridJobID;
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
                  glite::ce::cream_client_api::cream_exceptions::ConnectionTimeoutException&,
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
                  glite::ce::cream_client_api::cream_exceptions::ConnectionTimeoutException&,
		  glite::ce::cream_client_api::cream_exceptions::GenericException&,
		  glite::ce::cream_client_api::cream_exceptions::AuthenticationException&,
		  glite::ce::cream_client_api::cream_exceptions::AuthorizationException&,
		  glite::ce::cream_client_api::cream_exceptions::InternalException&,
		  glite::ce::cream_client_api::cream_exceptions::JobUnknownException&,
		  glite::ce::cream_client_api::cream_exceptions::JobStatusInvalidException&,
		  glite::ce::cream_client_api::soap_proxy::auth_ex&);

        const std::string m_service;
        const std::vector< std::string > m_jobids;
        const int m_increment;
        std::map< std::string, time_t>& m_leaseTimes;
    };

    /**
     * Wrapper class around the Info method of CreamProxy
     */ 
    class CreamProxy_Info : public CreamProxyMethod {
    public:
        CreamProxy_Info( const std::string& service,
                         const std::vector<std::string>& JID,
                         const std::vector<std::string>& STATES,
                         std::vector<glite::ce::cream_client_api::soap_proxy::JobInfo>& target,
                         int since=-1,
                         int to=-1 );
    protected:        
        void method_call( glite::ce::cream_client_api::soap_proxy::CreamProxy* p ) 
	    throw(glite::ce::cream_client_api::cream_exceptions::BaseException&,
                  glite::ce::cream_client_api::cream_exceptions::ConnectionTimeoutException&,
		  glite::ce::cream_client_api::cream_exceptions::JobUnknownException&,
		  glite::ce::cream_client_api::cream_exceptions::InvalidArgumentException&,
		  glite::ce::cream_client_api::cream_exceptions::GenericException&,
		  glite::ce::cream_client_api::cream_exceptions::AuthenticationException&,
		  glite::ce::cream_client_api::cream_exceptions::AuthorizationException&,
		  glite::ce::cream_client_api::cream_exceptions::InternalException&,
		  glite::ce::cream_client_api::soap_proxy::invalidTimestamp_ex&,
		  glite::ce::cream_client_api::soap_proxy::auth_ex&);

        const std::string m_service;
        const std::vector<std::string> m_jid;
        const std::vector<std::string> m_states;
        std::vector<glite::ce::cream_client_api::soap_proxy::JobInfo>& m_target;
        int m_since;
        int m_to;
    };


    /**
     * Wrapper class around the Purge method of CreamProxy
     */ 
    class CreamProxy_Purge : public CreamProxyMethod {
    public:
        CreamProxy_Purge( const std::string& service,
                          const std::vector<std::string>& jobids );
    protected:        
        void method_call( glite::ce::cream_client_api::soap_proxy::CreamProxy* p ) 
            throw(glite::ce::cream_client_api::cream_exceptions::BaseException&,                  glite::ce::cream_client_api::cream_exceptions::JobUnknownException&,
                  glite::ce::cream_client_api::cream_exceptions::InvalidArgumentException&,
                  glite::ce::cream_client_api::cream_exceptions::GenericException&,
                  glite::ce::cream_client_api::cream_exceptions::AuthenticationException&,
                  glite::ce::cream_client_api::cream_exceptions::AuthorizationException&,
                  glite::ce::cream_client_api::cream_exceptions::InternalException&,
                  glite::ce::cream_client_api::cream_exceptions::ConnectionTimeoutException&,
                  glite::ce::cream_client_api::soap_proxy::auth_ex&);
        
        const std::string m_service;
        const std::vector<std::string> m_jid;
    };


    /**
     * Wrapper class around the Start method of CreamProxy
     */ 
    class CreamProxy_Start : public CreamProxyMethod {
    public:
        CreamProxy_Start( const std::string& service,
                          const std::string& jobid );
        virtual void execute( glite::ce::cream_client_api::soap_proxy::CreamProxy* p, int ntries ); // This method behaves differently for Start
    protected:        
        void method_call( glite::ce::cream_client_api::soap_proxy::CreamProxy* p ) 
            throw(glite::ce::cream_client_api::cream_exceptions::BaseException&,                  glite::ce::cream_client_api::cream_exceptions::JobUnknownException&,
                  glite::ce::cream_client_api::cream_exceptions::InvalidArgumentException&,
                  glite::ce::cream_client_api::cream_exceptions::JobStatusInvalidException&,
                  glite::ce::cream_client_api::cream_exceptions::GenericException&,
                  glite::ce::cream_client_api::cream_exceptions::AuthenticationException&,
                  glite::ce::cream_client_api::cream_exceptions::AuthorizationException&,
                  glite::ce::cream_client_api::cream_exceptions::InternalException&,
                  glite::ce::cream_client_api::cream_exceptions::ConnectionTimeoutException&,
                  glite::ce::cream_client_api::cream_exceptions::JobSubmissionDisabledException&,
                  glite::ce::cream_client_api::soap_proxy::auth_ex&);
        
        const std::string m_service;
        const std::string m_jid;
    };


} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite
#endif
