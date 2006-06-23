/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmputils.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include "wmputils.h"

#include <iostream>
#include <fstream>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/param.h> // MAXLENPATH
#include <fcntl.h> // O_RDONLY
#include <netdb.h> // gethostbyname
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h> // wait
#include <sys/file.h> // flock

// boost
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>

// JobId
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

// Logging
#include "logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#ifndef GLITE_WMS_WMPROXY_TOOLS
#include "quota.h"
#include "glite/wms/purger/purger.h" 
#include "glite/wms/common/utilities/quota.h"

// Exceptions
#include "wmpexceptions.h"
#include "wmpexception_codes.h"

// ISB compression functionalities
#include "zlib.h"
#include "libtar.h"

#define local

// Global variables for configuration attributes
extern std::string sandboxdir_global;

namespace logger		  = glite::wms::common::logger;
namespace commonutilities = glite::wms::common::utilities;
namespace jobid			  = glite::wmsutils::jobid;
namespace purger          = glite::wms::purger;
#endif // #ifndef GLITE_WMS_WMPROXY_TOOLS

using namespace std;

namespace glite {
namespace wms {
namespace wmproxy {
namespace utilities {

// Define File Separator 
#ifdef WIN
	// Windows File Separator 
	const string FILE_SEP = "\\";
#else 
	// Linux File Separator 
   	const string FILE_SEP ="/"; 
#endif

#ifndef GLITE_WMS_WMPROXY_TOOLS

const int SUCCESS = 0;
const int FAILURE = 1;

// gLite environment variables
const char* GLITE_LOCATION = "GLITE_LOCATION";
const char* GLITE_WMS_LOCATION = "GLITE_WMS_LOCATION";

// Environment variable name to get User Distinguished Name
const char* SSL_CLIENT_DN = "SSL_CLIENT_S_DN";

const char* DOCUMENT_ROOT = "DOCUMENT_ROOT";

const string INPUT_SB_DIRECTORY = "input";
const string OUTPUT_SB_DIRECTORY = "output";
const string PEEK_DIRECTORY = "peek";

// Default name of the delegated Proxy copied inside private job directory
const string USER_PROXY_NAME = "user.proxy";
const string USER_PROXY_NAME_BAK = ".user.proxy.bak";

const string JDL_ORIGINAL_FILE_NAME = "JDLOriginal";
const string JDL_TO_START_FILE_NAME = "JDLToStart";
const string JDL_STARTED_FILE_NAME = "JDLStarted";

const string START_LOCK_FILE_NAME = ".startLockFile.lock";
const string GET_OUTPUT_LOCK_FILE_NAME = ".getOutputLockFile.lock";

const string ALL_PROTOCOLS = "all";
const string DEFAULT_PROTOCOL = "default";


vector<string>
computeOutputSBDestURIBase(vector<string> outputsb, const string &baseuri)
{
	GLITE_STACK_TRY("computeOutputSBDestURIBase()");
	unsigned int pos;
	int size;
	string path;
	vector<string> returnvector;
	
	vector<string>::iterator iter = outputsb.begin();
	vector<string>::iterator const end = outputsb.end();
	for (; iter != end; ++iter) {
		path = *iter;
		size = path.size();
		pos = path.rfind("/", size);
		if (pos != string::npos) {
			returnvector.push_back(baseuri + "/" + path.substr(pos + 1, size));
		} else {
			returnvector.push_back(baseuri + "/" + path);
		}
	}
	return returnvector;
	GLITE_STACK_CATCH();
}


vector<string>
computeOutputSBDestURI(vector<string> osbdesturi, const string &dest_uri)
{
	GLITE_STACK_TRY("computeOutputSBDestURI()");
	unsigned int pos;
	string uri;
	string path;
	string outputdir;
	vector<string> returnvector;
	
	vector<string>::iterator iter = osbdesturi.begin();
	vector<string>::iterator const end = osbdesturi.end();
	for (; iter != end; ++iter) {
		path = *iter;
		edglog(debug)<<"osbdesturi[i]: "<<*iter<<endl;
		pos = path.find("://");
		if (pos != string::npos) {
			// The path is an URL
			returnvector.push_back(path);
		} else {
			// The path is not an URL
			returnvector.push_back(dest_uri + "/" + OUTPUT_SB_DIRECTORY
				+ "/" + path);
		}
	}
	return returnvector;
	GLITE_STACK_CATCH();
}

vector<string> *
getJobDirectoryURIsVector(vector<pair<string, int> > protocols,
	const string &defaultprotocol, int defaultport, int httpsport,
	const string &jid, const string &protocol, const string &extradir)
{
	GLITE_STACK_TRY("getJobDirectoryURIsVector()");
	edglog_fn("wmpoperations::getJobDirectoryURIsVector");
	
	edglog(debug)<<"Computing job directory URIs for job: "<<jid<<endl;
	edglog(debug)<<"Requested protocol: "<<protocol<<endl;
	
	// Protocol + host:port + path
	string extra = (extradir != "") ? (FILE_SEP + extradir) : "";
	string httppath = FILE_SEP + to_filename(jobid::JobId(jid), 0) + extra;
	string path = getenv(DOCUMENT_ROOT) + httppath;
	string serverhost = getServerHost();
	
	vector<string> *returnvector = new vector<string>();
	
	vector<pair<string, int> > returnprotocols;
	if (protocol == ALL_PROTOCOLS) {
		returnprotocols = protocols;
	} else if (protocol == DEFAULT_PROTOCOL) {
		pair<string, int> itempair(defaultprotocol, defaultport);
		returnprotocols.push_back(itempair);
	} else {
		if (protocol != "https") {
			// if (check if the protocol is supported)
			// if protocol in protocols!
			int port = -1;
			for (unsigned int i = 0; i < protocols.size(); i++) {
				if (protocols[i].first == protocol) {
					port = protocols[i].second;
					break;
				}	
			}
			if (port == -1) {
				throw JobOperationException(__FILE__, __LINE__,
					"getJobDirectoryURIsVector()", WMS_INVALID_ARGUMENT,
					"requested protocol not available");
			}
			pair<string, int> itempair(protocol, port);
			returnprotocols.push_back(itempair);
		}
	}
	
	string item;
	unsigned int size = returnprotocols.size();
	for (unsigned int i = 0; i < size; i++) {
		item = returnprotocols[i].first
			+ "://" + serverhost
			+ ((returnprotocols[i].second == 0) 
				? "" 
				: (":" + boost::lexical_cast<string>
					(returnprotocols[i].second)))
			+ path;
		edglog(debug)<<"Job directory URI: "<<item<<endl;
		returnvector->push_back(item);
	}
	
	// Adding https protocol
	if ((protocol == ALL_PROTOCOLS) || (protocol == "https")) {
		if (httpsport) {
			item = "https://" + string(getenv("SERVER_ADDR")) + ":"
				+ boost::lexical_cast<string>(httpsport) + httppath;
		} else {
			item = "https://" + string(getenv("HTTP_HOST")) + httppath;
		}
		edglog(debug)<<"Job directory URI: "<<item<<endl;
		returnvector->push_back(item);
	}
	
	return returnvector;
	GLITE_STACK_CATCH();
}

vector<string> *
getDestURIsVector(vector<pair<string, int> > protocols, int httpsport,
	const string &jid, bool addhttps)
{
	GLITE_STACK_TRY("getDestURIsVector()");
	edglog_fn("wmpoperations::getDestURIsVector");
	
	edglog(debug)<<"Computing destination URIs for job: "<<jid<<endl;
	
	// Protocol + host:port + path
	string httppath = FILE_SEP + to_filename(jobid::JobId(jid), 0) + "/input";
	string path = getenv(DOCUMENT_ROOT) + httppath;
	string serverhost = getServerHost();
	
	vector<string> *returnvector = new vector<string>(0);
	string port;
	string item;
	
	for (unsigned int i = 0; i < protocols.size(); i++) {
		port = boost::lexical_cast<string>(protocols[i].second);
		item = protocols[i].first
			+ "://" + serverhost
			+ ((protocols[i].second == 0) ? "" : (":" + port))
			//+ ((protocols[i].first != "https") ? path : httppath);
			+ path;
		edglog(debug)<<"PROTOCOL: "<<protocols[i].first<<endl;
		edglog(debug)<<"Destination URI: "<<item<<endl;
		returnvector->push_back(item);
	}
	// Adding https protocol
	if (addhttps) {
		if (httpsport != 0) {
			item = "https://" + string(getenv("SERVER_ADDR")) + ":"
				+ boost::lexical_cast<string>(httpsport) + httppath;
		} else {
			item = "https://" + string(getenv("HTTP_HOST")) + httppath;
		}
	}
	edglog(debug)<<"Destination URI: "<<item<<endl;
	returnvector->push_back(item);

	return returnvector;
	GLITE_STACK_CATCH();
}

vector<string>
parseFQAN(const string &fqan)
{
	GLITE_STACK_TRY("parseFQAN()");
	vector<string> returnvector;
	boost::char_separator<char> separator("/");
	boost::tokenizer<boost::char_separator<char> >
    	tok(fqan, separator);
    for (boost::tokenizer<boost::char_separator<char> >::iterator
    		token = tok.begin(); token != tok.end(); token++) {
    	returnvector.push_back(*token);
	}
	return returnvector;
	GLITE_STACK_CATCH();
}


string
getEnvVO()
{
	GLITE_STACK_TRY("getEnvVO()");
	string vo = "";
	string cred2 = getenv("GRST_CRED_2") ? string(getenv("GRST_CRED_2")) : "";
	if (cred2 != "") {
		unsigned int pos = cred2.find("/", 0);
		if (pos != string::npos) {
			string fqan = cred2.substr(pos, cred2.size());
			vo = (parseFQAN(fqan))[0];
		}
	}
	return vo;
	GLITE_STACK_CATCH();
}

string
getEnvFQAN()
{
	GLITE_STACK_TRY("getEnvFQAN()");
	string fqan = "";
	//string cred2 = getenv("GRST_CRED_2") ? string(getenv("GRST_CRED_2")) : "";
	string cred2 = "";
	if (getenv("GRST_CRED_2")) {
		cred2 = string(getenv("GRST_CRED_2"));
	}
	if (cred2 != "") {
		unsigned int pos = cred2.find("/", 0);
		if (pos != string::npos) {
			fqan = cred2.substr(pos, cred2.size());
		}
	}
	return fqan;
	GLITE_STACK_CATCH();
}

void
parseAddressPort(const string &addressport, pair<string, int> &addresspair)
{
	GLITE_STACK_TRY("parseAddressPort()");
	unsigned int pos;
	if (addressport != "") {
		if ((pos = addressport.rfind(":", addressport.size()))
				!= string::npos) {
			addresspair.first = addressport.substr(0, pos);
			addresspair.second = 
				atoi(addressport.substr(pos + 1, addressport.size()).c_str());
		} else {
			addresspair.first = addressport;
			addresspair.second = 0;
		}
	} else {
        addresspair.first = "localhost";
        addresspair.second = 0;
    }
	GLITE_STACK_CATCH();
}

/* // TBR
 * ----- WARNING!! ----------------------------------------------------------
 * This method is a patch to grant correct behaviour for components using old
 * version of OpenSSL library.
 * 
 * it converts:
 * - emailAddress to Email
 * - UID          to USERID
 * 
 * N.B. To be removed when all components will use OpenSSL 0.9.7
 * --------------------------------------------------------------------------
 */
string
convertDNEMailAddress(const string & dn)
{
	GLITE_STACK_TRY("getEnvFQAN()");
	edglog_fn("wmputils::convertDNEMailAddress");
	
	edglog(debug)<<"Original DN: "<<dn<<endl;
	string newdn = dn;
    string toreplace = "emailAddress";
	unsigned int pos = dn.rfind(toreplace, dn.size());
	if (pos != string::npos) {
		newdn.replace(pos, toreplace.size(), "Email");
	}
    toreplace = "UID";
	pos = newdn.rfind(toreplace, newdn.size());
	if (pos != string::npos) {
		newdn.replace(pos, toreplace.size(), "USERID");
	}
	edglog(debug)<<"Converted DN: "<<newdn<<endl;
	return newdn;
	
	GLITE_STACK_CATCH();
}

int
fileExists(const string &path)
{
	GLITE_STACK_TRY("fileExists()");
	struct stat buffer;
	if (stat(path.c_str(), &buffer)) {
		return 0;
	}
	return 1;
	GLITE_STACK_CATCH();
}

string
getFileName(const string &path)
{
	GLITE_STACK_TRY("getFileName()");
	string filename = path;
	unsigned int pos = path.rfind("/");
	if (pos != string::npos) {
		filename = path.substr(pos + 1, string::npos);
	}
	return filename;
	GLITE_STACK_CATCH();
}

string
getDestURI(const string &jobid, const string &protocol,
	int port)
{
	GLITE_STACK_TRY("getDestURI()");
	string dest_uri(protocol + "://" + getServerHost()
		+ ((port == 0) ? "" : (":" + boost::lexical_cast<string>(port)))
		+ getenv(DOCUMENT_ROOT) + FILE_SEP
		+ to_filename(jobid));
	return dest_uri;
	GLITE_STACK_CATCH();
}

string
getJobReducedPath(jobid::JobId jid, int level)
{
	GLITE_STACK_TRY("getJobReducedPath()");
	return string(getenv(DOCUMENT_ROOT) + FILE_SEP
		+ glite::wmsutils::jobid::get_reduced_part(jid, level));
	GLITE_STACK_CATCH();
}

string
getJobDirectoryPath(jobid::JobId jid, int level)
{
	GLITE_STACK_TRY("getJobDirectoryPath()");
	return string(getenv(DOCUMENT_ROOT) + FILE_SEP
		+ to_filename(jid, level));
	GLITE_STACK_CATCH();
}

string
getJobInputSBRelativePath(glite::wmsutils::jobid::JobId jid, int level){
	GLITE_STACK_TRY("getJobInputRelativePath()");
	return (  to_filename(jid, level) + FILE_SEP
		+ INPUT_SB_DIRECTORY);
	GLITE_STACK_CATCH();
}

string
getJobStartLockFilePath(jobid::JobId jid, int level)
{
	GLITE_STACK_TRY("getJobStartLockFilePath()");
	return string(getenv(DOCUMENT_ROOT) + FILE_SEP
		+ to_filename(jid, level) + FILE_SEP + START_LOCK_FILE_NAME);
	GLITE_STACK_CATCH();
}


string
getGetOutputFileListLockFilePath(jobid::JobId jid, int level)
{
	GLITE_STACK_TRY("getGetOutputFileListLockFilePath()");
	return string(getenv(DOCUMENT_ROOT) + FILE_SEP
		+ to_filename(jid, level) + FILE_SEP + GET_OUTPUT_LOCK_FILE_NAME);
	GLITE_STACK_CATCH();
}

string
getInputSBDirectoryPath(jobid::JobId jid, int level)
{
	GLITE_STACK_TRY("getInputSBDirectoryPath()");
	return string(getenv(DOCUMENT_ROOT) + FILE_SEP
		+ to_filename(jid, level) + FILE_SEP + INPUT_SB_DIRECTORY);
	GLITE_STACK_CATCH();
}

string
getOutputSBDirectoryPath(jobid::JobId jid, int level)
{
	GLITE_STACK_TRY("getOutputSBDirectoryPath()");
	return string(getenv(DOCUMENT_ROOT) + FILE_SEP
		+ to_filename(jid, level) + FILE_SEP + OUTPUT_SB_DIRECTORY);
	GLITE_STACK_CATCH();
}

string
getPeekDirectoryPath(jobid::JobId jid, int level, bool docroot)
{
	GLITE_STACK_TRY("getPeekDirectoryPath()");
	string path;
	if (docroot) {
		path = string(getenv(DOCUMENT_ROOT) + FILE_SEP + to_filename(jid, level)
			+ FILE_SEP + PEEK_DIRECTORY);
	} else {
		path = string(FILE_SEP + to_filename(jid, level) + FILE_SEP
			+ PEEK_DIRECTORY);
	}
	return path;
	GLITE_STACK_CATCH();
}

string
getJobDelegatedProxyPath(jobid::JobId jid, int level)
{	
	GLITE_STACK_TRY("getJobDelegatedProxyPath(JobId jid)");
	//TBD Check path
	return string(getenv(DOCUMENT_ROOT) 
		+ FILE_SEP + to_filename(jid, level)
		+ FILE_SEP + USER_PROXY_NAME);
	GLITE_STACK_CATCH();
}

string
getJobDelegatedProxyPathBak(jobid::JobId jid, int level)
{	
	GLITE_STACK_TRY("getJobDelegatedProxyPath(JobId jid)");
	//TBD Check path
	return string(getenv(DOCUMENT_ROOT) 
		+ FILE_SEP + to_filename(jid, level)
		+ FILE_SEP + USER_PROXY_NAME_BAK);
	GLITE_STACK_CATCH();
}

string
getJobJDLOriginalPath(jobid::JobId jid, bool isrelative, int level)
{	
	GLITE_STACK_TRY("getJobJDLOriginalPath(JobId jid)");
	//TBD Check path
	if (!isrelative) {
		return string(getenv(DOCUMENT_ROOT) 
			+ FILE_SEP + to_filename(jid, level)
			+ FILE_SEP + JDL_ORIGINAL_FILE_NAME);
	} else {
		return string(to_filename(jid, level)
			+ FILE_SEP + JDL_ORIGINAL_FILE_NAME);
	}
	GLITE_STACK_CATCH();
}

string
getJobJDLToStartPath(jobid::JobId jid, bool isrelative, int level)
{	
	GLITE_STACK_TRY("getJobJDLToStartPath(JobId jid)");
	//TBD Check path
	if (!isrelative) {
		return string(getenv(DOCUMENT_ROOT) 
			+ FILE_SEP + to_filename(jid, level)
			+ FILE_SEP + JDL_TO_START_FILE_NAME);
	} else {
		return string(to_filename(jid, level)
			+ FILE_SEP + JDL_TO_START_FILE_NAME);
	}
	GLITE_STACK_CATCH();
}

string
getJobJDLStartedPath(jobid::JobId jid, bool isrelative, int level)
{	
	GLITE_STACK_TRY("getJobJDLStartedPath()");
	//TBD Check path
	if (!isrelative) {
		return string(getenv(DOCUMENT_ROOT) 
			+ FILE_SEP + to_filename(jid, level)
			+ FILE_SEP + JDL_STARTED_FILE_NAME);
	} else {
		return string(to_filename(jid, level)
			+ FILE_SEP + JDL_STARTED_FILE_NAME);
	}
	GLITE_STACK_CATCH();
}

string
getJobJDLExistingStartPath(jobid::JobId jid, bool isrelative, int level)
{
	GLITE_STACK_TRY("getJobJDLStartedPath()");
	
	string started = getJobJDLStartedPath(jid);
	if (fileExists(started)) {
		return started;
	} else {
		return getJobJDLToStartPath(jid);
	}
	GLITE_STACK_CATCH();
}

string
getEndpoint() {
	GLITE_STACK_TRY("getEndpoint()");
	return ((string(getenv("HTTPS")) == "on") ? "https://" : "http://")
		+ string(getenv("SERVER_ADDR")) + ":"
		+ string(getenv("SERVER_PORT"))
		+ string(getenv("SCRIPT_NAME"));
	GLITE_STACK_CATCH();
}

string
getServerHost() {
	GLITE_STACK_TRY("getServerHost()");
	edglog_fn("wmputils::getServerHost");
	
    struct hostent *server = NULL;
    char * servername = getenv("SERVER_NAME");
    if (servername) {
	    edglog(debug)<<"SERVER_NAME: "<<string(servername)<<endl;
		if ((server = gethostbyname(servername)) == NULL) {
			edglog(critical)<<"Unable to get server address"<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"getServerHost()", WMS_PROXY_ERROR,
				"Unable to get server address");
		}
    } else {
    	edglog(critical)<<"Environment variable SERVER_NAME null\n(please "
    		"contact server administrator)";
    	throw FileSystemException(__FILE__, __LINE__,
			"getServerHost()", WMS_PROXY_ERROR,
			"Environment variable SERVER_NAME null\n(please contact server "
				"administrator)");
    }
    return string(server->h_name);
    
