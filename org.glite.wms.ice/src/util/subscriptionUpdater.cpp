#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "subscriptionUpdater.h"
#include "subscriptionManager.h"
#include "jobCache.h"
#include "cemonUrlCache.h"
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
  
  /**
   * let's Determine our hostname to use as consumerURL
   */
  char name[256];
  memset((void*)name, 0, 256);

  if(gethostname(name, 256) == -1) {
    m_log_dev->fatalStream() << "subscriptionUpdater::CTOR - Couldn't resolve local hostname: "
                           << strerror(errno)
                           << log4cpp::CategoryStream::ENDLINE;
    m_valid = false;
    return;
  }
  struct hostent *H=gethostbyname(name);
  if(!H) {
      m_log_dev->fatalStream() << "subscriptionUpdater::CTOR - Couldn't resolve local hostname: "
                               << strerror(h_errno)
                               << log4cpp::CategoryStream::ENDLINE;
    m_valid = false;
    return;
  }
  {
    boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
    m_myname = boost::str( boost::format("http://%1%:%2%") % H->h_name % m_conf->getListenerPort() );
  }
  
  m_subMgr->setConsumerURLName( m_myname );
}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::body( void )
{
  vector<Subscription> vec;
  vec.reserve(100);
  set<string> ceurls;

  while( !isStopped() ) {
    m_log_dev->debugStream() << "subscriptionUpdater::body() - Checking "
                             << "subscription's time validity..."
			     << log4cpp::CategoryStream::ENDLINE;
    ceurls.clear();
    retrieveCEURLs(ceurls);

    for(set<string>::iterator it=ceurls.begin(); it != ceurls.end(); it++) 
    {
      vec.clear();
      try {
	boost::recursive_mutex::scoped_lock M( iceUtil::subscriptionManager::mutex );
        m_subMgr->list(*it, vec);
      }
      catch(exception& ex) {
        m_log_dev->errorStream() << "subscriptionUpdater::body() - "
	 		         << "Error retrieving list of subscriptions: "
			         << ex.what() << log4cpp::CategoryStream::ENDLINE;
	return;
      }
      
      bool lost_subscription = true;
      for( vector<Subscription>::const_iterator sit=vec.begin();
           sit != vec.end();
	   ++sit)
      {
	 if( sit->getConsumerURL() == m_myname ) {
	   lost_subscription = false;
	   break;
	 }
      }
	
      if( lost_subscription/*vec.empty()*/ ) 
      {
        // this means that ICE is not subscribed to current CEMon
	// it should be! something happened... must re-subscribe
	m_log_dev->warnStream() << "subscriptionUpdater::body() - "
	 		         << "Subscription to [" << *it << "] disappeared! Going to re-subscribe to it."
			         << log4cpp::CategoryStream::ENDLINE;
	m_subMgr->subscribe( *it );
      } else
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
      {
        boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
        if(timeleft < m_conf->getSubscriptionUpdateThresholdTime()) {
          m_log_dev->infoStream() << "subscriptionUpdater::renewSubscriptions() - "
	     << "Updating subscription ["<<sit->getSubscriptionID() << "]"
	     << " at [" <<sit->getEndpoint()<<"]"
	     << log4cpp::CategoryStream::ENDLINE;
	  m_log_dev->infoStream()  << "subscriptionUpdater::renewSubscriptions() - Update params: "
	     << "ConsumerURL=["<<sit->getConsumerURL()
	     << "] - TopicName=[" << sit->getTopicName() << "] - "
	     << "Duration=" << m_conf->getSubscriptionDuration()
	     << " secs since now - rate="
	     << m_conf->getNotificationFrequency()
	     << " secs"
	     << log4cpp::CategoryStream::ENDLINE;
          {
	    boost::recursive_mutex::scoped_lock M( iceUtil::subscriptionManager::mutex );
	    string newID;
 	    if(m_subMgr->updateSubscription( sit->getEndpoint(), sit->getSubscriptionID(), newID )) {
	      m_log_dev->infoStream() << "subscriptionUpdater::renewSubscriptions() - "
	    	  		    << "New subscription ID after renewal is ["
	  			    << newID << "]" << log4cpp::CategoryStream::ENDLINE;
	      sit->setSubscriptionID(newID);
	      sit->setExpirationTime( time(NULL) + m_conf->getSubscriptionDuration() );
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
    } // unlock the jobCache (other thread can continue their job)
    
//    for(iceUtil::jobCache::iterator it=iceUtil::jobCache::getInstance()->begin();
//       it != iceUtil::jobCache::getInstance()->end(); ++it) 
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
	      urls.insert( cemonURL );
	  } catch(exception& ex) {
	      m_log_dev->errorStream() << "subscriptionUpdater::retrieveCEURLs() - Error retrieving"
	      			     <<" CEMon's URL from CREAM's URL: "
			 	     << ex.what()
				     << ". Composing URL from configuration file..."
			 	     << log4cpp::CategoryStream::ENDLINE;
	      cemonURL = ceurl;
	      boost::replace_first(cemonURL,
                                   m_conf->getCreamUrlPostfix(),
                                   m_conf->getCEMonUrlPostfix()
                                  );
	      m_log_dev->infoStream() << "subscriptionUpdater::retrieveCEURLs() - Using CEMon URL ["
	      			    << cemonURL << "]" << log4cpp::CategoryStream::ENDLINE;
              urls.insert( cemonURL );
	      cemonUrlCache::getInstance()->putCEMonUrl( ceurl, cemonURL );
	  }
	} else {
          urls.insert( cemonURL );
	}
    }
}
