/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpcommon.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include "wmpcommon.h"

#include <ctype.h> // isspace

// WMP Configuration
#include "wmpconfiguration.h"

// Utilities
#include "utilities/wmputils.h"

// WMP Exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"

// JDL Attributes
#include "glite/jdl/JDLAttributes.h"
#include "glite/jdl/jdl_attributes.h"

// Logger
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"


// Global variables for configuration
extern WMProxyConfiguration conf;

// Global variables for configuration attributes (ENV dependant)
extern std::string sandboxdir_global;

// Document root variable
const char * DOC_ROOT = "DOCUMENT_ROOT";

// Defining File Separator
#ifdef WIN 
   // Windows File Separator 
   const std::string FILE_SEPARATOR = "\\"; 
#else 
   // Linux File Separator 
   const std::string FILE_SEPARATOR = "/";
#endif


using namespace std;
using namespace glite::jdl; // Ad
using namespace glite::wms::wmproxy::utilities; //Exception

namespace logger         = glite::wms::common::logger;
namespace wmputilities	 = glite::wms::wmproxy::utilities;


void
setGlobalSandboxDir()
{
	GLITE_STACK_TRY("setGlobalSandboxDir()");
	edglog_fn("wmpcommon::setGlobalSandboxDir");
	
	string sandboxstagingpath = conf.getSandboxStagingPath();
	
	if (!getenv(DOC_ROOT)) {
		edglog(fatal)<<"DOCUMENT_ROOT variable not set"<<endl;
		throw FileSystemException( __FILE__, __LINE__,
  			"setGlobalSandboxDir()", wmputilities::WMS_FILE_SYSTEM_ERROR,
   			"DOCUMENT_ROOT variable not set");
	}
	string documentroot = getenv(DOC_ROOT);
	if (documentroot == "") {
		edglog(fatal)<<"DOCUMENT_ROOT is an empty string"<<endl;
		throw FileSystemException( __FILE__, __LINE__,
  			"setGlobalSandboxDir()", wmputilities::WMS_FILE_SYSTEM_ERROR,
   			"DOCUMENT_ROOT is an empty string"
   			"\n(please contact server administrator)");
	}
	
	// Checking if sandbox staging path starts with $DOCUMENT_ROOT
	if (sandboxstagingpath.find(documentroot) != 0) {
		edglog(severe)<<"ERROR: SandboxStagingPath inside "
			"configuration file MUST start with $DOCUMENT_ROOT"<<endl;
		throw FileSystemException( __FILE__, __LINE__,
  			"setGlobalSandboxDir()", wmputilities::WMS_FILE_SYSTEM_ERROR,
   			"SandboxStagingPath inside configuration file MUST start "
   			"with $DOCUMENT_ROOT\n(please contact server administrator)");
	}
	
	if (sandboxstagingpath != documentroot) {
		sandboxdir_global = sandboxstagingpath.substr(documentroot.length() + 1,
			sandboxstagingpath.length() - 1);
		if (sandboxdir_global.find(FILE_SEPARATOR) != string::npos) {
			string msg = "SandboxStagingPath configuration attribute MUST be "
				"in the form:"
	   			"\n$DOCUMENT_ROOT/<single directory name>"
	   			"\nwhere DOCUMENT_ROOT MUST be as defined in httpd configuration file"
	   			"\n(please contact server administrator)";
			edglog(fatal)<<msg<<endl;
			throw FileSystemException( __FILE__, __LINE__,
	  			"setGlobalSandboxDir()", wmputilities::WMS_FILE_SYSTEM_ERROR,
	   			msg);
			}
	} else {
		sandboxdir_global = documentroot;
	}
	edglog(debug)<<"Sandbox directory: "<<sandboxdir_global<<endl;
	
	GLITE_STACK_CATCH();
}

