/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#include "jobperusal.h"
#include <string>
#include <sys/stat.h> //mkdir
// streams
#include<sstream>
#include <sys/stat.h> //mkdir
// wmproxy-api
#include "glite/wms/wmproxyapi/wmproxy_api.h"
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"
// JDL
#include "glite/wms/jdl/Ad.h"
#include "glite/wms/jdl/JDLAttributes.h"
// exceptions
#include "utilities/excman.h"
// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/options_utils.h"
// wmp-client LB API
#include"lbapi.h"
//BOOST
#include "boost/lexical_cast.hpp" // types conversion
// CURL
#include "curl/curl.h"

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;

namespace glite {
namespace wms {
namespace client {
namespace services {

const string DEFAULT_STORAGE_LOCATION = "/tmp";
const string DISPLAY_CMD = "more";
/*
* 	Default constructor
*/
JobPerusal::JobPerusal () : Job() {
	// init of the string  options
	inOpt = NULL;
	inFileOpt = NULL;
	outOpt = NULL;
	dirOpt = NULL;
	// boolean options
	getOpt = false;
	setOpt = false;
	unsetOpt = false;
	nodisplayOpt = false;
	nointOpt = false;
	allOpt = false;
	jobId = "";
	fileProto = NULL;
}
/**
* Default Destructor
*/
JobPerusal::~JobPerusal ()  {
	// "free memory" for the string  attributes
	if (inFileOpt) { free(inFileOpt);}
	if (inOpt) { free(inOpt);}
	if (outOpt ) { free(outOpt);}
	if (dirOpt ) { free(dirOpt);}
	if (fileProto){ delete(fileProto); }
}
/**
* Reads the input arguments
*/
void JobPerusal::readOptions ( int argc,char **argv)  {
	ostringstream err ;
	string files = "";
	string dircfg = "";
	string logname = "";
	ostringstream warn;
	vector<string> jobids ;
	// Reads the input options
 	Job::readOptions  (argc, argv, Options::JOBPERUSAL);
        // --get
	getOpt = wmcOpts->getBoolAttribute(Options::GET);
	// --set
	setOpt = wmcOpts->getBoolAttribute(Options::SET);
	// --unset
	unsetOpt = wmcOpts->getBoolAttribute(Options::UNSET);
	// --all
	allOpt = wmcOpts->getBoolAttribute(Options::ALL);
	if (!getOpt && !setOpt && !unsetOpt){
		err << "A mandatory option is missing:\n" ;
		err << wmcOpts->getAttributeUsage(Options::GET) << " | ";
		err << wmcOpts->getAttributeUsage(Options::SET) << " | ";
		err << wmcOpts->getAttributeUsage(Options::UNSET) << "\n";
	} else if ((getOpt && setOpt) ||
			(getOpt &&unsetOpt) ||
			(setOpt &&unsetOpt))	{
		err << "The following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::GET) << " | ";
		err << wmcOpts->getAttributeUsage(Options::SET) << " | ";
		err << wmcOpts->getAttributeUsage(Options::UNSET) << "\n";
	} else if (setOpt && allOpt) {
		err << "The following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::SET) << " | ";
		err << wmcOpts->getAttributeUsage(Options::ALL) << "\n";
	}
	if (err.str().size() > 0) {
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
	// --proto
	fileProto = wmcOpts->getStringAttribute(Options::PROTO);
	if (setOpt && fileProto) {
		logInfo->print (WMS_WARNING, "--proto: option ignored (SET operation doesn't need any file transferring)\n", "", true );
	}  else if (unsetOpt && fileProto) {
		logInfo->print (WMS_WARNING, "--proto: option ignored (UNSET operation doesn't need any file transferring)\n", "", true );
	} else if (fileProto==NULL) {
		fileProto= new string (Options::TRANSFER_FILES_DEF_PROTO );
		logInfo->print (WMS_DEBUG,
			"--proto option not specified; using the default File Transferring Protocol:", *fileProto  );
	} else {
		logInfo->print (WMS_DEBUG,
			"--proto option - File Transferring Protocol:", *fileProto  );
	}
	// --input
        inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	// --input-file
        inFileOpt = wmcOpts->getStringAttribute(Options::INPUTFILE);
	// -- filename
	peekFiles = wmcOpts->getListAttribute(Options::FILENAME);
	if (inFileOpt && peekFiles.size() > 0 ) {
		err << "The following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::INPUTFILE) << "\n";
		err << wmcOpts->getAttributeUsage(Options::FILENAME) << "\n";
	} else if (getOpt && peekFiles.size() > 1){
		err << wmcOpts->getAttributeUsage(Options::GET) << " : no multiple filenames can be specified.\n";
		err <<  "Use the following option only once to get a job's file:\n";
		err << wmcOpts->getAttributeUsage(Options::FILENAME) << "\n";
		err << "or "  << wmcOpts->getAttributeUsage(Options::ALL) << " to retrieve all job's files.\n";
	} else if (inFileOpt && unsetOpt) {
		err << "The unset operation disables job's files perusal; the following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::UNSET) << "\n";
		err << wmcOpts->getAttributeUsage(Options::INPUTFILE) << "\n";
	}
	if (err.str().size() > 0) {
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}

	nointOpt = wmcOpts->getBoolAttribute(Options::NOINT);
	// --input-file
        if (inFileOpt) {
        	peekFiles = wmcUtils->getItemsFromFile(*inFileOpt);
		if (peekFiles.empty()) {
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error",
				"The input file is empty: " + Utils::getAbsolutePath(*inFileOpt) );
		} else if (peekFiles.size() == 1) {
			logInfo->print (WMS_DEBUG, "filename read by input file: ", peekFiles[0] );
		} else if (nointOpt && getOpt) {
			err <<	wmcOpts->getAttributeUsage(Options::GET)  ;
			err << ": too many items in the input file; only one job's file can be requested.\n";
			err << "To specify a job's file:\nuse " << wmcOpts->getAttributeUsage(Options::FILENAME) << "\n";
			err << "or " << wmcOpts->getAttributeUsage(Options::INPUTFILE) ;
			err <<  " without " << wmcOpts->getAttributeUsage(Options::NOINT) << "\n";
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
		} else if (getOpt && nointOpt == false) {
			cout << "Filenames in the input file: " << Utils::getAbsolutePath(*inFileOpt) << "\n";
			cout << wmcUtils->getStripe(74, "-") << "\n";
			peekFiles = wmcUtils->askMenu(peekFiles, Utils::MENU_SINGLEFILE);
		} else if (nointOpt == false) {
			cout << "Filenames in the input file: " << Utils::getAbsolutePath(*inFileOpt) << "\n";
			peekFiles =  wmcUtils->askMenu(peekFiles, Utils::MENU_FILE);
		}
		// Writes in the log file the list of filenames chosen by the input file
		int size =  peekFiles.size();
		if (size > 0) {
			for (unsigned int i = 0; i < size; i++) { files += peekFiles[i] + "; ";}
			logInfo->print (WMS_INFO, "filenames from the input file:", files, false);
		}
    	 }
	 // other incompatible
	if ( peekFiles.empty ()  && ( setOpt||getOpt ) )  {
		err << "No valid job's file specified; use one of these options:\n";
		err << wmcOpts->getAttributeUsage(Options::FILENAME) << "\n";
		err << wmcOpts->getAttributeUsage(Options::INPUTFILE) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
			"readOptions",DEFAULT_ERR_CODE,
			"Input Arguments Error",
			err.str());
	}
	// output file
	outOpt = wmcOpts->getStringAttribute(Options::OUTPUT);
	// File directory for --get
	dirOpt = wmcOpts->getStringAttribute(Options::DIR);
	// --nodisplay
	nodisplayOpt = wmcOpts->getBoolAttribute(Options::NODISPLAY);
	// WARNING MESSAGES
	// =================================
	// Ignored options with --unset
	if (unsetOpt){
		// each --filename specified
		int size = peekFiles.size();
		for (int i=0; i < size; i++ ) {
			warn << "--filename " + peekFiles[i]  + " (unset operation disables perusal of all job's files)\n";
		}
		// --all
		if (allOpt) {
			warn << wmcOpts->getAttributeUsage(Options::ALL)  << " (unset operation disables perusal of all job's files)\n";
		}
		// --nodisplay
		if (nodisplayOpt) {
			warn << wmcOpts->getAttributeUsage(Options::NODISPLAY) << " (no files to be displayed on the standard output)\n";
		}
		//--dir
		if (dirOpt){
			warn << "--dir " << *dirOpt << " (no files to be retrieved)\n";
		}
		//--output
		if (outOpt) {
			warn << "--output " << *outOpt <<" (no useful information to be saved into a file)\n";
		}
		if (warn.str().size()>0) {
			logInfo->print(WMS_WARNING,
				"The following option(s) is(are) ignored:",
				warn.str());
		}
	} else if (setOpt) {
		//--dir
		if (dirOpt){
			warn << "--dir " << *dirOpt << " (no files to be retrieved)\n";
		}
		// --nodisplay
		if (nodisplayOpt) {
			warn << wmcOpts->getAttributeUsage(Options::NODISPLAY) << " (no files to be displayed on the standard output)\n";
		}
		if (warn.str().size()>0) {
			logInfo->print(WMS_WARNING,
				"The following option(s) is(are) ignored:",
				warn.str());
		}

	}
	// Reading jobid from input file
	if (inOpt){
		// From input file
		logInfo->print (WMS_DEBUG, "Reading JobId(s) from the input file:", Utils::getAbsolutePath(*inOpt));
		jobids = wmcUtils->getItemsFromFile(*inOpt);
		if (jobids.size() > 1 && ! wmcOpts->getBoolAttribute(Options::NOINT) ){
			logInfo->print (WMS_DEBUG, "Multiple JobIds found:", "asking for choosing one id in the list ");
			jobids = wmcUtils->askMenu(jobids,Utils::MENU_SINGLEJOBID);
			jobId = Utils::checkJobId(jobids[0]);
			logInfo->print (WMS_DEBUG, "Chosen JobId:", jobId );
   		}
        } else {
		// from command line
        	jobId = wmcOpts->getJobId();
        }

