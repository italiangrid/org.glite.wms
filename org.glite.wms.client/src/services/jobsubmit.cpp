

#include "jobsubmit.h"
// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/adutils.h"
#include "utilities/options_utils.h"
//  wmp-client exceptions
#include "utilities/excman.h"
// streams
#include <sstream>
#include <iostream>
// wmproxy-API
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"
// Ad attributes and JDL methods
#include "glite/wms/jdl/jdl_attributes.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/PrivateAttributes.h"
#include "glite/wms/jdl/extractfiles.h"
#include "glite/wms/jdl/adconverter.h"
// BOOST
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/exception.hpp"
//#include "boost/tokenizer.hpp"
// CURL
#include "curl/curl.h"
// TAR
#include "libtar.h"

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;
using namespace glite::wms::jdl;
using namespace glite::wmsutils::exception;
namespace fs = boost::filesystem;

namespace glite {
namespace wms{
namespace client {
namespace services {
const string FILE_PROTOCOL = "file://" ;
const string ISBFILE_DEFAULT = "ISBfiles";
const string TMP_DEFAULT_LOCATION = "/tmp";

const int WMPROXY_OLD_VERSION = 1;
// Max size (bytes) allowed for tar files
const long MAX_TAR_SIZE = 2147483647;
//const int MAX_TAR_SIZE =  620000;
// Max file size for globus-url-copy
const long MAX_GUC_SIZE = 2147483647;
//const long MAX_GUC_SIZE = 620000;
// Max file size for CURL
const long MAX_CURL_SIZE = 2147483647;
//const long MAX_CURL_SIZE = 620000;

/*
*	Default constructor
*/
JobSubmit::JobSubmit( ){
	// init of the string attributes
//	dgOpt = NULL;
	chkptOpt  = NULL;
	collectOpt = NULL;
	fileProto= NULL;
	lrmsOpt = NULL ;
	toOpt = NULL ;
	inOpt  = NULL;
	resourceOpt = NULL ;
	startOpt = NULL ;
	// init of the valid attribute (long type)
	validOpt = 0 ;
	// init of the boolean attributes
	nomsgOpt = false ;
	nolistenOpt = false ;
	startJob = false ;
	// JDL file
	jdlFile = NULL ;
	// Ad's
	jobAd = NULL;
	dagAd = NULL;
	collectAd = NULL;
	// shadow
	jobShadow = NULL;
	// time opt
	expireTime = 0;
	// Transfer Files Message
	infoMsg = "";
	// As default  file archiving and compression is allowed
	zipAllowed = false;
	// InputSandox attributes
	isbSize = 0;
	isbURI = "";
	// Major version of the server
	wmpVersion = 0;
};

/*
*	Default destructor
*/
JobSubmit::~JobSubmit( ){
	if (collectOpt){ delete(collectOpt); }
	if (chkptOpt){ delete(chkptOpt); }
	if (fileProto){ delete(fileProto); }
	if (lrmsOpt){ delete(lrmsOpt); }
	if (toOpt){ delete(toOpt); }
	if (inOpt){ delete(inOpt); }
	if (jdlFile){ delete(jdlFile); }
	if (jobAd){ delete(jobAd); }
	if (dagAd){ delete(dagAd); }
	if (collectAd){ delete(collectAd); };
	if ( jobShadow){ delete( jobShadow); }
	if ( startOpt){ delete( startOpt); }
};


/*
* Handles the command line options
*/
void JobSubmit::readOptions (int argc,char **argv){
	ostringstream info ;
	vector<string> wrongids;
	vector<string> resources;
	// days+hours+minutes for --to and --valid
	int d = 0;
	int h = 0;
	int m = 0;
	string opts = Job::readOptions  (argc, argv, Options::JOBSUBMIT);
	// writes the information on the specified option in the log file, it has been created
	logInfo->print(WMS_INFO,   "Function : " + wmcOpts->getApplicationName( ),
					"\n Options : " + opts, false);
	// input & resource (no together)
	inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	resourceOpt = wmcOpts->getStringAttribute(Options::RESOURCE);
	nodesresOpt = wmcOpts->getStringAttribute(Options::NODESRES);
	if (inOpt && (resourceOpt||nodesresOpt) ){
		info << "the following options cannot be specified together:\n" ;
		info << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		info << wmcOpts->getAttributeUsage(Options::RESOURCE) << "\n";
		info << wmcOpts->getAttributeUsage(Options::NODESRES) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", info.str());
	}
	if (inOpt){
		// Retrieves and check resources from file
		resources= wmcUtils->getItemsFromFile(*inOpt);
		resources = wmcUtils->checkResources (resources);
		if (resources.empty()){
			// Not even a right resource
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions", DEFAULT_ERR_CODE,
				"Wrong Input Value",
				"all parsed resources in bad format" );
		} else{
			if ( resources.size( ) > 1 && ! wmcOpts->getBoolAttribute(Options::NOINT) ){
				resources = wmcUtils->askMenu(resources, Utils::MENU_SINGLECE);
				resourceOpt = new string(resources[0]);
			} else {
				resourceOpt = new string(resources[0]);
			}
			logInfo->print(WMS_DEBUG,   "--input: The job will be submitted to the resource", *resourceOpt);
		}
	}
	// lrms has to be used with input o resource
	lrmsOpt = wmcOpts->getStringAttribute(Options::LRMS);
	if (lrmsOpt && !( resourceOpt || inOpt ) ){
		info << "LRMS option cannot be specified without a resource:\n";
		info << "use " + wmcOpts->getAttributeUsage(Options::LRMS) << " with\n";
		info << wmcOpts->getAttributeUsage(Options::RESOURCE) << "\n";
		info << "or\n" + wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", info.str());
	}
	// "valid" & "to" (no together)
	validOpt = wmcOpts->getStringAttribute(Options::VALID);
	toOpt = wmcOpts->getStringAttribute(Options::TO);
	if (validOpt && toOpt){
		info << "the following options cannot be specified together:\n" ;
		info << wmcOpts->getAttributeUsage(Options::VALID) << "\n";
		info << wmcOpts->getAttributeUsage(Options::TO) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", info.str());
	}
	// --valid
	if (validOpt){
		try{
			expireTime =  Utils::checkTime(*validOpt, d, h, m, Options::TIME_VALID) ;
		} catch (WmsClientException &exc) {
			info << exc.what() << " (use: " << wmcOpts->getAttributeUsage(Options::VALID) << ")\n";
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Wrong Time Value",info.str() );
		}
	}
	// --to
	if (toOpt) {
		try{
			expireTime= Utils::checkTime(*toOpt, d, h, m,Options::TIME_TO) ;
		} catch (WmsClientException &exc) {
			info << exc.what() << " (use: " << wmcOpts->getAttributeUsage(Options::TO) <<")\n";
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Wrong Time Value",info.str() );
		}
	}
	// Info message for --to / --valid
	if (expireTime > 0 && (h > 0 || m > 0 ) ){
		ostringstream info;
		info << "The job request will expire in ";
		if (d > 0){ info << d << " days, ";}
		if (h > 0){ info << h << " hours and ";}
		info << m << " minutes";
		if (validOpt) {
			logInfo->print(WMS_DEBUG,  info.str( ) , "(--valid option)");
		} else if (toOpt)  {
			logInfo->print(WMS_DEBUG,  info.str( ) , "(--to option)");
		}
	}
	// --chkpt
	chkptOpt =  wmcOpts->getStringAttribute( Options::CHKPT) ;
	// --collect
	collectOpt = wmcOpts->getStringAttribute(Options::COLLECTION);
	// register-only & start
	startOpt = wmcOpts->getStringAttribute(Options::START);
	registerOnly = wmcOpts->getBoolAttribute(Options::REGISTERONLY);
	if (startOpt &&
	(registerOnly || inOpt || resourceOpt || nodesresOpt || toOpt || validOpt || chkptOpt || collectOpt ||
		(wmcOpts->getStringAttribute(Options::DELEGATION) != NULL) ||
		wmcOpts->getBoolAttribute(Options::AUTODG)  )){
		info << "the following options cannot be specified together:\n" ;
		info << wmcOpts->getAttributeUsage(Options::REGISTERONLY) << "\n";
		info << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		info << wmcOpts->getAttributeUsage(Options::RESOURCE) << "\n";
		info << wmcOpts->getAttributeUsage(Options::NODESRES) << "\n";
		info << wmcOpts->getAttributeUsage(Options::TO) << "\n";
		info << wmcOpts->getAttributeUsage(Options::VALID) << "\n";
		info << wmcOpts->getAttributeUsage(Options::CHKPT) << "\n";
		info << wmcOpts->getAttributeUsage(Options::COLLECTION) << "\n";
		info << wmcOpts->getAttributeUsage(Options::AUTODG) << "\n";
		info << wmcOpts->getAttributeUsage(Options::DELEGATION) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error: " +wmcOpts->getAttributeUsage(Options::START),
				info.str());
	}
	bool transfer_files = wmcOpts->getBoolAttribute(Options::TRANSFER);
	if (registerOnly){
		startJob = false;
		if ( transfer_files) {
			registerOnly = false ;
		} else{
			registerOnly = true;
		}
	} else{
		if (transfer_files){
			info << wmcOpts->getAttributeUsage(Options::TRANSFER) ;
			info << ": this option can only be used with " ;
			info << wmcOpts->getAttributeUsage(Options::REGISTERONLY) << "\n";
			throw WmsClientException(__FILE__,__LINE__,
					"readOptions",DEFAULT_ERR_CODE,
					"Input Option Error", info.str());
		}
		registerOnly = false;
		startJob = true;
	}
	// checks the JobId argument for the --start option
	if (startOpt){
		startOpt = new string(Utils::checkJobId(*startOpt));
	} else{
		// Delegation-id (--delegation or autogeneration in case of --autm-delegation)
		dgOpt = wmcUtils->getDelegationId ();
	}
	// file Protocol
	fileProto= wmcOpts->getStringAttribute( Options::PROTO) ;
	if (startOpt && fileProto) {
		logInfo->print (WMS_WARNING, "--proto: option ignored (start operation doesn't need any file transferring)\n", "", true );
	}  else if (registerOnly && fileProto) {
		logInfo->print (WMS_WARNING, "--proto: option ignored (register-only operation doesn't need any file transferring)\n", "", true );
	}
	if (!fileProto) {
		fileProto= new string (Options::TRANSFER_FILES_DEF_PROTO );
	}
	// --nolisten
	nolistenOpt =  wmcOpts->getBoolAttribute (Options::NOLISTEN);
	// path to the JDL file
	jdlFile = wmcOpts->getPath2Jdl( );
	// checks if the proxy file pathname is set
	if (proxyFile) {
		// Proxy file set, do nothing
	} else {
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Invalid Credential",
				"No valid proxy file pathname" );
	}
}

std::string JobSubmit::getEndPoint( ) {
	string endpoint = "";
	string version = "";
	vector<string> urls;
	if  (wmpVersion == 0) {
		// checks if endpoint already contains the WMProxy URL
		if (endPoint){
			endpoint = string(*endPoint);
		} else if (autodgOpt && dgOpt==NULL) {
			// delegationId
			dgOpt = wmcUtils->getDelegationId( );
			// if the autodelegation is needed
			endpoint = wmcUtils->delegateProxy (cfgCxt, *dgOpt);
		}
		Job::getEndPointVersion(endpoint, version);
		logInfo->print (WMS_DEBUG, "Version:", version);
		wmpVersion = atoi (version.substr(0,1).c_str() );
	} else {
		if (endPoint) {
			endpoint = string(*endPoint);
		} else {
			throw WmsClientException(__FILE__,__LINE__,
				"getEndPoint",  DEFAULT_ERR_CODE ,
				"Null Pointer Error",
				"null pointer to endPoint object"   );
		}
	}
	return endpoint;
}
/**
* Performs the main operation for the submission
*/
void JobSubmit::submission ( ){
	// proxy validity must be at least 20 minutes
	postOptionchecks(20);
	ostringstream out ;
	string jobid = "";
	bool toBretrieved = false;
	// in case of --start option
	if (startOpt){
		jobid = *startOpt;
		jobStarter(jobid);
	} else {  // startOpt = FALSE
		if (!wmcOpts){
			throw WmsClientException(__FILE__,__LINE__,
				"submission",  DEFAULT_ERR_CODE ,
				"Null Pointer Error",
				"null pointer to wmcOpts URL object"   );
		}
		// checks that the needed attributes are not null
		if ( ! dgOpt){
			throw WmsClientException(__FILE__,__LINE__,
				"submission",  DEFAULT_ERR_CODE,
				"Null Pointer Error",
				"null pointer to DelegationID String"   );
		}
		// Endpoint
		endPoint =  new string(this->getEndPoint());
		cfgCxt = new ConfigContext ("", *endPoint, "");
		// AutoDeleagation
		if (wmcOpts->getBoolAttribute(Options::AUTODG)){
			wmcUtils->delegateProxy (cfgCxt, *dgOpt);
		};
		// reads and checks the JDL('s)
		wmsJobType jobtype ;
		this->checkAd(toBretrieved, jobtype);
		// Perform sSubmission when:
		// (RegisterOnly has not been specified in CLI) && (no file to be transferred)
		jobid = jobRegOrSub(startJob && !toBretrieved);
		logInfo->print(WMS_DEBUG, "The JobId is: ", jobid) ;
		// Perform File Transfer when:
		// (Registeronly is NOT specified [or specified with --tranfer-file]) AND (some files are to be transferred)
		if (toBretrieved){
			try{
				// JOBTYPE
				switch (jobtype) {
					case (WMS_JOB) : {
						jobid = this->normalJob( );
						break;
					}
					case WMS_DAG:
					case WMS_PARAMETRIC:
						jobid = this->dagJob( );
						break;
					case (WMS_COLLECTION) : {
							jobid = this->collectionJob( );
							break;
					}
					default : {
							throw WmsClientException(__FILE__,__LINE__,
								"submission",  DEFAULT_ERR_CODE ,
								"Uknown JobType",
								"unable to process the job (check the JDL)");
					}
				}
			}catch (Exception &exc){
				throw WmsClientException(__FILE__,__LINE__,
					"submission",  DEFAULT_ERR_CODE ,
					"The job has been successfully registered (the JobId is: " + jobid + "),"
					" but an error occurred while transferring files:",
					string (exc.what()));
			}
		} else{
			logInfo->print(WMS_DEBUG, "No local files to be transferred", "") ;
			if (!startJob) {
				infoMsg = "To complete the operation start the job by issuing a submission with the option:\n";
				infoMsg += " --start " + jobid + "\n";
			}
		}

		// Perform JobStart when:
		// (RegisterOnly has not been specified in CLI) AND (There were files to transfer)
		if (startJob && toBretrieved){
			// Perform JobStart
			jobStarter(jobid);
		}

	}  // startOpt = FALSE branch
	// HEADER OF THE OUTPUT MESSAGE (submission)============================================
	out << "\n" << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " Success") ) << "\n\n";
	if (registerOnly){
		out << "The job has been successfully registered to the WMProxy\n";
	} else if (startOpt){
		out << "The job has been successfully started to the WMProxy\n";
	} else {
		// OUTPUT MESSAGE (register+start)============================================
		out << "The job has been successfully submitted to the WMProxy\n";
	}
	/// OUTPUT MESSAGE (jobid and other information)============================================
	out << "Your job identifier is:\n\n";
	out << jobid << "\n";
	if (jobShadow!=NULL){
		// The job is interactive
		jobShadow->setJobId(jobid);
		if (jobShadow->isLocalConsole()){
			// console-shadow running
			if (nolistenOpt){
				// console-listener NOT running (only shadow)
				out << "\nInteractive Shadow Console successfully launched"<<"\n";
			} else {
				// console-listener running
				out << "\nInteractive Session Listener successfully launched"<<"\n";
			}
			out <<"With the following parameters:"<<"\n";
			out << "- Host: " << jobShadow->getHost() <<"\n";
			out << "- Port: " << jobShadow->getPort() <<"\n";
			if (nolistenOpt|| wmcOpts->getBoolAttribute(Options::DBG)) {
				out << "- Shadow process Id: " << jobShadow->getPid() << "\n";
				out << "- Input Stream  location: " << jobShadow->getPipeIn() <<"\n";
				out << "- Output Stream  location: " << jobShadow->getPipeOut() <<"\n";
				if (nolistenOpt){
					out << "*** Warning ***\n Make sure you will kill the Shadow process"<<"\n";
					out <<" and remove the input/output streams when interaction finishes"<<"\n";
				}
			}
		}else {
			// console-shadow NOT running
			out << "Remote Shadow console set: " << jobShadow->getHost() <<"\n";
		}
	}

	// saves the result
	if (outOpt){
		if ( wmcUtils->saveJobIdToFile(*outOpt, jobid) < 0 ){
			logInfo->print (WMS_WARNING, "Unable to write the jobid to the output file " , Utils::getAbsolutePath(*outOpt));
		} else{
			logInfo->print (WMS_DEBUG, "The JobId has been saved in the output file ", Utils::getAbsolutePath(*outOpt));
			out << "\nThe job identifier has been saved in the following file:\n";
			out << Utils::getAbsolutePath(*outOpt) << "\n";
		}
	}
	out << "\n" << wmcUtils->getStripe(74, "=") << "\n\n";

	if (infoMsg.size() > 0) {
		out << infoMsg << "\n";
		logInfo->print (WMS_INFO, infoMsg , "", false);
	}
	out << getLogFileMsg ( ) << "\n";
	// ==============================================================
	// Displays the output message
	cout << out.str() ;
	// Interactive Jobs management:
	if (jobShadow!=NULL){
		if (jobShadow->isLocalConsole()){
			if (!nolistenOpt){
				// Interactive console needed
				logInfo->print(WMS_DEBUG,"Running Listener",jobid);
				Listener listener(jobShadow);
				listener.run();
			}
		}
	}
}

