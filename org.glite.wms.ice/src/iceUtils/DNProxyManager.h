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

#include "SourceProxyNotFoundException.h"

#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "creamJob.h"

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
         * The m_DNProxyMap[ dn ] is a tuple (proxycert file, expirationTime, duration).
	 * The key 'dn' is the distinguished name (DN+FQAN) of the user
         */
        
	std::map< std::string, std::set< std::string> > m_temp_dnproxy_Map;
        log4cpp::Category *m_log_dev;
        
    protected:
        
        DNProxyManager() throw();
        ~DNProxyManager() throw() {}
        static boost::recursive_mutex  s_mutex;
        
    public:
        
        static DNProxyManager* getInstance() throw();
        void setUserProxyIfLonger_Legacy( const std::string& proxy) throw();
        void setUserProxyIfLonger_Legacy( const std::string& dn, const std::string& proxy) throw();
	void setUserProxyIfLonger_Legacy( const std::string& dn, const std::string& proxy, const time_t ) throw();
       
/* 	void incrementUserProxyCounter( const std::string& dn,  */
/* 					const std::string& myproxyname ) throw(); */

	void incrementUserProxyCounter( const CreamJob& aJob,
					const time_t proxy_time_end) throw();

	void decrementUserProxyCounter( const std::string& dn, 
					const std::string& myproxyname ) throw();

	void setBetterProxy( const std::string& dn,
			     const std::string& proxyfile,
			     const std::string& myproxyname,
			     const time_t,
			     const unsigned long long) throw();

	boost::tuple<std::string, time_t, long long int> getExactBetterProxyByDN( const std::string& dn,
										  const std::string& myproxyname) const throw() ;

	boost::tuple<std::string, time_t, long long int> getAnyBetterProxyByDN( const std::string& dn ) const throw();

	
	void removeBetterProxy( const std::string& dn, const std::string& myproxyname ) throw();

	void updateBetterProxy( const std::string& userDN, 
				const std::string& myproxyname,
				const boost::tuple<std::string, 
				time_t, 
				long long int>& newEntry ) throw();

	
	
    private:

	std::string make_betterproxy_path( const std::string& dn, const std::string& myproxy ) throw();
	
        void copyProxy( const std::string& source, const std::string& target ) throw(SourceProxyNotFoundException&);

	std::string composite( const std::string& userDN, const std::string& myproxy_name) const throw()
	  {
	    return userDN+"_"+myproxy_name;
	  }

    };
    
}
}
}
}

#endif
