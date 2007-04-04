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
#include <set>
#include <stdexcept>
#include <ctime>
#include <utility>
#include <sys/types.h>
#include <sys/socket.h>

#include "creamJob.h"

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
    std::string describe_job( const CreamJob& job );

    void makePath(const std::string& file) throw(std::exception&); // FIXME: never used?

    std::string getNotificationClientDN( const std::string& );

    std::string getCompleteHostname( const std::string& );
 
} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
