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
#include "DNProxyManager.h"
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
namespace {

  class singleSubUpdater {
    ice_util::subscriptionManager *m_subManager;
    string m_dn;

  public:
    singleSubUpdater( ice_util::subscriptionManager *subManager, string dn ) : m_subManager( subManager ), m_dn( dn ) { }

    void operator()( string endpoint ) {

      ice_util::iceSubscription sub;
      if(!m_subManager->getSubscriptionByDNCEMon(m_dn, endpoint, sub)) {
	// this should almost never happen because before to update
	// the caller also invokes the subManager->checkSubscription()
	CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->errorStream() 
		       << "iceCommandSubUpdater::execute() - "
		       << "Not found any subscription for DN ["
		       << m_dn << "] and CEMon ["
		       << endpoint << "]. Skipping..."
		       << log4cpp::CategoryStream::ENDLINE);
	return;
      }
      
      CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->infoStream() 
		     << "iceCommandSubUpdater::execute() - "
		     << "Checking Subscription validity for DN ["
		     << m_dn << "] to CEMon ["
		     << endpoint << "]"
		     << log4cpp::CategoryStream::ENDLINE);
      
      time_t timeleft = sub.getExpirationTime() - time(NULL);

      if(timeleft < ice_util::iceConfManager::getInstance()->getConfiguration()->ice()->subscription_update_threshold_time()) {
	string betterProxy;
	{
	  boost::recursive_mutex::scoped_lock M( ice_util::DNProxyManager::mutex );
	  betterProxy = ice_util::DNProxyManager::getInstance()->getBetterProxyByDN( m_dn );
	}
	
	if(betterProxy == "") {
	  CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->errorStream() 
			 << "iceCommandSubUpdater::execute() - "
			 << "Cannot get better proxy file for DN ["
			 << m_dn << "]. Skipping..."
			 << log4cpp::CategoryStream::ENDLINE);
	  return;
	}
	
	CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->infoStream() 
		       << "iceCommandSubUpdater::execute() - "
		       << "Updating subscription ["<<sub.getSubscriptionID() 
		       << "] to CEMon [" << endpoint <<"] for user DN ["
		       << m_dn << "] using proxy ["
		       << betterProxy << "]."
		       << log4cpp::CategoryStream::ENDLINE);
	
	m_subManager->renewSubscription( betterProxy , endpoint );
      }
    }
  }; // end class
} // end anonymous namespace

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

    //subManager->purgeOldSubscription( it );

    subManager->checkSubscription( *it );
    
    singleSubUpdater updater( subManager, it->first );

    for_each(it->second.begin(), it->second.end(), updater);

//     for(set<string>::const_iterator cit = it->second.begin();
// 	cit != it->second.end();
// 	++cit)
//       {
	
//       }

  }
}
