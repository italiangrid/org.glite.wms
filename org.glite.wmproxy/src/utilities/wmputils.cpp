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

#include "logging.h"
#include "quota.h"
#include "purger.h" 

// Exceptions
#include "wmpexceptions.h"
#include "wmpexception_codes.h"

#include <iostream>
#include <fstream>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <boost/tokenizer.hpp>

// by dirmanagement
#include <boost/lexical_cast.hpp>

using namespace std;
//using namespace glite::wms::wmproxy::server;

namespace logger		  = glite::wms::common::logger;
namespace commonutilities = glite::wms::common::utilities;
namespace jobid			  = glite::wmsutils::jobid;
namespace purger          = glite::wms::purger;

namespace glite {
namespace wms {
namespace wmproxy {
namespace utilities {

// Environment variable name to get User Distinguished Name
const char* SSL_CLIENT_DN = "SSL_CLIENT_S_DN";

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
doPurge(std::string dg_jobid, std::string sandboxdir)
{ 
	if ( dg_jobid.length() ) { 
  		const jobid::JobId jobid(dg_jobid);
  		edglog(warning) << "JobId object for purging created: "<< dg_jobid << std::endl;
 
     	return purger::purgeStorage(jobid, sandboxdir);
    } else {
   		edglog(critical) << logger::setfunction("CFSI::doPurge()") << 
			"Error in Purging: Invalid Job Id. Purge not done." << std::endl;
      return false; 
    } 
}

bool
getUserQuota(std::pair<long, long>& result, std::string uname)
{
	result = commonutilities::quota::getQuota(uname);
}

bool
getUserFreeQuota(std::pair<long, long>& result, std::string uname)
{
	result = commonutilities::quota::getFreeQuota(uname);
}

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
	p = strstr(user_dn, "/CN=proxy");
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
fileCopy(const std::string& source, const std::string& target)
{
  edglog_fn("	fileCopy");
  edglog(info)<<"Copying file..."<<std::endl;
  edglog(debug)<<"Source: "<<source<<" Target: "<<target<<std::endl;
  
  std::ifstream in(source.c_str());
  if (!in.good()) {
    throw FileSystemException(__FILE__, __LINE__,
		"fileCopy(const std::string& source, const std::string& target)",
		WMS_IS_FAILURE, "Unable to copy file");
  }
  std::ofstream out(target.c_str());
  if (!out.good()) {
    throw FileSystemException(__FILE__, __LINE__,
		"fileCopy(const std::string& source, const std::string& target)",
		WMS_IS_FAILURE, "Unable to copy file");
  }
  out<<in.rdbuf(); // read original file into target
  
  struct stat from_stat;
  if (stat(source.c_str(), &from_stat) ||
        chown(target.c_str(), from_stat.st_uid, from_stat.st_gid) ||
        chmod(target.c_str(), from_stat.st_mode)) {
    edglog(severe)<<"Copy failed. Source: "<<source<<" Target: "<<target<<std::endl;

    throw FileSystemException(__FILE__, __LINE__,
		"fileCopy(const std::string& source, const std::string& target)",
		WMS_IS_FAILURE, "Unable to copy file");
  }
  edglog(debug)<<"Copy done."<<std::endl;
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

int execute (const string &command)
{
	return system( command.c_str() );
}

int managedir( const std::string &document_root , int userid , std::vector<std::string> jobids)
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
   // Set Common Arguments 
   string arguments =" "; 
   arguments += " -c " + boost::lexical_cast<std::string>( userid  ); // UID 
   arguments += " -g " + boost::lexical_cast<std::string>( getgid()); // GROUP 
   arguments += " -m 0770 "; // MODE 
   int level = 0; 
   bool extended_path = true ; 
   const string executable = gliteDirmanExe + arguments + document_root + FILE_SEP; 
   boost::char_separator<char> sep(FILE_SEP.c_str()); 
   // Iterate over the jobs 
   for (unsigned int i = 0 ; i < jobids.size() ; i++){ 
   		// boost::tokenizer<> 
       boost::tokenizer<boost::char_separator<char> > 
               tok(to_filename (glite::wmsutils::jobid::JobId ( jobids[i] ), level, extended_path ), sep); 
       // For each job more than one directory might be created 
               for(boost::tokenizer<boost::char_separator<char> >::iterator beg=tok.begin(); beg!=tok.end();++beg){ 
                       if( execute (executable +*beg) ) {return 1;} 
               } 
  	}
    return exit_code; 
} 


} // namespace utilities
} // namespace wmproxy
} // namespace wms
} // namespace glite

