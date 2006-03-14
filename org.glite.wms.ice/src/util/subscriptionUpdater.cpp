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
    subMgr( subscriptionManager::getInstance() ) // the subManager's instance has already been created by ice-core module; so no need to lock the mutex
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
	  try{
	    boost::recursive_mutex::scoped_lock M( iceUtil::subscriptionManager::mutex );
	    subMgr->list(*it, vec);
	  }
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
void iceUtil::subscriptionUpdater::renewSubscriptions(vector<Subscription>& vec)
{
  for(vector<Subscription>::iterator sit = vec.begin();
      sit != vec.end();
      sit++)
    {
      time_t timeleft = sit->getExpirationTime() - time(NULL);
//       cout  << "\t["<<sit->getSubscriptionID()<<"]\n\t"
//            << "["<<sit->getConsumerURL() << "]\n\t"
// 	   << "["<<sit->getTopicName() << "]\n\t"
// 	   << "["<<sit->getEndpoint() << "]\n\t"
// 	   << "timeleft=[" << timeleft << "]\n\t"
// 	   << "threshold=["<<conf->getSubscriptionUpdateThresholdTime()<<"]\n"
// 	   << log4cpp::CategoryStream::ENDLINE;
      {
        boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
        if(timeleft < conf->getSubscriptionUpdateThresholdTime()) {
          log_dev->infoStream() << "subscriptionUpdater::renewSubscriptions() - "
	     << "Updating subscription ["<<sit->getSubscriptionID() << "]"
	     << " at [" <<sit->getEndpoint()<<"]"<<log4cpp::CategoryStream::ENDLINE;
	  log_dev->infoStream()  << "subscriptionUpdater::renewSubscriptions() - Update params: "
	     << "ConsumerURL=["<<sit->getConsumerURL()
	     << "] - TopicName=[" << sit->getTopicName() << "] - "
	     << "Duration=" << conf->getSubscriptionDuration()
	     << " secs since now - rate="
	     << conf->getNotificationFrequency()
	     << " secs"
	     << log4cpp::CategoryStream::ENDLINE;
          {
	    boost::recursive_mutex::scoped_lock M( iceUtil::subscriptionManager::mutex );
	    string newID;
 	    if(subMgr->updateSubscription( sit->getEndpoint(), sit->getSubscriptionID(), newID )) {
	      log_dev->infoStream() << "subscriptionUpdater::renewSubscriptions() - "
	    	  		    << "New subscription ID after renewal is ["
	  			    << newID << "]" << log4cpp::CategoryStream::ENDLINE;
	      sit->setSubscriptionID(newID);
	      sit->setExpirationTime( time(NULL) + conf->getSubscriptionDuration() );
	    }
	  }
	} // if(timeleft < ....)
      }
    }
}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::retrieveCEURLs(set<string>& urls)
{
    urls.clear();
    string ceurl;
    boost::recursive_mutex::scoped_lock M( iceUtil::jobCache::mutex );
    for(iceUtil::jobCache::iterator it=iceUtil::jobCache::getInstance()->begin();
        it != iceUtil::jobCache::getInstance()->end(); it++) {

        ceurl = it->getCreamURL();
        boost::replace_first(ceurl, conf->getCreamUrlPostfix(),
                             conf->getCEMonUrlPostfix());
        urls.insert( ceurl );
    }
}
