#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "subscriptionUpdater.h"
#include "subscriptionManager.h"
#include "jobCache.h"
#include <iostream>
#include <boost/algorithm/string.hpp>

namespace iceUtil = glite::wms::ice::util;
using namespace std;

void retrieveCEURLs(vector<string>&);

//______________________________________________________________________________
iceUtil::subscriptionUpdater::subscriptionUpdater(const string& cert)
  : iceThread( "subscription Updater" ),
    conf(glite::wms::ice::util::iceConfManager::getInstance()),
    log_dev( api::util::creamApiLogger::instance()->getLogger() ),
    subMgr( subscriptionManager::getInstance() )
{

}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::body( void )
{
  vector<Subscription> vec;
  vec.reserve(100);
  set<string> ceurls;

  while( !isStopped() ) {
    log_dev->infoStream() << "subscriptionUpdater::body() - Checking "
                          << "subscription's time validity..."
			  << log4cpp::CategoryStream::ENDLINE;
    ceurls.clear();
    retrieveCEURLs(ceurls);

    for(set<string>::iterator it=ceurls.begin(); it != ceurls.end(); it++) {

	  try{ subMgr->list(*it, vec); }
	  catch(exception& ex) {
	    log_dev->errorStream() << "subscriptionUpdater::body() - "
	    			   << "Error retrieving list of subscriptions: "
				   << ex.what() << log4cpp::CategoryStream::ENDLINE;
	    return;
	  }
	  this->renewSubscriptions(vec);
    }
    sleep(60);
  }
}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::renewSubscriptions(const vector<Subscription>& vec)
{
  for(vector<Subscription>::const_iterator it = vec.begin();
      it != vec.end();
      it++)
    {
      time_t timeleft = (*it).getExpirationTime() - time(NULL);
/*      cout  << "\t["<<(*it).getSubscriptionID()<<"]\n\t"
           << "["<<(*it).getConsumerURL() << "]\n\t"
	   << "["<<(*it).getTopicName() << "]\n\t"
	   << "["<<(*it).getEndpoint() << "]\n\t"
	   << "timeleft=[" << timeleft << "]\n\t"
	   << "expiration=["<<conf->getSubscriptionUpdateThresholdTime()<<"]\n"
	   << log4cpp::CategoryStream::ENDLINE;*/
      {
        boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
        if(timeleft < conf->getSubscriptionUpdateThresholdTime()) {
          log_dev->infoStream() << "subscriptionUpdater::renewSubscriptions() - "
	     << "Updating subscription ["<<(*it).getSubscriptionID() << "]"
	     << " at [" <<(*it).getEndpoint()<<"]"<<log4cpp::CategoryStream::ENDLINE;
	  log_dev->infoStream()  << "subscriptionUpdater::renewSubscriptions() - Update params: "
	     << "ConsumerURL=["<<(*it).getConsumerURL()
	     << "] - TopicName=[" << (*it).getTopicName() << "] - "
	     << "Duration=" << conf->getSubscriptionDuration()
	     << " secs since now - rate="
	     << conf->getNotificationFrequency()
	     << " secs"
	     << log4cpp::CategoryStream::ENDLINE;
	}

	subMgr->updateSubscription( (const string&)(*it).getEndpoint(),
				    (const string&)(*it).getSubscriptionID());

      }
    }
}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::retrieveCEURLs(set<string>& urls)
{
    urls.clear();
    string ceurl;
    for(iceUtil::jobCache::iterator it=iceUtil::jobCache::getInstance()->begin();
        it != iceUtil::jobCache::getInstance()->end(); it++) {

        ceurl = it->getCreamURL();
        boost::replace_first(ceurl, conf->getCreamUrlPostfix(),
                             conf->getCEMonUrlPostfix());
        urls.insert( ceurl );
    }
}
