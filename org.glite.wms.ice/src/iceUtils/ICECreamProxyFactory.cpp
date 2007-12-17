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

#include "ICECreamProxyFactory.h"
#include "iceConfManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <boost/algorithm/string.hpp>

namespace cream_api=glite::ce::cream_client_api;

using namespace glite::ce::cream_client_api::soap_proxy;

using namespace glite::wms::ice::util;
using namespace std;

string ICECreamProxyFactory::hostdn = string();
boost::recursive_mutex ICECreamProxyFactory::mutex;

// void ICECreamProxyFactory::setHostDN( const string& hdn )
// {
//     hostdn = hdn;
//     boost::trim_if(hostdn, boost::is_any_of("/"));
//     boost::replace_all( hostdn, "/", "_" );
//     boost::replace_all( hostdn, "=", "_" );
// }

//______________________________________________________________________________
AbsCreamProxy* 
ICECreamProxyFactory::makeCreamProxyRegister(const AbsCreamProxy::RegisterArrayRequest* param1,
					     AbsCreamProxy::RegisterArrayResult* param2 )
{

    boost::recursive_mutex::scoped_lock M( ICECreamProxyFactory::mutex );

    return CreamProxyFactory::make_CreamProxyRegister( param1, param2 );
}

//______________________________________________________________________________
AbsCreamProxy* 
ICECreamProxyFactory::makeCreamProxyStart(JobFilterWrapper* param1,
					  ResultWrapper* param2,
					  )
{

    boost::recursive_mutex::scoped_lock M( ICECreamProxyFactory::mutex );

    return CreamProxyFactory::make_CreamProxyStart( param1, param2 );
}

//______________________________________________________________________________
AbsCreamProxy* 
ICECreamProxyFactory::makeCreamProxyStatus(JobFilterWrapper* param1,
					   StatusArrayResult* param2
					   )
{

    boost::recursive_mutex::scoped_lock M( ICECreamProxyFactory::mutex );

    return CreamProxyFactory::make_CreamProxyStatus( param1, param2 );
}

//______________________________________________________________________________
AbsCreamProxy* 
ICECreamProxyFactory::makeCreamProxyLease(const std::pair<std::string, time_t>& param1, 
					  std::pair<std::string, time_t>* param2
					  )
{

    boost::recursive_mutex::scoped_lock M( ICECreamProxyFactory::mutex );

    return CreamProxyFactory::make_CreamProxyLease( param1, param2 );
}

//______________________________________________________________________________
AbsCreamProxy* 
ICECreamProxyFactory::makeCreamProxyCancel(JobFilterWrapper* param1,
					   ResultWrapper* param2
					   )
{

    boost::recursive_mutex::scoped_lock M( ICECreamProxyFactory::mutex );

    return CreamProxyFactory::make_CreamProxyCancel( param1, param2 );
}

//______________________________________________________________________________
AbsCreamProxy* 
ICECreamProxyFactory::makeCreamProxyPurge(JobFilterWrapper* param1,
					  ResultWrapper* param2
					  )
{

    boost::recursive_mutex::scoped_lock M( ICECreamProxyFactory::mutex );

    return CreamProxyFactory::make_CreamProxyPurge( param1, param2 );
}

//______________________________________________________________________________
AbsCreamProxy* 
ICECreamProxyFactory::makeCreamProxyDelegate(JobFilterWrapper* param1,
					  ResultWrapper* param2
					  )
{

    boost::recursive_mutex::scoped_lock M( ICECreamProxyFactory::mutex );

    return CreamProxyFactory::make_CreamProxyDelegate( param1, param2 );
}

//______________________________________________________________________________
AbsCreamProxy* 
ICECreamProxyFactory::makeCreamProxyDelegateRenew(JobFilterWrapper* param1,
					  ResultWrapper* param2
					  )
{

    boost::recursive_mutex::scoped_lock M( ICECreamProxyFactory::mutex );

    return CreamProxyFactory::make_CreamProxyDelegateRenew( param1, param2 );
}

//______________________________________________________________________________
AbsCreamProxy*
ICECreamProxyFactory::makeCreamProxyServiceInfo( ServiceInfoWrapper* param1 )
{
  boost::recursive_mutex::scoped_lock M( ICECreamProxyFactory::mutex );

  return CreamProxyFactory::make_CreamProxyGetServiceInfo( param1 );
}
