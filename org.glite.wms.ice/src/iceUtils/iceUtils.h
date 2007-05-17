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
 * ICE utility functions
 *
 * Authors: Moreno Marzolla <moreno.marzolla@pd.infn.it>
 *	    Alvise Dorigo <alvise.dorigo@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_ICEUTILS_HH
#define GLITE_WMS_ICE_UTIL_ICEUTILS_HH

#include <string>
#include <map>
#include <list>
#include <set>
#include <stdexcept>
#include <ctime>
#include <utility>
#include <sys/types.h>
#include <sys/socket.h>

#include "creamJob.h"

#include <algorithm>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>

namespace glite {
namespace wms {
namespace ice {
namespace util {

    class jobCache;

    struct ltstring {
      bool operator()( const std::pair<std::string, std::string>& s1, 
		       const std::pair<std::string, std::string>& s2) const
      {
	
	if ( s1.first.compare(s2.first) < 0 ) return true;
	else {
	  if(s2.first.compare(s1.first) < 0 ) return false;
	  else {
	    if( s1.second.compare(s2.second) < 0 ) return true;
	    else return false;
	  }
	}
	
      }
    };
  
    /**
     * Utility function to return the hostname
     *
     * @return the host name
     */
    std::string getHostName( void ) throw ( std::runtime_error& );

    std::string getURL( void ) throw ( std::runtime_error& );

    /**
     * Converts a time_t value to a string. This function
     * uses the thread-safe function ctime_r.
     *
     * @param tval the time_t value to convert
     * @result the textual representation of timestamp tval
     */ 
    std::string time_t_to_string( time_t tval );

    /**
     * This function outputs a string containing the CREAM and grid
     * jobid for a given job. This function should be used whenever
     * any information related to a job needs to be logged into the
     * log files.
     *
     * @param job the job whose cream/grid id should be returned
     *
     * @return the string containing cream and grid job id, in
     * human-readable form
     */
    //std::string describe_job( const CreamJob& job );

    void makePath(const std::string& file) throw(std::exception&); // FIXME: never used?

    std::string getNotificationClientDN( const std::string& );

    std::string getCompleteHostname( const std::string& );

    std::string canonizeString( const std::string& ) throw();

    /**
      * Breaks the list of jobs into sublists of size at most max_size
      *
      * @param jobs the list of jobs
      *
      * @param max_size the max size of the returned sublists
      *
      * @return the list of chunks
      */
    //void createChunksOfCreamJobs(const std::list< CreamJob >&, std::list< std::list< CreamJob > >&, const unsigned int);

  	/**
	 * Ad-hoc implementation of the copy_n algorithm, returning an
	 * InputIterator. There actually is a copy_n algorithm defined in
	 * GNU C++ implementation of the STL
	 * (/usr/include/g++-3/stl_algobase.h), but it says that it is not
	 * part of the C++ standard.
	 *
	 * This function copies at most n elements from the range
	 * [InputIterator, InputIterator+n-1] (bounds included) into the
	 * range [OutputIterator, OutputIterator+n-1] (bounds included).
	 * If the source range is less than n elements wide, only the
	 * elements in the range are copied.
	 *
	 * This function assumes that first and end are iterators to a
	 * container of CreamJob objects. dest must be an iterator to a
	 * container of string objects. This function copies the CREAM Job
	 * ID from object referenced by the first iterator into the second
	 * iterator.
	 *
	 * @param first the iterator of the first element in the source range
	 *
	 * @param end the iterator of the end of the source range. This
	 * iterator is used to check if the input range is less than n
	 * elements wide.
	 *
	 * @param n the maximum number of items to copy
	 *
	 * @param dest the iterator to the first element in the destination range
	 *
	 * @return an iterator to the input element PAST the last element copied.
	 */
    template <class InputIterator, class Size, class OutputIterator> 
    InputIterator copy_n_elements( InputIterator first, InputIterator end, Size n, OutputIterator dest) {
      for ( ; n > 0 && first != end; --n ) {
	*dest = first->getCreamJobID();
	++first;
	++dest;
      }
      return first;
    }

    template <class InputIterator, class Size, class OutputIterator, class UnaryOperation> 
    InputIterator transform_n_elements( InputIterator first, InputIterator end, Size n, OutputIterator dest, UnaryOperation f) {
      for ( ; n > 0 && first != end; --n ) {
	*dest = f( *first );
	++first;
	++dest;
      }
      return first;
    }

 

  class jobMap_appender {

    std::map< std::pair<std::string, std::string>, std::list< CreamJob >, ltstring>& m_jobMap;
    bool (*m_predicate)(const CreamJob&);

  public:
    jobMap_appender( std::map< std::pair<std::string, std::string>, std::list< CreamJob >, ltstring>& jobMap, bool (*pred)(const CreamJob&)) : m_jobMap( jobMap ), m_predicate( pred ) {}
    
    void operator()( const CreamJob& j ) {
      if( m_predicate( j ) )
	m_jobMap[std::make_pair( j.getUserDN(), j.getCreamURL())].push_back( j );
    }
    
  };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
