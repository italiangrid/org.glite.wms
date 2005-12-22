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
}
/**
* Default Destructor
*/
JobPerusal::~JobPerusal ()  {
	// "free memory" for the string  attributes
	if (inOpt) { free(inOpt);}
	if (outOpt ) { free(outOpt);}
	if (dirOpt ) { free(dirOpt);}
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
	// --input
        inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	// -- filename
	peekFiles = wmcOpts->getListAttribute(Options::FILENAME);
	if (inOpt && peekFiles.size() > 0 ) {
		err << "The following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		err << wmcOpts->getAttributeUsage(Options::FILENAME) << "\n";
	} else if (getOpt && peekFiles.size() > 1){
		err << wmcOpts->getAttributeUsage(Options::GET) << " : no multiple filenames can be specified.\n";
		err <<  "Use the following option only once to get a job's file:\n";
		err << wmcOpts->getAttributeUsage(Options::FILENAME) << "\n";
		err << "or "  << wmcOpts->getAttributeUsage(Options::ALL) << " to retrieve all job's files.\n";
	} else if (inOpt && unsetOpt) {
		err << "The unset operation disables job's files perusal; the following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::UNSET) << "\n";
		err << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
	} else if (inOpt && allOpt) {
		err << "The following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		err << wmcOpts->getAttributeUsage(Options::ALL) << "\n";
	}

	if (err.str().size() > 0) {
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}

	nointOpt = wmcOpts->getBoolAttribute(Options::NOINT);
	// --input
        if (inOpt) {
        	peekFiles = wmcUtils->getItemsFromFile(*inOpt);
		if (peekFiles.empty()) {
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error",
				"The input file is empty: " + Utils::getAbsolutePath(*inOpt) );
		} else if (peekFiles.size() == 1) {
			logInfo->print (WMS_DEBUG, "filename read by input file: ", peekFiles[0] );
		} else if (nointOpt && getOpt) {
			err <<	wmcOpts->getAttributeUsage(Options::GET)  ;
			err << ": too many items in the input file; only one job's file can be requested.\n";
			err << "To specify a job's file:\nuse " << wmcOpts->getAttributeUsage(Options::FILENAME) << "\n";
			err << "or " << wmcOpts->getAttributeUsage(Options::INPUT) ;
			err <<  " without " << wmcOpts->getAttributeUsage(Options::NOINT) << "\n";
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());

		} else if (getOpt && nointOpt == false) {
			cout << "Filenames in the input file: " << Utils::getAbsolutePath(*inOpt) << "\n";
			cout << wmcUtils->getStripe(74, "-") << "\n";
			peekFiles = wmcUtils->askMenu(peekFiles, Utils::MENU_SINGLEFILE);
		} else if (nointOpt == false) {
			cout << "Filenames in the input file: " << Utils::getAbsolutePath(*inOpt) << "\n";
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
	if ( peekFiles.empty ()  && ( setOpt ||  (getOpt && !allOpt)))  {
		err << "No valid job's file specified; use one of these options:\n";
		err << wmcOpts->getAttributeUsage(Options::FILENAME) << "\n";
		err << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		if (getOpt) { err << wmcOpts->getAttributeUsage(Options::ALL) << "\n"; }
		throw WmsClientException(__FILE__,__LINE__,
			"readOptions",DEFAULT_ERR_CODE,
			"Input Arguments Error",
			err.str());
	}
	/*
	* The jobid
	*/
	jobId = wmcOpts->getJobId( );
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

	} else if (getOpt) {
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
	string errors = "";
	string file = "";
	try {
		logInfo->print(WMS_INFO, "Connecting to the service", getEndPoint());
		if (peekFiles.empty()){
			throw WmsClientException(__FILE__,__LINE__,
				"perusalGet",DEFAULT_ERR_CODE,
				"Input Arguments Error",
				"No valid job's files specified");
		} else {
			file = peekFiles[0] ;
		}
		uris = getPerusalFiles (jobId, file , allOpt, getContext());
	} catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
			"perusalGet", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
	}
	if (uris.size()>0) {
		this->gsiFtpGetFiles(uris, paths, errors);
		if (paths.empty() && errors.size( )>0){
			throw WmsClientException(__FILE__,__LINE__,
				"perusalGet", DEFAULT_ERR_CODE,
				"Get Files Error",
				"GET - The following error(s) occured while transferring the file(s):\n" + errors);
		} else if  (errors.size( )>0){
			logInfo->print(WMS_WARNING,
				"GET - The following error(s) occured while transferring the file(s)\n" +errors, "");
		}
	}
}
/**
* SET operation
*/
void JobPerusal::perusalSet ( ){
	try {
		logInfo->print(WMS_INFO, "Connecting to the service", getEndPoint());
		enableFilePerusal (jobId, peekFiles, getContext());
	} catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
			"enablePerusalFiles", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
	}
}
/**
* UNSET operation
*/
void JobPerusal::perusalUnset( ){
	vector<string> empty;
	try {
		logInfo->print(WMS_INFO, "Connecting to the service", getEndPoint());
		enableFilePerusal (jobId, empty, getContext());
	} catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
			"enablePerusalFiles", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
	}
}
/**
* File downloading
*/
void JobPerusal::gsiFtpGetFiles (const std::vector <std::string> &uris, std::vector<std::string> &paths, std::string &errors) {
	string local = "";
	int code = 0;
	int size = uris.size();
        //globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
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
				errors +=	"Error(globus-url-copy) unable to download: " + uris[i] + "(error code: " + boost::lexical_cast<string>(code) + ")\n";
			} else if (! Utils::isFile(local)) {
				errors +=	"Error(globus-url-copy)  unable to download: " + uris[i]  + " to " + local + "\n";
			} else {
				// Saves the local path
				paths.push_back(local);
			}
		} else {
			errors += "Warning - existing file not overwritten: " + local + "\n";
		}
         }
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

