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
 * Author: Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_ICEUTILS_HH
#define GLITE_WMS_ICE_UTIL_ICEUTILS_HH

#include <string>
#include <stdexcept>

namespace glite {
namespace wms {
namespace ice {
namespace util {

    /**
     * Utility function to return the hostname
     */
    std::string getHostName( void ) throw ( std::runtime_error& );

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
