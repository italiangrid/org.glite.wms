/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
#include "wmputils.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include <iostream>
// by dirmanagement
#include <boost/lexical_cast.hpp>

using namespace std;
void
waitForSeconds(int seconds)
{
	cerr<<"-----> Waiting for "<<seconds<<" seconds..."<<endl;
	time_t startTime = time(NULL);
	time_t endTime = time(NULL);
	int counter = 0;
	while((endTime - startTime) < seconds) {
		if ((endTime%3600) != counter) {
			switch (counter%4) {
				case 0:
					cerr<<"-"<<endl;
					break;
				case 1:
					cerr<<"\\"<<endl;
					break;
				case 2:
					cerr<<"|"<<endl;
					break;
				case 3:
					cerr<<"/"<<endl;
					break;
				default:
					break;
			}
			counter = endTime%3600;
		}
		endTime = time(NULL);
	}
	cerr<<"-----> End waiting"<<endl;
}

std::string
to_filename(glite::wmsutils::jobid::JobId j, int level, bool extended_path)
{
	std::string path(glite::wmsutils::jobid::get_reduced_part(j, level));
	if (extended_path) {
		path.append(std::string("/") + glite::wmsutils::jobid::to_filename(j));
	}
	return path;
}

void 
managedir ( const string &dir , int userid ){
	// Try to find managedirexecutable
	char* glite_path = getenv ("GLITE_WMS_LOCATION");
	string gliteDirmanExe = (glite_path==NULL)?("/opt/glite"):(string(glite_path));
	gliteDirmanExe += "/bin/glite-wms-dirmanager";
	// SET UP arguments
	string arguments ="";
	arguments += " -c "; // UID
	arguments += " -g " + boost::lexical_cast<std::string>(getgid()); // GROUP
	arguments += " -m 0770 "; // MODE
	arguments += dir; // DIRECTORY
	string command = gliteDirmanExe  + " " + arguments;
	string error_msg ="";
	int exit_value= system(command.c_str());
	switch (exit_value){
		case 0:
			break;
		default:
			error_msg ="Unexpected error while launching:\n" + gliteDirmanExe ;
	}
	if (exit_value!=0) throw JobOperationException(__FILE__, __LINE__, "managedir()" , WMS_OPERATION_NOT_ALLOWED,error_msg);
}
