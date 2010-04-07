/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

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
/**
* Default constructor
*/
Log::Log (LogLevel level){
        dbgLevel = level;
	m_logFile = "";
}

void Log::createLogFile(const std::string &path){
	ofstream outputstream(path.c_str(), ios::app);
	if (outputstream.is_open() ) {
		m_logFile = path;
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
			m_logFile = "";
		}
	}
}
/**
*Writes the exception messsage in the file
*/
void Log::print (severity sev, const std::string &header,glite::wmsutils::exception::Exception &exc, const bool debug, const bool cache) {
	bool dbg = false;
	string message = "";
	const string stripe= "-----------------------------------------";
        if ( debug &&  (int)sev >= (int)dbgLevel){ dbg = true;}
        message = errMsg(sev, header, exc, dbg, &m_logFile);
	// adds the message to the internal cache
	if (cache) { logCache += stripe + "\n" + message ;}
}
/**
* Writes the messsage string in the file
*/
void Log::print (severity sev, const std::string &header, const std::string &msg, const bool debug, const bool cache) {
	bool dbg = false;
	string message = "";
	const string stripe= "------------------------------------------";
        if ( debug && (int)sev >= (int)dbgLevel){ dbg = true;}
        message = errMsg(sev, header, msg, dbg, &m_logFile);
	// adds the message to the internal cache
	if (cache) { logCache +=  stripe + "\n" + message ;}
}
/**
* Writes the messsage string in the file
*/
void Log::service(const std::string& service) {
	print(WMS_DEBUG, "Calling the WMProxy " + service, "service" );
}
/**
* Writes the messsage string in the file
*/
void Log::service(const std::string& service, const std::string& jobid) {
	print(WMS_DEBUG,
		"Calling the WMProxy " + service + " for the job: " + jobid,
		"");
}
/**
* Writes the messsage string in the file
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
/**
* Writes the result messsage string in the file
*/
void Log::result(const std::string& service, const std::string msg) {
	print(WMS_DEBUG, string(service + " - ") , msg );
}

}}}} // ending namespaces
