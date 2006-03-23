#include "cemonUrlCache.h"

using namespace std;
using namespace glite::wms::ice::util;

cemonUrlCache* cemonUrlCache::instance = NULL;
boost::recursive_mutex cemonUrlCache::mutex;

//______________________________________________________________________________
cemonUrlCache* cemonUrlCache::getInstance() {
  boost::recursive_mutex::scoped_lock M( mutex );
  if( !instance ) instance = new cemonUrlCache();
  return instance;
}

//______________________________________________________________________________
void cemonUrlCache::putCEMonUrl(const string& cream, const string& cemon) {

  cemon_cream_urlMap[cream] = cemon;
}

//______________________________________________________________________________
string cemonUrlCache::getCEMonUrl(const string& cream) {
  if( cemon_cream_urlMap.find( cream ) == cemon_cream_urlMap.end() )
    return "";
  return cemon_cream_urlMap[cream];
}