/*====================================
	PRIVATE METHODS
==================================== */

/**
* Retrieves the list of InputSandbox URI's for a DAG
*/
std::string JobSubmit::getDagISBURI (const std::string &node){
	vector<string> nodes;
	vector<string>::iterator it1;
	vector<pair<string,string > >::iterator it2;
	string tmp = "";
	string uri = "";
	if (dagAd) {
		// ISB URI for the DAG parent node
		if (node.size() == 0 ){
			if( isbURI.size()==0) {
				uri = dagAd->hasAttribute(JDL::ISB_BASE_URI)?dagAd->getAttribute(ExpDagAd::ISB_DEST_URI ):"";
			} else {
				uri = this->isbURI;
			}
		} else {
			// ISB URI for a DAG child node
			if ( dagISBURIs.empty()){
				// Looks for the all ISB URI in the JDL
				// list of the nodes
				nodes = dagAd->getNodes();
				for (it1 = nodes.begin( ); it1 != nodes.end(); it1++){
					// Checks if the node has ISB attribute
					if (dagAd->hasNodeAttribute(string(*it1), JDL::ISB_BASE_URI)){
						tmp = dagAd->getNodeAttribute(string(*it1), JDL::ISB_BASE_URI );
						dagISBURIs.push_back(make_pair((*it1), tmp));
						// Looks for the ISB URI of the input node
						if (node == string(*it1)){
							uri = uri.assign(tmp);
						}
					}
				}
			} else {
				// Looks for the ISB URI of the node in the filled vector
				for (it2 = dagISBURIs.begin( ); it2 != dagISBURIs.end(); it2++){
					// Looks for the ISB URI of the input node
					if (node == string(it2->first)){
						uri = string(it2->second);
						break;
					}
				}
			}
		}
	}
	return uri;
}
/**
* Retrieves the paths of the local InputSandbox files from the
* from the JDL of a normal job
*/
void JobSubmit::jobISBFiles(vector<string> &paths){
	vector<string> files;
	if (jobAd){
		if (jobAd->hasAttribute(JDL::INPUTSB)){
			files = jobAd->getStringValue(JDL::INPUTSB);
		}
		if (!files.empty()){
			// Gets back in node_files only the local files
			extractFiles(JDL::INPUTSB,
				files,
				paths,
				glite::wms::jdl::ONLYLOCAL,
				"", isbURI );
		}
	}
}
/**
* Retrieves the paths of the local InputSandbox files from the
* from the JDL for a collection
*/
void JobSubmit::collectionISBFiles (vector<string> &paths){
	vector< pair<string ,vector< string > > > nodes;
	vector<string> files;
	if (collectAd && collectAd->hasAttribute(JDL::INPUTSB)){
		files = collectAd->getStringValue(JDL::INPUTSB);
		// Puts in "paths" only the local files
		extractFiles(JDL::INPUTSB,
			files,
			paths,
			glite::wms::jdl::ONLYLOCAL,
			"", isbURI );
	}
}
/**
* Retrieves the paths of the local InputSandbox files from the
* from the JDL for a DAG
*/
void  JobSubmit::dagISBFiles(vector<string> &paths, const bool &children){
	vector<string> files;
	vector<string> nodes;
	vector<string> node_files;
	vector<string>::iterator it;
	if (dagAd){
		// parent's ISB
		if (dagAd->hasAttribute(JDL::INPUTSB)){
			files = dagAd->getInputSandbox();
			// Puts in "paths" only the local files
			extractFiles(JDL::INPUTSB,
				files,
				paths,
				glite::wms::jdl::ONLYLOCAL,
				"", isbURI );
		}
		if (children) {
			// List of the nodes (their names)
			nodes = dagAd->getNodes();

			for (it = nodes.begin( ); it != nodes.end(); ++it){
				if (dagAd->hasNodeAttribute(string(*it), JDL::INPUTSB)){
					// files of the node ( it1= Its name )
					node_files = dagAd->getNodeStringValue((*it), JDL::INPUTSB);
					// Puts in "paths" only the local files
					extractFiles(JDL::INPUTSB,
						node_files,
						paths,
						glite::wms::jdl::ONLYLOCAL,
						"", this->getDagISBURI(*it) );
					node_files.clear( );
				}
			}
		}
	}
}
/**
* Gets the total size (in bytes) of the local files in
* the JDL InputSandbox
*/
int JobSubmit::getInputSandboxSize(const wmsJobType &jobtype){
	vector<string> isb_files ;
	vector<string>::iterator it;
	ostringstream err ;
	string file = "";
	int size = 0 ;
	int fs = 0;
	if (isbSize > 0) {
		size = this->isbSize ;
	} else {
		// Retrieves the local ISB files from the user JDL
		switch (jobtype) {
			case (WMS_JOB) : {
				this->jobISBFiles(isb_files);
				break;
			}
			case WMS_DAG: {
				this->dagISBFiles(isb_files);
				break;
			}
			case WMS_PARAMETRIC: {
				this->dagISBFiles(isb_files, false);
				break;
			}
			case (WMS_COLLECTION) : {
				this->collectionISBFiles(isb_files);
				break;
			}
			default : {
				throw WmsClientException(__FILE__,__LINE__,
					"getInputSandboxSize",  DEFAULT_ERR_CODE ,
					"Uknown JobType",
					"unable to process the job (check the JDL)");
			}
		}
		// Gets the total size of the ISB files
		for (it = isb_files.begin(); it != isb_files.end(); ++it){
			// Remove file protocol string
			file = Utils::normalizeFile(*it);
			// size for one of the files in the ISB list
			fs = Utils::getFileSize(file);
			if (fileProto && *fileProto == Options::TRANSFER_FILES_GUC_PROTO
				&& (fs > MAX_GUC_SIZE || fs > MAX_TAR_SIZE) ){
				err << (*it) << "  (" << fs << " bytes)\n";
				err << "The file exceeds the size limit allowed by:\n";
				if (fs > MAX_GUC_SIZE ){ err << "- " << Options::TRANSFER_FILES_GUC_PROTO << " protocol (" << MAX_GUC_SIZE <<" bytes)\n";}
				if (fs > MAX_TAR_SIZE ){ err << "- tar archive tool (" << MAX_TAR_SIZE <<" bytes)\n";}
			} else if ( fileProto && *fileProto == Options::TRANSFER_FILES_CURL_PROTO
				&& (fs > MAX_CURL_SIZE || fs > MAX_TAR_SIZE)){
				err << (*it) << "  (" << fs << " bytes)\n";
				err << "The file exceeds the size limit allowed by:\n";
				if (fs > MAX_CURL_SIZE ){ err << "- " << Options::TRANSFER_FILES_CURL_PROTO << " protocol (" << MAX_GUC_SIZE <<" bytes)\n";}
				if (fs > MAX_TAR_SIZE ){ err << "- tar archive tool (" << MAX_TAR_SIZE <<" bytes)\n";}
			}
			if (err.str().size()>0){
				throw WmsClientException(__FILE__,__LINE__,
						"getInputSandboxSize",  DEFAULT_ERR_CODE ,
						"File Size Error",
						err.str());
			}
			// adds the size of the file to the total ISB size
			size += fs ;
		}
		this->isbSize = size;
	}
	return size ;
}
/**
* Checks that the total size (in bytes) of the local files in
* the JDL InputSandbox don't exceed the size limit fixed
* on the server side:
* either the user free quota or the max ISB size (if the first one is not set)
*/
void JobSubmit::checkInputSandboxSize (const wmsJobType &jobtype) {
	// results of getFrewQuota
	pair<long, long> free_quota ;
	long isbsize = 0;
	long max_isbsize = 0;
	long limit = 0;
	int tars;
	// ISB size
	isbsize = this->getInputSandboxSize(jobtype);
	if (cfgCxt) {
		// User free quota -----------
		try{
			// Gets the user-free quota from the WMProxy server
			logInfo->print(WMS_DEBUG, "Getting the User FreeQuota from the server", cfgCxt->endpoint);
			free_quota = getFreeQuota(cfgCxt);
		} catch (BaseException &exc){
				throw WmsClientException(__FILE__,__LINE__,
					"checkInputSandboxSize ", ECONNABORTED,
					"WMProxy Server Error", errMsg(exc));
		}
		// soft limit
		limit = free_quota.first;
		// if the previous function gets back a negative number
		// the user free quota is not set on the server and
		// no check is performed (this functions returns not exceed=true)
		if (limit >0) {
			if (isbsize > limit  ) {
				ostringstream err ;
				err << "Not enough User FreeQuota (" << limit << " bytes) on the server for the InputSandbox files (" ;
				err << isbsize << " bytes)";
				throw WmsClientException( __FILE__,__LINE__,
					"checkInputSandboxSize",  DEFAULT_ERR_CODE,
					"UserFreeQuota Error" ,
					err.str());
			} else {
				ostringstream q;
				q << "The InputSandbox size (" << isbsize << " bytes) doesn't exceed the User FreeQuota (" << limit << " bytes)";
				logInfo->print (WMS_DEBUG, q.str(), "File transferring is allowed" );
			}
		} else {
			// User quota is not set on the server: check of Max InputSB size
			logInfo->print (WMS_DEBUG, "The User FreeQuota is not set", "");
			try{
				// Gets the maxISb size from the WMProxy server
				logInfo->print(WMS_DEBUG, "Getting the max ISB size from the server", cfgCxt->endpoint );
				max_isbsize = getMaxInputSandboxSize(cfgCxt);
			} catch (BaseException &exc){
					throw WmsClientException(__FILE__,__LINE__,
						"checkInputSandboxSize", ECONNABORTED,
						"WMProxy Server Error", errMsg(exc));
			}
			// MAX ISB size -----------
			if (max_isbsize>0 ) {
				if (isbsize > max_isbsize) {
					ostringstream err ;
					err << "The size of the InputSandbox (" << isbsize <<" bytes) ";
					err << "exceeds the MAX InputSandbox size limit on the server (" << max_isbsize << " bytes)";
					throw WmsClientException( __FILE__,__LINE__,
						"checkInputSandboxSize",  DEFAULT_ERR_CODE,
						"InputSandboxSize Error" , err.str());
				} else {
					ostringstream q;
					q << "The InputSandbox size (" << isbsize << " bytes) doesn't exceed the max size limit of " << max_isbsize << " bytes: ";
					logInfo->print (WMS_DEBUG, q.str(), "file transferring is allowed" );
				}
			} else {
				// User quota is not set on the server: check of Max InputSB size
				logInfo->print (WMS_DEBUG, "The max ISB size is not set on the server", "");
			}
		}
	}
	// tar.gz files ---------
	if (isbsize>0 ) {
		// Number of the tar.gz files to be created
		if (isbsize > MAX_TAR_SIZE) {
			tars = isbsize / MAX_TAR_SIZE  ;
			if ((isbsize % MAX_TAR_SIZE )>0)  {tars++;}
		} else {
			tars = 1;
		}
		// Unique string to be used for the name of the tar.gz files
		string * us = Utils::getUniqueString( );
		if (us==NULL){
			ostringstream u ;
			u << getpid( ) << "_" << getuid( );
			us = new string(u.str());
		}
		// "Saves" into a vector the name of the tar.gz files
		for (int i = 0; i < tars ; i++) {
			ostringstream f ;
			f << ISBFILE_DEFAULT << "_" << *us << "_" << i <<Utils::getArchiveExtension( ) << Utils::getZipExtension( ) ;
			gzFiles.push_back( f.str( ) );
		}
	}
}

