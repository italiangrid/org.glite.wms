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

#include "iceUtils.h"
#include <unistd.h>
#include <netdb.h>
#include <cerrno>

extern int h_errno;
extern int errno;

namespace glite {
namespace wms {
namespace ice {
namespace util {

using namespace std;

string getHostName( void ) throw ( runtime_error& )
{
    char name[256];
    
    if ( ::gethostname(name, 256) == -1 ) {
        throw runtime_error( string( "Could not resolve local hostname: ") 
                             + string(strerror(errno) ) );
    }
    struct hostent *H=::gethostbyname(name);
    if ( !H ) {
        throw runtime_error( string( "Could not resolve local hostname: ") 
                             + string(strerror(errno) ) );
    }
    return string(H->h_name);
}

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite
