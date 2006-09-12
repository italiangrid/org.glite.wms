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

#ifndef GLITE_WMS_ICE_UTIL_CEMONURLCACHE_H
#define GLITE_WMS_ICE_UTIL_CEMONURLCACHE_H

#include <string>
#include <set>
#include "boost/thread/recursive_mutex.hpp"

namespace log4cpp {
    class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	class iceConfManager;
	class subscriptionManager;
	
	
	class cemonUrlCache {
	  
	  
	  
	  std::set<std::string>               m_cemonURL;
	  std::set<std::string>               m_DN;
	  std::map<std::string, std::string>  mappingCreamCemon;
	  std::map<std::string, std::string>  mappingCemonDN;
	  
	  iceConfManager                           *m_conf;  
	  subscriptionManager                      *m_subMgr;
	  static cemonUrlCache                     *s_instance;
	  log4cpp::Category                        *m_log_dev;
	  
	 protected:
	  cemonUrlCache() throw();
	  ~cemonUrlCache() throw() {}
	  
	 public:
	  static cemonUrlCache* getInstance();
	  
          std::string     getCEMonURL(const std::string& creamURL);
          bool            getCEMonDN( const std::string& cemonURL,std::string& DN);
	  bool            hasCEMon( const std::string& ) const;
	  bool            isAuthorized( const std::string& ) const;
	  void            insertCEMon( const std::string& cemon ) 
	  {
	    m_cemonURL.insert( cemon );
	  }
	  void            insertDN( const std::string& DN)
	  {
	    m_DN.insert( DN );
	  }
	  
	  void            getCEMons( std::vector<std::string>& );
          void            getCEMons( std::set<std::string>& );
	  
	  static boost::recursive_mutex  mutex;
 	};
	
      }
    }
  }
}

#endif