/**
*  Checks the user JDL
*/
void JobSubmit::checkAd(bool &toBretrieved, wmsJobType &jobtype){
	vector<string>::iterator it ;
	string message = "";
	jobtype = WMS_JOB;
	toBretrieved =true ;
	glite::wms::common::configuration::WMCConfiguration* wmcConf =wmcUtils->getConf();
	if (collectOpt) {
		logInfo->print (WMS_DEBUG, "A collection of jobs is being submitted; JDL files in:",
			Utils::getAbsolutePath( *collectOpt));
		jobtype = WMS_COLLECTION ;
		try {
			//fs::path cp ( Utils::normalizePath(*collectOpt), fs::system_specific); // Boost 1.29.1
			fs::path cp ( Utils::normalizePath(*collectOpt));
			if ( fs::is_directory( cp ) ) {
				*collectOpt= Utils::addStarWildCard2Path(*collectOpt);
			}
		} catch ( const fs::filesystem_error & ex ){
			throw WmsClientException(__FILE__,__LINE__,
				"submission",  DEFAULT_ERR_CODE,
				"Invalid JDL collection Path",
				ex.what()  );
		}
		collectAd = AdConverter::createCollectionFromPath (*collectOpt);
		collectAd->setLocalAccess(true);
		// Simple Ad manipulation
		AdUtils::setDefaultValuesAd(collectAd,wmcConf);
		// Collect Ad manipulation
		AdUtils::setDefaultValues(collectAd,wmcConf);
		if (nodesresOpt) {
			collectAd->setAttribute(JDL::SUBMIT_TO, *nodesresOpt);
		}
		// JDL string
		collectAd = collectAd->check();
		toBretrieved =collectAd->gettoBretrieved();
		if (toBretrieved){
			// InputSB URI
			isbURI = collectAd->hasAttribute(JDL::ISB_BASE_URI)?collectAd->getString(JDL::ISB_BASE_URI):"";
			// Checks the size of the ISB
			this->checkInputSandboxSize (jobtype);
			// checks if file archiving and compression is allowed
			if (wmpVersion > WMPROXY_OLD_VERSION) {

				// checks if the file archiving and compression is denied (if ALLOW_ZIPPED_ISB is not present, default value is FALSE)
				if (collectAd->hasAttribute(JDL::ALLOW_ZIPPED_ISB)){
					zipAllowed = collectAd->getBool(JDL::ALLOW_ZIPPED_ISB) ;
					if (zipAllowed) { message ="allowed by user in the JDL";}
					else { message ="disabled by user in the JDL"; }
					// Adds the ZIPPED_ISB attribute to the JDL (with the list of tar.gz files)
					if (zipAllowed) {
						for (it = gzFiles.begin(); it !=gzFiles.end(); it++){
							collectAd->addAttribute(JDLPrivate::ZIPPED_ISB, (*it));
						}
					}
				} else {
					// Default value if the JDL attribute is not present
					zipAllowed = false;
					message ="disabled by default";
					// adds the attribute with the default value (FALSE)
					collectAd->addAttribute(JDL::ALLOW_ZIPPED_ISB, false);
				}
				logInfo->print (WMS_DEBUG, "File archiving and file compression", message);
			} else {
				logInfo->print (WMS_DEBUG, "The WMProxy server doesn't support file archiving and file compression", "");
				if (collectAd->hasAttribute(JDL::ALLOW_ZIPPED_ISB)) {
					collectAd->delAttribute(JDL::ALLOW_ZIPPED_ISB);
				}
				collectAd->addAttribute(JDL::ALLOW_ZIPPED_ISB, false);
			}

		}
		jdlString = new string(collectAd->toString());
	} else {
		// ClassAd
		jobAd = new Ad();
		if (! jdlFile){
			throw WmsClientException(__FILE__,__LINE__,
				"checkAd",  DEFAULT_ERR_CODE,
				"JDL File Missing",
				"uknown JDL file pathame (Last Argument of the command must be a JDL file)"   );
		}
		logInfo->print (WMS_DEBUG, "The JDL files is:", Utils::getAbsolutePath(*jdlFile));
		jobAd->fromFile (*jdlFile);
		// Adds ExpireTime JDL attribute
		if ((int)expireTime>0) {
			jobAd->addAttribute (JDL::EXPIRY_TIME, (double)expireTime);
		}
		// Simple Ad manipulation (common)
		AdUtils::setDefaultValuesAd(jobAd,wmcConf);
		// checks if file archiving and compression is allowed
		if (wmpVersion > WMPROXY_OLD_VERSION) {
			if (jobAd->hasAttribute(JDL::ALLOW_ZIPPED_ISB)){
				zipAllowed = jobAd->getBool(JDL::ALLOW_ZIPPED_ISB) ;
				if (zipAllowed) { message ="allowed by user in the JDL";}
				else { message ="disabled by user in the JDL"; }
			} else {
				// Default value if the JDL attribute is not present
				zipAllowed = false;
				message ="disabled by default";
				jobAd->addAttribute(JDL::ALLOW_ZIPPED_ISB, false);
			}
			logInfo->print (WMS_DEBUG, "File archiving and file compression", message);
		} else {
			logInfo->print (WMS_DEBUG, "The WMProxy server doesn't support file archiving and file compression", "");
			if (jobAd->hasAttribute(JDL::ALLOW_ZIPPED_ISB)) {
				jobAd->delAttribute(JDL::ALLOW_ZIPPED_ISB);
			}
			jobAd->addAttribute(JDL::ALLOW_ZIPPED_ISB, false);
		}
		// COLLECTION ========================================
		if ( jobAd->hasAttribute(JDL::TYPE , JDL_TYPE_COLLECTION) ) {
			logInfo->print (WMS_DEBUG, "A collection of jobs is being submitted", "");
			jobtype = WMS_COLLECTION ;
			try{
				collectAd = new CollectionAd(*(jobAd->ad()));
				collectAd->setLocalAccess(true);
				// Collect Ad manipulation
				AdUtils::setDefaultValues(collectAd,wmcConf);
				if (nodesresOpt) {
					collectAd->setAttribute(JDL::SUBMIT_TO, *nodesresOpt);
				}
				// JDL string
				collectAd = collectAd->check();
				toBretrieved =collectAd->gettoBretrieved();
				if (toBretrieved){
					// InputSB URI
					isbURI = collectAd->hasAttribute(JDL::ISB_BASE_URI)?collectAd->getString(JDL::ISB_BASE_URI):"";
					// Checks the size of the ISB
					this->checkInputSandboxSize (jobtype);
					// Adds the ZIPPED_ISB attribute to the JDL (with the list of tar.gz files)
					if (zipAllowed) {
						for (it = gzFiles.begin(); it !=gzFiles.end(); it++){
							collectAd->addAttribute(JDLPrivate::ZIPPED_ISB, (*it));
						}
					}
				}
				jdlString = new string(collectAd->toString());
			}catch (Exception &ex){
				throw WmsClientException(__FILE__,__LINE__,
					"submission",  DEFAULT_ERR_CODE,
					"Invalid JDL collection",
					ex.what()   );
			}
		}  else
		// DAG  ========================================
		if ( jobAd->hasAttribute(JDL::TYPE , JDL_TYPE_DAG) ) {
				logInfo->print (WMS_DEBUG, "A DAG job is being submitted", "");
				jobtype = WMS_DAG ;
				if (nodesresOpt) {
					jobAd->setAttribute(JDL::SUBMIT_TO, *nodesresOpt);
				}
				dagAd = new ExpDagAd (jobAd->toString());
				dagAd->setLocalAccess(true);
				AdUtils::setDefaultValues(dagAd,wmcConf);
				// expands the DAG loading all JDL files
				dagAd->getSubmissionStrings();
				toBretrieved=dagAd->gettoBretrieved();
				if (toBretrieved) {
					// InputSB URI of the parent node
					isbURI = getDagISBURI ( );
					// checks the size of the ISB
					this->checkInputSandboxSize (jobtype);
					if (zipAllowed){
						// Adds the ZIPPED_ISB attribute to the JDL (with the list of tar.gz files)
						dagAd->setAttribute(ExpDagAd::ZIPPED_ISB, gzFiles);
					}
				}
				// JDL string for the DAG
				jdlString = new string(dagAd->toString()) ;
		}else{
			jobtype = WMS_JOB ;
			// resource <ce_id> ----> SubmitTo JDL attribute
			if (resourceOpt) {
				if (jobAd->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_PARTITIONABLE)){
					throw WmsClientException(__FILE__,__LINE__,
						"checkAd",  DEFAULT_ERR_CODE,
						"Incompatible Argument: " + wmcOpts->getAttributeUsage(Options::RESOURCE),
						"cannot be used for  DAG, collection, partitionable and parametric jobs");
				}else{jobAd->setAttribute(JDL::SUBMIT_TO, *resourceOpt);}
			}
			// INTERACTIVE =================================
			if (  jobAd->hasAttribute(JDL::JOBTYPE , JDL_JOBTYPE_INTERACTIVE )  ){
				// Interactive Job management
				logInfo->print (WMS_DEBUG, "An interactive job is being submitted.", "");
				jobShadow = new Shadow();
				jobShadow->setPrefix(wmcUtils->getPrefix()+"/bin");
				// Insert jdl attributes port/pipe/host inside shadow(if present)
				if (jobAd->hasAttribute(JDL::SHPORT)){
					jobShadow->setPort(jobAd->getInt ( JDL::SHPORT ));
				}
				if (jobAd->hasAttribute(JDL::SHPIPEPATH)){
					jobShadow->setPipe(jobAd->getString(JDL::SHPIPEPATH));
				}else{
					jobAd->setAttribute(JDL::SHPIPEPATH,jobShadow->getPipe());
				}
				if (jobAd->hasAttribute(JDL::SHHOST)){
					jobShadow->setHost(jobAd->getString(JDL::SHHOST));
				}else{
					jobAd->setAttribute(JDL::SHHOST,jobShadow->getHost());
				}
				// Launch console
				if (jobShadow->isLocalConsole()){
					logInfo->print(WMS_DEBUG,"Running console shadow","");
					jobShadow->console();
					logInfo->print(WMS_DEBUG,"Console properly started","");
					// Insert listenin port number (if necessary replace old value)
					if (jobAd->hasAttribute(JDL::SHPORT)){jobAd->delAttribute(JDL::SHPORT);}
					jobAd->setAttribute(JDL::SHPORT,jobShadow->getPort()) ;
				}
			}
			// MPICH ==================================================
			if (  jobAd->hasAttribute(JDL::JOBTYPE,JDL_JOBTYPE_MPICH)){
				// MpiCh Job:
				if (lrmsOpt){
					// Override previous value (if present)
					if (jobAd->hasAttribute(JDL::LRMS_TYPE)){jobAd->delAttribute(JDL::LRMS_TYPE);}
					jobAd->setAttribute(JDL::LRMS_TYPE,*lrmsOpt);
				}
			}
			JobAd *pass= new JobAd(*(jobAd->ad()));
			AdUtils::setDefaultValues(pass,wmcConf);
			// check JobAd without restoring attributes
			pass->check(false);
			// InputSandbox Files
			toBretrieved=pass->gettoBretrieved();
			// PARAMETRIC  ===============================================
			if (  jobAd->hasAttribute(JDL::JOBTYPE,JDL_JOBTYPE_PARAMETRIC)){
				jobtype = WMS_PARAMETRIC;
				if (nodesresOpt) {
					pass->setAttribute(JDL::SUBMIT_TO, *nodesresOpt);
				}
				dagAd=AdConverter::bulk2dag(pass);
				AdUtils::setDefaultValues(dagAd, wmcConf);
				dagAd->getSubmissionStrings();
				toBretrieved = dagAd->gettoBretrieved();
				if (toBretrieved){
					// isbURI is needed by checkInputSandboxSize
					isbURI = getDagISBURI( );
				}
			} else {
				if (toBretrieved){
					// isbURI is needed by checkInputSandboxSize
					isbURI=jobAd->hasAttribute(JDL::ISB_BASE_URI)?jobAd->getString(JDL::ISB_BASE_URI):"";
				}
			}
			// ZIP ISB file(s) Management
			if (toBretrieved){
				// Checks the size of the ISB
				this->checkInputSandboxSize (jobtype);
				if (zipAllowed) {
					// Adds the ZIPPED_ISB attribute to the JDL
					for (it = gzFiles.begin(); it !=gzFiles.end(); it++){
						pass->addAttribute(JDLPrivate::ZIPPED_ISB, (*it));
					}
				}
				if (jobtype==WMS_PARAMETRIC){
					jdlString = new string(pass->toString());
				}else if  (jobtype==WMS_JOB){
					jdlString = new string(pass->toSubmissionString());
				}
			}
			delete(pass);
		}
	}
	// --resource : incompatible argument
	if( (resourceOpt) && (jobtype != WMS_JOB)){
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
			"Incompatible Argument: " + wmcOpts->getAttributeUsage(Options::RESOURCE),
			"cannot be used for  DAG, collection, partitionable and parametric jobs");
	} else if (resourceOpt) {
		logInfo->print (WMS_DEBUG, "--resource: The job will be submitted to this resource: ", *resourceOpt );
	}else if( (nodesresOpt) && (jobtype == WMS_JOB)){
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
			"Incompatible Argument: " + wmcOpts->getAttributeUsage(Options::NODESRES),
			"cannot be used for jobs");
	}
	// if --nolisten has been selected for a not interactive job
	if (nolistenOpt && jobShadow==NULL) {
		logInfo->print (WMS_WARNING, "--nolisten: option ignored (the job is not interactive)\n", "", true );
	}
}
/**
*Performs:
*	- Job registration when --register-only is selected
*	-  Job submission otherwise
*/
std::string JobSubmit::jobRegOrSub(const bool &submit) {
	// checks if jdlstring is not null
	if (!jdlString){
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE ,
			"Null Pointer Error",
			"null pointer to JDL string");
	}

	// checks if jdlstring is not null
	if (!endPoint){
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE ,
			"Null Pointer Error",
			"null pointer to endPoint string");
	}
	// jobRegister
	logInfo->print(WMS_INFO, "Connecting to the service", this->getEndPoint());
			string method ;
			if (submit){ method = "submit";}
			else {method = "register";}
	try{
		if (submit){
			logInfo->print(WMS_DEBUG, "Submitting JDL", *jdlString);
			//Suibmitting....
			jobIds = jobSubmit(*jdlString, *dgOpt, cfgCxt);
		}else {
			logInfo->print(WMS_DEBUG, "Registering JDL", *jdlString);
			// registering ...
			jobIds = jobRegister(*jdlString , *dgOpt, cfgCxt);
		}
	} catch (BaseException &exc) {
		ostringstream err ;
		err << "Unable to "<< method << " the job to the service: " << this->getEndPoint()<< "\n";
		err << errMsg(exc) ;
		// in case of any error on the only specified endpoint
		throw WmsClientException(__FILE__,__LINE__,
			"job"+method, ECONNABORTED,
			"Operation failed", err.str());
	}


	return jobIds.jobid;
}

