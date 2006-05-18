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
 * ICE subscription cache
 *
 * Author: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_SUBSCRIPTIONCACHE_H
#define GLITE_WMS_ICE_UTIL_SUBSCRIPTIONCACHE_H

#include <string>
#include <set>
#include "boost/thread/recursive_mutex.hpp"

namespace glite {
namespace wms {
namespace ice {
namespace util {

  class subscriptionCache {
      std::set<std::string> m_cemons;
      std::set<std::string>::const_iterator m_it;
      static subscriptionCache* s_instance;    
      
  protected:
      subscriptionCache();
      
  public:
      static boost::recursive_mutex mutex;
      static subscriptionCache* getInstance( );
      void insert( const std::string& s );
      void remove( const std::string& s );
      bool has( const std::string& );
  };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
