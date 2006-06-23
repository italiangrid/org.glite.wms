#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "cemonUrlCache.h"
#include "iceUtils.h"
#include <iostream>

using namespace std;
namespace iceUtil = glite::wms::ice::util;
namespace api_util = glite::ce::cream_client_api::util;
iceUtil::cemonUrlCache* iceUtil::cemonUrlCache::s_instance = NULL;
boost::recursive_mutex iceUtil::cemonUrlCache::mutex;

//______________________________________________________________________________
iceUtil::cemonUrlCache* iceUtil::cemonUrlCache::getInstance() {
  boost::recursive_mutex::scoped_lock M( mutex );
  if( !s_instance ) s_instance = new cemonUrlCache();
  return s_instance;
}

//______________________________________________________________________________
void iceUtil::cemonUrlCache::putCEMonUrl(const string& cream, const string& cemon)
{

  m_cemon_cream_urlMap[cream] = cemon;
  string endpoint = api_util::CEUrl::extractEndpointFromURL( cemon );
  endpoint = iceUtil::getCompleteHostname( endpoint );
  //std::cerr << "*** ADDING CEMON endpoint [" << endpoint << "]" << std::endl;
  //exit(1);
  authorized_cemons.insert( endpoint );
}

//______________________________________________________________________________
string iceUtil::cemonUrlCache::getCEMonUrl(const string& cream)
{
  if( m_cemon_cream_urlMap.find( cream ) == m_cemon_cream_urlMap.end() )
    return "";
  return m_cemon_cream_urlMap[cream];
}

//______________________________________________________________________________
bool iceUtil::cemonUrlCache::hasCemon( const std::string& cemon )
{
  if( authorized_cemons.find( cemon ) == authorized_cemons.end() )
    return false;
  return true;
}