/**
* Performs Job start when:
*	- start option is selected
*	- job has been already registered and ready to be started
*/

void JobSubmit::jobStarter(const std::string &jobid ) {
	vector<string> urls ;
	int index = 0;
	// flag to stop while-loop
	bool success = false;
	// number of enpoint URL's
	int n = 0;
	// checks if ConfigContext already contains the WMProxy URL
	if (endPoint){
		urls.push_back(*endPoint);
	} else {
		// list of endpoints from configuration file
		urls = wmcUtils->getWmps ( );
	}
	// initial number of Url's
	n = urls.size( );
	if (!cfgCxt){
		cfgCxt = new ConfigContext("", "", "");
	}
	while ( ! urls.empty( ) ){
		int size = urls.size();
		if (size > 1){
			// randomic extraction of one URL from the list
			index = wmcUtils->getRandom(size);
		} else{
			index = 0;
		}
		// endpoint URL
		endPoint = new string(urls[index]);
		// setting of the EndPoint ConfigContext field
		cfgCxt->endpoint=urls[index];
		// Removes the extracted URL from the list
		urls.erase ( (urls.begin( ) + index) );
		// Prints this message only if the --start option has been requested
		// (without --start it has been already printed before the job registration)
		if (startOpt){ logInfo->print(WMS_INFO, "Connecting to the service", *endPoint);}
		// JobStart if --register-only has not been requested
		try {
			// START
			logInfo->print(WMS_DEBUG, "Starting the job: " , jobid);
			jobStart(jobid, cfgCxt);
			success = true;
		} catch (BaseException &exc) {
			if (n==1) {
				ostringstream err ;
				err << "Unable to start the job to the service: " << *endPoint << "\n";
				err << errMsg(exc) ;
				// in case of any error on the only specified endpoint
				throw WmsClientException(__FILE__,__LINE__,
					"jobStart", ECONNABORTED,
					"Operation failed", err.str());
			} else {
				logInfo->print  (WMS_INFO, "Connection failed:", errMsg(exc));
				sleep(1);
				if (urls.empty( )){
					throw WmsClientException(__FILE__,__LINE__,
					"jobStart", ECONNABORTED,
					"Operation failed",
					"Unable to start the job to any specified endpoint");
				}
			}
		}
		// exits from the loop in case of success
		if (success){ break;}
	}
}
/**
*       Contacts the endpoint configurated in the context
*       in order to retrieve the list of the destionationURIs of the job
*       identified by the jobid
*	(for compund jobs it gets back the URIs of the parents and all its children nodes)
*/