	if (getOpt) {
		// Directory path for file downloading (only for --get)
		if (!dirOpt){
			char* environ=getenv("LOGNAME");
			if (environ){logname="/"+string(environ);}
			else{logname="/jobPerusal";}
			dircfg = Utils::getAbsolutePath(wmcUtils->getOutputStorage()) ;
			logInfo->print(WMS_DEBUG, "Output Storage (by configuration file):", dircfg);
			dirOpt = new string (dircfg + logname + "_" + Utils::getUnique(jobId));
		} else {
			*dirOpt = Utils::getAbsolutePath(*dirOpt);
			logInfo->print(WMS_DEBUG, "Output Storage (by --dir option):", *dirOpt);
		}
		// makes directory
		if (! Utils::isDirectory(*dirOpt)) {
			logInfo->print(WMS_DEBUG, "Creating directory:", *dirOpt);
			if (mkdir(dirOpt->c_str(), 0755)){
				// Error while creating directory
				throw WmsClientException(__FILE__,__LINE__,
				"retrieveOutput", ECONNABORTED,
				"Unable create dir: ",*dirOpt);
			}
		}
	}

}
/**
* Performs the main operations
*/
void JobPerusal::jobPerusal ( ){
	vector<string> paths;
	postOptionchecks();
	// checks the status of the job
	checkStatus( );
	// Operation
	if (setOpt){
		perusalSet( );
		printResult(PERUSAL_SET, paths);
	} else if (getOpt) {
		perusalGet(paths);
		printResult(PERUSAL_GET, paths);
	} else if (unsetOpt) {
		perusalUnset( );
		printResult(PERUSAL_UNSET, paths);
	}
}


/******************************
*	PRIVATE methods
******************************/
/**
* Checks the status of the job
*/
void JobPerusal::checkStatus( ){
	string warnings = "";
	int code = 0;
	LbApi lbApi;
	lbApi.setJobId(jobId);
	Status status=lbApi.getStatus(true,true);
	if (getOpt){
		code = status.checkCodes(Status::OP_PERUSAL_GET, warnings);
	}else if (setOpt){
		code = status.checkCodes(Status::OP_PERUSAL_SET, warnings);
	}else if (unsetOpt){
		code = status.checkCodes(Status::OP_PERUSAL_UNSET, warnings);
	}
	if (warnings.size()>0){ logInfo->print(WMS_WARNING, warnings, "", true);}
	if (code == 0){
		// Initialize ENDPOINT (start a new (thread of) job (s)
		setEndPoint (status.getEndpoint());
		// checks if --endpoint option has been specified with a different endpoint url
		string *endpoint =  wmcOpts->getStringAttribute (Options::ENDPOINT) ;
		if (endpoint && endpoint->compare(getEndPoint( )) !=0 ) {
			logInfo->print(WMS_WARNING, "--endpoint " + string(*endpoint) + " : option ignored", "");
		}
	}
}
/**
* GET operation
*/

void JobPerusal::perusalGet (std::vector<std::string> &paths){
	vector<string> uris;
	vector<string> extracted;
	string errors = "";
	string file = "";
	vector<pair<string, string> > pm;
	int size = 0;
	try {
		if (peekFiles.empty()){
			throw WmsClientException(__FILE__,__LINE__,
				"perusalGet",DEFAULT_ERR_CODE,
				"Input Arguments Error",
				"No valid job's files specified");
		} else {
			file = peekFiles[0] ;
		}
		// "Calling-service" message
		pm.push_back(make_pair("jobid",jobId));
		pm.push_back(make_pair("file",file));
		pm.push_back(make_pair("allOpt",boost::lexical_cast<string>(allOpt)));
		logInfo->service(WMP_GETPERUSAL_SERVICE, pm);
		logInfo->print(WMS_INFO, "Connecting to the service", getEndPoint());
		// Calling service : getPerusal =============================
		uris = getPerusalFiles (jobId, file , allOpt, getContext());

		uris.push_back("https://ghemon.cnaf.infn.it:7443/SandboxDir/6v/https_3a_2f_2fghemon.cnaf.infn.it_3a9000_2f6vFQoStDkh1VwokgC9XGNg/peek/prova.txt");

	} catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
			"perusalGet", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
	}
	size =uris.size();
	if (size>0) {
		logInfo->result(WMP_GETPERUSAL_SERVICE, "operation successfully ended; number of files to be retrieved :" + boost::lexical_cast<string>(size));
		/// globus-url-copy ===============
		if (*fileProto == Options::TRANSFER_FILES_GUC_PROTO){
			extracted = Utils::extractProtocolURIs(uris,  Options::TRANSFER_FILES_GUC_PROTO);
			size = extracted.size();
			if (size == 0) {
				throw WmsClientException(__FILE__,__LINE__,
					"jobPerusal", ECONNABORTED,
					"File Protocol Error",
					"Protocol not available in the list of URIs retrieved by the WMProxy server: "
					+ *fileProto +
					"\n(try with another protocol file, for example " + Options::TRANSFER_FILES_CURL_PROTO + " )");
			}
			// Downloads the files
			this->gsiFtpGetFiles(extracted, paths, errors);
		} else
			// CURL ==========
			if (*fileProto == Options::TRANSFER_FILES_CURL_PROTO){
				extracted = Utils::extractProtocolURIs(uris,  Options::TRANSFER_FILES_CURL_PROTO);
				size = extracted.size();
				if (size == 0) {
					throw WmsClientException(__FILE__,__LINE__,
						"jobPerusal", ECONNABORTED,
						"File Protocol Error",
						"Protocol not available in the list of URIs retrieved by the WMProxy server: "
						+ *fileProto +
						"\n(try with another protocol file, for example " + Options::TRANSFER_FILES_GUC_PROTO + " )");
				}
				this->curlGetFiles(extracted, paths, errors);
		} else {
			throw WmsClientException(__FILE__,__LINE__,
			"jobPerusal", ECONNABORTED,
			"File Protocol Error", "protocol not supported: " + *fileProto);
		}
		if (paths.empty() && errors.size( )>0){
			throw WmsClientException(__FILE__,__LINE__,
				"perusalGet", DEFAULT_ERR_CODE,
				"File Download Error",
				"GET - The following error(s) occured while transferring the file(s):\n" + errors);
		} else if  (errors.size( )>0){
			logInfo->print(WMS_WARNING,
				"GET - The following error(s) occured while transferring the file(s)\n" + errors, "");
		}
	} else {
		logInfo->result(WMP_GETPERUSAL_SERVICE, "operation successfully ended; no files to be retrieved");
	}
}
/**
* SET operation
*/
void JobPerusal::perusalSet ( ){
	vector<pair<string, string> > pm;
	// "Calling-service" message
	int size = peekFiles.size( );
	string m = "";
	pm.push_back(make_pair("jobid",jobId));
	for (int i=0; i < size; i++){
		if (i > 0  && i <= size-1){ m += ", ";}
		m += peekFiles[i] ;
	}
	pm.push_back(make_pair("files",m));
	logInfo->service(WMP_SETPERUSAL_SERVICE, pm);
	logInfo->print(WMS_INFO, "Connecting to the service", getEndPoint());
	try {
		// Calling service : setPerusal =============================
		enableFilePerusal (jobId, peekFiles, getContext());
	} catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
			"enablePerusalFiles", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
	}
	logInfo->result(WMP_SETPERUSAL_SERVICE, "operation successfully ended");
}
/**
* UNSET operation
*/
void JobPerusal::perusalUnset( ){
	vector<string> empty;
	logInfo->print(WMS_DEBUG, "Calling the " + string(WMP_SETPERUSAL_SERVICE) + " with an empty list of files to unset the peeking for the job", jobId);
	logInfo->print(WMS_INFO, "Connecting to the service", getEndPoint());
	try {
		enableFilePerusal (jobId, empty, getContext());
	} catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
			"enablePerusalFiles", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
	}
	logInfo->result(WMP_SETPERUSAL_SERVICE, "operation successfully ended");
}
/**
* File downloading with globus URL copy
*/
int JobPerusal::gsiFtpGetFiles (std::vector <std::string> &uris, std::vector<std::string> &paths, std::string &errors) {
	string local = "";
	int code = 0;
	int size = uris.size();
	string err = "";
	int res = -1;
	if (size==0)  {
		errors = "Warning - no valid URIs of files to be downloaded\n";
	} else {
		for ( int i = 0; i < size ; i++ ){
			// command
			string cmd= "globus-url-copy ";
			if (getenv("GLOBUS_LOCATION")){
				cmd=string(getenv("GLOBUS_LOCATION"))+"/bin/"+cmd;
			}else if (Utils::isDirectory ("/opt/globus/bin")){
				cmd="/opt/globus/bin/"+cmd;
			}else {
				throw WmsClientException(__FILE__,__LINE__,
					"gsiFtpGetFiles", ECONNABORTED,
					"Unable to find",
					"globus-url-copy executable");
			}
			// local path
			local = *dirOpt + "/" + Utils::getFileName (uris[i]) ;
			if (wmcUtils->askForFileOverwriting(local)){
				// COMMAND
				cmd+=string(uris[i]) + " file://"  + local;
				logInfo->print(WMS_DEBUG, "File Transferring (gsiftp command)\n", cmd);
				// launches the command
				code = system( cmd.c_str() ) ;
				if ( code != 0 ) {
					err =	"Error(globus-url-copy) unable to download: " + uris[i] + "(error code: " + boost::lexical_cast<string>(code) + ")";
					logInfo->print(WMS_WARNING, "Transfer FAILED", err);
					errors += err + "\n";
				} else if (! Utils::isFile(local)) {
					err += "Error(globus-url-copy)  unable to copy: " + uris[i]  + " to " + local + "\n";
					logInfo->print(WMS_WARNING, "Transfer FAILED", err);
					errors += err + "\n";
				} else {
					res = 0;
					logInfo->print(WMS_DEBUG, "File Transferring SUCCESS:", local);
					paths.push_back(local);
				}
			} else {
				errors += "Warning - existing file not overwritten: " + local + "\n";
			}
		}
	}
	return res;
}
/*
* writing callback for curl operations
*/
int JobPerusal::storegprBody(void *buffer, size_t size, size_t nmemb, void *stream) {
	struct httpfile *out_stream=(struct httpfile*)stream;
	if(out_stream && !out_stream->stream) {
		// open stream
		out_stream->stream=fopen(out_stream->filename, "wb");
		if(!out_stream->stream) {
			return -1;
		}
   	}
	return fwrite(buffer, size, nmemb, out_stream->stream);
 }


