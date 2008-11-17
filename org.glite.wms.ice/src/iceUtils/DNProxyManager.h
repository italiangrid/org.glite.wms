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

#include <string>
#include <map>
#include <boost/thread/recursive_mutex.hpp>

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
        std::map<std::string, std::pair<std::string, time_t> > m_DNProxyMap;
        log4cpp::Category *m_log_dev;
        
    protected:
        
        DNProxyManager() throw();
        ~DNProxyManager() throw() {}
        static boost::recursive_mutex  mutex;
        
    public:
        
        static DNProxyManager* getInstance() throw();
        void setUserProxyIfLonger( const std::string& proxy) throw();
        void setUserProxyIfLonger( const std::string& dn, const std::string& proxy) throw();
	void setUserProxyIfLonger( const std::string& dn, const std::string& proxy, const time_t ) throw();
        
        std::string getBetterProxyByDN( const std::string& dn ) const throw() {
            
	    boost::recursive_mutex::scoped_lock M( mutex );
	    std::map<std::string, std::pair<std::string, time_t> >::const_iterator it = m_DNProxyMap.find( dn );
	    if( it == m_DNProxyMap.end()) return "";
	    return it->second.first; // return the local copy of the proxy, because the sandbox's one could be removed
            
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
