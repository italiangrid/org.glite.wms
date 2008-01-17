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
 * ICE job cache Iterator
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_JOBCACHEITERATOR_H
#define GLITE_WMS_ICE_UTIL_JOBCACHEITERATOR_H

#include<set>
#include<string>
#include<sstream>

#include "creamJob.h"

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	class jobCache;
	
	class jobCacheIterator {
	
	  friend class jobCache;
	  
	  bool m_valid_it;
	  
	 protected:
	  std::set<std::string>::iterator  m_it;
	  static jobCache                 *s_cache;
	  CreamJob        	           m_theJob;
	  
	 public:
	 
	  jobCacheIterator( const std::set<std::string>::iterator anIt) throw() : m_valid_it( false ), m_it( anIt ) { }
	  jobCacheIterator( const jobCacheIterator& anIt ) throw() : m_valid_it( false ), m_it(anIt.m_it), m_theJob(anIt.m_theJob) { }
	  
	  jobCacheIterator&  operator++() throw()          { m_it++; m_valid_it=false; return *this; }
	  jobCacheIterator&  operator--() throw()          { m_it--; m_valid_it=false; return *this; }
	  
	  CreamJob&          operator*() throw();
	  CreamJob*          operator->() throw();
	  bool               operator==( const jobCacheIterator& ) const throw();
	  bool               operator==( const std::set<std::string>::iterator ) const throw();
	  bool               operator!=( const jobCacheIterator& ) const throw();
	  bool               operator!=( const std::set<std::string>::iterator ) const throw();
	  jobCacheIterator&  operator=( const jobCacheIterator& ) throw();
	  jobCacheIterator&  operator=( const std::set<std::string>::iterator ) throw();
	  
	};
	     
      }
    }
  }
}

#endif
