/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#include "jobsubmit.h"
#include "lbapi.h"
// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/adutils.h"
#include "utilities/options_utils.h"
//  wmp-client exceptions
#include "utilities/excman.h"
// streams
#include <sstream>
#include <iostream>
// map type
#include <map>
// wmproxy-API
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"
// Configuration
#include "glite/wms/common/configuration/WMCConfiguration.h" // Configuration
// Ad attributes and JDL methods
#include "glite/wms/jdl/jdl_attributes.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/PrivateAttributes.h"
#include "glite/wms/jdl/adconverter.h"
// BOOST
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/exception.hpp"
#include <boost/lexical_cast.hpp>
// CURL
#include "curl/curl.h"
// TAR
#include "libtar.h"

using namespace std ;
using namespace glite::wms::client::utilities ;

using namespace glite::wms::wmproxyapiutils;
using namespace glite::wms::jdl;
using namespace glite::wmsutils::exception;
namespace fs = boost::filesystem;
namespace api = glite::wms::wmproxyapi;

namespace glite {
namespace wms{
namespace client {
namespace services {


JobFileAd::JobFileAd ( ){
	this->jobid = "";
	this->node  = "";
//	this->files = new std::vector<glite::wms::jdl::FileAd>;
};

ZipFileAd::ZipFileAd( ) {
	this->filename  = "";

//	this->fileads = new std::vector<JobFileAd>;
};


const string FILE_PROTOCOL = "file://" ;
const string ISBFILE_DEFAULT = "ISBfiles";
const string TMP_DEFAULT_LOCATION = "/tmp";

/**
*	Default constructor
*/
JobSubmit::JobSubmit( ){
	// init of the string attributes
	chkptOpt  = NULL; // TBD
	collectOpt = NULL;
	dagOpt = NULL;
	defJdlOpt = NULL;
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
	adObj = NULL;
	jobAd = NULL;
	dagAd = NULL;
	collectAd = NULL;
	extractAd = NULL;
	// shadow
	jobShadow = NULL;
	// time opt
	expireTime = 0;
	// Transfer Files Message
	infoMsg = "";
	// As default  file archiving and compression is allowed
	zipAllowed = false;
};

/*
*	Default destructor
*/
JobSubmit::~JobSubmit( ){
	if (collectOpt){ delete(collectOpt); }
	if (chkptOpt){ delete(chkptOpt); } //TBD
	if (dagOpt){ delete(dagOpt); }
	if (defJdlOpt){ delete(defJdlOpt); }
	if (fileProto){ delete(fileProto); }
	if (lrmsOpt){ delete(lrmsOpt); }
	if (toOpt){ delete(toOpt); }
	if (inOpt){ delete(inOpt); }
	if (jdlFile){ delete(jdlFile); }
	if (adObj){ delete(adObj); }
	if (jobAd){ delete(jobAd); }
	if (dagAd){ delete(dagAd); }
	if (collectAd){ delete(collectAd); };
	if (jobShadow){ delete( jobShadow); }
};


/*
* Handles the command line options
*/
void JobSubmit::readOptions (int argc,char **argv){
	ostringstream info ;
	vector<string> wrongids;
	vector<string> resources;
	vector<string> protocols;
	// days+hours+minutes for --to and --valid
	int d = 0;
	int h = 0;
	int m = 0;
	Job::readOptions  (argc, argv, Options::JOBSUBMIT);
	// input & resource (no together)
	inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	resourceOpt = wmcOpts->getStringAttribute(Options::RESOURCE);
	nodesresOpt = wmcOpts->getStringAttribute(Options::NODESRES);
	if (inOpt && (resourceOpt||nodesresOpt) ){
		info << "The following options cannot be specified together:\n" ;
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
			logInfo->print(WMS_DEBUG,   "--input option: The job will be submitted to the resource", *resourceOpt);
		}
	}
	// --chkpt TBD !!!
	chkptOpt =  wmcOpts->getStringAttribute( Options::CHKPT) ;
	// --collect
	collectOpt = wmcOpts->getStringAttribute(Options::COLLECTION);
	// --dag
	dagOpt = wmcOpts->getStringAttribute(Options::DAG);
	// --default-jdl
	defJdlOpt = wmcOpts->getStringAttribute(Options::DEFJDL);
	// register-only & start
	startOpt = wmcOpts->getStringAttribute(Options::START);
	registerOnly = wmcOpts->getBoolAttribute(Options::REGISTERONLY);
	// --valid & --to
	validOpt = wmcOpts->getStringAttribute(Options::VALID);
	toOpt = wmcOpts->getStringAttribute(Options::TO);
	// --start: incompatible options
	if (startOpt &&
		(registerOnly || inOpt || resourceOpt || nodesresOpt || toOpt || validOpt ||
			chkptOpt || collectOpt || dagOpt || defJdlOpt ||
			(wmcOpts->getStringAttribute(Options::DELEGATION) != NULL) ||
			wmcOpts->getBoolAttribute(Options::AUTODG)  )){
		info << "The following options cannot be specified together with --start:\n" ;
		info << wmcOpts->getAttributeUsage(Options::REGISTERONLY) << "\n";
		info << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		info << wmcOpts->getAttributeUsage(Options::RESOURCE) << "\n";
		info << wmcOpts->getAttributeUsage(Options::NODESRES) << "\n";
		info << wmcOpts->getAttributeUsage(Options::TO) << "\n";
		info << wmcOpts->getAttributeUsage(Options::VALID) << "\n";
		info << wmcOpts->getAttributeUsage(Options::CHKPT) << "\n";
		info << wmcOpts->getAttributeUsage(Options::COLLECTION) << "\n";
		info << wmcOpts->getAttributeUsage(Options::DAG) << "\n";
		info << wmcOpts->getAttributeUsage(Options::DEFJDL) << "\n";
		info << wmcOpts->getAttributeUsage(Options::AUTODG) << "\n";
		info << wmcOpts->getAttributeUsage(Options::DELEGATION) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error",
				info.str());
	}
	// "valid" & "to" (no together)
	if (validOpt && toOpt){
		info << "The following options cannot be specified together:\n" ;
		info << wmcOpts->getAttributeUsage(Options::VALID) << "\n";
		info << wmcOpts->getAttributeUsage(Options::TO) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", info.str());
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
	// --transfer-files
	bool transfer_files = wmcOpts->getBoolAttribute(Options::TRANSFER);
	// Flags for --register-only , --transfer-files, --start
	if (registerOnly){
		startJob = false;
		if ( transfer_files) {
			registerOnly = false ;
		}
	} else {
		if (transfer_files){
			info << wmcOpts->getAttributeUsage(Options::TRANSFER) ;
			info << ": this option can only be used with " ;
			info << wmcOpts->getAttributeUsage(Options::REGISTERONLY) << "\n";
			throw WmsClientException(__FILE__,__LINE__,
					"readOptions",DEFAULT_ERR_CODE,
					"Input Option Error", info.str());
		}
		startJob = true;
	}
	// checks the JobId argument for the --start option
	if (startOpt) {
		*startOpt =string(Utils::checkJobId(*startOpt));
		// Retrieves the endpoint URL in case of --start
		logInfo->print(WMS_DEBUG, "Getting the enpoint URL");
		LbApi lbApi;
		lbApi.setJobId(*startOpt);
		Status status=lbApi.getStatus(true,true);
		setEndPoint(status.getEndpoint());
		// checks if --endpoint option has been specified with a different endpoint url
		string *endpoint =  wmcOpts->getStringAttribute (Options::ENDPOINT) ;
		if (endpoint && endpoint->compare(getEndPoint( )) !=0 ) {
			logInfo->print(WMS_WARNING, "--endpoint " + string(*endpoint) + " : option ignored");
		}
		logInfo->print(WMS_INFO, "Connecting to the service", getEndPoint());

	} else {
		// retrieves the endpoint URL
		retrieveEndPointURL( );
	}
	// file Protocol
	fileProto= wmcOpts->getStringAttribute( Options::PROTO) ;
	if (startOpt && fileProto) {
		logInfo->print (WMS_WARNING, "--proto: option ignored (start operation doesn't need any file transfer)\n", "", true );
	}  else if (registerOnly && fileProto) {
		logInfo->print (WMS_WARNING, "--proto: option ignored (register-only operation doesn't need any file transfer)\n", "", true );
	} else {
		try {
			// --proto
			checkFileTransferProtocol( );
		} catch (WmsClientException &exc) {
			if (fileProto==NULL) {
				fileProto= new string (Options::TRANSFER_FILES_DEF_PROTO );
				logInfo->print(WMS_DEBUG, "Setting file protocol to default:", *fileProto);
			}
			logInfo->print (WMS_WARNING, "error while checking the file protocol: " + string(exc.what( )),
			"The protocol that will be used is: " + string(*fileProto));

		}
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
			logInfo->print(WMS_DEBUG,  "--valid option:", info.str( ) );
		} else if (toOpt)  {
			logInfo->print(WMS_DEBUG, "--to option:", info.str( ) );
		}
	}
	// --nolisten
	nolistenOpt =  wmcOpts->getBoolAttribute (Options::NOLISTEN);
	// path to the JDL file
	jdlFile = wmcOpts->getPath2Jdl( );
}

/**
* Performs the main operation for the submission
*/
void JobSubmit::submission ( ){
	// proxy validity time read from Configuration (default value alway present)
	if (wmcUtils){
		postOptionchecks(wmcUtils->getConf()->default_proxy_validity());
	}else{
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE ,
			"wmcUtils fatal Error",
			"Utilities not yet intialised");
	}
	ostringstream out ;
	string jobid = "";
	bool toBretrieved = false;
	// in case of --start option
	if (startOpt){
		jobid = *startOpt;
		jobStarter(jobid);
	} else {
		this->checkAd(toBretrieved);
		// Perform sSubmission when:
		// (RegisterOnly has not been specified in CLI) && (no file to be transferred)
		jobid = jobRegOrSub(startJob && !toBretrieved);
		logInfo->print(WMS_DEBUG, "The JobId is: ", jobid) ;
		// Perform File Transfer when:
		// (Registeronly is NOT specified [or specified with --tranfer-file]) AND (some files are to be transferred)
		if (toBretrieved){
			try{
				this->jobPostProcessing( );
			}catch (Exception &exc){
				throw WmsClientException(__FILE__,__LINE__,
					"submission",  DEFAULT_ERR_CODE ,
					"The job has been successfully registered (the JobId is: " + this->getJobId( ) + "),"+
					" but an error occurred while transferring files:",
					string (exc.what()));
			}
		} else{
			logInfo->print(WMS_DEBUG, "No local files to be transferred") ;
			if (!startJob) {
				infoMsg = "To complete the operation start the job by issuing a submission with the option:\n";
				infoMsg += " --start " + this->getJobId( ) + "\n";
			}
		}
		// Perform JobStart when:
		// (RegisterOnly has not been specified in CLI) AND (There were files to transfer)
		if (startJob && toBretrieved){
			// Perform JobStart
			jobStarter(this->getJobId( ));
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
	out << this->getJobId( ) << "\n";
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
				jobShadow->setGoodbyeMessage(true);
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
		if ( wmcUtils->saveJobIdToFile(*outOpt, this->getJobId( )) < 0 ){
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
* Returns the type of job is being submitted
*/
const wmsJobType JobSubmit::getJobType( ){
	return jobType;
}

/**
* Checks whether the total size of local files in the ISB is compatible with the
* limitation that could be set on the server
* (UserFreeQuota and max InputSandbox size)
*/
void JobSubmit::checkUserServerQuota(const long &isbSize) {
	pair<long, long> free_quota ;
	long max_isbsize = 0;
	long limit = 0;
	//  (1) User free quota -----------
	logInfo->print (WMS_DEBUG,"Checking the User-FreeQuota on the server", "" );
	try{
		// Gets the user-free quota from the WMProxy server
		logInfo->service(WMP_FREEQUOTA_SERVICE);
		free_quota = api::getFreeQuota(getContext( ));
	} catch (api::BaseException &exc){
			throw WmsClientException(__FILE__,__LINE__,
				"checkInputSandbox", ECONNABORTED,
				"WMProxy Server Error", errMsg(exc));
	}
	// soft limit
	limit = free_quota.first;
	// if the previous function gets back a negative number
	// the user free quota is not set on the server and
	// no check is performed (this functions returns not exceed=true)
	if (limit >0) {
		logInfo->result(WMP_FREEQUOTA_SERVICE, "User-FreeQuota information successfully retrieved");
		if (isbSize > limit  ) {
			ostringstream err ;
			err << "Not enough User-FreeQuota (" << limit << " bytes) on the server for the InputSandbox files (" ;
			err << isbSize << " bytes)";
			throw WmsClientException( __FILE__,__LINE__,
				"checkInputSandbox",  DEFAULT_ERR_CODE,
				"UserFreeQuota Error" ,
				err.str());
		} else {
			ostringstream q;
			q << "The InputSandbox size (" << isbSize << " bytes) doesn't exceed the User FreeQuota (" << limit << " bytes)";
			logInfo->print (WMS_DEBUG, q.str(), "File transfer is allowed" );
		}
	} else {
		// User quota is not set on the server: check of Max InputSB size
		logInfo->result(WMP_FREEQUOTA_SERVICE, "User freequota not set on the server");
		try{
			// Gets the maxISb size from the WMProxy server
			logInfo->print(WMS_DEBUG, "Getting the max ISB size from the server", getEndPoint( ) );
			logInfo->service(WMP_MAXISBSIZE_SERVICE);
			max_isbsize = api::getMaxInputSandboxSize(getContext( ));
		} catch (api::BaseException &exc){
				throw WmsClientException(__FILE__,__LINE__,
					"checkInputSandbox", ECONNABORTED,
					"WMProxy Server Error", errMsg(exc));
		}
		// (2) MAX ISB size -----------
		if (max_isbsize>0 ) {
			logInfo->result(WMP_MAXISBSIZE_SERVICE, "Max ISB size information successfully retrieved");
			if (isbSize > max_isbsize) {
				ostringstream err ;
				err << "The size of the InputSandbox (" << isbSize <<" bytes) ";
				err << "exceeds the MAX InputSandbox size limit on the server (" << max_isbsize << " bytes)";
				throw WmsClientException( __FILE__,__LINE__,
					"checkInputSandbox",  DEFAULT_ERR_CODE,
					"InputSandboxSize Error" , err.str());
			} else {
				ostringstream q;
				q << "The InputSandbox size (" << isbSize << " bytes) doesn't exceed the max size limit of " << max_isbsize << " bytes:";
				logInfo->print (WMS_DEBUG, q.str(), "File transfer is allowed" );
			}
		} else {
			// User quota is not set on the server: check of Max InputSB size
			logInfo->result(WMP_MAXISBSIZE_SERVICE, "Max ISB size is not set on the server");

		}
	}
 }


/**
* The composition of the ISB zipped files is stored
* in the memory in order to be used after the registration/submission
* of the job, when the zipped files are phisically prepared
*/
void JobSubmit::toBCopiedZippedFileList() {
	ZipFileAd *zipStruct = NULL;
	vector<FileAd> fileads ;
	JobFileAd *jobFiles = NULL;
	vector<ExtractedAd*> children;
	long tar_size = 0;
	long filead_size = 0;
	int ntar = 0;
	int n = 0;
	int nc = 0;
	int i = 0;
	// Unique string to be used for the name of the tar.gz files
	string *us = Utils::getUniqueString( );
	if (us==NULL) {
		ostringstream u ;
		u << getpid( ) << "_" << getuid( );
		us = new string(u.str());
	}
	logInfo->print(WMS_DEBUG,
		"Computing the composition of the zipped files" ,
		"", false);
	if (extractAd==NULL) {
		throw WmsClientException(__FILE__,__LINE__,
			"toBCopiedZippedFileList",  DEFAULT_ERR_CODE ,
			"Null Pointer Error",
			"Null pointer to extractAd\n"+ Options::BUG_MSG);
	}
	// zipStructs :   ZipFileAd {std::string filename ; FILEAD filead;}
	zipStruct = new ZipFileAd ();
	jobFiles = new JobFileAd ( );
	zipStruct->filename = ISBFILE_DEFAULT + "_" + *us + "_" + boost::lexical_cast<string>(ntar)
		+ Utils::getArchiveExtension( ) + Utils::getZipExtension( ) ;
//cout << "###JobSubmit::toBCopiedZippedFileList> filename=" << zipStruct->filename << "\n";

	fileads = extractAd->getFiles ( );
	n = fileads.size( );
//cout << "###JobSubmit::toBCopiedZippedFileList>	(zipStruct->fileads).size( )="<<(zipStruct->fileads).size( )<<"\n";
	if (n > 0) {
//cout << "###JobSubmit::toBCopiedZippedFileList> N="<<n<<"\n";
		// int i = index of the FileAd vector is being filled
		i++;
		jobFiles->node = "";
		while (n != 0){
			filead_size = fileads[0].size ;
//cout << "###obSubmit::toBCopiedZippedFileList>filead_size="<<filead_size<<"\n";
			// adding up the size of the first file in the list to the total tar size
			tar_size += filead_size;
//cout << "###obSubmit::toBCopiedZippedFileList>tar_size="<<tar_size<<"\n";
			if (tar_size < Options::MAX_TAR_SIZE) {
				// moving the last elment of the FileAd list to the list of the rootFiles
				(jobFiles->files).push_back(fileads.at(0));
				fileads.erase(fileads.begin());
			} else {
//cout << "###obSubmit::toBCopiedZippedFileList> closing tar ....\n";
				(zipStruct->fileads).push_back(*jobFiles);
				// Saving the list of files in the main vector of zip structs of this class
				(this->zippedFiles).push_back (*zipStruct);
				// increase the "Number of tars" index
				ntar++;
				// next zip filename
				delete(jobFiles);
				delete(zipStruct);
				tar_size = 0;
				zipStruct = new ZipFileAd ();
				jobFiles = new JobFileAd ( );
				// new filename
				zipStruct->filename = ISBFILE_DEFAULT + "_" + *us + "_" + boost::lexical_cast<string>(ntar)
					+ Utils::getArchiveExtension( ) + Utils::getZipExtension( ) ;
				jobFiles->jobid = extractAd->getJobId ( );
				jobFiles->node = "";
				// moving the last elment of the FileAd list to the new list of the rootFiles
				(jobFiles->files).push_back(fileads.at(0));
				fileads.erase(fileads.begin());
				tar_size = filead_size;
			}
			n = fileads.size( );
		}
		if (jobFiles) {
			(zipStruct->fileads).push_back(*jobFiles);
		}
	}
	children = extractAd->getChildren( );
	nc = children.size ( );
	for (int i = 0; i < nc ; i++) {
		if (children[i] != NULL) {
			jobFiles = new JobFileAd ( );
			//  node-name
			jobFiles->node = children[i]->getNodeName( );
			// list of files
			fileads = children[i]->getFiles( );
			n = fileads.size( );
			while (n != 0){
				filead_size = fileads[(n-1)].size ;
				// adding up the size of the first file in the list to the total tar size
				tar_size += filead_size;
				if (tar_size < Options::MAX_TAR_SIZE) {
					// moving the last elment of the FileAd list to the list of files
					(jobFiles->files).push_back(fileads.at(0));
					fileads.erase(fileads.begin());
				} else {
					(zipStruct->fileads).push_back(*jobFiles);
					// Saving the list of files in the main vector of zip structs of this class
					(this->zippedFiles).push_back (*zipStruct);
					// increasing the "Number of tars" index
					ntar++;
					// next zip filename
					delete(jobFiles);
					delete(zipStruct);
					zipStruct = new ZipFileAd( );
					jobFiles = new JobFileAd ( );
					zipStruct->filename = ISBFILE_DEFAULT + "_" + *us + "_" + boost::lexical_cast<string>(ntar)
						+ Utils::getArchiveExtension( ) + Utils::getZipExtension( ) ;
					// node-name
					jobFiles->jobid = extractAd->getJobId ( );
					jobFiles->node = children[i]->getNodeName( );
					// moving the last elment of the FileAd list to the new list of files
					(jobFiles->files).push_back(fileads.at(0));
					fileads.erase(fileads.begin());
					tar_size = filead_size;
				}
				n = fileads.size( );
			}
			if (jobFiles) {
				(zipStruct->fileads).push_back(*jobFiles);
			}
		}
	}
	if (zipStruct) {
		(this->zippedFiles).push_back (*zipStruct);
	}
}
/**
* The input vector is filled with the information related to the local user files that are
* in the JDL InputSandbox attribute and needed to be transferred to the WMProxy server.
* If file compression is allowed, all the user files are collected into zipped files.
*/

void JobSubmit::toBCopiedFileList( std::vector<std::pair<FileAd, std::string > > &tob_transferred) {
	vector<ExtractedAd*> children;
	vector<FileAd> fileads;
	string destURI = "";
	string jobid = "";
	int size = 0;
	if (zipAllowed) {
		vector<ZipFileAd>::iterator it1 = zippedFiles.begin() ;
		vector<ZipFileAd>::iterator const end1 = zippedFiles.end();
//cout << "##zippedFiles.size="<< zippedFiles.size() << "\n";
		for ( ; it1 != end1; it1++){
			createZipFile(it1->filename, it1->fileads, tob_transferred);
		}
	} else {
		if (extractAd==NULL) {
			throw WmsClientException(__FILE__,__LINE__,
				"toBCopiedFileList",  DEFAULT_ERR_CODE ,
				"Null Pointer Error",
				"Null pointer to extractAd\n"+ Options::BUG_MSG);
		}
		// ROOT =========
		fileads = extractAd->getFiles( );
		// JobId (root)
		//jobid = extractAd->getJobId( );
		jobid = this->getJobId( ) ;
//cout << "###JobSubmit::toBCopiedFileList> - 1 -jobid="<<jobid<<"\n";
		// DestinationURI (root)
		destURI = getDestinationURI (jobid);
		// List of root ISB files
		vector<FileAd>::iterator it2 = fileads.begin( );
		vector<FileAd>::iterator const end2 = fileads.end( );
		for ( ; it2 != end2; it2++){
			tob_transferred.push_back (make_pair(*it2, string(destURI + "/" +Utils::getFileName(it2->file))));
		}
		// CHILDREN NODES =========
		children = extractAd->getChildren ( );
		size = children.size ( );
		for (int i = 0; i < size; i++){
			if ( children[i] != NULL ){
				fileads = children[i]->getFiles( );
				// DestinationURI (child node)
//cout << "###JobSubmit::toBCopiedFileList> - 2 -jobid="<<jobid<<"\n";
				destURI = getDestinationURI (jobid, children[i]->getJobId( ));
				vector<FileAd>::iterator it3 = fileads.begin( );
				vector<FileAd>::iterator const end3 = fileads.end( );
				for ( ; it3 != end3; it3++){
					tob_transferred.push_back (make_pair(*it3, string(destURI + "/" + Utils::getFileName(it3->file))));
				}
			}
		}

	}

}
/**
* Checks that the total size (in bytes) of the local files in
* the JDL InputSandbox don't exceed the size limit fixed
* on the server side:
* either the user free quota or the max ISB size (if the first one is not set)
*/
int JobSubmit::checkInputSandbox ( ) {
	long isbsize = 0;
	string message = "";
	ostringstream err ;
	logInfo->print (WMS_DEBUG, "Retrieving the list of the local ISB files from the user JDL", "");
	// type of job
	wmsJobType job = this->getJobType() ;
	switch (job) {
		case WMS_JOB : {
			extractAd = jobAd->getExtractedAd( );
			break;
		}
		case WMS_DAG:
		case WMS_PARAMETRIC:  {
			extractAd = dagAd->getExtractedAd( );
			break;
		}
		case WMS_COLLECTION : {
			extractAd = collectAd->getExtractedAd( );
			break;
		}
		default : {
			throw WmsClientException(__FILE__,__LINE__,
				"getInputSandboxSize",  DEFAULT_ERR_CODE ,
				"Uknown JobType",
				"unable to process the job (check the JDL)");
		}
	}
	if (extractAd == NULL) {
		throw WmsClientException(__FILE__,__LINE__,
			"inputSandboxFiles",  DEFAULT_ERR_CODE ,
			"Null Pointer Error",
			"null pointer to extractAd\n"+ Options::BUG_MSG);
	}
	// Sets the max size allowed for file transfer
	if (fileProto) {
		FileAd::setMaxFileSize(Options::getMinimumAllowedFileSize(*fileProto, zipAllowed));
	} else {
		FileAd::setMaxFileSize(Options::getMinimumAllowedFileSize("", zipAllowed));
	}

	isbsize = extractAd->getTotalSize ( );
	if (isbsize > 0) {
		logInfo->print (WMS_DEBUG,
			"Total size of the ISB file(s) to be transferred to:",
			boost::lexical_cast<string>(isbsize) );
		// Checking whether ISB-total_size is supported by either UserFreeQUota or MAX-ISB =============
		checkUserServerQuota (isbsize);
//cout << "####ADDING ZIPPED_ISB> zipAllowed=" << zipAllowed << "\n";
		if (zipAllowed){
			// Prepares the zipped files
			toBCopiedZippedFileList( );
			// Adding the references to the zipped files to the user JDL =============================
			std::vector<ZipFileAd>::iterator it1 = zippedFiles.begin() ;
			std::vector<ZipFileAd>::iterator const end1 = zippedFiles.end();
//cout << "####ADDING ZIPPED_ISB ..... size=" << zippedFiles.size( ) << "\n";
			switch (getJobType()) {
				case (WMS_JOB) : {
					for ( ; it1 != end1; it1++){
//cout << "####ADDING ZIPPED_ISB: file=" << it1->filename << "\n";
						jobAd->addAttribute(JDLPrivate::ZIPPED_ISB, it1->filename);
					}
					break;
				}
				case WMS_DAG:
				case WMS_PARAMETRIC:  {
					vector<string> gz_files;
					for ( ; it1 != end1; it1++){
						gz_files.push_back(it1->filename);
					}
					dagAd->setAttribute(ExpDagAd::ZIPPED_ISB, gz_files);
					break;
				}
				case WMS_COLLECTION : {
					for ( ; it1 != end1; it1++){
						collectAd->addAttribute(JDLPrivate::ZIPPED_ISB, it1->filename);
					}
					break;
				}
				default : {
					throw WmsClientException(__FILE__,__LINE__,
						"getInputSandboxSize",  DEFAULT_ERR_CODE ,
						"Uknown JobType",
						"unable to process the job (check the JDL)");
				}
			}
		}

	} else {
		logInfo->print(WMS_DEBUG, "The user JDL does not contain any local ISB file:" ,
			"no ISB file transfer to be performed");
	}
	return isbsize ;
}
/**
*  Checks the user JDL
*/
void JobSubmit::checkAd(bool &toBretrieved){
	string message = "";
	jobType = WMS_JOB;
	glite::wms::common::configuration::WMCConfiguration* wmcConf = wmcUtils->getConf();
	// COLLECTION (--collection)
	if (collectOpt) {
		jobType = WMS_COLLECTION ;
		try {
			//fs::path cp ( Utils::normalizePath(*collectOpt), fs::system_specific); // Boost 1.29.1
			fs::path cp ( Utils::normalizePath(*collectOpt), fs::native);
			if ( fs::is_directory( cp ) ) {
				*collectOpt= Utils::addStarWildCard2Path(*collectOpt);
			} else {
				throw WmsClientException(__FILE__,__LINE__,
					"submission",  DEFAULT_ERR_CODE,
					"Invalid JDL collection Path",
					"--collection: no valid collection directory (" + *collectOpt + ")"  );
			}
		} catch ( const fs::filesystem_error & ex ){
			throw WmsClientException(__FILE__,__LINE__,
				"submission",  DEFAULT_ERR_CODE,
				"Invalid JDL collection Path",
				ex.what()  );
		}
		logInfo->print (WMS_DEBUG, "A collection of jobs is being submitted; JDL files in:",
			Utils::getAbsolutePath( *collectOpt));
		collectAd = AdConverter::createCollectionFromPath (*collectOpt);
		collectAd->setLocalAccess(true);
		// Simple Ad manipulation
		if (!collectAd->hasAttribute (JDL::VIRTUAL_ORGANISATION)){
			collectAd->setAttribute(JDL::VIRTUAL_ORGANISATION, *(wmcUtils->getVirtualOrganisation()));
		}
		AdUtils::setDefaultValuesAd(collectAd,wmcConf);
		// Collect Ad manipulation
		AdUtils::setDefaultValues(collectAd,wmcConf);
		if (nodesresOpt) {
			collectAd->setAttribute(JDL::SUBMIT_TO, *nodesresOpt);
		}
		// JDL string
		collectAd = collectAd->check();
		if (collectAd->hasAttribute(JDL::ALLOW_ZIPPED_ISB)){
			zipAllowed = collectAd->getBool(JDL::ALLOW_ZIPPED_ISB) ;
			if (zipAllowed) { message ="allowed by user in the JDL";}
			else { message = "disabled by user in the JDL"; }
			logInfo->print (WMS_DEBUG, "File archiving and file compression",
				message);
		} else {
			// Default value if the JDL attribute is not present
			zipAllowed = false;
			logInfo->print (WMS_DEBUG, "The user JDL does not contain the " + JDL::ALLOW_ZIPPED_ISB + " attribute: ",
			"adding the attribute to the JDL with the default value (FALSE)");
			collectAd->addAttribute(JDL::ALLOW_ZIPPED_ISB, false);
		}
		// Checks if there are local ISB file(s) to be transferred to
		toBretrieved = (this->checkInputSandbox( ) > 0)?true:false;
		// JDL submission string
		jdlString = new string(collectAd->toString());
	} else if (dagOpt) {
		jobType = WMS_DAG ;
		try {
			fs::path cp ( Utils::normalizePath(*dagOpt), fs::native);
			if ( fs::is_directory( cp ) ) {
				*dagOpt= Utils::addStarWildCard2Path(*dagOpt);
			} else {
				throw WmsClientException(__FILE__,__LINE__,
					"submission",  DEFAULT_ERR_CODE,
					"Invalid JDL collection Path",
					"--dag: no valid collection directory (" + *dagOpt + ")"  );
			}
		} catch ( const fs::filesystem_error & ex ){
			throw WmsClientException(__FILE__,__LINE__,
				"submission",  DEFAULT_ERR_CODE,
				"Invalid JDL DAG Path",
				ex.what()  );
		}
		logInfo->print (WMS_DEBUG, "A DAG is being submitted; JDL files in:", Utils::getAbsolutePath( *dagOpt));


		/*
			TBD !!!!
			toBretrieved = (this->checkInputSandbox( ) > 0)?true:false;

		*/


	} else {
		// ClassAd
		adObj = new Ad();
		if (jdlFile==NULL){
			throw WmsClientException(__FILE__,__LINE__,
				"checkAd",  DEFAULT_ERR_CODE,
				"JDL File Missing",
				"uknown JDL file pathame (Last Argument of the command must be a JDL file)"   );
		}
		logInfo->print (WMS_DEBUG, "The JDL file is:", Utils::getAbsolutePath(*jdlFile));
		adObj->fromFile (*jdlFile);
		// Adds ExpireTime JDL attribute
		if ((int)expireTime>0) {
			adObj->addAttribute (JDL::EXPIRY_TIME, (double)expireTime);
		}
		// Simple Ad manipulation (common)
		if (!adObj->hasAttribute (JDL::VIRTUAL_ORGANISATION)){
			adObj->setAttribute(JDL::VIRTUAL_ORGANISATION, *(wmcUtils->getVirtualOrganisation()));
		}
		AdUtils::setDefaultValuesAd(adObj,wmcConf);
		// Checking the ALLOW_ZIPPED_ISB attribute
		if (adObj->hasAttribute(JDL::ALLOW_ZIPPED_ISB)){
				zipAllowed = adObj->getBool(JDL::ALLOW_ZIPPED_ISB) ;
				if (zipAllowed) { message ="allowed by user in the JDL";}
				else { message = "disabled by user in the JDL"; }
				logInfo->print (WMS_DEBUG, "File archiving and file compression",
					message);
			} else {
				// Default value if the JDL attribute is not present
				zipAllowed = false;
				logInfo->print (WMS_DEBUG, "The user JDL does not contain the " + JDL::ALLOW_ZIPPED_ISB + " attribute: ",
				"adding the attribute to the JDL with the default value (FALSE)");
				adObj->addAttribute(JDL::ALLOW_ZIPPED_ISB, false);
			}
		// COLLECTION ========================================
		if ( adObj->hasAttribute(JDL::TYPE , JDL_TYPE_COLLECTION) ) {
			logInfo->print (WMS_DEBUG, "A collection of jobs is being submitted");
			jobType = WMS_COLLECTION ;
			try{
				collectAd = new CollectionAd(*(adObj->ad()));
				collectAd->setLocalAccess(true);
				// Collect Ad manipulation
				AdUtils::setDefaultValues(collectAd,wmcConf);
				if (nodesresOpt) {
					collectAd->setAttribute(JDL::SUBMIT_TO, *nodesresOpt);
				}
				// JDL string
				collectAd = collectAd->check();
				// Checks if there are local ISB file(s) to be transferred to
				toBretrieved = (this->checkInputSandbox( ) > 0)?true:false;
				// JDL submission string
				jdlString = new string(collectAd->toString());
			}catch (Exception &ex){
				throw WmsClientException(__FILE__,__LINE__,
					"submission",  DEFAULT_ERR_CODE,
					"Invalid JDL collection",
					ex.what()   );
			}
		}  else
		// DAG  ========================================
		if ( adObj->hasAttribute(JDL::TYPE , JDL_TYPE_DAG) ) {
				logInfo->print (WMS_DEBUG, "A DAG job is being submitted");
				jobType = WMS_DAG ;
				if (nodesresOpt) {
					adObj->setAttribute(JDL::SUBMIT_TO, *nodesresOpt);
				}
				dagAd = new ExpDagAd (adObj->toString());
				dagAd->setLocalAccess(true);
				AdUtils::setDefaultValues(dagAd,wmcConf);
				// expands the DAG loading all JDL files
				dagAd->getSubmissionStrings();
				// Checks if there are local ISB file(s) to be transferred to
				toBretrieved = (this->checkInputSandbox( ) > 0)?true:false;
				// JDL submission string
				jdlString = new string(dagAd->toString()) ;
		} else {
			jobType = WMS_JOB ;
			jobAd = new JobAd(*(adObj->ad()));
			AdUtils::setDefaultValues(jobAd,wmcConf);
			// check JobAd without restoring attributes
			jobAd->check(false);
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
				logInfo->print (WMS_DEBUG, "An interactive job is being submitted.");
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
					logInfo->print(WMS_DEBUG,"Running console shadow");
					jobShadow->console();
					logInfo->print(WMS_DEBUG,"Console properly started");
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
			// ????? JobAd *pass= new JobAd(*(jobAd->ad()));
			// ???? AdUtils::setDefaultValues(pass,wmcConf);
			// check JobAd without restoring attributes
			// ???? pass->check(false);
			// InputSandbox Files
			//###toBretrieved=pass->gettoBretrieved();
			// PARAMETRIC  ===============================================
			if (jobAd->hasAttribute(JDL::JOBTYPE,JDL_JOBTYPE_PARAMETRIC)) { cout << "###checkAd> 4\n";
				jobType = WMS_PARAMETRIC;
				logInfo->print (WMS_DEBUG, "A parametric job is being submitted");
				if (nodesresOpt) {
					// ????pass->setAttribute(JDL::SUBMIT_TO, *nodesresOpt);
					jobAd->setAttribute(JDL::SUBMIT_TO, *nodesresOpt);
				}
				// InputSandbox for the parametric job
				if (jobAd->hasAttribute(JDL::INPUTSB)) {
					// ????? dagAd=AdConverter::bulk2dag(pass);
					dagAd = AdConverter::bulk2dag(jobAd);
					AdUtils::setDefaultValues(dagAd, wmcConf);
					dagAd->getSubmissionStrings();

				} else {
					logInfo->print (WMS_DEBUG, "No InputSandbox in the user JDL", "");
				}
			}
			// Checks if there are local ISB file(s) to be transferred to
			toBretrieved = (this->checkInputSandbox ( )>0)?true:false;
//	cout <<"###checkAd> toBretrieved=" << toBretrieved << "\n";
			// Submission string
			if (jobType==WMS_PARAMETRIC){
				//??jdlString = new string(pass->toString());
				jdlString = new string(jobAd->toString());
			}else if  (jobType==WMS_JOB){
				//???jdlString = new string(pass->toSubmissionString());
				jdlString = new string(jobAd->toSubmissionString());
			}
			//???delete(pass);
		}
	}
	// --resource : incompatible argument
	if( (resourceOpt) && (jobType != WMS_JOB)){
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
			"Incompatible Argument: " + wmcOpts->getAttributeUsage(Options::RESOURCE),
			"cannot be used for  DAG, collection, partitionable and parametric jobs");
	} else if (resourceOpt) {
		logInfo->print (WMS_DEBUG, "--resource option: The job will be submitted to this resource", *resourceOpt );
	}else if( (nodesresOpt) && (jobType == WMS_JOB)){
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
	string method  = "";
	// checks if jdlstring is not null
	if (!jdlString){
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE ,
			"Null Pointer Error",
			"null pointer to JDL string\n" + Options::BUG_MSG);
	}
	try{
		if (submit){
			// jobSubmit
			method = "submit";
			logInfo->print(WMS_DEBUG, "Submitting JDL", *jdlString);
			logInfo->print(WMS_DEBUG, "Submitting the job to the service", getEndPoint());
			//Suibmitting....
			logInfo->service(WMP_SUBMIT_SERVICE);
			jobIds = api::jobSubmit(*jdlString, *dgOpt, getContext( ));
			logInfo->print(WMS_DEBUG, "The job has been successfully submitted" , "", false);
		} else {
			// jobRegister
			method = "register";
			logInfo->print(WMS_DEBUG, "Registering JDL", *jdlString);
			logInfo->print(WMS_DEBUG, "Registering the job to the service", getEndPoint());
			// registering ...
			logInfo->service(WMP_REGISTER_SERVICE);
			jobIds = api::jobRegister(*jdlString , *dgOpt, getContext( ));
			logInfo->print(WMS_DEBUG, "The job has been successfully registered" , "", false);
		}
	} catch (api::BaseException &exc) {
		ostringstream err ;
		err << "Unable to "<< method << " the job to the service: " << getEndPoint()<< "\n";
		err << errMsg(exc) ;
		// in case of any error on the only specified endpoint
		throw WmsClientException(__FILE__,__LINE__,
			"job"+method, ECONNABORTED,
			"Operation failed", err.str());
	}


	return this->getJobId( );
}

/**
* Performs Job start when:
*	- start option is selected
*	- job has been already registered and ready to be started
*/

void JobSubmit::jobStarter(const std::string &jobid ) {

	try {
		// START
		logInfo->print(WMS_DEBUG, "Starting the job: " , jobid);
		logInfo->service(WMP_START_SERVICE);
		api::jobStart(jobid, getContext( ));
	} catch (api::BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
		"jobStart", ECONNABORTED,
		"Operation failed",
		"Unable to start the job: " + errMsg(exc));
	}
	logInfo->result(WMP_START_SERVICE, "The job has been successfully started");
}

/**
*Returns the JobId string
*/

std::string JobSubmit::getJobId( ) {
	if (startOpt) {
		return (*startOpt);
	} else {
		return (jobIds.jobid) ;
	}
}
/**
* Returns the destionation URI of either the root node or one of the chlid nodes.
* If the URI of the root node is required, only the main JobId string is provided as input parameter (the first one)
* and an empty string is set for the child input parameter (the second one).
* For a child node both the main JobId and the child JobId must be provided.
*/
std::string JobSubmit::getDestinationURI(const std::string &jobid, const std::string &child, const std::string &protocol) {
      	string *destURI= NULL;
	string msg = "";
	string look_for = "";
	vector<string> jobids;
	string proto = "";
//cout << "###getDestinationURI> jobid=["<<jobid<<"]\n";
//cout << "###getDestinationURI> child=["<<child<<"]\n";
	bool ch = false; // TRUE=child node
        // The destinationURI's vector is empty: the WMProxy service is called
        if (dsURIs.empty( )){
                try{
                        logInfo->print(WMS_DEBUG, "Getting the SandboxBulkDestinationURI from the service" , getEndPoint( ));
			if (checkVersionForTransferProtocols( )){
				// jobPath is needed for the creation of the Zip files (The path used to archive the files)
				// If this information is not available in the struct returned by the jobReg/jobSubm service,
				// it has to be retrieved with the getBulkDestionationURI service
				if (protocol.size( )> 0){
					proto = protocol;
				} else if (jobIds.jobPath == NULL && zipAllowed) {
					 if (fileProto == NULL) {
						proto = string(Options::WMP_ALL_PROTOCOLS) ;
					} else if  (*fileProto == Options::JOBPATH_URI_PROTO) {
						proto = *fileProto ;
					} else {
						proto = string(Options::WMP_ALL_PROTOCOLS) ;
					}
				} else if (fileProto == NULL) {
					proto = string(Options::WMP_ALL_PROTOCOLS) ;
				} else {
					proto = *fileProto;
				}
				logInfo->print (WMS_DEBUG,
						"Calling the WMProxy " + string (WMP_BULKDESTURI_SERVICE) + " service with " + proto + " protocol", "" );
				dsURIs = api::getSandboxBulkDestURI(jobid, getContext( ), proto);
			} else {
				logInfo->print (WMS_DEBUG,
						"Calling the WMProxy " + string (WMP_BULKDESTURI_SERVICE) +
							" service with no request of specific protocol (all available protocols requested)");
				dsURIs = api::getSandboxBulkDestURI(jobid, getContext( ));
				if (protocol.size (  ) > 0) {
					proto = protocol ;
				} else if (fileProto) {
					proto = *fileProto;
				}
			}
                } catch (api::BaseException &exc){
                        throw WmsClientException(__FILE__,__LINE__,
                                "getDestinationURI", ECONNABORTED,
                                "WMProxy Server Error", errMsg(exc));
                }
                if (dsURIs.empty( )){
                        throw WmsClientException(__FILE__,__LINE__,
                                "getDestinationURI", ECONNABORTED,
                                "WMProxy Server Error",
                                "The server doesn't have any information on InputSBDestURI for :" + jobid + "\n(please contact the server administrator");
                } else{
			logInfo->result(WMP_BULKDESTURI_SERVICE, "Destination URIs sucessfully retrieved");
		}
        } else {
		if (protocol.size (  ) > 0) {
			proto = protocol ;
		} else if (fileProto) {
			proto = *fileProto;
		}
	}

        if (child.size()>0){
                // if the input parameter child is set ....
                look_for = child ;
		ch = true;
        } else {
                // parent (if the input string "child" is empty)
                look_for = jobid;
		ch = false;
        }
	if (proto.compare(Options::WMP_ALL_PROTOCOLS) == 0) {
		logInfo->print (WMS_DEBUG,
			"All protocols requested; looking for the URI with the default protocol in the received list:",
			 proto, false);
		proto = string (Options::TRANSFER_FILES_DEF_PROTO);
	} else {
		logInfo->print (WMS_DEBUG,
			"Looking for the URI with this protocol in the received list:", proto, false );
	}
        vector< pair<string ,vector<string > > >::iterator it1 = dsURIs.begin() ;
	vector< pair<string ,vector<string > > >::iterator const end1 = dsURIs.end();
        // Looks for the destURI's of the job
        for ( ; it1 != end1 ; it1++) {
                if (it1->first == look_for) { // parent or child found
			vector<string>::iterator it2 = (it1->second).begin() ;
			vector<string>::iterator const end2 = (it1->second).end() ;
                        for (; it2 != end2  ; it2++) {
                                // 1st check >>>> Looks for the destURi for file transfer
                                if ( it2->substr (0, (proto.size())) ==  proto){
					destURI= new string(*it2);
					 if (ch) {
					 	logInfo->print(WMS_DEBUG,  "Child node : " + child, " - DestinationURI : " + *destURI, false);
                                        } else {
						 logInfo->print(WMS_DEBUG,  "DestinationURI:", *destURI);
                                        }
					break ;
				}
			}
		}
	}
	if (destURI==NULL) {
		if (ch){
			throw WmsClientException(__FILE__,__LINE__, "getDestinationURI",DEFAULT_ERR_CODE,
			"Missing Information","unable to retrieve the InputSB DestinationURI for the job: " +jobid );
		} else{
			throw WmsClientException(__FILE__,__LINE__, "getDestinationURI",DEFAULT_ERR_CODE,
			"Missing Information","unable to retrieve the InputSB DestinationURI for the child node: " + child  );
		}
	}
        return *destURI;
}
/**
* Fills the destionationURI vector of this class (dsURIs) with the destionation URI related to
* the job is being submitted. All available protocols are requested.
* In case of compound jobs, the list contains the URI related to all child nodes.
*/
void JobSubmit::getDestinationURIList ( ){
	try{
		logInfo->print(WMS_DEBUG, "Getting the SandboxBulkDestinationURI from the service" , getEndPoint( ));
		logInfo->service(WMP_BULKDESTURI_SERVICE);
		if (checkVersionForTransferProtocols( )){
			logInfo->print (WMS_DEBUG,
				"Calling the WMProxy " + string (WMP_SBDESTURI_SERVICE) + " service requesting all available protocols" );
			dsURIs = api::getSandboxBulkDestURI(this->getJobId( ), getContext( ), Options::WMP_ALL_PROTOCOLS);
		} else {
			logInfo->print (WMS_DEBUG,
					"Calling the WMProxy " + string (WMP_SBDESTURI_SERVICE) +
						" service with no request of specific protocol (all available protocols requested)");
			dsURIs = api::getSandboxBulkDestURI(this->getJobId( ), getContext( ));
		}
	} catch (api::BaseException &exc){
		throw WmsClientException(__FILE__,__LINE__,
			"getDestinationURI", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
	}
	if (dsURIs.empty( )){
		throw WmsClientException(__FILE__,__LINE__,
			"getDestinationURI", ECONNABORTED,
			"WMProxy Server Error",
			"The server doesn't have any information on InputSBDestURI for :" + this->getJobId( ) + "\n(please contact the server administrator");
	} else {
		logInfo->result(WMP_BULKDESTURI_SERVICE, "Destination URIs sucessfully retrieved");
	}
}
/**
* Returns a relative path that is used to archive the ISB local file in the tar files.
* This path is based on the job DestionationURI with the input protocol
*/
std::string JobSubmit::getJobPathFromDestURI(const std::string& jobid, const std::string& protocol) {
	string* jobPath = NULL;
	string msg = "";
	int p = 0;
	// If DestinationURIs have not been retrieved yet
	if (dsURIs.empty( )){
		this->getDestinationURIList ( );
	}
	vector<pair<string , vector<string > > >::iterator it1= dsURIs.begin ( );
	vector<pair<string , vector<string > > >::iterator const end1 = dsURIs.end( );
	for ( ; it1 != end1 ; it1++) {
		// Looks for the jobId
                if (it1->first == jobid) {
			vector<string>::iterator it2 = (it1->second).begin() ;
			vector<string>::iterator const end2 = (it1->second).end() ;
                        for (; it2 != end2  ; it2++) {
                                // Looks for the destURi for file transfer
                                if ( it2->substr (0, (protocol.size())) ==  protocol){
					jobPath = new string(*it2);
					logInfo->print(WMS_DEBUG,  "JobId : " + jobid, " - JobPath : " + *jobPath, false);
					break;
				}
			}
		}
	}
	if (jobPath==NULL) {
		msg = "\nUnable to find Job InputSandbox Relative Path information needed to create the ISB Zipped File(s)\n";
		msg += "(DestinationURI with " + protocol + " protocol not found)\n";
		msg += "JobId: " + jobid + "\n";
		msg += "(please contact the server administrator)";
		throw WmsClientException(__FILE__,__LINE__,
				"getDestinationURI",
				DEFAULT_ERR_CODE,
				"Missing  Information", msg);
	} else {
		*jobPath = Utils::getAbsolutePathFromURI( *jobPath);
//cout << "#####jobPath=" << *jobPath << "\n";
	}
	return *jobPath ;

}
/**
* Returns a relative path that is used to archive the ISB local file in the tar files.
* Since WMProxy version 2.2.0, the information on this relative path is contained
* in one of the structure field returned by the call to JobRegister/JobSubmit.
* Calling a server with an earlier version, this information is computed
* form this client starting from the DestionationURi information
* (@seeJobSubmit::getJobPathFromDestURI)
*/
std::string JobSubmit::getJobPath(const std::string& node) {
	string* jobPath = NULL;
	string err = "";
	bool ch = false;
	int len = node.size( );
	// Root Node
	if (len == 0 ) {
		try {
			if (jobIds.jobPath != NULL) {
				// jobPath info from JobIdApi struct returned by jobRegister-jobSubmit service
				jobPath = new string(*(jobIds.jobPath));

			} else {
				logInfo->print(WMS_DEBUG,
					"JobPath: missing information in the struct returned by jobRegister/jobSubmit service",
					"Research based on the DestionationURI with protocol: "+ Options::JOBPATH_URI_PROTO, false);
				// jobPath info from BulkDestURI
				jobPath = new string(this->getJobPathFromDestURI(this->getJobId( ), string(Options::JOBPATH_URI_PROTO)));
			}
		} catch (WmsClientException &exc) {
			throw WmsClientException(__FILE__,__LINE__,
				"getJobPath",
				DEFAULT_ERR_CODE,
				"Missing  Information", exc.what() );
		}
	} else {
		// Looking for the jobid among the children
		ch = true;
		std::vector<api::JobIdApi*>::iterator it = (jobIds.children).begin( );
		std::vector<api::JobIdApi*>::iterator const end = (jobIds.children).end( );

		for ( ; it != end; it++) {
			if ( (*it) ) {
				if ((*it)->nodeName && node.compare(*(*it)->nodeName) ==0 ) {
					if ((*it)->jobPath != NULL) {
						// jobPath info from JobIdApi struct returned by jobRegister-jobSubmit service
						jobPath = new string(*((*it)->jobPath) );
						break;
					} else {
						logInfo->print(WMS_DEBUG,
						"JobPath for the child node " + node +": missing information in the struct returned by jobRegister/jobSubmit service",
						"Research based on the DestionationURI with protocol: " + string(Options::JOBPATH_URI_PROTO), false);
						// jobPath info from BulkDestURI
						jobPath = new string(this->getJobPathFromDestURI((*it)->jobid, string(Options::JOBPATH_URI_PROTO)));
					}
				}
			}
		}
	}
	if (jobPath==NULL) {
		err = "Unable to retrieve the job relative path for the ";
		if (len>0) {
			err += node + " child node.\n";
		} else {
			err += "root node.\n";
		}
		err += "These server data structures does not contain any useful information:\n";
		err += "- the struct returned by the " + string(WMP_REGISTER_SERVICE)  + " service\n";
		err += "- the struct returned by the " + string(WMP_SBDESTURI_SERVICE) + " service invoked for the http protocol\n";
		err += "(please contact the server administrator)\n";
		throw WmsClientException(__FILE__,__LINE__,
			"getJobPath",  DEFAULT_ERR_CODE,
			"Unknown Pathname", err);
	}
	return Utils::normalizePath(*jobPath);
}
/**
* Archives and compresses the InputSandbox files
*/
void JobSubmit::createZipFile (
	const std::string filename,
	std::vector<JobFileAd> fileads,
	std::vector<pair<glite::wms::jdl::FileAd, std::string > > &to_btransferred){
	int r = 0;
	TAR *t =NULL;
	tartype_t *type = NULL ;
	string file = "";
	string path = "";
	string jobpath = "";
	string tar = "";
	string gz = "";
	string jobPath = "";
	string jobid = "";
	// path of the tar file is being created
	// zipAd : ZIPPEDFILEAD={std::string filename; FILEAD files;}
	tar = TMP_DEFAULT_LOCATION + "/" + Utils::getArchiveFilename (filename);
	logInfo->print(WMS_DEBUG,"Archiving the ISB files:", tar);
	// opens the tar file
	r = tar_open ( &t,  (char*)tar.c_str(), type,
		O_CREAT|O_WRONLY,
		S_IRWXU, TAR_GNU |  TAR_NOOVERWRITE  );
	if ( r != 0 ){
		throw WmsClientException(__FILE__,__LINE__,
		"tar_open",  DEFAULT_ERR_CODE,
		"File i/o Error",
		"Unable to create tar file for InputSandbox: " + tar );
	}
	// files : FILEAD { std::string jobid; std::string node; std::vector<glite::wms::jdl::FileAd> files;};
	// RootFiles
//cout << "###createZipFile>  fileads.size( ) = " << fileads.size( )<<"\n";
	vector <JobFileAd>::iterator it1 = fileads.begin( );
	vector <JobFileAd>::iterator const end1 = fileads.end( );
	for ( ; it1 != end1; it1++ ) {
		jobpath = this->getJobPath(it1->node);
//cout << "###createZipFile>  (it1->files).size( ) = " << (it1->files).size( )  <<"\n";
		vector <FileAd>::iterator it2 = (it1->files).begin( );
		vector <FileAd>::iterator const end2 = (it1->files).end( );
		for ( ; it2 != end2; it2++ ) {
			file = it2->file;
			path = jobpath + "/" + Utils::getFileName(it2->file);
//cout << "###createZipFile> file="<< file << "\n";
//cout << "###createZipFile> path="<< path << "\n";
			logInfo->print(WMS_DEBUG, "tar - Archiving the local file: " + file,
				"with the following path: " + path, false);
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
	}
	if (t) {
		// close the file
		tar_append_eof(t);
		tar_close (t);
		logInfo->print(WMS_DEBUG,
				"This archive file has been successfully created:", tar);
		logInfo->print(WMS_DEBUG,
			"Compressing the file (" +Utils::getZipExtension() +"):", tar);
//cout << "###createZipFile> tarFile="<< tar << "\n";
		gz = wmcUtils->fileCompression(tar);
		logInfo->print(WMS_DEBUG,
			"ISB ZIPPED file successfully created:", gz);
	}
	FileAd source(FILE_PROTOCOL, gz, Utils::getFileSize(gz));
	string dest = this->getDestinationURI(this->getJobId( )) + "/" + filename;
	logInfo->print(WMS_DEBUG,
			"ISB Zipped File: " + source.file, "DestURI: " + dest, false);
	to_btransferred.push_back(make_pair(source, dest) );
}


/**
* File transfer by globus-url-copy (gsiftp protocol)
*/

void JobSubmit::gsiFtpTransfer(std::vector <std::pair<glite::wms::jdl::FileAd, std::string> > &paths) {
	string protocol = "";
	string file = "";
	string destination = "";
	//TBDMS: globus-url-copy searched several times
	while (paths.empty()==false) {
		// source
		file = (paths[0].first).file ;
		// destination
		destination = paths[0].second ;
//cout << "###JobSubmit::gsiFtpTransfer> file = " << file << "\n";
// Protocol has to be added only if not yet present
		protocol = (file.find("://")==string::npos)?FILE_PROTOCOL:"";
		// command
		string cmd= "globus-url-copy " + string (protocol+file) + " " + destination;
		if (getenv("GLOBUS_LOCATION")){
			cmd=string(getenv("GLOBUS_LOCATION"))+"/bin/"+cmd;
		}else if (Utils::isDirectory ("/opt/globus/bin")){
			cmd="/opt/globus/bin/"+cmd;
		}else {
			throw WmsClientException(__FILE__,__LINE__,
				"gsiFtpGetFiles", ECONNABORTED,
				"File  Error",
				"Unable to find globus-url-copy executable");
		}
		logInfo->print(WMS_DEBUG, "File Transfer (gsiftp)\n" , cmd);
		if ( system( cmd.c_str() ) ){
			ostringstream info;
			info << file << "\n";
			info << "to " << destination;
			throw WmsClientException(__FILE__,__LINE__,
				"gsiFtpTransfer", ECONNABORTED,"File Transfer Error",
				"unable to transfer the file " + info.str());
		} else{
			logInfo->print(WMS_DEBUG, "File Transfer (gsiftp)", "TRANSFER DONE");
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
			paths.erase(paths.begin());
		}
	}
}
/**
* File transfer by CURL (https protocol)
*/

void JobSubmit::curlTransfer (std::vector <std::pair<glite::wms::jdl::FileAd, std::string> > &paths) {
	// curl struct
	CURL *curl = NULL;
	// curl result code
	CURLcode res;
	// file struct
	FILE * hd_src  = NULL;
	int hd = 0;
	// local filepath
	string file = "" ;
	// destination
	string destination = "";
	ostringstream err;
	// result message
	long	httpcode = 0;
	if ( paths.empty()==false ){
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
			curl_easy_setopt(curl, CURLOPT_SSLCERT, getProxyPath( ));
			curl_easy_setopt(curl, CURLOPT_SSLKEY, getProxyPath( ));
			curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, NULL);
			// curl option: trusted cert dir
			curl_easy_setopt(curl, CURLOPT_CAPATH, getCertsPath());
			// curl options: no verify the ssl properties
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
			// enable uploading
			curl_easy_setopt(curl, CURLOPT_PUT, 1);
			// name of the file to be transferred
			while(paths.empty()==false){
				// path to local file (to be transferred)
				file = (paths[0].first).file;
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
				curl_easy_setopt(curl,CURLOPT_INFILESIZE, (paths[0].first).size);
				// curl options: destination (the 2nd elemnt of the vector)
				curl_easy_setopt(curl,CURLOPT_URL, destination.c_str());
				// log-debug message
				ostringstream info ;
				info << "\nInputSandbox file : " << file << "\n";
				info << "destination URI : " << destination ;
				info << "\nsize : " << (paths[0].first).size << " byte(s)"<< "\n";
				logInfo->print(WMS_DEBUG, "InputSandbox File Transfer", info.str());
				// FILE TRANSFERRING ------------------------------
				res = curl_easy_perform(curl);
				// result
				if ( res == 0 ){
					// SUCCESS !!!
					// Removes the zip file just transferred
					if (zipAllowed) {
						try {
							//###Utils::removeFile(file);
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
					// An error occurred during the file transfer
					curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &httpcode);
					err << "couldn't transfer the InputSandbox file : " <<file ;
					err << "to " << destination << "\nhttp error code: " << httpcode ;
					throw WmsClientException(__FILE__,__LINE__,
							"curlTransfer",  DEFAULT_ERR_CODE,
							"File Transfer Error", err.str()  );
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

/**
* Message for InputSB files that need to be transferred
*/
std::string JobSubmit::transferFilesList(const std::vector <std::pair<glite::wms::jdl::FileAd, std::string> > &paths, const std::string& jobid, const bool &zip) {
	ostringstream info;
	string header = "";
	string label = "";
	int size = paths.size( );
	if (size==0) {
		info << "To complete the submission:\n";
		info << "- no local file in the InputSandbox files to be transferred\n";
		info << "- ";
	} else {
		// Creates a zip file with the ISB files to be transferred if file compression is allowed
		if (zipAllowed && zip) {
			if (size==1) {
				header = "To complete the operation, the following file containing the InputSandbox of the job needs to be transferred:";
			} else {
				header = "To complete the operation, the following files containing the InputSandbox of the job need to be transferred:";
			}
			label = "ISB ZIP file : ";
		} else {
			header = "To complete the operation, the following InputSandbox files need to be transferred:\n";
			label = "InputSandbox file : " ;
		}
		// Message
		info << header << "\n";
		info << "==========================================================================================================\n";
		for (int i=0; i < size; i++) {
			info << label << (paths[i].first).file << "\n";
			info << "Destination : " << paths[i].second << "\n";
			info << "-----------------------------------------------------------------------------\n";
		}
		info << "\nthen " ;
	}
	info << "start the job by issuing a submissiong with the option:\n --start " << this->getJobId( ) << "\n";
	return info.str();
}
/*
* Transfers the ISB file to the endpoint
*/
void JobSubmit::transferFiles(std::vector<std::pair<glite::wms::jdl::FileAd,std::string > > &to_bcopied, const std::string &jobid){
	try {
		// File Transfer according to the chosen protocol
		if (fileProto && *fileProto == Options::TRANSFER_FILES_CURL_PROTO ) {
			this->curlTransfer (to_bcopied);
		} else {
			this->gsiFtpTransfer (to_bcopied);
		}
	} catch (WmsClientException &exc) {
		ostringstream err ;
		err << exc.what() << "\n\n";
		err << transferFilesList (to_bcopied,jobid, false) << "\n";
		throw WmsClientException( __FILE__,__LINE__,
				"transferFiles",  DEFAULT_ERR_CODE,
				"File Transfer Error" ,
				err.str());
	}
}
/**
*  Reads the JobRegister results and checks if there are local files
*  in the InputSandbox to be transferred to the WMProxy server.
* In case of file transfer is not requested (only job registration has to be performed),
* an info message with the list these file is provided
*/
void JobSubmit::jobPostProcessing( ){
	std::vector<glite::wms::jdl::FileAd> files;
	// jobid and nodename
	string jobid = "";
	// InputSB files
	vector <string> paths ;	// paths to ISB local files
	vector <pair<FileAd, string> > to_bcopied ; //paths to ISB local files+WMP URI's
	string gzip = "";	// gzip file
 	if (extractAd == NULL) {
		throw WmsClientException(__FILE__,__LINE__,
			"inputSandboxFiles",  DEFAULT_ERR_CODE ,
			"Null Pointer Error",
			"null pointer to extractAd\n"+Options::BUG_MSG  );
	}
	// JOB-ID
	//##jobid = jobIds.jobid ;
	//###Utils::checkJobId(jobid);
	if (extractAd->hasFiles( )) {
		this->toBCopiedFileList(to_bcopied);
		// if the vector is not empty, file transfer is performed
		if (to_bcopied.empty( )==false){
			if (registerOnly) {
				// If --register-only: message with ISB files list to be printed out
				infoMsg = transferFilesList (to_bcopied, jobid) + "\n";
			} else {
				// Transfers the ISB local files to the server
				transferFiles (to_bcopied, jobid);
			}
		}
	}
}

}}}} // ending namespaces
