
#include "subscriptionUpdater.h"
#include "jobCache.h"
#include <iostream>
#include <boost/algorithm/string.hpp>

namespace iceUtil = glite::wms::ice::util;
using namespace std;

void retrieveCEURLs(vector<string>&);

//______________________________________________________________________________
iceUtil::subscriptionUpdater::subscriptionUpdater(const string& cert)
  : subMgr(),
    end(false),
    proxyfile(cert),
    conf(glite::wms::ice::util::iceConfManager::getInstance())
{
  T = new Topic(conf->getICETopic());
  P = new Policy(5000);
}

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
         << "subscription's time validity..." << endl;
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
// 	  for(vector<Subscription>::iterator it = vec.begin();
//               it != vec.end();
// 	      it++)
// 	      cout << (*it)->getSubscriptionID() << endl;
	  this->renewSubscriptions(vec);
	} catch(AuthenticationInitException& ex) {
	  cerr << "subscriptionUpdater::()() - Error: "<<ex.what()<<endl;
	  exit(1);
	} catch(exception& ex) {
	  cerr << "subscriptionUpdater::()() - Error: "<<ex.what() << endl;
	  exit(1);
	}
      }

    sleep(60);
  }
  cout << "subscriptionUpdater::()() - ending..." << endl;
}

//______________________________________________________________________________
void
iceUtil::subscriptionUpdater::renewSubscriptions(const vector<Subscription>& vec)
{
  for(vector<Subscription>::const_iterator it = vec.begin();
      it != vec.end();
      it++)
    {
      //cout <<
      time_t timeleft = (*it).getExpirationTime() - time(NULL);
      cout << "\t["<<(*it).getSubscriptionID()<<"]\n\t"
           << "["<<((*it).getExpirationTime()-time(NULL)) << "]\n\t"
	   << "["<<(*it).getConsumerURL() << "]\n\t"
	   << "["<<(*it).getTopicName() << "]\n\t"
	   << "["<<(*it).getEndpoint() << "]" << endl;
      if(timeleft < conf->getSubscriptionUpdateThresholdTime()) {
        cout << "subscriptionUpdater::renewSubscriptions() - "
	     << "Updating subscription ["<<(*it).getSubscriptionID() << "]"
	     << " at [" <<(*it).getEndpoint()<<"]"<<endl;
	cout << "subscriptionUpdater::renewSubscriptions() - Update params:"
	     << "ConsumerURL=["<<(*it).getConsumerURL()
	     << "] - TopicName=[" << (*it).getTopicName() << "] - "
	     << "Duration=[" << conf->getSubscriptionDuration()
	     << "] since now" << endl;
	try {
	  subMgr.update(
	                (const string&)(*it).getEndpoint(),
		        (const string&)(*it).getSubscriptionID(),
		        (const string&)(*it).getConsumerURL(),
		        *T,
		        *P,
		        (const time_t&)(time(NULL)+conf->getSubscriptionDuration())
		       );
        } catch(exception& ex) {
	  cerr << "subscriptionUpdater::renewSubscriptions() - Error: "<<ex.what() << endl;
	  exit(1);
	}
      }
    }
}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::retrieveCEURLs(vector<string>& urls)
{
  map< string, int > tmpMap;
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
	it++) urls.push_back(it->first);
}
