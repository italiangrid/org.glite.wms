// HEADER
#include "logman.h"
#include "excman.h"
// utilities
#include "utils.h"
#include "fstream" // file streams
#include "sstream" // file streams
#include "time.h" // time fncts
#include "errno.h"
// wmproxy API's
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"
// BOOST
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/exception.hpp"


using namespace std ;
using namespace glite::wms::wmproxyapiutils;
namespace fs = boost::filesystem;

namespace glite{
namespace wms{
namespace client{
namespace utilities{
/*
* Default constructor
*/
Log::Log (std::string* path, LogLevel level){
        dbgLevel = level;
	// check writing permission
	if (path){
		this->createLogFile(*path);
	} else{
		logFile = NULL;
	}
}

/*
* Default destructor
*/
Log::~Log( ) {
	if (logFile) { delete(logFile);}
}

void Log::createLogFile(const std::string &path){
	ofstream outputstream(path.c_str(), ios::app);
	if (outputstream.is_open() ) {
		logFile = new string (path) ;
		if (logCache.size() > 0 ) {
			// writes the content of the cache into the new file
			outputstream << logCache ;
			// cleans the cache
			logCache = string("");
		 }
		outputstream.close ( );
	} else {
		if ( dbgLevel >= WMSLOG_WARNING ){
			errMsg (WMS_WARNING,
				"I/O error",
				"unable to open the logfile: " + path ,
				true);
			logFile = NULL;
		}
	}
}
/*
*Write the exception messsage in the file
*/
void Log::print (severity sev, const std::string &header,glite::wmsutils::exception::Exception &exc, const bool debug, const bool cache) {
	bool dbg = false;
	string message = "";
	const string stripe= "-----------------------------------------";
        if ( debug &&  (int)sev >= (int)dbgLevel){ dbg = true;}
        message = errMsg(sev, header, exc, dbg, logFile);
	// adds the message to the internal cache
	if (cache) { logCache += stripe + "\n" + message ;}
}
/*
* Write the messsage string in the file
*/
void Log::print (severity sev, const std::string &header, const std::string &msg, const bool debug, const bool cache) {
	bool dbg = false;
	string message = "";
	const string stripe= "------------------------------------------";
        if ( debug && (int)sev >= (int)dbgLevel){ dbg = true;}
        message = errMsg(sev, header, msg, dbg, logFile);
	// adds the message to the internal cache
	if (cache) { logCache +=  stripe + "\n" + message ;}
}
/*
* Write the messsage string in the file
*/
void Log::service(const std::string& service) {
	print(WMS_DEBUG, "Calling the WMProxy " + service, "service" );
}
/*
* Write the messsage string in the file
*/
void Log::service(const std::string& service, const std::string& jobid) {
	print(WMS_DEBUG,
		"Calling the WMProxy " + service + "for the job: " + jobid,
		"");
}
/*
* Write the messsage string in the file
*/
void Log::service(const std::string& service, const std::vector <std::pair<std::string , std::string> > &params) {
	int size = params.size( );
	string msg = "Calling the WMProxy " + service  ;
	if (size>0) {
		msg += " service with the following parameter(s):\n";
		for(int i=0; i < size; i++) {
			msg += "> " + params[i].first + "=[" + params[i].second + "]\n";
		}
	}
	print(WMS_DEBUG, msg, "");
}
/*
* Write the result messsage string in the file
*/
void Log::result(const std::string& service, const std::string msg) {
	print(WMS_DEBUG, string(service + " - ") , msg );
}

/*
* Gets the log file pathname
*/
std::string* Log::getPathName( ){
	if (logFile){
                return new string(Utils::getAbsolutePath(*logFile));
 	} else{
		return NULL;
        }
}

}}}} // ending namespaces