std::string* JobSubmit::getBulkDestURI(const std::string &jobid, const std::string &child, std::string &zipURI) {
	string *dest_uri = NULL;
	string look_for = "";
	vector<string> jobids;
	vector< pair<string ,vector<string > > >::iterator it1 ;
	vector<string>::iterator it2;
	bool found = false;
	// The destinationURI's vector is empty: the WMProxy service is called
	if (dsURIs.empty( )){
		try{
			logInfo->print(WMS_DEBUG, "Getting the SandboxBulkDestinationURI from the service" , cfgCxt->endpoint);
			dsURIs = getSandboxBulkDestURI(jobid, (ConfigContext *)cfgCxt);
		} catch (BaseException &exc){
			throw WmsClientException(__FILE__,__LINE__,
				"getSandboxDestURI ", ECONNABORTED,
				"WMProxy Server Error", errMsg(exc));
		}
		if (dsURIs.empty( )){
			throw WmsClientException(__FILE__,__LINE__,
				"getBulkDestURI ", ECONNABORTED,
				"WMProxy Server Error",
				"The server doesn't have any information on InputSBDestURI for :" + jobid );
		}
	}
	if (child.size()>0){
		// if the input parameter child is set ....
		look_for = child ;
	} else {
		// parent (if the input string "child" is empty)
		look_for = jobid;
	}
	// Looks for the destURI's of the job
	for (it1 = dsURIs.begin() ; it1 != dsURIs.end() ; it1++) {
		if (it1->first == look_for) { // parent or child found
			for (it2 = (it1->second).begin() ; it2 !=  (it1->second).end() ; it2++) {
				// Looks for the destURi for file transferring
				if ( it2->substr (0, (fileProto->size())) ==  *fileProto){
					dest_uri = new string( *it2 );
					// Prints out the info on URI's
					// (In compound jobs URI's of children nodes are only written in the log file, if it exists)
					if (jobid.compare(look_for) == 0) {
						logInfo->print(WMS_DEBUG,  "DestinationURI:  " +*dest_uri, "");
					} else {
						logInfo->print(WMS_DEBUG,  "Child node : " + child, " - DestinationURI : " + *dest_uri, false);
					}
					// loop-exit if "TAR/ZIP-destURI" has been already found
					if (found){ break; }
					else { found = true ;}
				}
				// Looks for the destURI for TAR/ZIP file creation
				if ( it2->substr (0, (Options::DESTURI_ZIP_PROTO.size()) ) == Options::DESTURI_ZIP_PROTO){
					zipURI = string (*it2);
					// loop-exit if "fileTransfer-destURI" has been already found
					if (found){ break; }
					else { found = true ;}
				}
			}
		}
		if (found) break;
	}
	return dest_uri ;
}
/**
* Gets the InputSandboxURI for a job or one of it child node
*/
std::string* JobSubmit::getSbDestURI(const std::string &jobid, const std::string &child, std::string &zipURI) {
	vector<string> uris ;
	vector<string>::iterator it1 ;
	string *dest_uri = NULL;
	bool parent = false;
	bool found = false;
	try {
		if (child.size()>0){
			logInfo->print(WMS_DEBUG, "Getting the SandboxDestinationURI for child node: " , child);
			// if the input parameter child is set ....
			uris = getSandboxDestURI(child, (ConfigContext *)cfgCxt);
		} else {
			logInfo->print(WMS_DEBUG, "Getting the SandboxDestinationURI from the service" , cfgCxt->endpoint);
			// parent (if the input string "child" is empty)
			uris = getSandboxDestURI(jobid, (ConfigContext *)cfgCxt);
			parent = true;

		}
	} catch (BaseException &exc){
		throw WmsClientException(__FILE__,__LINE__,
			"getSbDestURI ", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
	}
	// Looks for the destURI's of the job
	for (it1 =uris.begin() ; it1 != uris.end() ; it1++) {
		// Looks for the destURi for file transferring
		if ( it1->substr (0, (fileProto->size())) ==  *fileProto){
			dest_uri = new string( *it1 );
			if (parent) {
				logInfo->print(WMS_DEBUG,  "DestinationURI:  " +*dest_uri, "");
			} else {
				logInfo->print(WMS_DEBUG,  "Child node : " + child, " - DestinationURI : " + *dest_uri);
			}
			// loop-exit if "TAR/ZIP-destURI" has been already found
			if (found){ break; }
			else { found = true ;}
		}
		// Looks for the destURI for TAR/ZIP file creation
		if (it1->substr (0, (Options::DESTURI_ZIP_PROTO.size()) ) == Options::DESTURI_ZIP_PROTO){
			zipURI = string (*it1);
			// loop-exit if "fileTransfer-destURI" has been already found
			if (found){ break; }
			else { found = true ;}
		}
	}
	return dest_uri ;
}

/*
* According to the version of the WMProxy, the DestinationURI is retrieved
* either with the "Bulk" service or with the "single-node" service.
* For compund jobs, the WMProxy "Bulk" method gets back one-shot a list
* with the URI's of the parent and all its children nodes; instead with the other method,
* the WMProxy in each call can only get back the URIs for one
* node
*/
std::string* JobSubmit::getInputSbDestinationURI(const std::string &jobid, const std::string &child, std::string &zipURI ) {
	if (wmpVersion  > WMPROXY_OLD_VERSION) {
		// bulk service
		return getBulkDestURI(jobid, child, zipURI);
	} else {
		// single node service
		return getSbDestURI(jobid, child, zipURI);
	}
}

/**
* File transferring by globus-url-copy (gsiftp protocol)
*/

void JobSubmit::gsiFtpTransfer(std::vector<std::pair<std::string,std::string> > &paths) {
	string protocol = "";
	pair<string,string> it ;
	//TBDMS: globus-url-copy searched several times
	while (!paths.empty()) {
		it = paths[0];
		// Protocol has to be added only if not yet present
		protocol = (it.first.find("://")==string::npos)?FILE_PROTOCOL:"";
		// command
		string cmd= "globus-url-copy " + string (protocol+it.first) + " " + string( it.second );
		if (getenv("GLOBUS_LOCATION")){
			cmd=string(getenv("GLOBUS_LOCATION"))+"/bin/"+cmd;
		}else if (Utils::isDirectory ("/opt/globus/bin")){
			cmd="/opt/globus/bin/"+cmd;
		}else {
			throw WmsClientException(__FILE__,__LINE__,
				"gsiFtpGetFiles", ECONNABORTED,
				"File Transferring Error",
				"Unable to find globus-url-copy executable");
		}
		logInfo->print(WMS_DEBUG, "File Transferring (gsiftp)\n" , cmd);
	if ( system( cmd.c_str() ) ){
			ostringstream info;
			info << it.first << "\n";
			info << "to " << it.second;
			throw WmsClientException(__FILE__,__LINE__,
				"gsiFtpTransfer", ECONNABORTED,"File Transferring Error",
				"unable to transfer the file " + info.str());
		} else{
			logInfo->print(WMS_DEBUG, "File Transferring (gsiftp)", "TRANSFER DONE");
			// Removes the zip file just transferred
			if (zipAllowed) {
				try {
		//			Utils::removeFile(it.first);
				} catch (WmsClientException &exc) {
					logInfo->print (WMS_WARNING,
						"The following error occured during the removal of the file:",
						exc.what());
				}
			}
			paths.erase(paths.begin());
		}
	}
}
/*
* File transferring by CURL (https protocol)
*/

void JobSubmit::curlTransfer (std::vector<std::pair<std::string,std::string> > paths) {
	// curl struct
	CURL *curl = NULL;
	// curl result code
	CURLcode res;
	// file struct
	FILE * hd_src  = NULL;
	struct stat file_info;
	int fsize = 0;
	int hd = 0;
	// local filepath
	string file = "" ;
	// destination
	string destination = "";
	// result message
	long	httpcode = 0;
	if ( !paths.empty() ){
		// checks the user proxy pathname
		if (!proxyFile){
			throw WmsClientException(__FILE__,__LINE__,
				"curlTransfer", DEFAULT_ERR_CODE,
				"Missing Proxy",
				"unable to determine the proxy file" );
		}
		// checks the trusted cert dir
		if (!trustedCert){
			throw WmsClientException(__FILE__,__LINE__,
				"curlTransfer", DEFAULT_ERR_CODE,
				"Directory Not Found",
				"unable to determine the trusted certificate directory" );
		}
		// curl init
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();
		if(curl) {
			if ( wmcOpts->getBoolAttribute(Options::DBG) ) {
				curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			}
			// curl options: proxy
			curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
			curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE,   "PEM");
			curl_easy_setopt(curl, CURLOPT_SSLCERT, proxyFile);
			curl_easy_setopt(curl, CURLOPT_SSLKEY, proxyFile);
			curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, NULL);
			// curl option: trusted cert dir
			curl_easy_setopt(curl, CURLOPT_CAPATH, trustedCert);
			// curl options: no verify the ssl properties
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
			// enable uploading
			curl_easy_setopt(curl, CURLOPT_PUT, 1);
			// name of the file to be transferred
			while(!paths.empty()){
				// path to local file (to be transferred)
				file = Utils::normalizeFile(paths[0].first);
				// destinationURI where to transfer the file
				destination = paths[0].second ;
				// curl options: source (first element of the vector)
				hd_src = fopen(file.c_str(), "rb");
				if (hd_src == NULL) {
					throw WmsClientException(__FILE__,__LINE__,
						"curlTransfer",  DEFAULT_ERR_CODE,
						"File Not Found", "no such file : " + file );
				}
				// Reads the local file
				curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
				// curl options: file size
				hd = open(file.c_str(), O_RDONLY) ;
				fstat(hd, &file_info);
				close(hd) ;
				fsize = file_info.st_size;
				if (fsize < 0){
					ostringstream warn;
					warn << file << ": Invalid  File Size (" << fsize << "bytes)";
					logInfo->print(WMS_WARNING, warn.str(), file);
				} else {
					curl_easy_setopt(curl,CURLOPT_INFILESIZE, file_info.st_size);
					// curl options: destination (the 2nd elemnt of the vector)
					curl_easy_setopt(curl,CURLOPT_URL, destination.c_str());
					// log-debug message
					ostringstream info ;
					info << "\nInputSandbox file : " << file << "\n";
					info << "destination URI : " << destination ;
					info << "\nsize : " << fsize << " byte(s)"<< "\n";
					logInfo->print(WMS_DEBUG, "InputSandbox File Transferring", info.str());
					// FILE TRANSFERRING ------------------------------
					res = curl_easy_perform(curl);
					// result
					if ( res == 0 ){
						// SUCCESS !!!
						// Removes the zip file just transferred
						if (zipAllowed) {
							try {
								Utils::removeFile(file);
							} catch (WmsClientException &exc) {
								logInfo->print (WMS_WARNING,
								"The following error occured during the removal of the file:",
								exc.what());
							}
						}
						// Remove the file info from the vector
						paths.erase(paths.begin());
					} else {
						// ERROR !!!
						// An error occurred during the file transferring
						ostringstream err;
						curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &httpcode);
						err << "couldn't transfer the InputSandbox file : " <<file ;
						err << "to " << destination << "\nhttp error code: " << httpcode ;
						throw WmsClientException(__FILE__,__LINE__,
								"curlTransfer",  DEFAULT_ERR_CODE,
								"File Transfer Error", err.str()  );
					}
				}
				fclose(hd_src);
			}
			// cleanup
			curl_easy_cleanup(curl);
		}
	} else {
		logInfo->print (WMS_DEBUG, "JDL_INPUTSB", "No local InputSB files to be transferred");
	}
}


