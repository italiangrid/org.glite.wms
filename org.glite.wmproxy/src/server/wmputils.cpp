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
to_filename(glite::wmsutils::jobid::JobId j, int level, bool extended_path){
	std::string path(glite::wmsutils::jobid::get_reduced_part(j, level));
	if (extended_path) {
		path.append(std::string("/") + glite::wmsutils::jobid::to_filename(j));
	}
	return path;
}

int execute (const string &command, const string &executable){
	string error_msg ="";
	return system( command.c_str() );
}

int
managedir ( const string &dir , int userid ){
	int exit_code=0;
	// Define File Separator
#ifdef WIN
	// Windows File Separator
	const string FILE_SEP = "\\";
#else
        // Linux File Separator
	const string FILE_SEP ="/";
#endif
	// Try to find managedirexecutable
	char* glite_path = getenv ("GLITE_WMS_LOCATION");
	string gliteDirmanExe = (glite_path==NULL)?("/opt/glite"):(string(glite_path));
	gliteDirmanExe += "/bin/glite-wms-dirmanager";
	// Set Arguments
	string arguments =" ";
	arguments += " -c "; // UID
	arguments += " -g " + boost::lexical_cast<std::string>(getgid()); // GROUP
	arguments += " -m 0770 "; // MODE

	string argdir= dir  ; // DIRECTORY
	int sep_index = 0 ;
	sep_index = dir.find ( FILE_SEP , sep_index);
	argdir = dir.substr(0,sep_index) ;
	exit_code | execute ( gliteDirmanExe  + arguments + argdir  , gliteDirmanExe) ;
	argdir = dir.substr(sep_index) ;
	exit_code | execute ( gliteDirmanExe  + arguments + argdir  , gliteDirmanExe) ;
	return exit_code;
}
