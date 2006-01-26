/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmputils.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include <iostream>
#include <fstream>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <fcntl.h> // O_RDONLY
#include <netdb.h> // gethostbyname
#include <unistd.h>

#include <stdlib.h>

#include <sys/wait.h>


// boost
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>

#include "wmputils.h"

// JobId
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

// Logging
#include "logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#include "quota.h"
#include "purger.h" 
#include "glite/wms/common/utilities/quota.h"

// Exceptions
#include "wmpexceptions.h"
#include "wmpexception_codes.h"

// ISB compression functionalities
#include "zlib.h"
#include "libtar.h"

// Global variables for configuration attributes
extern std::string sandboxdir_global;

using namespace std;

namespace logger		  = glite::wms::common::logger;
namespace commonutilities = glite::wms::common::utilities;
namespace jobid			  = glite::wmsutils::jobid;
namespace purger          = glite::wms::purger;

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
const std::string USER_PROXY_NAME = "user.proxy";
const std::string USER_PROXY_NAME_BAK = ".user.proxy.bak";

const std::string JDL_ORIGINAL_FILE_NAME = "JDLOriginal";
const std::string JDL_TO_START_FILE_NAME = "JDLToStart";
const std::string START_LOCK_FILE_NAME = ".startLockFile";

const std::string ALL_PROTOCOLS = "all";
const std::string DEFAULT_PROTOCOL = "default";


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