void
logRemoteHostInfo()
{
	GLITE_STACK_TRY("logRemoteHostInfo()");
	edglog_fn("wmpcommon::logRemoteHostInfo");
	
	string msg = "Remote Host IP: ";
	string msg2 = "Remote CLIENT S DN: ";
	string msg3 = "Remote GRST CRED: ";
	string msg4 = "Service GRST PROXY LIMIT: ";
	edglog(info)
		<<"-------------------------------- Incoming Request "
			"--------------------------------"
		<<endl;
	
	if (getenv("REMOTE_ADDR")) {
		msg += string(getenv("REMOTE_ADDR"));
		if (getenv("REMOTE_PORT")) {
			msg += ":" + string(getenv("REMOTE_PORT"));
		}
	} else {
		msg += "Not Available";
	}
	msg += " - Remote Host Name: ";
	if (getenv("REMOTE_HOST")) {
		msg += string(getenv("REMOTE_HOST"));
	} else {
		msg += "Not Available";
	}
	if (getenv("SSL_CLIENT_S_DN")) {
		msg2 += string(getenv("SSL_CLIENT_S_DN"));
	} else {
		msg2 += "Not Available";
	}
	if (getenv("GRST_CRED_2")) {
		msg3 += string(getenv("GRST_CRED_2"));
	} else {
		msg3 += "Not Available";
	}
	if (getenv("GRST_GSIPROXY_LIMIT")) {
		msg4 += string(getenv("GRST_GSIPROXY_LIMIT"));
	} else {
		msg4 += "Not Available";
	}
	
	edglog(info)<<msg<<endl;
    edglog(info)<<msg2<<endl;
	edglog(info)<<msg3<<endl;
	edglog(info)<<msg4<<endl;
	edglog(info)
		<<"----------------------------------------"
			"------------------------------------------"
	<<endl;

	setGlobalSandboxDir();
		
	GLITE_STACK_CATCH();
}

/**
 * Returns an int value representing the type of the job described by the jdl
 * If type is not specified -> Job  // TBC change it in the future?
 */
int
getType(string jdl, Ad * ad)
{
	GLITE_STACK_TRY("getType()");
	edglog_fn("wmpcommon::getType");
	
	edglog(debug)<<"Getting type"<<endl;
	// Default type is normal
	int return_value = TYPE_JOB;
	
	Ad *in_ad = new Ad(jdl);
	
	if (in_ad->hasAttribute(JDL::TYPE)) {
		string type = 
			glite_wms_jdl_toLower((in_ad->getStringValue(JDL::TYPE))[0]);
		edglog(info) <<"Type: "<<type<<endl;
		if (type == JDL_TYPE_DAG) {
			return_value = TYPE_DAG;
		} else if (type == JDL_TYPE_JOB) {
			return_value = TYPE_JOB;
		} else if (type == JDL_TYPE_COLLECTION) {
			return_value = TYPE_COLLECTION;
		}
	}
	if (ad) {
		*ad = *in_ad;
	}
	delete in_ad;
	return return_value;

	GLITE_STACK_CATCH();
}

void
throwerror(const string &errorfile)
{
	string stderrormsg
		= wmputilities::readTextFile(errorfile);
	remove(errorfile.c_str());
	throw ServerOverloadedException(__FILE__, __LINE__,
		"callLoadScriptFile()", 
		wmputilities::WMS_SERVER_OVERLOADED,
		"System load is too high:\n" + stderrormsg);
}

void
callLoadScriptFile(const string &operation)
{
	GLITE_STACK_TRY("callLoadScriptFile()");
	edglog_fn("wmpcommon::callLoadScriptFile");
	
	string path = conf.getOperationLoadScriptPath(operation);
	if (path != "") {
		// Default values:
		// glite-load-monitor --oper jobRegister --load1 10 --load5 10
		//        --load15 10 --memusage 95 --diskusage 95 --fdnum 500
		
		string str = "";
		string command = "";
		vector<string> params;
		bool commandfound = false;
		string::const_iterator iter = path.begin();
  		string::const_iterator const end = path.end();
		while (iter != end) {
			// Removing spaces
			while ((iter != end) && isspace((char)*iter)) {
				iter++;
			}
			while ((iter != end) && !isspace((char)*iter)) {
				str += *iter;
				iter++;
			}
			edglog(debug)<<"Found token: "<<str<<endl;
			if (commandfound) {
				params.push_back(str);
			} else {
				command = str;
				commandfound = true;
			}
			str = "";
		}
		
		params.push_back("2>");
		string errorfile = "/tmp/wmpscriptcall.err."
			+ boost::lexical_cast<std::string>(getpid());
		edglog(debug)<<"Script error file: "<<errorfile<<endl;
		params.push_back(errorfile);
		
		if (!wmputilities::fileExists(command)) {
			edglog(warning)<<"Operation \""<<operation
				<<"\" load script file does not exist:\n"<<command
				<<"\nIgnoring load script call..."<<endl;
		} else {
			// Creating stderr file
			int fd = open(errorfile.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(fd, 2);
			close(fd);
			
			// Executing load script file
			string errormsg = "";
			edglog(debug)<<"Executing load script file: "<<command<<endl;
			if (int outcome = wmputilities::doExecv(command, params, errormsg)) {
				switch (outcome) {
					case -1:
						throwerror(errorfile);
	   					break;
	   				default:
	   					edglog(error)<<"Unable to execute load script file:\n"
	   						<<errormsg<<endl;
	   					edglog(error)<<"Error code: "<<outcome<<endl;
	   					break;
				}
			}
			remove(errorfile.c_str());
		}
	}
	
	GLITE_STACK_CATCH();
}
