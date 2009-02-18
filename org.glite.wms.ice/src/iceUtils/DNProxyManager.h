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

#ifndef GLITE_WMS_ICE_UTIL_DNPROXYMGR
#define GLITE_WMS_ICE_UTIL_DNPROXYMGR

#include <utility>
#include <string>
#include <map>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/tuple/tuple.hpp>
#include <cstring>
#include <string.h>

#include "jobCache.h"
#include "SourceProxyNotFoundException.h"

#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "iceConfManager.h"
#include "iceUtils.h"

namespace log4cpp {
  class Category;
};

namespace glite {
namespace wms {
namespace ice {
namespace util {

    class DNProxyManager {
        
        static DNProxyManager               *s_instance;
        /**
         * The m_DNProxyMap[ dn ] is a pair (proxy, expirationTime).
	 * The key 'dn' is the distinguished name (DN+FQAN) of the user
         */
        
	std::map< std::string, boost::tuple<std::string, time_t, long long int> > m_DNProxyMap;

        log4cpp::Category *m_log_dev;
        
    protected:
        
        DNProxyManager() throw();
        ~DNProxyManager() throw() {}
        static boost::recursive_mutex  mutex;
        
    public:
        
        static DNProxyManager* getInstance() throw();
        void setUserProxyIfLonger_Legacy( const std::string& proxy) throw();
        void setUserProxyIfLonger_Legacy( const std::string& dn, const std::string& proxy) throw();
	void setUserProxyIfLonger_Legacy( const std::string& dn, const std::string& proxy, const time_t ) throw();
        
	void registerUserProxy( const std::string& dn, const std::string& proxy, const std::string& server, const time_t proxy_time_end ) throw();
	void decrementUserProxyCounter( const std::string& dn, const std::string& myproxyname ) throw();

	//__________________________________________________________________________________________________________
	boost::tuple<std::string, time_t, long long int> getExactBetterProxyByDN( const std::string& dn,
										  const std::string& myproxyname) const throw() ;
// 	  {
// 	    
// 	    boost::recursive_mutex::scoped_lock M( mutex );
// 	    
// 	    std::map<std::string, boost::tuple<std::string, time_t, long long int> >::const_iterator it = m_DNProxyMap.find( dn+"_"+myproxyname );
// 	    
// 	    if( it == m_DNProxyMap.end() ) {
// 	      return boost::make_tuple("", 0, 0);
// 	    } else {
// 	      return boost::make_tuple(it->second.get<0>(), it->second.get<1>(), it->second.get<2>());
// 	    }
// 	    
// 	  }

	//__________________________________________________________________________________________________________
	boost::tuple<std::string, time_t, long long int> getAnyBetterProxyByDN( const std::string& dn ) const throw();
// 	  {
// 	    
// 	    boost::recursive_mutex::scoped_lock M( mutex );
// 	    
// 	    std::map<std::string, boost::tuple<std::string, time_t, long long int> >::const_iterator it = m_DNProxyMap.begin();
// 	    
// 	    //printf("*** DEBUG DNProxyManager: Searching a betterproxy for DN [%s]...\n", dn.c_str());
// 
// 	    while( it != m_DNProxyMap.end() ) {
// 	      
// 	      /**
// 		 does a regex match. The DN must be present in the key (that can also contains the myproxy server name)
// 	      */
// 	      
// 
// 
// 	      if( strstr( it->first.c_str(),dn.c_str() ) == 0 ) {
// 	       /**
// 		  'dn' is not found in the it->first string
// 	       */
// 	      // printf("*** DEBUG DNProxyManager: [%s] doesn't match...\n", it->first.c_str() );
// 	       ++it;
// 	       continue;
// 	     }
// 
// 
// 	      /**
// 		 The better proxy is used for operation about job management like: poll, purge, cancel... all but proxy renewal.
// 		 Then, it is not important that the better proxy is the most long-living one, but a valid one that is valid at least
// 		 for 2 times the SOAP_TIMEOUT.
// 	      */
// 	     if( it->second.get<1>() > (time(0)+(2*iceConfManager::getInstance()->getConfiguration()->ice()->soap_timeout())) ) 
// 	       {
// 		// printf("*** DEBUG DNProxyManager: [%s] MATCH and has a profer lifetime!...\n", it->first.c_str() );
// 		return (it->second);
// 	       }
// 	      
// 	     //printf("*** DEBUG DNProxyManager: [%s] MATCH but has NOT a profer lifetime [%s]!...\n", it->first.c_str(), time_t_to_string( it->second.get<1>() ).c_str() );
// 	     //	     cout << "*** DEBUG DNProxyManager: ["<<it->first.c_str()<<"] MATCH but has NOT a profer lifetime [" << it->second.get<1>() << "]" <<endl;
// 
// 	      ++it;
// 	    }
// 	    
// 	    //printf("*** DEBUG DNProxyManager: No PROXY suitable for DN [%s]...\n", dn.c_str() );
// 	    return boost::make_tuple("", 0, 0);
// 	    
// 	  }
	

	
	void removeBetterProxy( const std::string& dn, const std::string& myproxyname ) throw();

	void updateBetterProxy( const std::string& userDN, 
				const std::string& myproxyname,
				const boost::tuple<std::string, time_t, long long int>& newEntry ) throw();

    private:
        void copyProxy( const std::string& source, const std::string& target ) throw(SourceProxyNotFoundException&);
	//	std::pair<jobCache::iterator, time_t> searchBetterProxyForUser( const std::string& ) throw();

	std::pair<jobCache::iterator, time_t> searchBetterProxy( const std::string& ) throw();
        
    };
    
}
}
}
}

#endif