	GLITE_STACK_CATCH();
}

bool
doPurge(string dg_jobid)
{
	GLITE_STACK_TRY("doPurge()");
	edglog_fn("wmputils::doPurge");
	
	if (dg_jobid.length()) {
  		edglog(debug)<<"JobId object for purging created: "
  			<<dg_jobid<<endl;
  		boost::filesystem::path path(getJobDirectoryPath(jobid::JobId(dg_jobid)),
  			boost::filesystem::native);
  		return purger::purgeStorageEx(path, edg_wll_LogClearUSER);
    } else {
   		edglog(critical)
			<<"Error in Purging: Invalid Job Id. Purge not done."<<endl;
      return false; 
    }
    
    GLITE_STACK_CATCH();
}

bool
getUserQuota(pair<long, long>& result, string uname)
{
	GLITE_STACK_TRY("getUserQuota()");
	result = commonutilities::quota::getQuota(uname);
	return true;
	GLITE_STACK_CATCH();
}

bool
getUserFreeQuota(pair<long, long>& result, string uname)
{
	GLITE_STACK_TRY("getUserFreeQuota()");
	result = commonutilities::quota::getFreeQuota(uname);
	return true;
	GLITE_STACK_CATCH();
}

char *
getUserDN()
{
	GLITE_STACK_TRY("getUserDN()");
	edglog_fn("wmputils::getUserDN");
	edglog(debug)<<"Getting user DN..."<<endl;
	char* p = NULL;
	char* client_dn = NULL;
	char* user_dn = NULL;
	
	client_dn = getenv(SSL_CLIENT_DN);
	if ((client_dn == NULL) || (client_dn == '\0')) {
		edglog(debug)<<"Environment variable "<<string(SSL_CLIENT_DN)
			<<" not correctly defined"<<endl;
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
		edglog(debug)<<"Unable to get a valid user DN"<<endl;
		throw ProxyOperationException(__FILE__, __LINE__,
			"getUserDN()", WMS_PROXY_ERROR, "Unable to get a valid user DN");
	}
	edglog(debug)<<"User DN: "<<user_dn<<endl;
	return user_dn;
	GLITE_STACK_CATCH();
}

void
waitForSeconds(int seconds)
{
	GLITE_STACK_TRY("waitForSeconds()");
	edglog_fn("wmputils::waitForSeconds");
	edglog(debug)<<"Waiting for "<<seconds<<" seconds..."<<endl;
	time_t startTime = time(NULL);
	time_t endTime = time(NULL);
	int counter = 0;
	while((endTime - startTime) < seconds) {
		if ((endTime%3600) != counter) {
			switch (counter%4) {
				case 0:
					edglog(debug)<<"-"<<endl;
					break;
				case 1:
					edglog(debug)<<"\\"<<endl;
					break;
				case 2:
					edglog(debug)<<"|"<<endl;
					break;
				case 3:
					edglog(debug)<<"/"<<endl;
					break;
				default:
					break;
			}
			counter = endTime%3600;
		}
		endTime = time(NULL);
	}
	edglog(debug)<<"End waiting"<<endl;
	GLITE_STACK_CATCH();
}

int
generateRandomNumber(int lowerlimit, int upperlimit)
{
	GLITE_STACK_TRY("generateRandomNumber()");
    edglog_fn("wmputils::generateRandomNumber");
    
    edglog(debug)<<"Generating random between "<<lowerlimit<<" - "
    	<<upperlimit<<endl;
    	
    // Setting seeed
	srand((unsigned) time(0));
	return lowerlimit + static_cast<int>(rand()%(upperlimit - lowerlimit + 1));
	
	GLITE_STACK_CATCH();
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files.
  *//*
int gzUncompress (const string &source, const string &dest)
{
	GLITE_STACK_TRY("gzUncompress()");
    edglog_fn("wmputils::gzUncompress");
    
	FILE *out = NULL;
	string zmsg = "";
	string errmsg = "";
    gzFile in;
	int len = 0;
	int err = 0;
	long size = computeFileSize(source);
	local char buf[size];
	in = gzopen(source.c_str(), "rb");
        if (in == NULL) {
		errmsg = "Unable to uncompress the ISB file: " + source + "\n";
		errmsg += "(error while opening the file)\n";
		errmsg += "please, contact the server administrator";
                edglog(severe)<<errmsg<<endl;
                throw FileSystemException(__FILE__, __LINE__,
                                "uncompressFile()", WMS_IS_FAILURE,
				errmsg);
        }
        out = fopen(dest.c_str(), "wb");
        if (out == NULL) {
		errmsg = "error while uncompressing the ISB file: " + source + "\n";
                errmsg += "(unable to create the uncompressed file: " + dest + ")\n";
                errmsg += "please, contact the server administrator";
                edglog(severe)<<errmsg<<endl;
                throw FileSystemException(__FILE__, __LINE__,
                                "gzUncompress()", WMS_IS_FAILURE,
				errmsg);
        }
	for (;;) {
		len = gzread(in, buf, sizeof(buf));
		if (len < 0)  {
			errmsg = "error while uncompressing the zipped file: " + source + "\n";
			zmsg = gzerror(in, &err);
			if (zmsg.size()>0){ errmsg += "(" + zmsg + ")\n";}
			errmsg += "please, contact the server administrator";
			edglog(severe)<<errmsg<<endl;
			throw FileSystemException(__FILE__, __LINE__,
                                "uncompressFile()", WMS_IS_FAILURE,
                                errmsg );
		} else if (len == 0) { break;  }
		if ((int)fwrite(buf, 1, (unsigned)len, out) != len) {
			errmsg = "unable to uncompress the zipped file: " + source + "\n";
			errmsg += "(error while writing the file: "  + dest + ")\n"; 
			errmsg += "please, contact the server administrator";
			edglog(severe)<<errmsg<<endl;
			throw FileSystemException(__FILE__, __LINE__,
                                "gzUncompress()", WMS_IS_FAILURE,
                                errmsg );
		}
	}
	if (fclose(out)!=0) {
		errmsg = "unable to uncompress the zipped file: " + source + "\n";
                errmsg += "(error while closing the file: "  + dest + ")\n";
                errmsg += "please, contact the server administrator";
		edglog(severe)<<errmsg<<endl;
                throw FileSystemException(__FILE__, __LINE__,
                                "gzUncompress()", WMS_IS_FAILURE,
                                errmsg );
        }
	if (gzclose(in) != Z_OK) {
		edglog(warning)<<"Unable to remove the gz file "
		"(reason: error in closing the file): "<<string(source)<<endl;
	} else {
		remove(source.c_str());
	} 
        return Z_OK;
	GLITE_STACK_CATCH();
}


string gzError(int ret)
{
	string err ="";
	switch (ret) {
	case Z_ERRNO:
		err = "i/o error";
		break;
	case Z_STREAM_ERROR:
		err = "invalid compression level";
		break;
	case Z_DATA_ERROR:
		err = "invalid or incomplete deflate data";
		break;
	case Z_MEM_ERROR:
		err = "out of memory\n";
		break;
	case Z_VERSION_ERROR:
		err = "zlib version mismatch";
		break;
	}
	return err;
}

void
chmod_tar_extract_all(TAR *t, char *prefix)
{
	GLITE_STACK_TRY("chmod_tar_extract_all()");
	edglog_fn("wmputils::chmod_tar_extract_all");

	char *filename;
	char buf[MAXPATHLEN];
	int i;

	mode_t mode = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
	while ((i = th_read(t)) == 0) {
		filename = th_get_pathname(t);
		if (TH_ISREG(t)) {
			if (t->options & TAR_VERBOSE) {
				th_print_long_ls(t);
			}
			if (prefix != NULL) {
				snprintf(buf, sizeof(buf), "%s/%s", prefix, filename);
			} else {
				strncpy(buf, filename, sizeof(buf));
			}
			if (fileExists(string(buf))) {
				edglog(critical)<<"Already existing file in ISB file: "
					<<string(buf)<<endl;
				throw FileSystemException(__FILE__, __LINE__,
					"chmod_tar_extract_all()", WMS_IS_FAILURE,
					"Already existing file in ISB file");
				///
				//TBD Log ABORT after this error
				///
			}
			edglog(debug)<<"Extracting file: "<<string(buf)<<endl;
			if (tar_extract_file(t, buf)) {
				edglog(error)<<"Unable to uncompress ISB file: "
					<<string(buf)<<endl;
				throw FileSystemException(__FILE__, __LINE__,
					"chmod_tar_extract_all()", WMS_IS_FAILURE,
					"Unable to uncompress ISB file: " + string(buf));
			}
	
			int outcome = chmod(string(buf).c_str(), mode);
			edglog(debug)<<"chmod result: "<<outcome<<endl;
			if (outcome) {
				throw FileSystemException(__FILE__, __LINE__,
					"chmod_tar_extract_all()", WMS_IS_FAILURE,
					"Unable to change mode for ISB file: " + string(buf));
			}
		} else {
			edglog(debug)<<"Item in ISB file is not a regular file: "
				<<filename<<endl;
			edglog(debug)<<"Skipping..."<<endl;
		}
	}

    if (i != 1) {
        throw FileSystemException(__FILE__, __LINE__,
            "chmod_tar_extract_all()", WMS_IS_FAILURE,
            "Unable to uncompress ISB archive");
    }

    GLITE_STACK_CATCH();
}

void
uncompressFile(const string &filename, const string &startingpath)
{
	GLITE_STACK_TRY("uncompressFile()");
	edglog_fn("wmputils::uncompressFile");
	
	edglog(debug)<<"Uncompressing file: "<<filename<<endl;
	edglog(debug)<<"Starting path: "<<startingpath<<endl;
	char * file = const_cast<char*>(filename.c_str());
	char * prefix = const_cast<char*>(startingpath.c_str());
	
	char buf[2048];
	char * infile = NULL;
	char * outfile = NULL;

	uInt len = (uInt)strlen(file);
	strcpy(buf, file);

	if (len > 3 && (strcmp(file + len - 3, ".gz") == 0)) {
		infile = file;
		outfile = buf;
		outfile[len - 3] = '\0';
	} else {
		outfile = file;
		infile = buf;
		strcat(infile, ".gz");
	}
	edglog(debug)<<"Input file: "<<infile<<endl;
	edglog(debug)<<"Output file: "<<outfile<<endl;

	int result = gzUncompress(infile, outfile);
	edglog(debug)<<"unzip result: "<<result<<endl;
	if (result != Z_OK)  {
		throw FileSystemException(__FILE__, __LINE__,
			"uncompressFile()", WMS_IS_FAILURE,
			"Unable to uncompress ISB file: " + gzError(result));
	}

	TAR * tarfile = NULL;
	tar_open(&tarfile, outfile, NULL, O_RDONLY, S_IRWXU, TAR_GNU);
	chmod_tar_extract_all(tarfile, prefix);
	tar_close(tarfile);

	remove(outfile);

	GLITE_STACK_CATCH();
}*/

void 
fileCopy(const string& source, const string& target)
{
	GLITE_STACK_TRY("fileCopy()");
 	edglog_fn("wmputils::fileCopy");
  	edglog(debug)<<"Copying file...\n\tSource: "
  		<<source<<"\n\tTarget: "<<target<<endl;
  
  	ifstream in(source.c_str());
  	if (!in.good()) {
  		edglog(severe)<<"Copy failed, !in.good(). \n\tSource: "
  			<<source<<" Target: "<<target<<endl;
    	throw FileSystemException(__FILE__, __LINE__,
			"fileCopy(const string& source, const string& target)",
			WMS_IS_FAILURE, "Unable to copy file");
	 }
	 ofstream out(target.c_str());
	 if (!out.good()) {
  		edglog(severe)<<"Copy failed, !out.good(). \n\tSource: "
  			<<source<<"\n\tTarget: "<<target<<endl;
    	throw FileSystemException(__FILE__, __LINE__,
			"fileCopy(const string& source, const string& target)",
			WMS_IS_FAILURE, "Unable to copy file");
  	}
  	out<<in.rdbuf(); // read original file into target
  
  	struct stat from_stat;
  	if (stat(source.c_str(), &from_stat) ||
	    	chown(target.c_str(), from_stat.st_uid, from_stat.st_gid) ||
	        chmod(target.c_str(), from_stat.st_mode)) {
		edglog(severe)<<"Copy failed, chown/chmod. \n\tSource: "
			<<source<<"\n\tTarget: "<<target<<endl;

	    throw FileSystemException(__FILE__, __LINE__,
			"fileCopy(const string& source, const string& target)",
			WMS_IS_FAILURE, "Unable to copy file");
  	}
  	edglog(debug)<<"Copy done."<<endl;
  	GLITE_STACK_CATCH();
}

string
to_filename(glite::wmsutils::jobid::JobId j, int level, bool extended_path)
{
	GLITE_STACK_TRY("to_filename()");
	string path(sandboxdir_global + string(FILE_SEP)
		+ glite::wmsutils::jobid::get_reduced_part(j, level));
	if (extended_path) {
		path.append(string(FILE_SEP) + glite::wmsutils::jobid::to_filename(j));
	}
	return path;
	GLITE_STACK_CATCH();
}

long
computeFileSize(const string & path)
{
	GLITE_STACK_TRY("computeFileSize()");
	int fd = -1;
	long size = 0;
   	fd = open(path.c_str(), O_RDONLY);
    if (fd != -1) {
    	struct stat buf;
       	if (!fstat(fd, &buf)) {
        	size = buf.st_size;
       	}
       	close(fd);
	}
	// If file not found it returns 0
    return size;
    GLITE_STACK_CATCH();
}

string
searchForDirmanager()
{
	GLITE_STACK_TRY("searchForDirmanager()");
	
	// Try to find managedirexecutable 
   	char * glite_path = getenv(GLITE_WMS_LOCATION); 
   	if (!glite_path) {
   		glite_path = getenv(GLITE_LOCATION);
   	}
   	string gliteDirmanExe = (glite_path == NULL)
   		? ("/opt/glite")
   		:(string(glite_path)); 
   		
   	gliteDirmanExe += "/bin/glite_wms_wmproxy_dirmanager";
   	
   	return gliteDirmanExe;
   	
   	GLITE_STACK_CATCH();
}
	

int
doExecv(const string &command, vector<string> &params, const vector<string> &dirs,
	unsigned int startIndex, unsigned int endIndex)
{
	GLITE_STACK_TRY("doExecv()");
	edglog_fn("wmputils::doExecv");
	
	char **argvs;
	// +3 -> difference between index, command at first position, NULL at the end
	int size = params.size() + endIndex - startIndex + 3;
	argvs = (char **) calloc(size, sizeof(char *));
	
	unsigned int i = 0;
	
	argvs[i] = (char *) malloc(command.length() + 1);
	strcpy(argvs[i++], (command).c_str());
	
	vector<string>::iterator iter = params.begin();
	vector<string>::iterator const end = params.end();
	for (; iter != end; ++iter) {
		argvs[i] = (char *) malloc((*iter).length() + 1);
		strcpy(argvs[i++], (*iter).c_str());
	}
	for (unsigned int j = startIndex; j <= endIndex; j++) {
		argvs[i] = (char *) malloc(dirs[j].length() + 1);
		strcpy(argvs[i++], (dirs[j]).c_str());
	}
	argvs[i] = (char *) 0;
	
	edglog(debug)<<"Forking process..."<<endl;
	switch (fork()) {
		case -1:
			// Unable to fork
			edglog(critical)<<"Unable to fork process"<<endl;
			return FAILURE;
			break;
		case 0:
			// child
	        if (execv(command.c_str(), argvs)) {
	        	unsigned int middle;
	        	switch (errno) {
		        	case E2BIG:
	        			edglog(debug)<<"Command line too long, splitting..."<<endl;
	        			middle = startIndex + (endIndex - startIndex) / 2;
	                    edglog(debug)<<"Calling from index "<<startIndex
	                    	<<" to "<<middle<<endl;
	        			if (doExecv(command, params, dirs, startIndex, middle)) {
	        				return FAILURE;	
	        			}
	        			edglog(debug)<<"Calling from index "<<middle + 1
	                    	<<" to "<<endIndex<<endl; 
	        			if (doExecv(command, params, dirs, middle + 1, endIndex)) {
	        				return FAILURE;
	        			}
	        			break;
	        			
	        		case EACCES:
	        			edglog(severe)<<"Command not executable"<<endl;
	        		case EPERM:
	        			edglog(severe)<<"Wrong execution permissions"<<endl;
	        		case ENOENT:
	        			edglog(severe)<<"Unable to find command"<<endl;
	        		case ENOMEM:
	        			edglog(severe)<<"Insufficient memory to execute command"
	        				<<endl;
	        		case EIO:
	        			edglog(severe)<<"I/O error"<<endl;
	        		case ENFILE:
	        			edglog(severe)<<"Too many opened files"<<endl;
	        			
		        	default:
		        		edglog(severe)<<"Unable to execute command"<<endl;
		        		return FAILURE;
						break;
	        	}
	        } else {
	        	edglog(debug)<<"execv succesfully"<<endl;
	        }
	        break;
        default:
        	// parent
	    	int status = SUCCESS;
	    	wait(&status);
	    	if (WIFEXITED(status)) {
                edglog(debug)<<"Child wait succesfully (WIFEXITED(status))"<<endl;
                edglog(debug)<<"WEXITSTATUS(status): "<<WEXITSTATUS(status)<<endl;
            }
            if (WIFSIGNALED(status)) {
                edglog(severe)<<"WIFSIGNALED(status)"<<endl;
                edglog(severe)<<"WEXITSTATUS(status): "<<WTERMSIG(status)<<endl;
            }
            
#ifdef WCOREDUMP
			if (WCOREDUMP(status)) {
				edglog(critical)<<"Child dumped core!!!"<<endl;
			}
#endif // WCOREDUMP

	    	if (status) {
	    		edglog(severe)<<"Child failure, exit code: "<<status<<endl;
	    		return FAILURE;
	    	}
	    	break;
	}
	for (unsigned int j = 0; j <= i; j++) {
		free(argvs[j]);
	}
    free(argvs);
    
    return SUCCESS;
    
    GLITE_STACK_CATCH();
}

void
untarFile(const string &file, const string &untar_starting_path,
	uid_t userid, uid_t groupid)
{
	GLITE_STACK_TRY("untarFile()");
	edglog_fn("wmputils::untarFile");
	
	if (fileExists(file)) {
		string gliteDirmanExe = searchForDirmanager();
	   	
	   	// Creating parameters vector for zip file extraction
		vector<string> extparams;
		extparams.push_back("-c");
		extparams.push_back(boost::lexical_cast<string>(userid));
		extparams.push_back("-g");
		extparams.push_back(boost::lexical_cast<string>(groupid));
		extparams.push_back("-m");
		extparams.push_back("0770");
		extparams.push_back("-x");
		extparams.push_back(untar_starting_path);
		
		vector<string> extfiles;
		extfiles.push_back(file);
		
		// Extracting files
		if (doExecv(gliteDirmanExe, extparams, extfiles, 0, extfiles.size() - 1)) {
			edglog(critical)<<"Unable to untar ISB file:"<<file<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"untarFile()", WMS_FILE_SYSTEM_ERROR, "Unable to untar ISB file"
				"\n(please contact server administrator)");
		}
		
	} else {
		edglog(critical)<<"Unable to untar ISB file, file does not exist: "
			<<file<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"untarFile()", WMS_FILE_SYSTEM_ERROR,
			"Unable to untar ISB file\n(please contact server administrator)");
	}
	
