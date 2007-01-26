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
 * ICE core class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "CreamProxyFactory.h"
#include "iceConfManager.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <boost/algorithm/string.hpp>

namespace cream_api=glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;

string CreamProxyFactory::hostdn = string();
boost::recursive_mutex CreamProxyFactory::mutex;

void CreamProxyFactory::setHostDN( const string& hdn )
{
    hostdn = hdn;
    boost::trim_if(hostdn, boost::is_any_of("/"));
    boost::replace_all( hostdn, "/", "_" );
    boost::replace_all( hostdn, "=", "_" );
}

cream_api::soap_proxy::CreamProxy* CreamProxyFactory::makeCreamProxy( const bool autom_deleg )
{

    boost::recursive_mutex::scoped_lock M( CreamProxyFactory::mutex );

    cream_api::soap_proxy::CreamProxy *aProxy;
    try { 
        aProxy = new cream_api::soap_proxy::CreamProxy( autom_deleg, iceConfManager::getInstance()->getConfiguration()->ice()->soap_timeout() );
    } catch( cream_api::soap_proxy::soap_ex& ex) {
    
        CREAM_SAFE_LOG(
                       cream_api::util::creamApiLogger::instance()->getLogger()->fatalStream() << "CreamProxyFactory::makeCreamProxy() - Error creating a CreamProxy object: " 
                       << ex.what() << log4cpp::CategoryStream::ENDLINE;
                       );
        abort();
    }  
    if( !hostdn.empty() )
        aProxy->setSOAPHeaderID( hostdn );    
    return aProxy;
}
