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
#include <stdexcept>
#include <ctime>
#include <sys/types.h>
#include <sys/socket.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>

namespace glite {
namespace wms {
namespace ice {
namespace util {

    /**
     * Utility function to return the hostname
     *
     * @return the host name
     */
    std::string getHostName( void ) throw ( std::runtime_error& );

    /**
     * Converts a time_t value to a string. This function
     * uses the thread-safe function ctime_r.
     *
     * @param tval the time_t value to convert
     * @result the textual representation of timestamp tval
     */ 
    std::string time_t_to_string( time_t tval );

    void makePath(const std::string& file) throw(std::exception&);

    std::string getNotificationClientDN( const std::string& );

    std::string getCompleteHostname( const std::string& );
 
   
} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
