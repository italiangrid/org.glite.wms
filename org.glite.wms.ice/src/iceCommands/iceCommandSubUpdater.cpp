/**
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
#include "subscriptionManager.h"
//#include "subscriptionProxy.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <string>
#include <vector>

#include <boost/format.hpp>

using namespace std;

namespace api_util  = glite::ce::cream_client_api::util;
namespace cream_api = glite::ce::cream_client_api;
namespace ice_util  = glite::wms::ice::util;

//______________________________________________________________________________
ice_util::iceCommandSubUpdater::iceCommandSubUpdater( ) throw() : 
  m_log_dev( api_util::creamApiLogger::instance()->getLogger() ),
  m_conf( ice_util::iceConfManager::getInstance() )		     
{
}

//______________________________________________________________________________
void ice_util::iceCommandSubUpdater::execute( ) throw()
{
  map<string, set<string> > UserCEMons;
  ice_util::subscriptionManager *subManager = ice_util::subscriptionManager::getInstance();
  
  boost::recursive_mutex::scoped_lock M( ice_util::subscriptionManager::mutex );
  
  subManager->getUserCEMonMapping( UserCEMons, true );// this is a map userproxy -> set_of_cemons
  


  // first of all let's check all subs

  for(map<string, set<string> >::const_iterator it=UserCEMons.begin();
      it != UserCEMons.end();
      ++it)
  {
    CREAM_SAFE_LOG(m_log_dev->infoStream() 
		   << "iceCommandSubUpdater::execute() - "
		   << "Checking Subscriptions for owner of the DN ["
		   << it->first<< "]."
		   << log4cpp::CategoryStream::ENDLINE);

    subManager->checkSubscription( it );
    
    for(set<string>::const_iterator cit = it->second.begin();
	cit != it->second.end();
	++cit)
      {
	iceSubscription sub;
	if(!subManager->getSubscriptionByDNCEMon(it->first, *cit, sub)) {
	  CREAM_SAFE_LOG(m_log_dev->errorStream() 
			 << "iceCommandSubUpdater::execute() - "
			 << "Not found any subscription for DN ["
			 << it->first << "] and CEMon ["
			 << *cit << "]. Skipping..."
			 << log4cpp::CategoryStream::ENDLINE);
	  continue;
	}
	
	CREAM_SAFE_LOG(m_log_dev->infoStream() 
		       << "iceCommandSubUpdater::execute() - "
		       << "Checking Subscription validity for DN ["
		       << it->first << "] to CEMon ["
		       << *cit << "]"
		       << log4cpp::CategoryStream::ENDLINE);
	
	time_t timeleft = sub.getExpirationTime() - time(NULL);
	if(timeleft < m_conf->getConfiguration()->ice()->subscription_update_threshold_time()) {
	  CREAM_SAFE_LOG(m_log_dev->infoStream() 
			 << "iceCommandSubUpdater::execute() - "
			 << "Updating subscription ["<<sub.getSubscriptionID() 
			 << "] to CEMon [" << *cit <<"] for user DN ["
			 << it->first << "] using proxy ["
			 << subManager->getBetterProxyByDN(it->first) << "]."
			 << log4cpp::CategoryStream::ENDLINE);

	  subManager->renewSubscription( subManager->getBetterProxyByDN(it->first) , *cit );
	}
      }

  }
}
