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

                class CreamProxy;
  
            }
        }
    }
};

namespace glite {
    namespace wms {
        namespace ice {
            namespace util {
                
                class CreamProxyFactory {

                    static std::string hostdn;
                    static boost::recursive_mutex mutex;
		    
                public:
                    
                    static void setHostDN( const std::string& hdn ) { 
                        hostdn = hdn; 
                    }

                    /**
                     * Creates a new CreamProxy object. Ownership of
                     * the returned pointer is transferred to the
                     * caller.
                     *
                     * @param auto_del if true, the returned
                     * CreamProxy object will do automatic delegation.
                     */
                    static glite::ce::cream_client_api::soap_proxy::CreamProxy* 
                        makeCreamProxy( const bool auto_del );
                    
                };
                
            }
        }
    }
};

#endif