std::string* JobSubmit::toBCopiedFileList(const std::string &jobid,
						const std::string &child,
						const std::string &isb_uri,
						const std::vector <std::string> &paths,
						std::vector <std::pair<std::string, std::string> > &to_bcopied){
	string* dest_uri = NULL;
	string zip_uri = "";
	if (!paths.empty()){
		dest_uri = getInputSbDestinationURI(jobid, child, zip_uri);
		if (!dest_uri){
			throw WmsClientException(__FILE__,__LINE__,"getInputSbDestinationURI",DEFAULT_ERR_CODE,
			"Missing Information","unable to retrieve the InputSB DestinationURI for the job: " +jobid  );
		}
		if (zipAllowed) {
			// Gets the InputSandbox files to be included into tar.gz file to be transferred to the server
			// ("zip_uri" is needed to create the file paths into the tar file that will be transferred to "dest_uri")
			toBcopied(JDL::INPUTSB, paths, to_bcopied, zip_uri, isb_uri);
		} else {
			// Gets the InputSandbox files to be transferred to the server
			// (The files will be directly transferred to "dest_uri")
			toBcopied(JDL::INPUTSB, paths, to_bcopied, *dest_uri, isb_uri);
		}
	}
	return dest_uri ;
}

/*
* Archives and compresses the InputSandbox files
*/
void JobSubmit::createZipFile (std::vector <std::pair<std::string, std::string> > &to_bcopied, const std::string &destURI){
	vector <pair<string, string> >::iterator it ;
	int r = 0;
	TAR *t =NULL;
	tartype_t *type = NULL ;
	string file = "";
	string path = "";
	string tar = "";
	bool check_size;
	int tar_size = 0;
	int index = 0;
	// number of tar.gz file to be created
	int nf = gzFiles.size( );
	if (nf > 0) {
		if (nf  > 1){ check_size = true;}
		tar = TMP_DEFAULT_LOCATION + "/" + Utils::getArchiveFilename (gzFiles[index] );
		r = tar_open ( &t,  (char*)tar.c_str(), type,
			O_CREAT|O_WRONLY,
			S_IRWXU, TAR_GNU |  TAR_NOOVERWRITE  );
		if ( r != 0 ){
			throw WmsClientException(__FILE__,__LINE__,
			"tar_open",  DEFAULT_ERR_CODE,
			"File i/o Error", "Unable to create tar file for InputSandbox: " + tar );
		}
		for (it = to_bcopied.begin( ); it != to_bcopied.end( ); ++it){
			// local file to add to the archive
			file = Utils::normalizeFile(it->first);
			if (check_size){
				tar_size += Utils::getFileSize(file);
				if (tar_size > MAX_TAR_SIZE){
					tar_append_eof(t);
					tar_close (t);
					t = NULL ;
					Utils::compressFile(tar);
					tar = TMP_DEFAULT_LOCATION + "/" + Utils::getArchiveFilename( gzFiles[++index] );
					r = tar_open ( &t,  (char*)tar.c_str(), type,
						O_CREAT|O_WRONLY,
						S_IRWXU, TAR_GNU |  TAR_NOOVERWRITE  );
					if ( r != 0 ){
						throw WmsClientException(__FILE__,__LINE__,
						"tar_open",  DEFAULT_ERR_CODE,
						"File i/o Error", "Unable to create tar file for InputSandbox: " + tar );
					}
				}
			}
			// name of the file in the archive
			path = Utils::getAbsolutePathFromURI (it->second);
			r = tar_append_file (t, (char*) file.c_str(), (char*)path.c_str());
			if (r!=0){
				string m = "Error in adding the file "+ file+ " to " + tar ;
				char* em = strerror(errno);
				if (em) { m += string("\n(") + string(em) + ")"; }
				throw WmsClientException(__FILE__,__LINE__,
					"archiveFiles",  DEFAULT_ERR_CODE,
					"File i/o Error",
					"Unable to create tar file - " + m);
			}
		}
		if (t) {
			tar_append_eof(t);
			tar_close (t);
			Utils::compressFile (tar) ;
		}
		to_bcopied.clear( );
		index++;
		for (int i = 0; i < index; i++) {
			// Inserts in the vector the zips file to be transferred to the server
			// pair : first=the location of the gzip file to be transferred ;
			// second = the destionationURI
			to_bcopied.push_back(make_pair(TMP_DEFAULT_LOCATION + "/" + gzFiles[i] , string(destURI + "/" + gzFiles[i]) ) );
		}
	}
}

