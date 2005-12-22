
#include "subscriptionUpdater.h"
#include "jobCache.h"
#include <iostream>
#include <boost/algorithm/string.hpp>

namespace iceUtil = glite::wms::ice::util;
using namespace std;

void retrieveCEURLs(vector<string>&);

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::operator()()
{
  end=false;
  vector<Subscription> vec;
  vec.reserve(100);
  vector<string> ceurls;
  ceurls.reserve(100);

  while(!end) {
    cout << "subscriptionUpdater::()() - Checking "
         <<"subscription's time validity..." << endl;
    ceurls.clear();
    retrieveCEURLs(ceurls);
    for(vector<string>::iterator it=ceurls.begin();
        it != ceurls.end();
	it++)
      {
	try {
	  cout << "subscriptionUpdater::operator()() - "
	       << "Authenticating with proxy ["<<proxyfile<<"]"<<endl;
	  subMgr.authenticate(proxyfile.c_str(), "/");
	  vec.clear();
	  cout << "subscriptionUpdater::operator()() - "
	       << "Getting list of subscriptions from ["
	       << *it << "]" << endl;
	  subMgr.list(*it, vec);
	  this->renewSubscriptions(vec);
	} catch(AuthenticationInitException& ex) {
	  cerr << ex.what()<<endl;
	  exit(1);
	} catch(exception& ex) {
	  cerr << ex.what() << endl;
	  exit(1);
	}
      }

    sleep(60);
  }
  cout << "eventStatusListener::()() - ending..." << endl;
}

//______________________________________________________________________________
void
iceUtil::subscriptionUpdater::renewSubscriptions(const vector<Subscription>& vec)
{
  for(vector<Subscription>::const_iterator it = vec.begin();
      it != vec.end();
      it++) 
    {
      time_t timeleft = time(NULL) - (*it).getExpirationTime();
      if(timeleft < conf->getSubscriptionUpdateThresholdTime())
	// MUST UPDATE current SUBSCRIPTION
	;
    }
}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::retrieveCEURLs(vector<string>& urls)
{
  map< string , int > tmpMap;
  string ceurl;
  for(iceUtil::jobCache::iterator it=iceUtil::jobCache::getInstance()->begin();
      it != iceUtil::jobCache::getInstance()->end();
      it++)
    {
      ceurl = it->getCreamURL();
      boost::replace_first(ceurl, conf->getCreamUrlPostfix(),
      			   conf->getCEMonUrlPostfix());
      tmpMap[ceurl] = 1;
    }
    for(map<string, int>::iterator it = tmpMap.begin();
        it != tmpMap.end();
	it++)
	urls.push_back(it->first);
}
