/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
#ifndef GLITE_WMS_WMPROXY_WMPUTILS_H
#define GLITE_WMS_WMPROXY_WMPUTILS_H

#include "glite/wmsutils/jobid/JobId.h"
void waitForSeconds(int seconds);
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
#endif // GLITE_WMS_WMPROXY_WMPUTILS_H
