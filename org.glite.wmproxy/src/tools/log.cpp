/*
Copyright (c) Members of the EGEE Collaboration. 2004. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

    http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and 
limitations under the License.
*/

//
// Author: Marco Sottilaro <marco.sottilaro@datamat.it>
//

// HEADER
#include "log.h"

#include "fstream" // file streams
#include "sstream" // file streams
#include "time.h" // time fncts
#include "errno.h"

#include <sys/types.h>//getpid
#include <unistd.h>//getpid
#include <iostream>

#include "utilities/wmputils.h"
//BOOST
//#include "boost/filesystem/path.hpp" // prefix & files procedures
//#include "boost/filesystem/exception.hpp" //managing boost errors

using namespace std ;
namespace utils = glite::wms::wmproxy::utilities;


//namespace fs = boost::filesystem ;

namespace glite{
namespace wms{
namespace wmproxy{
namespace tools{

const string monthStr[]  = {"Jan", "Feb", "March", "Apr", "May", "June" ,"July", "Aug", "Sept", "Oct", "Nov", "Dec"};

/*
* Default constructor
*/
Log::Log (std::string path, LogLevel level){
        dbgLevel = level;
	// check writing permission
	ofstream outputstream(path.c_str(), ios::app);
	if (outputstream.is_open() ) {
		logFile = new string (path) ;
		outputstream.close ( );
	} else {
		logFile = NULL;
		print (WMS_WARNING,
			"I/O error",
			"unable to open the logfile: " + path ,
			true);
	}
}

/*
* Default destructor
*/
Log::~Log( ) {
	if (logFile) { delete(logFile);}
}

/*
* prints the error message
*/
void Log::errMsg(severity sev, const std::string &header, const std::string& err, const bool &debug, std::string* path){
        const string stripe= "-----------------------------------------";
        char ws=(char)32;
        ostringstream px ;
        string mglog = "";
        string mgout = "";
        WmcStdStream ss = (WmcStdStream)0;
        time_t now = time(NULL);
        // PREFIX MSG: timestamp
        struct tm *ns = localtime(&now);
        // PREFIX MSG: time stamp: day-month
        px << TIMESTAMP_FILLING << ns->tm_mday << ws << monthStr[ns->tm_mon] << ws << (ns->tm_year+1900) <<"," << ws;
        // PREFIX MSG: time stamp: hh::mm:ss
        px << TIMESTAMP_FILLING << ns->tm_hour << ":" << TIMESTAMP_FILLING << ns->tm_min << ":" << TIMESTAMP_FILLING << ns->tm_sec << ws;
        // PREFIX MSG: pid
        px << "-I- PID:" << ws << getpid( ) << ws   ;
        switch (sev){
                case WMS_DEBUG:{
                        // log message
                        mglog = px.str( ) +  string("(Debug) -") + ws + header + ws + err + string("\n");
                        if (debug){
                                // std out message
                                mgout += stripe +"\n";
                                mgout += mglog ;
                                mgout += stripe +"\n";
                                ss = WMC_OUT;
                        }
                        break;
                }
                case WMS_INFO:{
                        mglog = px.str( ) + string("(Info) :\n") + header + ws + err + string("\n");
                        if (debug){
                                mgout = string("\n") + header + ws + err + string("\n\n");
                                ss = WMC_OUT;
                        }
                        break;
                }
                case WMS_WARNING:{
                        mglog += px.str( ) +  string("(Warning) -") + ws + header + ws + err +string("\n");
                        if (debug){
                                mgout = string("Warning -") +  ws + header  ;
                                if (err.size()>0){ mgout += string("\n") + err ;}
                                ss = WMC_ERR;
                        }
                        break;
                }
                case WMS_ERROR:{
                        mglog += px.str( ) +  string("(Error) -") + ws + header + ws + string(":\n") + err + string("\n");
                        if (debug){
                                mgout  =  string("Error -") + ws +header + string("\n") + err +string("\n");
                                ss = WMC_ERR;
                        }
                        break;
                }
                case WMS_FATAL:{
                        mglog += px.str( ) + string( "(Fatal Error) -") + ws + header + ws +  string(":\n") + err + string("\n");
                        if (debug){
                                mgout =  string("Fatal Error -") + ws+ header +  string("\n") + err + string("\n");
                                ss = WMC_ERR;
                        }
                        break;
                }
                default:{
                        mglog += header + ws + err + string("\n");
                        break;
                }
        }
        // prints the message into a file
        if (path) {
                ofstream outstream(path->c_str(), ios::app);
                if (outstream.is_open() ) {
                        // to file
                        outstream << stripe << "\n";
                        outstream << mglog ;
                        // closes the file
                        outstream.close ( );
                }
        }
        // prints the message on the std-output/error
        if (debug){
                if (ss==WMC_OUT){
                        cout << mgout << flush ;
                } else if (ss==WMC_ERR){
                        cerr << "\n" << mgout << "\n" << flush ;
                }
        }
};

/*
* Write the messsage string in the file
*/
void Log::print (severity sev, const std::string &header, const std::string &msg, const bool debug) {
	bool dbg = false;
	string message = "";
	const string stripe= "------------------------------------------";
        if ( debug && (int)sev >= (int)dbgLevel){ dbg = true;}
        errMsg(sev, header, msg, dbg, logFile);
}

/*
* Gets the log file pathname
*/
std::string* Log::getPathName( ){
	if (logFile){
                return new string(utils::getAbsolutePath(*logFile));
 	} else{
		return NULL;
        }
}

}}}} // ending namespaces
