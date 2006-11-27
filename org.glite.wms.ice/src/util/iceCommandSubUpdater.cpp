/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE subscription updater
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceCommandSubUpdater.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include "cemonUrlCache.h"
#include "subscriptionManager.h"

#include <string>
#include <vector>

#include <boost/format.hpp>

using namespace std;

namespace api_util  = glite::ce::cream_client_api::util;
namespace cream_api = glite::ce::cream_client_api;
namespace ice_util  = glite::wms::ice::util;

//______________________________________________________________________________
ice_util::iceCommandSubUpdater::iceCommandSubUpdater( const string& certfile) throw() : 
  m_proxyfile( certfile ),
  m_subMgr( ice_util::subscriptionManager::getInstance() ),
  m_conf( ice_util::iceConfManager::getInstance() )			     
{
}

//______________________________________________________________________________
void ice_util::iceCommandSubUpdater::execute( ) throw()
{
  set<string> ceurls;
  vector<Subscription> vec;
  vec.reserve(10);
  
  CREAM_SAFE_LOG(m_log_dev->debugStream() << "iceCommandSubUpdater::execute() - Checking "
		   << "subscription's time validity..."
		   << log4cpp::CategoryStream::ENDLINE);
  ceurls.clear();
    
  retrieveCEURLs(ceurls);
    
  for(set<string>::iterator it=ceurls.begin(); it != ceurls.end(); ++it) 
  {
    vec.clear();
      
      
    bool subscribed;
    {
      boost::recursive_mutex::scoped_lock M( ice_util::subscriptionManager::mutex );
      try{subscribed = m_subMgr->subscribedTo( *it, vec );}
      catch(exception& ex) {
	CREAM_SAFE_LOG(m_log_dev->errorStream() << "subscriptionUpdater::body() - "
		       << "Could not determine if we're subscribed to [" << *it 
		       << "]. Retrying later."
		       << log4cpp::CategoryStream::ENDLINE);
	continue;
      }
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

}

//______________________________________________________________________________
void ice_util::iceCommandSubUpdater::retrieveCEURLs( set<string>& urls )
{
    urls.clear();
    string ceurl, cemonURL;
    list<CreamJob> jobList;
    
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
      boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
      for(ice_util::jobCache::iterator it=ice_util::jobCache::getInstance()->begin();
        it != ice_util::jobCache::getInstance()->end(); 
	++it) 
      {
	jobList.push_back( *it );
      }
    } 
    
    for(list<CreamJob>::const_iterator it = jobList.begin();
        it != jobList.end();
	++it)
    {
        ceurl = it->getCreamURL();
	{
	  boost::recursive_mutex::scoped_lock cemonM( ice_util::cemonUrlCache::mutex );	
	  cemonURL = ice_util::cemonUrlCache::getInstance()->getCEMonURL( ceurl );
	}
	
	urls.insert( cemonURL );
    }
    
    cemonUrlCache::getInstance()->getCEMons( urls );
}

//______________________________________________________________________________
void ice_util::iceCommandSubUpdater::renewSubscriptions(vector<Subscription>& vec)
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
	    boost::recursive_mutex::scoped_lock M( ice_util::subscriptionManager::mutex );
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