/**
* File downloading with CURL
*/
int JobPerusal::curlGetFiles (std::vector <std::string> &uris, std::vector<std::string> &paths, std::string &errors) {
	CURL *curl = NULL;
	string local = "";
	char curl_errorstr[CURL_ERROR_SIZE];
	CURLcode code ;
	long http_code = 0;
	string *logpath = NULL;
	FILE* log =  NULL;
	int res = -1;
	// Number of files to be transferred
	int size = uris.size();
	// user proxy
	if (size==0)  {
		errors = "Warning - no valid URIs of files to be downloaded\n";
	} else {
		// curl init
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();
		if ( curl ) {
			// user proxy
			curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE,  "PEM");
			curl_easy_setopt(curl, CURLOPT_SSLCERT, getProxyPath());
			curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE,   "PEM");
			curl_easy_setopt(curl, CURLOPT_SSLKEY, getProxyPath() );
			curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, NULL);
			//trusted certificate directory
			curl_easy_setopt(curl, CURLOPT_CAPATH, getCertsPath());
			//ssl options (no verify)
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
			// enables error message buffer
			curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errorstr);
			curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
			// DEBUG messages in the log file
			logpath = wmcUtils->getLogFileName( );
			if (logpath){
				log = fopen(logpath->c_str(), "a");
				if (log) {
					curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
					curl_easy_setopt(curl, CURLOPT_STDERR, log);
					logInfo->print(WMS_DEBUG, "CURL debug messages printed in the log file:", *logpath);
				} else {
					logInfo->print(WMS_WARNING,
						"couldn't open the log file to print CURL debug messages:",
						*logpath);
				}
			}
			// If there is no log file, messages on the std-out
			if ( log == NULL && wmcOpts->getBoolAttribute(Options::DBG)){
				curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			}
			 // writing function
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Utils::curlWritingCb);

			// files to be downloaded
			vector <std::string>::iterator it = uris.begin( ) ;
			vector <std::string>::iterator const end = uris.end( );
			for ( ; it != end ; it++) {
				// local path
				local = *dirOpt + "/" + Utils::getFileName (string(*it)) ;
				if (wmcUtils->askForFileOverwriting(local)){
					curl_easy_setopt(curl, CURLOPT_URL, it->c_str());
					ostringstream info ;
					info << "Source: " << (*it) << "\n";
					info << " Destination: " << local <<"\n";
					logInfo->print(WMS_DEBUG, "File Transferring\n", info.str());
					struct httpfile params={
						(char*)local.c_str() ,
						NULL
					};
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, &params);
					// downloading
					code = curl_easy_perform(curl);
					if (code != CURLE_OK ){
						errors += "- source URI: "+  (*it) + "\n  LOCAL PATH: " +  local + "\n" ;
						// ERROR !!!
						curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code );
						// error message according to the HTTP status code
						string err = Utils::httpErrorMessage(http_code);
						if (err.size()>0){
							errors += "  reason: " + err + "\n";
						} else
						// ... else CURL error message
						if ( strlen(curl_errorstr)>0 ){
							errors += "  reason: " + string(curl_errorstr) + "\n";
						} else {
							err += "  (unknown reason)\n";
						}
					} else {
						// SUCCESS
						res = 0;
						logInfo->print(WMS_DEBUG, "File Transferring SUCCESS", local);
						paths.push_back(local);
					}
				} else {
					errors += "Warning - existing file not overwritten: " + local + "\n";
				}
			}
			// closes the log file
			if(log) {fclose(log);}
			// cleanup
			curl_easy_cleanup(curl);
		}
	}
	return res;
}

