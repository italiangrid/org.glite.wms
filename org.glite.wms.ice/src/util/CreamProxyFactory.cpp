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

using namespace glite::wms::ice::util;
namespace cream_api=glite::ce::cream_client_api;

std::string CreamProxyFactory::hostdn = std::string();

cream_api::soap_proxy::CreamProxy* CreamProxyFactory::makeCreamProxy( const bool autom_deleg )
{
    cream_api::soap_proxy::CreamProxy *aProxy;
    try { 
        aProxy = new cream_api::soap_proxy::CreamProxy( autom_deleg, iceConfManager::getInstance()->getSoapTimeout() );
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