/*
* Message for InputSB files that need to be transferred
*/
std::string JobSubmit::transferFilesList(std::vector<std::pair<std::string,std::string> > &paths, const std::string& jobid){
	std::vector<std::pair<std::string,std::string> >::iterator it ;
	ostringstream info;
	if (paths.empty( ) ) {
		info << "To complete the submission:\n";
		info << "- no local file in the InputSandbox files to be transferred\n";
		info << "- ";
	} else {
		info << "To complete the operation, the following InputSandbox files need to be transferred:\n";
		info << "==========================================================================================================\n";
		for (it = paths.begin(); it != paths.end( ); ++it) {
			info << "InputSandbox file : " << it->first << "\n";
			info << "Destination URI : " << it->second << "\n";
			info << "-----------------------------------------------------------------------------\n";
		}
		info << "\nthen " ;
	}
	info << "start the job by issuing a submissiong with the option:\n --start " << jobid << "\n";
	return info.str();
}
/*
* Transfers the ISB file to the endpoint
*/
void JobSubmit::transferFiles(std::vector<std::pair<std::string,std::string> > &to_bcopied, const std::string &destURI, const std::string &jobid){
	try {
		// Creates a zip file with the ISB files to be transferred if file compression is allowed
		if (zipAllowed) {
			createZipFile(to_bcopied, destURI);
		}
		// File Transferring according to the chosen protocol
		if (fileProto && *fileProto == Options::TRANSFER_FILES_CURL_PROTO ) {
			this->curlTransfer (to_bcopied);
		} else {
			this->gsiFtpTransfer (to_bcopied);
		}
	} catch (WmsClientException &exc) {
		ostringstream err ;
		err << exc.what() << "\n\n";
		err << transferFilesList (to_bcopied, jobid) << "\n";
		throw WmsClientException( __FILE__,__LINE__,
				"transferFiles",  DEFAULT_ERR_CODE,
				"File Transferring Error" ,
				err.str());
	}
}
/**
*  Reads the JobRegister results for a normal job and checks if there are local files
*  in the InputSandbox to be transferred to the WMProxy server.
* In case of file transferring is not requested (only job registration has to be performed),
* an info message with the list these file is provided
*/
std::string JobSubmit::normalJob( ){
	// jobid and nodename
	string jobid = "";
	string node = "";
	// DestinationURI string (for file transferring)
	string *destURI = NULL;
	// InputSB files
	vector <string> paths ;
	vector <pair<string, string> > to_bcopied ;
	// gzip file
	string gzip = "";
	if (!jobAd){
		throw WmsClientException(__FILE__,__LINE__,
			"JobSubmit::normalJob",  DEFAULT_ERR_CODE,
			"Null Pointer Error",
			"null pointer to Ad object" );
	}
	// JOB-ID
	jobid = jobIds.jobid ;
	Utils::checkJobId(jobid);
	// INPUTSANDBOX
	if (jobAd->hasAttribute(JDL::INPUTSB)){
		paths = jobAd->getStringValue  (JDL::INPUTSB) ;
		// InputSandbox file transferring
		destURI = this->toBCopiedFileList(jobid, "", this->isbURI, paths, to_bcopied);
		// if the vector is not empty, file transferring is performed
		if (! to_bcopied.empty( ) ){
			if (registerOnly) {
				// If --register-only: message with ISB files list to be printed out
				infoMsg = transferFilesList (to_bcopied, jobid) + "\n";
			} else {
				// Transfers the ISB local files to the server
				transferFiles (to_bcopied, *destURI, jobid);
			}
		}
	}
	// Info message for start in case of register only
	if (!startJob && infoMsg.size()==0){
		infoMsg = "To complete the operation start the job by issuing a submission with the option:\n --start "
		+ jobid + "\n";
	}
	return jobid;
}
/**
*  Reads the JobRegister results for a job represented as a DAG and checks if there are local files
*  in the InputSandbox to be transferred to the WMProxy server.
* In case of file transferring is not requested (only job registration has to be performed),
* an info message with the list these file is provided
*/
std::string  JobSubmit::dagJob(){
	// jobid and node-name
	string jobid = "";
	string child = "";
	string node = "";
	//string *destURI = NULL;
	string zip_uri = "";
	string *dest_uri = NULL;
	// InputSB files
	vector <string> paths ;
	vector <pair<string, string> > to_bcopied ;
	// children
	vector <JobIdApi*> children;
	// iterators
	vector <JobIdApi*>::iterator it;
	// gzip file
	string gzip = "";
	if (!dagAd){
		throw WmsClientException(__FILE__,__LINE__,
		"JobSubmit::dagJob",  DEFAULT_ERR_CODE,
		"Null Pointer Error",
		"null pointer to dagAd object" );
	}
	// MAIN JOB ====================
	// jobid
	jobid = jobIds.jobid ;
	Utils::checkJobId(jobid);
	// MAIN JOB: InputSandox files
	string isb_uri;
	if (dagAd->hasAttribute(JDL::INPUTSB)) {
		// InputSB file of the parent node
		paths = dagAd->getInputSandbox();
		// Extracts all the local file
		dest_uri = toBCopiedFileList(jobid, "", this->isbURI, paths, to_bcopied) ;
	} else {
		dest_uri = getInputSbDestinationURI (jobid, "", zip_uri);
		if (!dest_uri){
			throw WmsClientException(__FILE__,__LINE__,"getInputSbDestinationURI",DEFAULT_ERR_CODE,
					"Missing Information","unable to retrieve the InputSB DestinationURI for the job: " +jobid  );
		}
	}
	// CHILDREN ====================
	children = jobIds.children ;
	if ( ! children.empty() ){
		for ( it = children.begin() ; it != children.end(); it++){
			if (*it){
				// child: jobid
				child = (*it)->jobid ;
				Utils::checkJobId(child);
				//child:  node name
				if ( ! (*it)->nodeName ){
					throw WmsClientException(__FILE__,__LINE__,
						"JobSubmit::dagJob",  DEFAULT_ERR_CODE,
						"Null Pointer Error",
						"unable to retrieve the node name for the child job: " + child );
				}
				node = *((*it)->nodeName);
				ostringstream info;
				info << "DAG child node\nNode name : " << node << " - JobId : " << child ;
				logInfo->print(WMS_DEBUG,  info.str(), "", false);
				// child: InputSandbox files
				if (dagAd->hasNodeAttribute(node, JDL::INPUTSB) ){
					// ISB files for the child node
					paths = dagAd->getNodeStringValue(node, JDL::INPUTSB);
					// Extracts the local ISB files for all DAG nodes
					this->toBCopiedFileList(jobid, child, this->getDagISBURI(node), paths, to_bcopied );
				}
			}
		}
	}
	if (! to_bcopied.empty( ) ){
		if (registerOnly) {
			// If --register-only: message with ISB files list to be printed out
			infoMsg = transferFilesList (to_bcopied, jobid) + "\n";
		} else {
			// Transfers the ISB local files to the server
			transferFiles (to_bcopied, *dest_uri, jobid);
		}
	}
	// Info message for start in case of register only
	if (!startJob && infoMsg.size()==0){
		infoMsg = "To complete the operation start the job by issuing a submission with the option:\n --start "
		+ jobid + "\n";
	}
	return jobid;
}
/**
*  Reads the JobRegister results for a collection and checks if there are local files
*  in the InputSandbox to be transferred to the WMProxy server.
* In case of file transferring is not requested (only job registration has to be performed),
* an info message with the list these file is provided
*/
std::string JobSubmit::collectionJob() {
	// jobid and node-name
	string jobid = "";
	string child = "";
	string node = "";
	string* destURI = NULL;
	string gzip = "";
	// InputSB files
	vector<string> paths;
	vector <pair<string, string> > to_bcopied ;
	if (!collectAd){
		throw WmsClientException(__FILE__,__LINE__,
		"JobSubmit::collectionJob",  DEFAULT_ERR_CODE,
		"Null Pointer Error",
		"null pointer to dagAd object" );
	}
	// jobid
	jobid = jobIds.jobid;
	Utils::checkJobId(jobid);
	// Main InputSandbox
	if (collectAd->hasAttribute(JDL::INPUTSB)){
		// InputSB files
		paths = collectAd->getStringValue(JDL::INPUTSB);
		// Extracts all the local ISB files
		destURI = this->toBCopiedFileList(jobid, "", this->isbURI, paths, to_bcopied);
		if (  ! to_bcopied.empty( ) ){
			if (registerOnly) {
				// If --register-only: message with ISB files list to be printed out
				infoMsg += transferFilesList (to_bcopied, jobid) + "\n";
			} else {
				// Transfers the ISB local files to the server
				this->transferFiles(to_bcopied, *destURI, jobid);
			}
		}
	}
	// Info message for start in case of register only
	if (!startJob && infoMsg.size()==0){
		infoMsg = "To complete the operation start the job by issuing a submission with the option:\n --start "
		+ jobid + "\n";
	}
	return jobid;
}

}}}} // ending namespaces
