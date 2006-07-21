#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "subscriptionUpdater.h"
#include "subscriptionManager.h"
#include "jobCache.h"
#include "cemonUrlCache.h"
#include "iceUtils.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

namespace iceUtil = glite::wms::ice::util;
using namespace std;

void retrieveCEURLs(vector<string>&);

//______________________________________________________________________________
iceUtil::subscriptionUpdater::subscriptionUpdater(const string& cert)
  : iceThread( "subscription Updater" ),
    m_conf(glite::wms::ice::util::iceConfManager::getInstance()),
    m_log_dev( api::util::creamApiLogger::instance()->getLogger() ),
    m_subMgr( subscriptionManager::getInstance() ), // the subManager's instance 
    						 // has already been created by 
						 // ice-core module; so no need
						 // to lock the mutex and no need
						 // to check if the singleton is
						 // NULL or not 
    m_valid(true)
{
  m_iteration_delay = (int)(m_conf->getSubscriptionUpdateThresholdTime()/2);
  if(!m_iteration_delay) m_iteration_delay=5;
  
  string tmp_myname;
  try {
    tmp_myname = iceUtil::getHostName();
  } catch( runtime_error& ex) {
    CREAM_SAFE_LOG(m_log_dev->fatalStream() << "subscriptionUpdater::CTOR - iceUtils::getHostName() returned an ERROR: "
		   << ex.what()
		   << log4cpp::CategoryStream::ENDLINE);
    m_valid = false;
    return;
  }

}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::body( void )
{
  vector<Subscription> vec;
  vec.reserve(10);
  set<string> ceurls;

  while( !isStopped() ) {
    CREAM_SAFE_LOG(m_log_dev->debugStream() << "subscriptionUpdater::body() - Checking "
		   << "subscription's time validity..."
		   << log4cpp::CategoryStream::ENDLINE);
    ceurls.clear();
    retrieveCEURLs(ceurls);
    for(set<string>::iterator it=ceurls.begin(); it != ceurls.end(); it++) 
    {
      vec.clear();
      //m_subMgr->list( *it, vec );
      
      if( !m_subMgr->subscribedTo( *it, vec ) ) {
        CREAM_SAFE_LOG(m_log_dev->warnStream() << "subscriptionUpdater::body() - "
 		       << "Subscription to [" << *it << "] disappeared! Going to re-subscribe to it."
 		       << log4cpp::CategoryStream::ENDLINE);
 	m_subMgr->subscribe( *it );
      }
      
      this->renewSubscriptions(vec);
    }
    
    sleep( m_iteration_delay );
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
   
      if(timeleft < m_conf->getSubscriptionUpdateThresholdTime()) {
          CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionUpdater::renewSubscriptions() - "
			 << "Updating subscription ["<<sit->getSubscriptionID() << "]"
			 << " at [" <<sit->getEndpoint()<<"]"
			 << log4cpp::CategoryStream::ENDLINE);
	  CREAM_SAFE_LOG(m_log_dev->infoStream()  << "subscriptionUpdater::renewSubscriptions() - Update params: "
			 << "ConsumerURL=["<<sit->getConsumerURL()
			 << "] - TopicName=[" << sit->getTopicName() << "] - "
			 << "Duration=" << m_conf->getSubscriptionDuration()
			 << " secs since now - rate="
			 << m_conf->getNotificationFrequency()
			 << " secs"
			 << log4cpp::CategoryStream::ENDLINE);
          {
	    boost::recursive_mutex::scoped_lock M( iceUtil::subscriptionManager::mutex );
	    string newID;
 	    if(m_subMgr->updateSubscription( sit->getEndpoint(), sit->getSubscriptionID(), newID )) {
	      CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionUpdater::renewSubscriptions() - "
			     << "New subscription ID after renewal is ["
			     << newID << "]" << log4cpp::CategoryStream::ENDLINE);
	      sit->setSubscriptionID(newID);
	      sit->setExpirationTime( time(NULL) + m_conf->getSubscriptionDuration() );
	    } else {
	      
	      
	      // subscription renewal failed. Try make a new one
	      if(!m_subMgr->subscribe( sit->getEndpoint() )) {
	        CREAM_SAFE_LOG(m_log_dev->errorStream() << "subscriptionUpdater::renewSubscriptions() - "
			       << "Failed while making new subscription. Wont receive notifications... "
			       << log4cpp::CategoryStream::ENDLINE);
		// let's proceed without notification. The poller will work for us ;)
	      }
	      
	      
	    } // else
	  }
       } // if(timeleft < ....)
      
    }
}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::retrieveCEURLs(set<string>& urls)
{
    urls.clear();
    string ceurl, cemonURL;
    vector<CreamJob> jobList;
    
    /**
     * The following code is higly HUGLY!!!
     * but we cannot keep the jobCache locked for long time
     * (sometime the CreamProxy::GetCEMonURL() could take a 
     * long time)
     * otherwise other threads could fail their operations
     * (like leaseUpdater that could wait to long before updater a
     * job lease that is expiring).
     */
    {
      boost::recursive_mutex::scoped_lock M( iceUtil::jobCache::mutex );
      for(iceUtil::jobCache::iterator it=iceUtil::jobCache::getInstance()->begin();
        it != iceUtil::jobCache::getInstance()->end(); 
	++it) 
      {
	jobList.push_back( *it );
      }
    } 
    for(vector<CreamJob>::iterator it = jobList.begin();
        it != jobList.end();
	++it)
    {
        ceurl = it->getCreamURL();
	boost::recursive_mutex::scoped_lock cemonM( cemonUrlCache::mutex );
 	cemonURL = cemonUrlCache::getInstance()->getCEMonUrl( ceurl );
	
	if( cemonURL == "" ) {
	  try {
	      api::soap_proxy::CreamProxyFactory::getProxy()->Authenticate( m_conf->getHostProxyFile() );
	      api::soap_proxy::CreamProxyFactory::getProxy()->GetCEMonURL( ceurl.c_str(), cemonURL );
	      cemonUrlCache::getInstance()->putCEMonUrl( ceurl, cemonURL );
	      //urls.insert( cemonURL );
	  } catch(exception& ex) {
	    CREAM_SAFE_LOG(m_log_dev->errorStream() << "subscriptionUpdater::retrieveCEURLs() - Error retrieving"
			   <<" CEMon's URL from CREAM's URL: "
			   << ex.what()
			   << ". Composing URL from configuration file..."
			   << log4cpp::CategoryStream::ENDLINE);
	      cemonURL = ceurl;
	      boost::replace_first(cemonURL,
                                   m_conf->getCreamUrlPostfix(),
                                   m_conf->getCEMonUrlPostfix()
                                  );
	      CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionUpdater::retrieveCEURLs() - Using CEMon URL ["
			     << cemonURL << "]" << log4cpp::CategoryStream::ENDLINE);
	      //urls.insert( cemonURL );
	      cemonUrlCache::getInstance()->putCEMonUrl( ceurl, cemonURL );
	  }
	} /* else {
          urls.insert( cemonURL );
	} */
	urls.insert( cemonURL );
    }
}
