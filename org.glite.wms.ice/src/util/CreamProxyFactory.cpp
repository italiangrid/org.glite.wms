
#include "CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

std::string glite::wms::ice::util::CreamProxyFactory::hostdn = std::string("");

glite::ce::cream_client_api::soap_proxy::CreamProxy*
glite::wms::ice::util::CreamProxyFactory::makeCreamProxy( const bool autom_deleg )
{
  glite::ce::cream_client_api::soap_proxy::CreamProxy *aProxy;
  try { 
    aProxy = new glite::ce::cream_client_api::soap_proxy::CreamProxy( autom_deleg );
  } catch(glite::ce::cream_client_api::soap_proxy::soap_ex& ex) {
    
    CREAM_SAFE_LOG(
      glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->errorStream() << "CreamProxyFactory::makeCreamProxy() - Error creating a CreamProxy object: " 
      << ex.what() << log4cpp::CategoryStream::ENDLINE;
    );
    
    return NULL;
  }
  
  if(hostdn != "")
    aProxy->setSOAPHeaderID( hostdn );

  return aProxy;
}
