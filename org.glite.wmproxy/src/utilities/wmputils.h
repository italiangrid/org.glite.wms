/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_UTILITIES_WMPUTILS_H
#define GLITE_WMS_WMPROXY_UTILITIES_WMPUTILS_H

#include "glite/wmsutils/jobid/JobId.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace utilities {
	
/*extern "C++"
{*/
bool doPurge(std::string dg_jobid);

bool getUserQuota(std::pair<long, long>& result, std::string uname);

bool getUserFreeQuota(std::pair<long, long>& result, std::string uname);
//}


/**
 * Gets the user Distinguished Name of the user. This is the DN of the request
 * environment
 * @return the user DN
 */
char* getUserDN();

void waitForSeconds(int seconds);

/**
 * Copy a file
 * @param source the source file
 * @param target the target file
 */
void fileCopy(const std::string &source, const std::string &target);

/**
 * Transform a Jobid into a valid filename
 * @param j the JobId instance
 * @param level
 * @param extended_path
 */
std::string to_filename(glite::wmsutils::jobid::JobId j, int level = 0,
	bool extended_path = true);
	
/**
* Generate / Manage directory properties
* @param dir name of the dir to be generated
* @param userid the id of the user
*/
int managedir ( const std::string &dir , int userid );

} // namespace utilities
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_UTILITIES_WMPUTILS_H
