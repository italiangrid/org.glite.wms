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
namespace wms{
namespace client {
namespace services {

const string DEFAULT_STORAGE_LOCATION = "/tmp";
const string DISPLAY_CMD = "more";
/*
* Default constructor
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
void JobPerusal::readOptions ( int argc,char **argv)  {
	ostringstream err ;
 	string opts = Job::readOptions  (argc, argv, Options::JOBPERUSAL);
        // writes the information on the specified option in the log file
        logInfo->print(WMS_INFO, "Function Called:", wmcOpts->getApplicationName( ), false);
        logInfo->print(WMS_INFO, "Options specified:", opts, false);
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
	} if (setOpt && allOpt) {
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
		err << wmcOpts->getAttributeUsage(Options::GET) << " : no multiple filename can be specified.\n";
		err <<  "Use the following option only once to get a perusal file:\n";
		err << wmcOpts->getAttributeUsage(Options::FILENAME) << "\n";
		err << "or "  << wmcOpts->getAttributeUsage(Options::ALL) << " to retrieve all files.\n";
	} else if (inOpt && unsetOpt) {
		err << "The unset operation disables all perusal files; the following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::UNSET) << "\n";
		err << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
	} else if (unsetOpt && peekFiles.size() >0 ) {
		err << "The unset operation disables all perusal files; the following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::UNSET) << "\n";
		err << wmcOpts->getAttributeUsage(Options::FILENAME) << "\n";
	}

	if (err.str().size() > 0) {
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
	if (unsetOpt && allOpt) {
		logInfo->print(WMS_WARNING,
			wmcOpts->getAttributeUsage(Options::ALL) +
			": ignored (unset operation always disable all perusal files", "" );
	}
	if (err.str().size() > 0) {
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
	nointOpt = wmcOpts->getBoolAttribute(Options::NOINT);
	// Files
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
			err << ": too many items in the input file; only one perusal file can be requested.\n";
			err << "To specify a perusal file:\nuse " << wmcOpts->getAttributeUsage(Options::FILENAME) << "\n";
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
     }
	if ( peekFiles.empty ()>0 && ( setOpt ||  (getOpt && !allOpt)))  {
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Arguments Error",
				"No valid perusal files specified");
	}
	/**
	* The jobid
	*/
	jobId = wmcOpts->getJobId( );
	// --dir
	dirOpt = wmcOpts->getStringAttribute(Options::DIR);
	if (dirOpt) {
		if (Utils::isDirectory(*dirOpt)) {
			*dirOpt = Utils::normalizePath(*dirOpt);
		} else {
			err << wmcOpts->getAttributeUsage(Options::OUTPUT) << "\n";
			err << "unvalid path: " << *dirOpt << "\n";
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error",err.str());
		}
	} else {
		dirOpt = new string(DEFAULT_STORAGE_LOCATION);
		*dirOpt += "/" + Utils::getUnique(jobId);
	}
	if (! Utils::isDirectory(*dirOpt)) {
		if (mkdir(dirOpt->c_str(), 0755)){
			// Error while creating directory
			throw WmsClientException(__FILE__,__LINE__,
			"retrieveOutput", ECONNABORTED,
			"Unable create dir: ",*dirOpt);
		}
	}
	// output file
	outOpt = wmcOpts->getStringAttribute(Options::OUTPUT);
}
/**
* Perfroms the main operations
*/
void JobPerusal::jobPerusal ( ){
	vector<string> paths;
	postOptionchecks();
	// checks the status of the job
	//checkStatus( );
	// Operation
	if (setOpt){
		setPerusal( );
		printResult(PERUSAL_SET, paths);
	} else if (getOpt) {
		getPerusal(paths);
		printResult(PERUSAL_GET, paths);
	} else if (unsetOpt) {
		unsetPerusal( );
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
	logInfo->print(WMS_DEBUG, "Checking the status of the job", "");
	Status status=lbApi.getStatus(true,true);
cout << "#### checkStatus> 1\n";
	code = status.checkCodes(Status::OP_PERUSAL, warnings);
cout << "#### checkStatus> 2\n";
	if (warnings.size()>0){ logInfo->print(WMS_WARNING, warnings, "", true);}
	if (code == 0){
		cfgCxt = new ConfigContext("", "", "");
		// Initialize ENDPOINT (start a new (thread of) job (s)
		cfgCxt->endpoint= status.getEndpoint();
		logInfo->print(WMS_DEBUG, "Endpoint set to: ", cfgCxt->endpoint);
	}
}
/**
* GET operation
*/

void JobPerusal::getPerusal (std::vector<std::string> &paths){
	vector<string> uris;
	string errors = "";
	string file = "";
	try {
		logInfo->print(WMS_INFO, "GET  - Connecting to the service", cfgCxt->endpoint);
		if (peekFiles.empty()){
			throw WmsClientException(__FILE__,__LINE__,
				"getPerusal",DEFAULT_ERR_CODE,
				"Input Arguments Error",
				"No valid perusal files specified");
		} else {
			file = peekFiles[0] ;
		}
		uris = getPerusalFiles (jobId, file , allOpt, cfgCxt);
	} catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
			"getPerusal", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
	}
	if (uris.size()>0) {
		this->gsiFtpGetFiles(uris, paths, errors);
		if (paths.empty() && errors.size( )>0){
			throw WmsClientException(__FILE__,__LINE__,
				"getPerusal", DEFAULT_ERR_CODE,
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
void JobPerusal::setPerusal ( ){
	try {
		logInfo->print(WMS_INFO, "SET  - Connecting to the service", cfgCxt->endpoint);
		enableFilePerusal (jobId, peekFiles, cfgCxt);
	} catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
			"enablePerusalFiles", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
	}
}
/**
* UNSET operation
*/
void JobPerusal::unsetPerusal ( ){
	vector<string> empty;
	try {
		logInfo->print(WMS_INFO, "UNSET  - Connecting to the service", cfgCxt->endpoint);
		enableFilePerusal (jobId, empty, cfgCxt);
	} catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
			"enablePerusalFiles", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
	}
}

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

void JobPerusal::printResult(const perusalOperations &operation, std::vector<std::string> &paths){
	ostringstream out;
	string subj = "";
	string verb = "";
	string count = "";
	string cmd = "";
	string ws = " ";
	int size = 0;
	out << "\n" << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " Success") ) << "\n\n";
	if (paths.size() == 1 ) {
		subj = "perusal file";
		verb = "has";
		count = "name of";
	} else {
		subj = "perusal files";
		verb = "have";
		count = "list of";
	}
	if (operation == PERUSAL_SET) {
		out << "Your" << ws << subj << ws <<  verb << ws << "been successfully enabled for the job:\n";
		out << jobId << "\n";
		// saves the result
		if (outOpt){
			if ( wmcUtils->saveListToFile(*outOpt,  peekFiles) < 0 ){
				logInfo->print (WMS_WARNING, "unable to save"+ ws + count + ws + "your" + ws + subj + ws + "in the output file " ,
						 Utils::getAbsolutePath(*outOpt));
			} else {
				out << "\nThe" + ws + count + ws + "your" + ws + subj + ws + "been saved in the following file:\n";
				out << Utils::getAbsolutePath(*outOpt) << "\n";
			}
		}
	} else if (operation == PERUSAL_GET) {
		if (paths.empty()) {
			out << "No" << ws << subj << ws << "to be retrieved  for the job:\n";
			out << jobId << "\n";
		} else {
			out << "Your" << ws << subj << ws << verb << ws << "been successfully stored in:\n";
			out << *dirOpt << "\n";
			if (outOpt){
				if ( wmcUtils->saveListToFile(*outOpt, paths) < 0 ){
					logInfo->print (WMS_WARNING, "unable to save"+ ws + count +  ws + "your" + ws + subj + ws + "in the output file " ,
							Utils::getAbsolutePath(*outOpt));
				} else {
					out << "\nThe"<< ws << count <<  ws << "your" << ws << subj << ws << "is stored in the file:\n";
					out << Utils::getAbsolutePath(*outOpt) << "\n";
				}
			}
		}
	} else if (operation == PERUSAL_UNSET) {
		out << "Your" << ws << subj << ws <<  verb << ws << "been successfully disabled for the job:\n";
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

