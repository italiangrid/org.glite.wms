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

#include "jobCache.h"
#include "SourceProxyNotFoundException.h"

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
        //std::map<std::string, std::pair<std::string, time_t> > m_DNProxyMap;
	std::map< std::string, boost::tuple<std::string, time_t> > m_DNProxyMap_Legacy;
	std::map< std::string, boost::tuple<std::string, time_t, long long int> > m_DNProxyMap_NEW;

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

	void decrementUserProxyCounter( const std::string& dn ) throw();

	//std::pair<std::string, time_t> getBetterProxyByDN( const std::string& dn ) const throw() {
	//__________________________________________________________________________________________________________
	boost::tuple<std::string, time_t, int> getBetterProxyByDN( const std::string& dn ) const throw() {
            
	    boost::recursive_mutex::scoped_lock M( mutex );
	    //std::map<std::string, std::pair<std::string, time_t> >::const_iterator it = m_DNProxyMap.find( dn );
	    std::map<std::string, boost::tuple<std::string, time_t, long long int> >::const_iterator it = m_DNProxyMap_NEW.find( dn );

	    //if( it == m_DNProxyMap.end()) return std::make_pair("", 0);

	    if( it == m_DNProxyMap_NEW.end() ) {

	      /**
		 Not found in the primary map (the proxies that have been register by ICE into the myproxy server.
		 Try with the old one because probably the user sent the job without the MYPROXYSERVER set.
	      */
	      std::map<std::string, boost::tuple<std::string, time_t> >::const_iterator it_old = m_DNProxyMap_Legacy.find( dn );
	      if( it_old == m_DNProxyMap_Legacy.end() ) {
		return boost::make_tuple("", 0, 0);
	      } else {
		return boost::make_tuple( it_old->second.get<0>(), it_old->second.get<1>(), 0 );
	      }
	      
	    } else {

	      //	    return std::make_pair(it->second.first, it->second.second); // return the local copy of the proxy, because the sandbox's one could be removed
	      
	      return boost::make_tuple(it->second.get<0>(), it->second.get<1>(), it->second.get<2>());
	    }

            
        }
        
    private:
        void copyProxy( const std::string& source, const std::string& target ) throw(SourceProxyNotFoundException&);
	std::pair<jobCache::iterator, time_t> searchBetterProxyForUser( const std::string& ) throw();
        
    };
    
}
}
}
}

#endif
