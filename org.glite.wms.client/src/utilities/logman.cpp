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