	GLITE_STACK_CATCH();
}

void 
managedir(const string &document_root, uid_t userid, uid_t jobdiruserid,
	vector<string> jobids)
{
	GLITE_STACK_TRY("managedir()");
	edglog_fn("wmputils::managedir");
	
	time_t starttime = time(NULL);
	
	unsigned int size = jobids.size();
	edglog(info)<<"Job id vector size: "<<size<<endl;
	
	if (size) {
	   	string gliteDirmanExe = searchForDirmanager();
	   	
	   	string useridtxt = boost::lexical_cast<string>(userid);
	   	string grouptxt = boost::lexical_cast<string>(getgid());
	   	
		int level = 0; 
	   	bool extended_path = true; 
		// Vector contains at least one element
		string path = to_filename (glite::wmsutils::jobid::JobId(jobids[0]),
		   	level, extended_path);
		int pos = path.find(FILE_SEP, 0);
		
		// Creating SanboxDir directory if needed
		string sandboxdir = document_root + FILE_SEP + path.substr(0, pos)
			+ FILE_SEP;
		if (!fileExists(sandboxdir)) {
			string run = gliteDirmanExe
				+ " -c " + useridtxt
				+ " -g " + grouptxt
				+ " -m 0773 " + sandboxdir;
		   	edglog(debug)<<"Creating SandboxDir..."<<endl;
			edglog(debug)<<"Executing: \n\t"<<run<<endl;
			if (system(run.c_str())) {
				edglog(fatal)<<"Unable to create sandbox directory"<<endl;
		   		throw FileSystemException(__FILE__, __LINE__,
					"managedir()", WMS_FILE_SYSTEM_ERROR,
					"Unable to create sandbox directory\n"
					"(please contact server administrator)");
		   	}
		}
		
		// Creating parameters vector for reduced directories creation
		vector<string> redparams;
		redparams.push_back("-c");
		string juser = useridtxt;
		redparams.push_back(juser);
		redparams.push_back("-g");
		string group = grouptxt;
		redparams.push_back(group);
		redparams.push_back("-m");
		redparams.push_back("0773");
		
		// Creating parameters vector for job directories creation
		vector<string> jobparams;
		jobparams.push_back("-c");
		juser = boost::lexical_cast<string>(jobdiruserid);
		jobparams.push_back(juser);
		jobparams.push_back("-g");
		group = grouptxt;
		jobparams.push_back(group);
		jobparams.push_back("-m");
		jobparams.push_back("0770");
	
		vector<string> reddirs;
		vector<string> jobdirs;
	   	string allpath;
		string reduceddir;
		
		// Populating directories to create vector
		vector<string>::iterator iter = jobids.begin();
		vector<string>::iterator const end = jobids.end();
		for (; iter != end; ++iter) {
		   	path = to_filename(glite::wmsutils::jobid::JobId(*iter),
		   		level, extended_path);
		   	allpath = path;
		   	pos = path.find(FILE_SEP, 0);
		   	sandboxdir = path.substr(0, pos);
		   	path.erase(0, pos);
		   	reduceddir = path.substr(1, path.find(FILE_SEP, 1));
		   	
		   	// Reduced directories
		   	reddirs.push_back(document_root + FILE_SEP + sandboxdir
		   			+ FILE_SEP + reduceddir);
		   	
		   	// Job directories
		   	path = document_root + FILE_SEP + allpath;
		    jobdirs.push_back(path);
		    jobdirs.push_back(path + FILE_SEP + INPUT_SB_DIRECTORY);
		    jobdirs.push_back(path + FILE_SEP + OUTPUT_SB_DIRECTORY);
		    jobdirs.push_back(path + FILE_SEP + PEEK_DIRECTORY);
		}
		
		// Creating reduced directories
	   	if (doExecv(gliteDirmanExe, redparams, reddirs, 0, reddirs.size() - 1)) {
	   		edglog(fatal)<<"Unable to create job local directory (reduced)"<<endl;
	   		throw FileSystemException(__FILE__, __LINE__,
				"managedir()", WMS_FILE_SYSTEM_ERROR,
				"Unable to create job local directory\n"
				"(please contact server administrator)");	
	   	}
	   	
	   	// Creating job directories
	   	if (doExecv(gliteDirmanExe, jobparams, jobdirs, 0, jobdirs.size() - 1)) {
	   		edglog(fatal)<<"Unable to create job local directory (job)"<<endl;
	   		throw FileSystemException(__FILE__, __LINE__,
				"managedir()", WMS_FILE_SYSTEM_ERROR,
				"Unable to create job local directory\n"
				"(please contact server administrator)");	
	   	}
	}
	time_t stoptime = time(NULL);
	edglog(debug)<<"Directory creation elapsed time: "
		<<(stoptime - starttime)<<endl;
	
    GLITE_STACK_CATCH();
}

void
setFlagFile(const string &file, bool flag)
{
	GLITE_STACK_TRY("setFlagFile()");
	edglog_fn("wmputils::setFlagFile");
	
	if (flag) {
		fstream outfile(file.c_str(), ios::out);
		if (!outfile.good()) {
			edglog(severe)<<file<<": !outfile.good()"<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"setFlagFile()", WMS_IS_FAILURE, "Unable to set flag file"
				"\n(please contact server administrator)");
		}
		outfile<<"flag";
		outfile.close();
	} else {
		remove(file.c_str());	
	}
	GLITE_STACK_CATCH();
}

int
operationLock(const string &lockfile, const string &opname)
{
	GLITE_STACK_TRY("operationLock()");
	edglog_fn("wmputils::operationLock");
	
	edglog(debug)<<"Opening lock file: "<<lockfile<<endl;
	int fd = open(lockfile.c_str(), O_CREAT | O_RDONLY, S_IRWXU);
	if (fd == -1) {
		edglog(debug)<<"Unable to open lock file: "<<lockfile<<endl;
		throw FileSystemException( __FILE__, __LINE__,
  			"operationLock()", WMS_FILE_SYSTEM_ERROR,
   			"unable to open lock file");
	}
	if (flock(fd, LOCK_EX | LOCK_NB)) {
		close(fd);
		throw JobOperationException( __FILE__, __LINE__,
  			"operationLock()", WMS_OPERATION_NOT_ALLOWED,
   			opname + " operation already in progress");
	}
	
	return fd;
			
	GLITE_STACK_CATCH();
}

void
operationUnlock(int fd)
{
	GLITE_STACK_TRY("operationUnlock()");
	edglog_fn("wmputils::operationUnlock");
	
	if (flock(fd, LOCK_UN)) {
		edglog(severe)<<"Unable to remove lock file, fd: "<<fd<<endl;
	}
	close(fd);
	GLITE_STACK_CATCH();
}

bool
isOperationLocked(const string &lockfile)
{
	GLITE_STACK_TRY("isOperationLocked()");
	edglog_fn("wmputils::isOperationLocked");
	
	edglog(debug)<<"Opening lock file: "<<lockfile<<endl;
	int fd = open(lockfile.c_str(), O_CREAT | O_RDONLY, S_IRWXU);
	if (fd == -1) {
		edglog(debug)<<"Unable to open lock file: "<<lockfile
			<<" during lock check"<<endl;
		// TBC
		return false;
	}
	
	if (flock(fd, LOCK_EX | LOCK_NB)) {
		// Unable to lock file, already locked
		//if (errno == EWOULDBLOCK) {
			close(fd);
			return true;
		//}
	}
	if (flock(fd, LOCK_UN)) {
		edglog(severe)<<"Unable to remove lock file during lock check, fd: "
			<<fd<<endl;
	}
	close(fd);
	return false;
	
	GLITE_STACK_CATCH();
}

void
createSuidDirectory(const string &directory)
{
	GLITE_STACK_TRY("createSuidDirectory()");
	edglog_fn("wmputils::createSuidDirectory");
	
	if (!fileExists(directory)) {
		string gliteDirmanExe = searchForDirmanager();
	   	
		string dirpermissions = " -m 0773 ";
		string user = " -c " + boost::lexical_cast<string>(getuid()); // UID
		string group = " -g " + boost::lexical_cast<string>(getgid()); // GROUP
		
		string command = gliteDirmanExe + user + group + dirpermissions + directory;
		edglog(debug)<<"Excecuting command: "<<command<<endl;
		if (system(command.c_str())) {
			edglog(critical)<<"Unable to create directory: "<<directory<<endl;
		   	throw FileSystemException(__FILE__, __LINE__,
				"createDirectory()", WMS_FILE_SYSTEM_ERROR,
				"Unable to create directory\n(please contact server "
					"administrator)");
		}
	}
	
	GLITE_STACK_CATCH();
}

void
writeTextFile(const string &file, const string &text)
{
	GLITE_STACK_TRY("writeTextFile()");
	edglog_fn("wmputils::writeTextFile");
	
	fstream outfile(file.c_str(), ios::out);
	if (!outfile.good()) {
		edglog(severe)<<file<<": !outfile.good()"<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"writeTextFile()", WMS_IS_FAILURE, "Unable to write file: "
			+ file + "\n(please contact server administrator)");
	}
	outfile<<text;
	outfile.close();
	
