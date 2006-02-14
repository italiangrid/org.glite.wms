#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "subscriptionUpdater.h"
#include "jobCache.h"
#include <iostream>
#include <boost/algorithm/string.hpp>

namespace iceUtil = glite::wms::ice::util;
using namespace std;

void retrieveCEURLs(vector<string>&);

//______________________________________________________________________________
iceUtil::subscriptionUpdater::subscriptionUpdater(const string& cert)
  : iceThread( "subscription Updater" ),
    subMgr(),
    proxyfile(cert),
    conf(glite::wms::ice::util::iceConfManager::getInstance()),
    log_dev( api::util::creamApiLogger::instance()->getLogger() )
{
  T = new Topic(conf->getICETopic());
  P = new Policy(5000);
}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::body( void )
{
  vector<Subscription> vec;
  vec.reserve(100);
  vector<string> ceurls;
  ceurls.reserve(100);

  while( !isStopped() ) {
    log_dev->infoStream() << "subscriptionUpdater::()() - Checking "
                          << "subscription's time validity..." << log4cpp::CategoryStream::ENDLINE;
    ceurls.clear();
    retrieveCEURLs(ceurls);

    for(vector<string>::iterator it=ceurls.begin();
        it != ceurls.end();
	it++)
      {
	try {
	  log_dev->infoStream() << "subscriptionUpdater::operator()() - "
	                        << "Authenticating with proxy ["<<proxyfile<<"]"<<log4cpp::CategoryStream::ENDLINE;
	  subMgr.authenticate(proxyfile.c_str(), "/");
	  vec.clear();
	  log_dev->infoStream()  << "subscriptionUpdater::operator()() - "
	       << "Getting list of subscriptions from ["
	       << *it << "]" << log4cpp::CategoryStream::ENDLINE;
	  subMgr.list(*it, vec);
// 	  for(vector<Subscription>::iterator it = vec.begin();
//               it != vec.end();
// 	      it++)
// 	      cout << (*it)->getSubscriptionID() << endl;
	  this->renewSubscriptions(vec);
	} catch(AuthenticationInitException& ex) {
	  log_dev->errorStream()  << "subscriptionUpdater::()() - Error: "<<ex.what()<<log4cpp::CategoryStream::ENDLINE;
	  exit(1); // FIXME
	} catch(exception& ex) {
	  log_dev->errorStream()  << "subscriptionUpdater::()() - Error: "<<ex.what() << log4cpp::CategoryStream::ENDLINE;
	  exit(1); // FIXME
	}
      }

    sleep(60);
  }
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
      log_dev->infoStream()  << "\t["<<(*it).getSubscriptionID()<<"]\n\t"
           << "["<<((*it).getExpirationTime()-time(NULL)) << "]\n\t"
	   << "["<<(*it).getConsumerURL() << "]\n\t"
	   << "["<<(*it).getTopicName() << "]\n\t"
	   << "["<<(*it).getEndpoint() << "]" << log4cpp::CategoryStream::ENDLINE;
      if(timeleft < conf->getSubscriptionUpdateThresholdTime()) {
        log_dev->infoStream() << "subscriptionUpdater::renewSubscriptions() - "
	     << "Updating subscription ["<<(*it).getSubscriptionID() << "]"
	     << " at [" <<(*it).getEndpoint()<<"]"<<log4cpp::CategoryStream::ENDLINE;
	log_dev->infoStream()  << "subscriptionUpdater::renewSubscriptions() - Update params:"
	     << "ConsumerURL=["<<(*it).getConsumerURL()
	     << "] - TopicName=[" << (*it).getTopicName() << "] - "
	     << "Duration=[" << conf->getSubscriptionDuration()
	     << "] since now rate=[" << log4cpp::CategoryStream::ENDLINE;
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
	  log_dev->errorStream()  << "subscriptionUpdater::renewSubscriptions() - Error: "<<ex.what() << log4cpp::CategoryStream::ENDLINE;
	  exit(1); // FIXME
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
