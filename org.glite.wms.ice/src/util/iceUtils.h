/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *    http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * ICE utility functions
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_ICEUTILS_HH
#define GLITE_WMS_ICE_UTIL_ICEUTILS_HH

#include <string>
#include <stdexcept>
#include <ctime>
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

    void makePath(const std::string& file) throw(std::exception&);

    std::string getNotificationClientDN( const std::string& );

    std::string getCompleteHostname( const std::string& );
 
   
} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