/**
* Prints out the final results on the std-out
*/
void JobPerusal::printResult(const perusalOperations &operation, std::vector<std::string> &paths){
	ostringstream out;
	string subj = "";
	string verb = "";
	string count = "";
	string cmd = "";
	string ws = " ";
	string header = "";
	int size = 0;
	out << "\n" << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " Success") ) << "\n\n";
	if (peekFiles.size() == 1 ) {
		subj = "File perusal";
		verb = "has";
		count = "name of the file";
	} else {
		subj = "Files perusal";
		verb = "have";
		count = "list of files";
	}
	if (operation == PERUSAL_SET) {
		out << subj << ws << "has been successfully enabled for the job:\n";
		out << jobId << "\n";
		// saves the result
		if (outOpt){
			header = "###" + subj + ws + "enabled for the job " + jobId + "###";
			if ( wmcUtils->saveListToFile(*outOpt,  peekFiles, header) < 0 ){
				logInfo->print (WMS_WARNING, "unable to save the result into the output file " ,
						 Utils::getAbsolutePath(*outOpt));
			} else {
				out << "\nThe" + ws + count + ws + "to peruse" + ws + "has been saved in the following file:\n";
				out << Utils::getAbsolutePath(*outOpt) << "\n";
			}
		}
	} else if (operation == PERUSAL_GET) {
		if (paths.empty()) {
			out << "No" << ws << "files" << ws << "to be retrieved for the job:\n";
			out << jobId << "\n";
		} else {
			out << "The retrieved files have been successfully stored in:\n";
			out << *dirOpt << "\n";
			if (outOpt){
				header = "###Perusal: file(s) retrieved for the job " + jobId + "###";
				if ( wmcUtils->saveListToFile(*outOpt, paths, header) < 0 ){
					logInfo->print (WMS_WARNING, "unable to save" + ws + count +  ws + "to peruse" + ws + "in the output file " ,
						Utils::getAbsolutePath(*outOpt));
				} else {
					out << "\nThe"<< ws << count <<  ws << "to peruse" << ws << "is stored in the file:\n";
					out << Utils::getAbsolutePath(*outOpt) << "\n";
				}
			}
		}
	} else if (operation == PERUSAL_UNSET) {
		out << "File(s) perusal has been successfully disabled for the job:" << endl;
		out << jobId << "\n";
	}

	out << "\n" << wmcUtils->getStripe(74, "=") << "\n\n";
	out << getLogFileMsg ( ) << "\n";
	// Prints out the msg on the std out
	cout << out.str();
	// GET: shows the files
	if ( operation == PERUSAL_GET && ! nodisplayOpt && paths.size( ) > 0 ) {
		if (wmcUtils->answerYes ("Do you want to display the files ?", false, true)){
			size = paths.size( );
			for (int i = 0; i < size ; i++) {
				cout << wmcUtils->getStripe(74, "-") << "\n";
				cout << "file " << (i+1) << "/" << size << ": " << Utils::getFileName(paths[i]) << "\n";
				cout << wmcUtils->getStripe(74, "-") << "\n\n";
				cmd = DISPLAY_CMD + " " + string(paths[i]);
				system(cmd.c_str());
			}
		}
	}
}
}}}} // ending namespaces

