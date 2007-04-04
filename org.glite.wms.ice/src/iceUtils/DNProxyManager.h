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

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class DNProxyManager {

	  static DNProxyManager               *s_instance;
	  std::map<std::string, std::string>   m_DNProxyMap;
	protected:

	  DNProxyManager() throw() {};
	  ~DNProxyManager() throw() {}
	  
	public:

	  static DNProxyManager* getInstance() throw();
	  void                   setUserProxyIfLonger( const std::string& proxy) throw();
	  void                   setUserProxyIfLonger( const std::string& dn, const std::string& proxy) throw();
	  std::string            getBetterProxyByDN( const std::string& dn ) const throw() {
	    std::map<std::string, std::string>::const_iterator it = m_DNProxyMap.find( dn );
	    if( it == m_DNProxyMap.end()) return "";
	    return it->second;
	  }

	  static boost::recursive_mutex  mutex;

	};

      }
    }
  }
}

#endif
