#include "cemonUrlCache.h"

using namespace std;
using namespace glite::wms::ice::util;

cemonUrlCache* cemonUrlCache::s_instance = NULL;
boost::recursive_mutex cemonUrlCache::mutex;

//______________________________________________________________________________
cemonUrlCache* cemonUrlCache::getInstance() {
  boost::recursive_mutex::scoped_lock M( mutex );
  if( !s_instance ) s_instance = new cemonUrlCache();
  return s_instance;
}

//______________________________________________________________________________
void cemonUrlCache::putCEMonUrl(const string& cream, const string& cemon) {

  m_cemon_cream_urlMap[cream] = cemon;
}

//______________________________________________________________________________
string cemonUrlCache::getCEMonUrl(const string& cream) {
  if( m_cemon_cream_urlMap.find( cream ) == m_cemon_cream_urlMap.end() )
    return "";
  return m_cemon_cream_urlMap[cream];
}