	GLITE_STACK_CATCH();
}

string
readTextFile(const string &file)
{
	GLITE_STACK_TRY("readTextFile()");
	edglog_fn("wmputils::readTextFile");
	
	ifstream in(file.c_str(), ios::in);
	if (!in.good()) {
		edglog(debug)<<file<<": !in.good()"<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"readTextFile()", WMS_IS_FAILURE, "Unable to read file: "
			+ file + "\n(please contact server administrator)");
		exit(1);	
	}
	string line;
	string text = "";
	while (getline(in, line, '\n')) {
		text += line + "\n";
	}
	in.close();
	return text;
	
	GLITE_STACK_CATCH();
}

bool 
isNull(string field)
{
	GLITE_STACK_TRY("isNull()");
	bool is_null = false ;
	int p1 = field.size() - 5 ;
	int p2 = field.find("=NULL");

	if ((p1 > 0) && (p1 == p2)) {
		is_null = true;
	}
	return is_null;
	GLITE_STACK_CATCH();
}
#endif // #ifndef GLITE_WMS_WMPROXY_TOOLS

/*
* Removes white spaces form the begininng and from the end of the input string
*/
const string 
cleanString(string str)
{
	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_TRY("cleanString()");
	#endif
	int len = 0;
	string ws = " "; //white space char
	len = str.size( );
	if (len > 0) {
		// erases white space at the beginning of the string
		while (len>1) {
			if ( str.compare(0,1,ws) == 0) {
				str = str.substr(1, len);
			} else {
				break;
			}
			len = str.size();
		}
		// erases white space at the end of the string
		while (len>1) {
			if( str.compare(len-1,1,ws) == 0 ) {
				str = str.substr(0, len-1);
			} else {
				break;
			}
			len = str.size();
		}
		// 1 white space
		if (len == 1 & str.compare(ws)==0) {
			str = "";
		}
	}
	return str;
	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_CATCH();
	#endif
}

 /**
* Converts all of the characters in this String to lower case
 */
