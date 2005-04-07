/*
        Copyright (c) Members of the EGEE Collaboration. 2004.
        See http://public.eu-egee.org/partners/ for details on the copyright holders.
        For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXYAPICPP_WMP_JOB_EXAMPLES_H
#define GLITE_WMS_WMPROXYAPICPP_WMP_JOB_EXAMPLES_H

#include "glite/wms/wmproxyapi/wmproxy_api.h"

// Exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"

#include <iostream>
#include <vector>

extern "C" {
#include <getopt.h>
}
#include <fstream>
#include <sstream>

namespace glite {
namespace wms {
namespace wmproxyapi {
namespace examples {
namespace utilities {

// environment variable for the wmproxy service URL
#define GLITE_WMPROXY_ENDPOINT 	"GLITE_WMPROXY_ENDPOINT"
#define GLITE_TRUSTED_CERTS			 "/etc/grid-security/certificates"


char* clean(char *str);

std::string* jobidFromFile (const std::string &path);

std::string handle_exception (const glite::wms::wmproxyapi::BaseException &b_ex );

int saveToFile (const std::string &path, const std::string &bfr) ;


} //glite
}//wms
}//wmproxyapi
}//examples
}//utilities



#endif  // GLITE_WMS_WMPROXYAPICPP_WMP_JOB_EXAMPLES_H