vector<string>
getJobDirectoryURIsVector(vector<pair<std::string, int> > protocols,
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
	
	vector<string> returnvector;
	
	vector<pair<std::string, int> > returnprotocols;
	if ((protocol == "") || (protocol == ALL_PROTOCOLS)) {
		returnprotocols = protocols;
	} else if (protocol == DEFAULT_PROTOCOL) {
		pair<string, int> itempair(defaultprotocol, defaultport);
		returnprotocols.push_back(itempair);
	} else {
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
	
	string item;
	unsigned int size = returnprotocols.size();
	for (unsigned int i = 0; i < size; i++) {
		item = returnprotocols[i].first
			+ "://" + serverhost
			+ ((returnprotocols[i].second == 0) 
				? "" 
				: (":" + boost::lexical_cast<std::string>
					(returnprotocols[i].second)))
			+ path;
		edglog(debug)<<"Job directory URI: "<<item<<endl;
		returnvector.push_back(item);
	}
	
	// Adding https protocol
	if ((protocol == ALL_PROTOCOLS) || (protocol == "https")) {
		if (httpsport) {
			item = "https://" + string(getenv("SERVER_ADDR")) + ":"
				+ boost::lexical_cast<std::string>(httpsport) + httppath;
		} else {
			item = "https://" + string(getenv("HTTP_HOST")) + httppath;
		}
		edglog(debug)<<"Job directory URI: "<<item<<endl;
		returnvector.push_back(item);
	}
	
	return returnvector;
	GLITE_STACK_CATCH();
}

vector<string> *
getDestURIsVector(vector<pair<std::string, int> > protocols, int httpsport,
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
		port = boost::lexical_cast<std::string>(protocols[i].second);
		item = protocols[i].first
			+ "://" + serverhost
			+ ((protocols[i].second == 0) ? "" : (":" + port))
			//+ ((protocols[i].first != "https") ? path : httppath);
			+ path;
		edglog(debug)<<"Destination URI: "<<item<<endl;
		returnvector->push_back(item);
	}
	// Adding https protocol
	if (addhttps) {
		if (httpsport != 0) {
			item = "https://" + string(getenv("SERVER_ADDR")) + ":"
				+ boost::lexical_cast<std::string>(httpsport) + httppath;
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

/*bool
fileExists(const string &path)
{
	ifstream file(path.c_str());
	if (!file.good()) {
		return false;
	}
	return true;
}*/
	
	//#include <sys/stat.h>

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
getDestURI(const string &jobid, const std::string &protocol,
	int port)
{
	GLITE_STACK_TRY("getDestURI()");
	string dest_uri(protocol + "://" + getServerHost()
		+ ((port == 0) ? "" : (":" + boost::lexical_cast<std::string>(port)))
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
getJobStartLockFilePath(jobid::JobId jid, int level)
{
	GLITE_STACK_TRY("getJobStartLockFilePath()");
	return string(getenv(DOCUMENT_ROOT) + FILE_SEP
		+ to_filename(jid, level) + FILE_SEP + START_LOCK_FILE_NAME);
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
		path = string(FILE_SEP + to_filename(jid, level)
			+ FILE_SEP + PEEK_DIRECTORY);
	} else {
		path = string(getenv(DOCUMENT_ROOT) + FILE_SEP
			+ to_filename(jid, level) + FILE_SEP + PEEK_DIRECTORY);
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
getJobJDLOriginalPath(jobid::JobId jid, int level)
{	
	GLITE_STACK_TRY("getJobJDLOriginalPath(JobId jid)");
	//TBD Check path
	return string(getenv(DOCUMENT_ROOT) 
		+ FILE_SEP + to_filename(jid, level)
		+ FILE_SEP + JDL_ORIGINAL_FILE_NAME);
	GLITE_STACK_CATCH();
}

string
getJobJDLToStartPath(jobid::JobId jid, int level)
{	
	GLITE_STACK_TRY("getJobJDLToStartPath(JobId jid)");
	//TBD Check path
	return string(getenv(DOCUMENT_ROOT) 
		+ FILE_SEP + to_filename(jid, level)
		+ FILE_SEP + JDL_TO_START_FILE_NAME);
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
doPurge(std::string dg_jobid)
{
	GLITE_STACK_TRY("doPurge()");
	edglog_fn("wmputils::doPurge");
	
	if (dg_jobid.length()) {
  		edglog(debug)<<"JobId object for purging created: "
  			<<dg_jobid<<std::endl;
  		boost::filesystem::path path(getJobDirectoryPath(jobid::JobId(dg_jobid)),
  			boost::filesystem::native);
  		return purger::purgeStorageEx(path, edg_wll_LogClearUSER);
    } else {
   		edglog(critical)
			<<"Error in Purging: Invalid Job Id. Purge not done."<<std::endl;
      return false; 
    }
    
    GLITE_STACK_CATCH();
}

bool
getUserQuota(std::pair<long, long>& result, std::string uname)
{
	GLITE_STACK_TRY("getUserQuota()");
	result = commonutilities::quota::getQuota(uname);
	return true;
	GLITE_STACK_CATCH();
}

bool
getUserFreeQuota(std::pair<long, long>& result, std::string uname)
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

void 
gzUncompress(gzFile in, FILE * out, char * file)
{
	GLITE_STACK_TRY("gzUncompress()");
	edglog_fn("wmputils::gzUncompress");
	
	long filesize = computeFileSize(string(file));
    char buf[filesize + 1];
    int len;
    
    for (;;) {
        len = gzread(in, buf, sizeof(buf));
        if (len < 0) {
        	edglog(severe)<<"Error in gzread, file: "<<string(file)<<std::endl;
	    	throw FileSystemException(__FILE__, __LINE__,
				"uncompressFile()", WMS_IS_FAILURE,
				"Unable to uncompress ISB file\n(please contact server administrator)");
        }
        if (len == 0) {
        	break;
        }
        if ((int)fwrite(buf, 1, (unsigned)len, out) != len) {
            edglog(severe)<<"Error in fwrite, file: "<<string(file)<<std::endl;
	    	throw FileSystemException(__FILE__, __LINE__,
				"uncompressFile()", WMS_IS_FAILURE,
				"Unable to uncompress ISB file\n(please contact server administrator)");
	        }
    }
    if (fclose(out)) {
    	edglog(severe)<<"Error in fclose, file: "<<string(file)<<std::endl;
    	throw FileSystemException(__FILE__, __LINE__,
			"uncompressFile()", WMS_IS_FAILURE,
			"Unable to uncompress ISB file\n(please contact server administrator)");
    }
    if (gzclose(in) != Z_OK) {
    	edglog(severe)<<"Error in gzclose, file: "<<string(file)<<std::endl;
    	throw FileSystemException(__FILE__, __LINE__,
			"uncompressFile()", WMS_IS_FAILURE,
			"Unable to uncompress ISB file\n(please contact server administrator)");
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
    FILE * out = NULL;
    gzFile in;
    
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
    in = gzopen(infile, "rb");
    if (in == NULL) {
    	edglog(severe)<<"Error in gzopen, file: "<<string(file)<<std::endl;
    	throw FileSystemException(__FILE__, __LINE__,
			"uncompressFile()", WMS_IS_FAILURE,
			"Unable to uncompress ISB file\n(please contact server administrator)");
    }
    out = fopen(outfile, "wb");
    if (out == NULL) {
        edglog(severe)<<"Error in opening output file: "<<string(outfile)<<std::endl;
    	throw FileSystemException(__FILE__, __LINE__,
			"uncompressFile()", WMS_IS_FAILURE,
			"Unable to uncompress ISB file\n(please contact server administrator)");
    }
    gzUncompress(in, out, file);
    unlink(infile);

    TAR * tarfile = NULL;
    tar_open(&tarfile, outfile, NULL, O_RDONLY, S_IRWXU, TAR_GNU);
    tar_extract_all(tarfile, prefix);
    tar_close(tarfile);
    
    remove(outfile);
 
    GLITE_STACK_CATCH();
}

void 
fileCopy(const std::string& source, const std::string& target)
{
	GLITE_STACK_TRY("fileCopy()");
 	edglog_fn("wmputils::fileCopy");
  	edglog(debug)<<"Copying file...\n\tSource: "
  		<<source<<"\n\tTarget: "<<target<<std::endl;
  
  	std::ifstream in(source.c_str());
  	if (!in.good()) {
  		edglog(severe)<<"Copy failed, !in.good(). \n\tSource: "
  			<<source<<" Target: "<<target<<std::endl;
    	throw FileSystemException(__FILE__, __LINE__,
			"fileCopy(const std::string& source, const std::string& target)",
			WMS_IS_FAILURE, "Unable to copy file");
	 }
	 std::ofstream out(target.c_str());
	 if (!out.good()) {
  		edglog(severe)<<"Copy failed, !out.good(). \n\tSource: "
  			<<source<<"\n\tTarget: "<<target<<std::endl;
    	throw FileSystemException(__FILE__, __LINE__,
			"fileCopy(const std::string& source, const std::string& target)",
			WMS_IS_FAILURE, "Unable to copy file");
  	}
  	out<<in.rdbuf(); // read original file into target
  
  	struct stat from_stat;
  	if (stat(source.c_str(), &from_stat) ||
	    	chown(target.c_str(), from_stat.st_uid, from_stat.st_gid) ||
	        chmod(target.c_str(), from_stat.st_mode)) {
		edglog(severe)<<"Copy failed, chown/chmod. \n\tSource: "
		<<source<<"\n\tTarget: "<<target<<std::endl;

	    throw FileSystemException(__FILE__, __LINE__,
			"fileCopy(const std::string& source, const std::string& target)",
			WMS_IS_FAILURE, "Unable to copy file");
  	}
  	edglog(debug)<<"Copy done."<<std::endl;
  	GLITE_STACK_CATCH();
}

std::string
to_filename(glite::wmsutils::jobid::JobId j, int level, bool extended_path)
{
	GLITE_STACK_TRY("to_filename()");
	std::string path(sandboxdir_global + std::string(FILE_SEP)
		+ glite::wmsutils::jobid::get_reduced_part(j, level));
	if (extended_path) {
		path.append(std::string(FILE_SEP) + glite::wmsutils::jobid::to_filename(j));
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

void
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
	
	switch (fork()) {
		case -1:
			// Unable to fork
			edglog(critical)<<"Unable to fork process during job local "
				"directory creation"<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"doExecv()", WMS_IS_FAILURE, "Unable to fork process"
				"\n(please contact server administartor");
			break;
		case 0:
			// child
	        if (execv(command.c_str(), argvs)) {
	        	if (errno == E2BIG) {
        			edglog(debug)<<"Command line too long, splitting..."<<endl;
        			unsigned int middle = startIndex
        				+ (endIndex - startIndex) / 2;
                    edglog(info)<<"Calling from index "<<startIndex
                    	<<" to "<<middle<<endl;
        			doExecv(command, params, dirs, startIndex, middle);
        			edglog(info)<<"Calling from index "<<middle + 1
                    	<<" to "<<endIndex<<endl; 
        			doExecv(command, params, dirs, middle + 1, endIndex);
	        	} else {
	        		edglog(critical)<<"Unable to execute command during job local "
						"directory creation"<<endl;
	        		throw FileSystemException(__FILE__, __LINE__,
						"doExecv()", WMS_IS_FAILURE, "Unable to execute command"
						"\n(please contact server administartor");
				}
	        }
	        break;
        default:
        	// parent
	    	int status;
	    	wait(&status);
	    	if (status) {
	    		edglog(critical)<<"Unable to create job local directory, "
					"exit code: "<<status<<endl;
	    		throw FileSystemException(__FILE__, __LINE__,
					"doExecv()", WMS_IS_FAILURE, "Unable to create job local "
					"directory\n(please contact server administartor)");
	    	}
	    	break;
	}
	for (unsigned int j = 0; j <= i; j++) {
		free(argvs[j]);
	}
    free(argvs);
    
    GLITE_STACK_CATCH();
}

int 
managedir(const std::string &document_root, uid_t userid, uid_t jobdiruserid,
	std::vector<std::string> jobids)
{
	GLITE_STACK_TRY("managedir()");
	edglog_fn("wmputils::managedir");
	
	time_t starttime = time(NULL);
	
	int exit_code = 0; 
	unsigned int size = jobids.size();
	edglog(info)<<"Job id vector size: "<<size<<endl;
	
	if (size != 0) {
	   	// Try to find managedirexecutable 
	   	char * glite_path = getenv(GLITE_WMS_LOCATION); 
	   	if (glite_path == NULL) {
	   		glite_path = getenv(GLITE_LOCATION);
	   	}
	   	string gliteDirmanExe = (glite_path == NULL)
	   		? (FILE_SEP + "opt" + FILE_SEP + "glite")
	   		:(string(glite_path)); 
	   	gliteDirmanExe += FILE_SEP + "bin" + FILE_SEP
	   		+ "glite_wms_wmproxy_dirmanager";
	   	
	   	string useridtxt = boost::lexical_cast<std::string>(userid);
	   	string grouptxt = boost::lexical_cast<std::string>(getgid());
	   	
		int level = 0; 
	   	bool extended_path = true ; 
		// Vector contains at least one element
		string path = to_filename (glite::wmsutils::jobid::JobId(jobids[0]),
		   	level, extended_path);
		int pos = path.find(FILE_SEP, 0);
		string sandboxdir = document_root + FILE_SEP + path.substr(0, pos)
			+ FILE_SEP;
		// Creating SanboxDir directory if needed
		if (!fileExists(sandboxdir)) {
			string run = gliteDirmanExe
				+ " -c " + useridtxt
				+ " -g " + grouptxt
				+ " -m 0773 " + sandboxdir;
		   	edglog(debug)<<"Creating SandboxDir..."<<endl;
			edglog(debug)<<"Executing: \n\t"<<run<<endl;
			if (system(run.c_str())) {
		   		return 1;
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
		juser = boost::lexical_cast<std::string>(jobdiruserid);
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
		
	   	doExecv(gliteDirmanExe, redparams, reddirs, 0, reddirs.size() - 1);
	   	doExecv(gliteDirmanExe, jobparams, jobdirs, 0, jobdirs.size() - 1);
	}
	time_t stoptime = time(NULL);
	edglog(debug)<<"Directory creation elapsed time: "<<(stoptime - starttime)<<endl;
	
    return exit_code;
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
		outfile << "flag";
		outfile.close();
	} else {
		remove(file.c_str());	
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
	outfile << text;
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
/*
* Removes white spaces form the begininng and from the end of the input string
*/

const std::string cleanString(std::string str) {
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
}

 /**
 *
 */
const std::string toLower ( const std::string &src) {
	std::string result(src);
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
 }
 /**
 *
 */
void split (const std::string &field, std::string &label, std::string &value){
	unsigned int size = field.size();
	if (size>0) {
		unsigned int p = field.find("=") ;
		if ( p != string::npos & ( p < size) ){
			label = field.substr(0, p);
			value = field.substr(p+1, size-(p+1));
			// removes white spaces at the beginning and a the end of the strings (if present)
			// and converts the uppercase letters to the corresponding lowercase
			label = toLower(cleanString(label));
			value = toLower(cleanString(value));
		}
	}
};
} // namespace utilities
} // namespace wmproxy
} // namespace wms
} // namespace glite