const string 
toLower(const string &src)
{
	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_TRY("toLower()");
	#endif
	string result(src);
	transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_CATCH();
	#endif
 }
 
 /**
* Cuts the input string in two pieces (label and value) according to
 * the separator character "="
 */
void 
split(const string &field, string &label, string &value)
{
	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_TRY("split()");
	#endif
	unsigned int size = field.size();
	if (size > 0) {
		unsigned int p = field.find("=") ;
		if (p != string::npos & (p < size)) {
			label = field.substr(0, p);
			value = field.substr(p+1, size-(p+1));
			// removes white spaces at the beginning and a the end of the 
			// strings (if present) and converts the uppercase letters to the 
			// corresponding lowercase
			label = toLower(cleanString(label));
			value = toLower(cleanString(value));
		}
	}
	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_CATCH();
	#endif
};


bool 
hasElement(const std::vector<std::string> &vect, const std::string &elem)
{
	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_TRY("hasElement()");
	#endif
	bool result = false;
	int size = vect.size();
	for (int i=0; i < size; i++){
		if (elem.compare(vect[i]) == 0) {
			result = true;
			break;
		}
	}
	return result;
	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_CATCH();
	#endif
};

/**
 * Removes '/' characters at the end of the of the input pathname
 */
 const std::string
 normalizePath( const std::string &fpath )
 {
 	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_TRY("normalizePath()");
	#endif
	string                   modified;
	string::const_iterator   last, next;
	string::reverse_iterator check;

	last = fpath.begin();
	do {
	next = find( last, fpath.end(), '/' );

	if( next != fpath.end() ) {
	modified.append( last, next + 1 );

	for( last = next; *last == '/'; ++last );
	}
	else modified.append( last, fpath.end() );
	} while( next != fpath.end() );

	check = modified.rbegin();
	if( *check == '/' ) modified.assign( modified.begin(), modified.end() - 1 );

	return modified;
	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_CATCH();
	#endif
}

/*
* Gets the absolute path of the file
*/
const std::string
getAbsolutePath(const string &file)
{
	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_TRY("getAbsolutePath()");
	#endif
	string path = file;
	char* pwd = getenv ("PWD");
	if (path.find("./")==0 || path.compare(".")==0){
		// PWD path  (./)
		if (pwd) {
			string leaf = path.substr(1,string::npos);
			if (leaf.size()>0) {
				if ( leaf.find("/",0) !=0 ) {
					path = normalizePath(pwd) + "/"  + leaf;
				} else {
					path = normalizePath(pwd) + leaf;
				}
			} else{
				path = normalizePath(pwd) + leaf;
			}
		}
	} else if (path.find("/") ==0 ){
		// ABsolute Path
		path = normalizePath(path);
	} else {
		// Relative path: append PWD
		if (pwd){
			path = normalizePath(pwd) + "/" + path;
		}
	}
	return path;
	#ifndef GLITE_WMS_WMPROXY_TOOLS
	GLITE_STACK_CATCH();
	#endif
}

} // namespace utilities
} // namespace wmproxy
} // namespace wms
} // namespace glite

