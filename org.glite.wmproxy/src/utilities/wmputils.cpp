/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include "wmputils.h"

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

#include "glite/wms/common/utilities/quota.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

//#include "logging.h"
//#include "purger.h" 

// Exceptions
#include "wmpexceptions.h"
#include "wmpexception_codes.h"

#include <iostream>
#include <fstream>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

// by dirmanagement
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace glite::wms::wmproxy::server;

namespace logger        = glite::wms::common::logger;
namespace utilities     = glite::wms::common::utilities;
// namespace requestad     = glite::wms::jdl;
namespace jobid         = glite::wmsutils::jobid;
//namespace purger        = glite::wms::purger;

namespace glite {
namespace wms {
namespace wmproxy {
namespace utilities {

/*
bool
doPurge(std::string dg_jobid)
{ 
	if ( dg_jobid.length() ) { 
  		const jobid::JobId jobid(dg_jobid);
  		edglog(warning) << "JobId object for purging created: "<< dg_jobid << std::endl;
 
     	return purger::purgeStorage(jobid);
    } else {
   		edglog(critical) << logger::setfunction("CFSI::doPurge()") << 
			"Error in Purging: Invalid Job Id. Purge not done." << std::endl; 
      return false; 
    } 
}

bool
getUserQuota(std::pair<long, long>& result, std::string uname)
{
	result = utilities::quota::getQuota(uname);
}

bool
getUserFreeQuota(std::pair<long, long>& result, std::string uname)
{
	result = utilities::quota::getFreeQuota(uname);
}
*/

const char* SSL_CLIENT_DN = "SSL_CLIENT_S_DN";

char *
getUserDN()
{
	char* p = NULL;
	char* client_dn = NULL;
	char* user_dn = NULL;
	
	client_dn = getenv(SSL_CLIENT_DN);
	if ((client_dn == NULL) || (client_dn == '\0')) {
		throw ProxyOperationException(__FILE__, __LINE__,
			"getUserDN()", WMS_PROXY_ERROR, "Unable to get a valid user DN");
	}
	
	user_dn = strdup(client_dn);
	p = strstr(user_dn, "/CN=proxy"); ///TBC ITERATE????
	if (p != NULL) {
		*p = '\0';      
	}
	p = strstr(user_dn, "/CN=limited proxy");
	if (p != NULL) {
		*p = '\0';      
	}
	if ((user_dn == NULL) || (user_dn[0] == '\0')) {
		throw ProxyOperationException(__FILE__, __LINE__,
			"getUserDN()", WMS_PROXY_ERROR, "Unable to get a valid user DN");
	}
	
	return user_dn;
}

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


void
fileCopy(const string &source, const string &target) 
{
	char ch;
	ifstream source_stream;
  	ofstream target_stream;
  	filebuf *source_buffer;
  	filebuf *target_buffer;

  	source_stream.open(source.c_str());
  	target_stream.open(target.c_str());

  	source_buffer=source_stream.rdbuf();
  	target_buffer=source_stream.rdbuf();

  	ch = source_buffer->sgetc();
  	while (ch != EOF) {
		target_buffer->sputc(ch);
		ch = source_buffer->snextc();
  	}

  	target_stream.close();
  	source_stream.close();
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
managedir ( const string &dir , int userid )
{
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
	if (glite_path==NULL) glite_path = getenv ("GLITE_LOCATION");
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

} // namespace utilities
} // namespace wmproxy
} // namespace wms
} // namespace glite

