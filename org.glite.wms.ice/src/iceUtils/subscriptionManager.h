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
 * ICE CEMON URL Cache
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_SUBMGR_H
#define GLITE_WMS_ICE_UTIL_SUBMGR_H

#include <string>
#include <set>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/scoped_ptr.hpp>
#include "CEDbManager.h"
#include "iceSubscription.h"
#include "iceUtils.h"

#include <utility>

#include <iostream>

#ifndef DEFAULT_PERSIST_DIR
#define DEFAULT_PERSIST_DIR "/tmp/ice_persist_dir"
#endif

//using namespace std;

namespace log4cpp {
    class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	class iceConfManager;
	
	class subscriptionProxy;
	
	class jobCache;
	
	class subscriptionManager {
	  
	  boost::scoped_ptr< glite::wms::ice::util::CEDbManager > m_dbMgr;
	  
	  std::set<std::string>                   		  m_cemonURL;
	  
/* 	  struct ltstring { */
/* 	    bool operator()( const std::pair<std::string, std::string>& s1,  */
/* 			     const std::pair<std::string, std::string>& s2) const */
/* 	    { */

/* 	      if ( s1.first.compare(s2.first) < 0 ) return true; */
/* 	      else { */
/* 		if(s2.first.compare(s1.first) < 0 ) return false; */
/* 		else { */
/* 		  if( s1.second.compare(s2.second) < 0 ) return true; */
/* 		  else return false; */
/* 		} */
/* 	      } */

/* 	    } */
/* 	  }; */

	  std::map< std::pair<std::string, std::string> , iceSubscription, glite::wms::ice::util::ltstring >  m_Subs;
	  
	  std::set<std::string>                   		  m_DN;
	  std::map<std::string, std::string>      		  m_mappingCreamCemon;
	  std::map<std::string, std::string>      		  m_mappingCemonDN;
	  
	  //std::map<std::string, std::string>                      m_DNProxyMap;

	  iceConfManager                         		 *m_conf;  
	  subscriptionProxy                     		 *m_subProxy;
	  static subscriptionManager                   		 *s_instance;
	  log4cpp::Category                      		 *m_log_dev;
	  
	  static std::string 			  		  s_persist_dir;
	  static bool				  		  s_recoverable_db;
	  
	  jobCache						 *m_cache;
	  
	  bool							  m_authz;
	  bool							  m_authn;
	  //std::set<std::string>					  
	  
	  void init( void ) throw();
	  
	 protected:
	  subscriptionManager() throw();
	  ~subscriptionManager() throw() {}
	  
	 public:
	 
	  //typedef std::set<iceSubscription>::iterator iterator;
	  //typedef std::set<iceSubscription>::const_iterator const_iterator;
	  
	  typedef std::map< std::pair<std::string, std::string> , iceSubscription>::iterator iterator;
	  typedef std::map< std::pair<std::string, std::string> , iceSubscription>::const_iterator const_iterator;
	  
	  subscriptionManager::iterator begin()
	  {
	    return m_Subs.begin();
	  }
	  
	  subscriptionManager::iterator end()
	  {
	    return m_Subs.end();
	  }
	  
	  subscriptionManager::const_iterator begin() const
	  {
	    return m_Subs.begin();
	  }
	  
	  subscriptionManager::const_iterator end() const
	  {
	    return m_Subs.end();
	  }
	  
	  static subscriptionManager* getInstance() throw();
	  
	  void getCEMonURL(const std::string& proxy, const std::string& creamURL, std::string& cemonURL) throw();
          
	  bool getCEMonDN( const std::string& proxy, const std::string& cemonURL,std::string& DN) throw();

	  bool hasCEMon( const std::string& cemon ) const throw()
	  {
 	    return( m_cemonURL.find( cemon ) != m_cemonURL.end() );
	  }
	  
	  bool isAuthorized( const std::string& DN ) const throw()
	  {
	    return ( m_DN.find(DN) != m_DN.end() );
	  }

 	  void insertSubscription( const std::string& userProxy,
					      const std::string& cemonURL,
					      const iceSubscription& S ) throw();
	  
	  void getCEMons( std::vector<std::string>& ) throw();
          void getCEMons( std::set<std::string>& ) throw();
	  
	  bool hasSubscription( const std::string& userProxy, const std::string& cemon ) const throw();
	  
	  void removeSubscription( const std::string& userProxy, const std::string& cemon) throw();
	  
	  void getUserCEMonMapping( std::map< std::string, std::set<std::string> >& target, 
				    const bool only_active_jobs = false ) throw();
	  
	  void renewSubscription(const std::string& userProxy, const std::string& cemon) throw();
	  void checkSubscription(std::map<std::string, std::set<std::string> >::const_iterator) throw();
	  void purgeOldSubscription( std::map<std::string, std::set<std::string> >::const_iterator it ) throw();

	  static void setPersistDirectory(const std::string& dir) { s_persist_dir = dir; }
	  static void setRecoverableDb( const bool recover ) { s_recoverable_db=recover; }
	  
	  int numOfSubscriptionKeys( void ) const { return m_Subs.size(); }

	  //void setUserProxyIfLonger( const std::string& proxy);
	  //void setUserProxyIfLonger( const std::string& dn, const std::string& proxy);

// 	  std::string getBetterProxyByDN( const std::string& dn ) const throw() {
// 	    std::map<std::string, std::string>::const_iterator it = m_DNProxyMap.find( dn );
// 	    if( it == m_DNProxyMap.end()) return "";
// 	    return it->second;
// 	  }

	  bool getSubscriptionByDNCEMon( const std::string& dn, const std::string& cemon, iceSubscription& target) {
	    
	    std::map< std::pair<std::string, std::string> , iceSubscription, ltstring >::const_iterator it = m_Subs.find( make_pair(dn, cemon) );
	    
	    if( it == m_Subs.end() ) return false;
	    
	    /* target.setSubscriptionID( it->second.getSubscriptionID() ); */
/* 	    target.setExpirationTime( it->second.getExpirationTime() ); */

	    target = it->second;

	    return true;
	  }

	  static boost::recursive_mutex  mutex;

 	};
	
      }
    }
  }
}

#endif
