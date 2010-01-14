
#include "iceCommandDelegationRenewal.h"
#include "Delegation_manager.h"
#include "CreamProxyMethod.h"
#include "DNProxyManager.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "iceDb/GetJobByGid.h"
#include "iceDb/Transaction.h"
#include "iceDb/UpdateJobByGid.h"
#include "glite/security/proxyrenewal/renewal.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
/**
 *
 * Cream Client API C++ Headers
 *
 */
#include "glite/ce/cream-client-api-c/scoped_timer.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"


/**
 *
 * OS Headers
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <cerrno>

extern int errno;

namespace cream_api = glite::ce::cream_client_api;

using namespace std;
using namespace glite::wms::ice::util;

#define DELEGATION_EXPIRATION_THRESHOLD_TIME 3600
//int iceCommandDelegationRenewal::DELEGATION_EXPIRATION_THRESHOLD_TIME = 3600
//______________________________________________________________________________
iceCommandDelegationRenewal::iceCommandDelegationRenewal( ) :
    iceAbsCommand( "iceCommandDelegationRenewal" ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_ctx( NULL )
{
}

//______________________________________________________________________________
iceCommandDelegationRenewal::~iceCommandDelegationRenewal( )
{
}

//______________________________________________________________________________
void iceCommandDelegationRenewal::execute( void ) throw()
{  
#ifdef ICE_PROFILE
  ice_timer timer("iceCommandDelegationRenewal::execute");
#endif
  this->renewAllDelegations();
    
}

//______________________________________________________________________________
void iceCommandDelegationRenewal::renewAllDelegations( void ) throw() 
{
    static const char* method_name = "iceCommandDelegationRenewal::renewAllDelegations() - ";
    
    //char* new_proxy = NULL;
 
    /**
       Now, let's check all delegations for expiration and renew them
    */
    
    vector< Delegation_manager::table_entry > allDelegations;
    
    int alldeleg_size = Delegation_manager::instance()->getDelegationEntries( allDelegations, true );
    
    map<string, pair<time_t, int> > mapDelegTime; // delegationID -> (Expiration time, Absolute Duration)
    
    CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                    << "There are [" << alldeleg_size
                    << "] Delegation(s) to check..."
                    );
    
    if( alldeleg_size == 0 ) return;
    
    /**
       Loop over all different delegations
    */
    for( vector<Delegation_manager::table_entry >::const_iterator it = allDelegations.begin();
	 it != allDelegations.end(); 
	 ++it)
      {
	
	
	time_t thisExpTime   = it->m_expiration_time;
	string thisDelegID   = it->m_delegation_id;        
	int    thisDuration  = it->m_delegation_duration;
	int    remainingTime = thisExpTime - time(0);
	if( (remainingTime > 0.2 * thisDuration) && 
	    (remainingTime > DELEGATION_EXPIRATION_THRESHOLD_TIME) )
	  {
	    CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
			    << "Delegation ID ["
			    << thisDelegID << "] will expire in ["
			    << remainingTime << "] seconds (on ["
			    << time_t_to_string(thisExpTime) << "]). Duration is ["
			    << thisDuration << "] seconds. Will NOT renew it now..."
			    );
	    mapDelegTime.erase( thisDelegID );
	    continue;
	    
	  }
	

	/**
	   Let's try to download from MyProxy Service
	   a fresh and hopefully most long-living proxy for current user
	*/
	boost::tuple<string, time_t, long long int> SBP( DNProxyManager::getInstance()->getExactBetterProxyByDN(it->m_user_dn, it->m_myproxyserver) );
	string certfile( SBP.get<0>() );
	if( certfile.empty() ) {
	  CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
			  << "There is NOT a better proxy for user [" 
			  << it->m_user_dn << "] related to current delegation id to check [" 
			  << it->m_delegation_id <<"]. Skipping this delegation renewal check..."
			  );
	  mapDelegTime.erase( thisDelegID );
	  continue;
	}
	
	CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
			<< "Contacting MyProxy server [" 
			<< it->m_myproxyserver << "] for user dn ["
			<< it->m_user_dn << "] with proxy certificate ["
			<< certfile << "] to renew it..."
			);

	/**
	 *
	 * CODE to POPEN to glite-wms-ice-proxy-renew
	 *
	 */
	string output;
	string command = "export X509_USER_CERT=" + iceConfManager::getInstance()->getConfiguration()->common()->host_proxy_file();
	command += "; export X509_USER_KEY=" + iceConfManager::getInstance()->getConfiguration()->common()->host_proxy_file();
	command += "; /opt/glite/bin/glite-wms-ice-proxy-renew -s " + it->m_myproxyserver;
	command += " -p " + certfile;
	command += " -o " + certfile + ".renewed";

	CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
			<< "Executing command [" 
			<< command << "]..."
			);

	FILE *res = popen(command.c_str(), "r");
	if(!res) {
	  CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
			  << "Couldn't popen command [" 		  
			  << command << "]: " << strerror(errno)
			  << ". Skipping this delegation renew..."
			  );
	} else {

	  while(!feof(res)) {
	    output += fgetc(res);
	  }

	  int pcloseret = pclose( res );

	  CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
			  << "Command output is ["
			  << output << "]"
			  );

	  if( pcloseret == -1 ) // the command failed for some reason
	    {
	      CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
			  << "pclose failed for error: " 		  
			  << strerror(errno)
			  << ". Cannot determine if proxy has been correctly renewed..."
			  );
	     
	    }
	  if( pcloseret == 1 ) {
	    CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
			  << "Proxy renewal failed: [" 		  
			    << output << "]"
			  );
	  }

	  if( pcloseret == 0 ) {
	    //	    new_proxy = (char*)output.c_str();
	    //int pos = output.find('\n');
	    //new_proxy = (char*)output.substr(0, pos).c_str();
	    CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
			    << "Proxy renewal successful: new proxy is [" 		  
			    << certfile + ".renewed" << "]. It will overwrite the better one..."
			    );
	    /**
	       Must substitute old better proxy with new downloaded one, if it's more long-living 
	    */
	    cream_api::soap_proxy::VOMSWrapper V( certfile + ".renewed", !::getenv("GLITE_WMS_ICE_DISABLE_ACVER") );
	    if( !V.IsValid( ) ) {
	      CREAM_SAFE_LOG(m_log_dev->errorStream() 
			     << method_name
			     << "Cannot parse downloaded new proxy ["
			     << certfile + ".renewed" << "] from MyProxy Service ["
			     << it->m_myproxyserver << "]. Error is: "
			     << V.getErrorMessage()
			     );
	      
	    } else {
	      
	      /** SUBSTITUTE old better proxy with downloaded one. 
	       */
	      boost::tuple<string, time_t, long long int> newPrx = boost::make_tuple( certfile + ".renewed", V.getProxyTimeEnd(), -1 ); // -1 means to not modify the job counter
	      DNProxyManager::getInstance()->updateBetterProxy( it->m_user_dn,
								it->m_myproxyserver,
								newPrx );
	      ::unlink( (certfile + ".renewed").c_str() );
	      
	    }
	    
	    
	  }
	}
	
	mapDelegTime[ thisDelegID ] = make_pair(thisExpTime, thisDuration);
	
	/**
	   if the current delegation ID is not expiring then skip it
	   but remove it from the map mapDelegTime anyway, in order to not update
	   the job cache later
	*/
	string thisUserDN    = it->m_user_dn;
	string thisCEUrl     = it->m_cream_url;
	string thisMyPR      = it->m_myproxyserver;
	
	/**
	   Obtain the better proxy for the DN-FQAN related to current
	   delegation ID.
	   Looking at the "new" code of DNProxyManager one can see that this better
	   proxy can be obtained in two different ways:
	   1. as the proxy that ICE registered to the myproxyserver and that is automatically renewed by it
	   2. as the ICE-calculated better proxy (calculated at each submission)
	*/
	
	CREAM_SAFE_LOG(m_log_dev->debugStream() 
		       << method_name
		       << "Looking for the better proxy for DN ["
		       << thisUserDN << "] MyProxy Server name ["
		       << thisMyPR << "]..."
		       );
	
	boost::tuple<string, time_t, long long int> thisBetterPrx = DNProxyManager::getInstance()->getExactBetterProxyByDN( thisUserDN, thisMyPR );
	
	/**
	   Proxy NOT found for the userdn related to this delegation.
	   Remove the delegation
	*/
	if(thisBetterPrx.get<0>().empty()) {
	  CREAM_SAFE_LOG(m_log_dev->debugStream() 
			 << method_name
			 << "DNProxyManager::getExactBetterProxyByDN didn't return any better proxy for DN ["
			 << thisUserDN << "] and MyProxy Server name ["
			 << thisMyPR << "]... Skipping renew of this delegation ["
			 << thisDelegID << "]."
			 );
	  mapDelegTime.erase( thisDelegID );

	  Delegation_manager::instance()->removeDelegation( thisDelegID );

	  continue;
	}
	
	/**
	   if the returned better proxy is the "new" one must get the actual expiration time
	   (remember that it is renewed by myproxyserver asynchronously and ICE doesn't know
	   when this happen)
	*/
	
	//	cream_api::soap_proxy::VOMSWrapper V( thisBetterPrx.get<0>() );

	pair<bool, time_t> result_validity = isgood( thisBetterPrx.get<0>() );
	if(!result_validity.first) {
	  CREAM_SAFE_LOG(m_log_dev->errorStream() 
			 << "iceCommandProxyRenewal::renewAllDelegations() - "
			 << "iceUtil::isgood() function reported an error while" 
			 << " parsing proxy [" << thisBetterPrx.get<0>() 
			 << "]. Skipping renew of delegation ["
			 << thisDelegID <<"]..."
			 );
	  mapDelegTime.erase( thisDelegID );
	  continue;
	}
	
	time_t proxy_time_end = result_validity.second;
	
	/**
	   If the super better proxy for this delegation ID is expired, it
	   means that there're no new jobs for this DN-FQAN_MYPROXY since
	   long and/or has not been done connections to myProxy or those connections
	   have failed... then we can remove the delegatiob ID from memory.
	*/
	if( proxy_time_end <= (time(0)-5) ) {
	  CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
			  << "For current Delegation ID [" << thisDelegID 
			  <<"] DNProxyManager returned the Better Proxy ["
			  << thisBetterPrx.get<0>() 
			  << "] that is EXPIRED! Removing this delegation ID from "
			  << "Delegation_manager, removing proxy from DNProxyManager's cache, won't renew delegation ..."
			  );
	  
	  Delegation_manager::instance()->removeDelegation( thisDelegID );
	  
	  DNProxyManager::getInstance()->removeBetterProxy( thisUserDN, thisMyPR );
	  
	  // TODO: Bisogna loggare un aborted per tutti i job relativi a questa delega
	  // che non verra' piu' rinnovata ed e' stata rimossa dal DB di ICE.

	  mapDelegTime.erase( thisDelegID );
	  continue;
	}
	
	/**
	   If the BetterProxy for this delegation is not expiring after the delegation itself
	   let's continue with next delegation...
	*/
	if( !(proxy_time_end > thisExpTime)) {
	  CREAM_SAFE_LOG(m_log_dev->warnStream() 
			 << "iceCommandProxyRenewal::renewAllDelegations() - "
			 << "The better proxy ["
			 << thisBetterPrx.get<0>() << "] is expiring NOT AFTER the current delegation ["
			 << thisDelegID << "]. Skipping ... "
			 );
	  
	  mapDelegTime.erase( thisDelegID );
	  continue;
	}

	CREAM_SAFE_LOG( m_log_dev->infoStream() << method_name
			<< "Will Renew Delegation ID ["
			<< thisDelegID << "] with BetterProxy ["
			<< thisBetterPrx.get<0>()
			<< "] that will expire on ["
			<< time_t_to_string(thisBetterPrx.get<1>()) << "]"
			);
	try {
	  
	  string thisDelegUrl = thisCEUrl;
	  
	  boost::replace_all( thisDelegUrl, 
			      iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_postfix(), 
			      iceConfManager::getInstance()->getConfiguration()->ice()->creamdelegation_url_postfix() 
			      );
	  
	  CreamProxy_ProxyRenew( thisDelegUrl,
				 thisBetterPrx.get<0>(),
				 thisDelegID).execute( 3 );
	  
	  mapDelegTime[ thisDelegID ] = make_pair(thisBetterPrx.get<1>(), thisBetterPrx.get<1>() - time(0) );
	  
	} catch( exception& ex ) {
	  CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
			  << "Proxy renew for delegation ID ["
			  << thisDelegID << "] failed: " << ex.what() 
			  );
	  
	  mapDelegTime.erase( thisDelegID );
	  continue;
	}
	
      }
    
    for(map<string, pair<time_t, int> >::iterator it = mapDelegTime.begin();
        it != mapDelegTime.end(); ++it) {
      
      Delegation_manager::instance()->updateDelegation( boost::make_tuple((*it).first, (*it).second.first, (*it).second.second ) );
      
    }    
}
