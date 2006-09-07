#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "subscriptionUpdater.h"
#include "subscriptionManager.h"
//#include "subscriptionCache.h"
#include "jobCache.h"
#include "cemonUrlCache.h"
#include "iceUtils.h"
//#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

namespace iceUtil = glite::wms::ice::util;
namespace api = glite::ce::cream_client_api;
using namespace std;

void retrieveCEURLs(vector<string>&);

//______________________________________________________________________________
iceUtil::subscriptionUpdater::subscriptionUpdater(const string& cert) :
  iceThread( "subscription Updater" ),
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
    CREAM_SAFE_LOG(m_log_dev->fatalStream() 
                   << "subscriptionUpdater::CTOR - iceUtils::getHostName() "
		   << "returned an ERROR: "
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
    
    for(set<string>::iterator it=ceurls.begin(); it != ceurls.end(); ++it) 
    {
      vec.clear();
      
      
      bool subscribed;
      {
	boost::recursive_mutex::scoped_lock M( iceUtil::subscriptionManager::mutex );
	subscribed = m_subMgr->subscribedTo( *it, vec );
      }
      if( !subscribed ) {
	CREAM_SAFE_LOG(m_log_dev->warnStream() << "subscriptionUpdater::body() - "
		       << "!!! DISAPPEARED subscription to [" << *it 
		       << "]. Going to re-subscribe to it."
		       << log4cpp::CategoryStream::ENDLINE);
	
	if( m_conf->getListenerEnableAuthZ() )
	  {
	    string DN;
	    boost::recursive_mutex::scoped_lock M( cemonUrlCache::mutex );
	    if( !cemonUrlCache::getInstance()->getCEMonDN( *it, DN ) )
	      {
		CREAM_SAFE_LOG(m_log_dev->errorStream()
			       << "subscriptionUpdater::body() - "
			       << "Couldn't get DN for CEMon ["
			       << *it << "]. Cannot subscribe to it because "
			       << "notification authorization is enabled."
			       << log4cpp::CategoryStream::ENDLINE);
		ceurls.erase( *it );
		continue;
	      } else {
	      boost::recursive_mutex::scoped_lock M( cemonUrlCache::mutex );
	      cemonUrlCache::getInstance()->insertDN( DN );
	    }
	    
	  } // if( m_conf->getListenerEnableAuthZ() )
	
	if( !m_subMgr->subscribe( *it ) )
	  {
	    CREAM_SAFE_LOG(m_log_dev->errorStream()
	    		   << "subscriptionUpdater::body() - "
			   << "Couldn't subscribe to ["
			   << *it << "]. The Status Poller will take care "
			   << "of job status; subscriptionUpdater or the next job submission will "
			   << "try to subscribe"
			   << log4cpp::CategoryStream::ENDLINE);
	    continue;
	    
	  }  // if( !m_subMgr->subscribe( *it ) )
	  else {
	    boost::recursive_mutex::scoped_lock M( cemonUrlCache::mutex );
	    cemonUrlCache::getInstance()->insertCEMon( *it );
	  }
	
      } // if(!subscribed)
      else {
	this->renewSubscriptions(vec);
      }
    } // for loop over CEMon URLs
    
    sleep( /*m_iteration_delay*/ 60 );
  } // while( !stopped )
} // end function

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::renewSubscriptions(vector<Subscription>& vec)
{
  for(vector<Subscription>::iterator sit = vec.begin();
      sit != vec.end();
      sit++)
    {
      time_t timeleft = sit->getExpirationTime() - time(NULL);
   
      if(timeleft < m_conf->getSubscriptionUpdateThresholdTime()) {
          CREAM_SAFE_LOG(m_log_dev->infoStream() 
			 << "subscriptionUpdater::renewSubscriptions() - "
			 << "Updating subscription ["<<sit->getSubscriptionID() 
			 << "] to CEMon [" <<sit->getEndpoint()<<"]"
			 << log4cpp::CategoryStream::ENDLINE);

	  CREAM_SAFE_LOG(m_log_dev->infoStream()  
			 << "subscriptionUpdater::renewSubscriptions() - "
			 << "Update params: "
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
 	    if(m_subMgr->updateSubscription( sit->getEndpoint(), 
					     sit->getSubscriptionID(), newID )) 
	      {
		CREAM_SAFE_LOG(m_log_dev->infoStream() << "subscriptionUpdater::renewSubscriptions() - "
			       << "New subscription ID after renewal is ["
			       << newID << "]" << log4cpp::CategoryStream::ENDLINE);
		sit->setSubscriptionID(newID);
		sit->setExpirationTime( time(NULL) + m_conf->getSubscriptionDuration() );
	      } else {
	      // subscription renewal failed. Try make a new one
	      if(!m_subMgr->subscribe( sit->getEndpoint() )) {
	        CREAM_SAFE_LOG(m_log_dev->errorStream() 
			       << "subscriptionUpdater::renewSubscriptions() - "
			       << "Failed while making new subscription. "
			       << "Wont receive notifications... "
			       << log4cpp::CategoryStream::ENDLINE);
		// let's proceed without notification. The poller will work for us ;)
	      } else {

		// We made a new subscription because the update
		// failed. Then the CEMon and it's DN are supposed to be
		// already registered in the cemonUrlCache

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
     * (like leaseUpdater that could wait too long before update a
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
	{
	  boost::recursive_mutex::scoped_lock cemonM( cemonUrlCache::mutex );	
	  cemonURL = cemonUrlCache::getInstance()->getCEMonURL( ceurl );
	}
	
	urls.insert( cemonURL );
    }
    
    cemonUrlCache::getInstance()->getCEMons( urls );
}
