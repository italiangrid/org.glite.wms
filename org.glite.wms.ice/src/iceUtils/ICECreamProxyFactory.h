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
 * ICE Cream Proxy Factory
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_CREAMPROXYFACTORY_H
#define GLITE_WMS_ICE_UTIL_CREAMPROXYFACTORY_H

#include <string>
#include "boost/thread/recursive_mutex.hpp"

namespace glite {
namespace ce {
namespace cream_client_api {
namespace soap_proxy {

    class AbsCreamProxy;
  
}
}
}
};

namespace glite {
namespace wms {
namespace ice {
namespace util {
                
    class ICECreamProxyFactory {
        
        static std::string hostdn;
        static boost::recursive_mutex mutex;
        
    public:
        
        static void setHostDN( const std::string& hdn );
        
        /**
         * Creates a new CreamProxy object. Ownership of
         * the returned pointer is transferred to the
         * caller.
         *
         * @param auto_del if true, the returned
         * CreamProxy object will do automatic delegation.
         *
         * @return a new CreamProxy object. The caller is responsible
         * for freeing the returned pointer.
         */
        static glite::ce::cream_client_api::soap_proxy::AbsCreamProxy* 
	  makeCreamProxyRegister( const glite::ce::cream_client_api::soap_proxy::AbsCreamProxy::RegisterArrayRequest* param1,
			  glite::ce::cream_client_api::soap_proxy::AbsCreamProxy::RegisterArrayResult* param2
			  );

	static glite::ce::cream_client_api::soap_proxy::AbsCreamProxy* 
	  makeCreamProxyStart( const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* param1,
			       glite::ce::cream_client_api::soap_proxy::ResultWrapper* param2
			       );

	static glite::ce::cream_client_api::soap_proxy::AbsCreamProxy* 
	  makeCreamProxyStatus( const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* param1,
				glite::ce::cream_client_api::soap_proxy::AbsCreamProxy::StatusArrayResult* param2
			       );

	static glite::ce::cream_client_api::soap_proxy::AbsCreamProxy* 
	  makeCreamProxyLease( const std::pair<std::string, time_t>& param1, 
			       std::pair<std::string, time_t>* param2
			       );

	static glite::ce::cream_client_api::soap_proxy::AbsCreamProxy* 
	  makeCreamProxyCancel( const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* param1,
			       glite::ce::cream_client_api::soap_proxy::ResultWrapper* param2
			       );

	static glite::ce::cream_client_api::soap_proxy::AbsCreamProxy* 
	  makeCreamProxyPurge( const glite::ce::cream_client_api::soap_proxy::JobFilterWrapper* param1,
			       glite::ce::cream_client_api::soap_proxy::ResultWrapper* param2
			       );

	static glite::ce::cream_client_api::soap_proxy::AbsCreamProxy* 
	  makeCreamProxyDelegate( const std::string& param1, std::string* param2 );
        
    };
    
} // namespace util
} // namespace ice
} // namespace wms
}; // namespace glite

#endif
