
#include "subscriptionCache.h"

using namespace std;
using namespace glite::wms::ice::util;

subscriptionCache* subscriptionCache::instance = NULL;
boost::recursive_mutex subscriptionCache::mutex;

//______________________________________________________________________________
void subscriptionCache::remove(const string& s)
{
  boost::recursive_mutex::scoped_lock M( mutex );
  it = cemons.find(s);
  if( it != cemons.end() ) cemons.erase( it );
}

//______________________________________________________________________________
bool subscriptionCache::has(const string& s)
{

  boost::recursive_mutex::scoped_lock M( mutex );
  it = cemons.find(s);
  if( it != cemons.end() ) return true;
  return false;
}

//______________________________________________________________________________
subscriptionCache* subscriptionCache::getInstance()
{
  //boost::recursive_mutex::scoped_lock M( mutex );
  if(!instance)
    instance = new subscriptionCache();
  return instance;
}
