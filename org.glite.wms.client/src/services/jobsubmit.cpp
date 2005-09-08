

#include "jobsubmit.h"
// streams
#include <sstream>
#include <iostream>
// exceptions
#include "utilities/excman.h"
// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/adutils.h"
#include "utilities/options_utils.h"

// wmproxy-api
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
#include "boost/tokenizer.hpp" //lrms checks

// CURL
#include "curl/curl.h"



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


const string ISBFILE_DEFAULT = "ISBfiles";
const int MAX_FILE_SIZE =  2140000000;	//2147 48 36 47

/*
*	default constructor
*/
JobSubmit::JobSubmit( ){
	// init of the string attributes
	dgOpt = NULL;
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
	transferMsg = "";
	// TAR filename
	tarFileName = Utils::getUniqueString ( )  ;
	if (tarFileName){
		*tarFileName = ISBFILE_DEFAULT + "_" + *tarFileName + Utils::getArchiveExtension( );
	} else{
		ostringstream oss;
		oss << ISBFILE_DEFAULT << "_" << getpid( ) << "_" << getuid( ) <<Utils::getArchiveExtension( );
		tarFileName = new string (oss.str());
	}
	// As default  file archiving and compression is allowed
	zipAllowed = true;
};

/*
*	Default destructor
*/
JobSubmit::~JobSubmit( ){

	if (dgOpt){ delete(dgOpt); }
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
	if ( tarFileName){ delete( tarFileName); }

};


/*
* Handle the command line options
*/
void JobSubmit::readOptions (int argc,char **argv){
	ostringstream info ;
	vector<string> wrongids;
	vector<string> resources;
	string opts = Job::readOptions  (argc, argv, Options::JOBSUBMIT);
	// writes the information on the specified option in the log file
	logInfo->print(WMS_INFO,   ">>>>>>> Function Called:", wmcOpts->getApplicationName( ), false);
	logInfo->print(WMS_INFO, ">>>>>>> Options specified:", opts ,false);
	// input & resource (no together)
	inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	resourceOpt = wmcOpts->getStringAttribute(Options::RESOURCE);
	if (inOpt && resourceOpt ){
		info << "the following options cannot be specified together:\n" ;
		info << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		info << wmcOpts->getAttributeUsage(Options::RESOURCE) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", info.str());
	}
	if (inOpt){
		// Retrieves and check resources from file
		resources= wmcUtils->getItemsFromFile(*inOpt, Utils::CE_TYPE);
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
	if (validOpt){
		try{
			expireTime =  Utils::checkTime(*validOpt, Options::TIME_VALID) ;
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
		expireTime= Utils::checkTime(*toOpt, Options::TIME_TO) ;
		} catch (WmsClientException &exc) {
			info << exc.what() << " (use: " << wmcOpts->getAttributeUsage(Options::TO) <<")\n";
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Wrong Time Value",info.str() );
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
	(registerOnly || inOpt || resourceOpt || toOpt || validOpt || chkptOpt || collectOpt ||
		(wmcOpts->getStringAttribute(Options::DELEGATION) != NULL) ||
		wmcOpts->getBoolAttribute(Options::AUTODG)  )){
		info <<"it can not be used together the following options:\n";
		info << wmcOpts->getAttributeUsage(Options::REGISTERONLY) << "\n";
		info << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		info << wmcOpts->getAttributeUsage(Options::RESOURCE) << "\n";
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
	if (startOpt){startOpt = new string(Utils::checkJobId(*startOpt));}
	// file Protocol
	fileProto= wmcOpts->getStringAttribute( Options::PROTO) ;
	if (!fileProto) {
		fileProto= new string (Options::TRANSFER_FILES_DEF_PROTO );
	} else  if (startOpt) {
		logInfo->print (WMS_WARNING, "--proto: option ignored (start operation doesn't need any file transferring)", "", true );
	}
	if (!startOpt){
		// Delegation ID
		dgOpt = wmcUtils->getDelegationId ();
		if ( ! dgOpt  ){
			throw WmsClientException(__FILE__,__LINE__,
					"readOptions",DEFAULT_ERR_CODE,
					"Missing Information",
					"no proxy delegation ID" );
		}
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

/*
* performs the main operation for the submission
*/
void JobSubmit::submission ( ){
	// proxy validity must be at least 20 minutes
	postOptionchecks(20);
	ostringstream out ;
	string jobid = "";
	bool toBretrieved = false;
	if (!cfgCxt){
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE ,
			"Null Pointer Error",
			"null pointer to ConfigContext object"   );
	}
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
		// if the autodelegation is needed
		if (wmcOpts->getBoolAttribute(Options::AUTODG)) {
			endPoint = new string(wmcUtils->delegateProxy (cfgCxt, *dgOpt) );
		}
		// reads and checks the JDL('s)
		wmsJobType jobtype ;
		this->checkAd(toBretrieved, jobtype);
/*
<<<<<<< jobsubmit.cpp
		jobid = jobRegOrSub(startJob);
		//if (registerOnly || toBretrieved) {
		if (toBretrieved) {
=======
*/
		// Perform Submission when:
		// (RegisterOnly has not been specified in CLI) && (no file to be transferred)
		jobid = jobRegOrSub(startJob && !toBretrieved);
		logInfo->print(WMS_DEBUG, "The JobId is: ", jobid) ;
		// Perform File Transfer when:
		// (Registeronly is NOT specified [or specified with --tranfer-file]) AND (some files are to be transferred)
		if (toBretrieved){
			try{
//>>>>>>> 1.8.2.72
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
			logInfo->print(WMS_DEBUG, "No local files in the InputSandbox to be transferred", "") ;
			if (!startJob) {
				transferMsg = "To complete the operation start the job by issuing a submission with the option:\n";
				transferMsg += " --start " + jobid + "\n";
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
		jobShadow->setJobId(jobid);
		out << "Input Stream  location: " << jobShadow->getPipeIn() <<"\n";
		out << "Output Stream  location: " << jobShadow->getPipeOut() <<"\n";
		if (jobShadow->isLocalConsole()){
			out << "Shadow process Id: " << jobShadow->getPid() << "\n";
			if (wmcOpts->getBoolAttribute(Options::DBG)){
				out << "Interactive Listening Host: " << jobShadow->getHost() <<"\n";
				out << "Interactive Listening Port: " << jobShadow->getPort() <<"\n";
			}
		}else {
			out << "Remote Shadow console set: " << jobShadow->getHost() <<"\n";
		}
	}
	// saves the result
	if (outOpt){
		if ( wmcUtils->saveJobIdToFile(*outOpt, jobid) < 0 ){
			logInfo->print (WMS_WARNING, "unable to write the jobid to the output file " , Utils::getAbsolutePath(*outOpt));
		} else{
			logInfo->print (WMS_DEBUG, "The JobId has been saved in the output file ", Utils::getAbsolutePath(*outOpt));
			out << "\nThe job identifier has been saved in the following file:\n";
			out << Utils::getAbsolutePath(*outOpt) << "\n";
		}
	}
	out << "\n" << wmcUtils->getStripe(74, "=") << "\n\n";

	if (transferMsg.size() > 0) {
		out << transferMsg << "\n";
		logInfo->print (WMS_INFO, transferMsg , "", false);
	}
	out << getLogFileMsg ( ) << "\n";
	// ==============================================================
	// Displays the output message
	cout << out.str() ;
	// Interactive Jobs management:
	if (jobShadow!=NULL){
		if (jobShadow->isLocalConsole()){
			// Interactive console needed
			logInfo->print(WMS_DEBUG,"Running Listener",jobid);
			Listener listener(jobShadow);
			listener.run();
		}
	}
}
/*====================================
	private methods
==================================== */
/*
* reads and checks the JDL file
*/
void JobSubmit::checkAd(bool &toBretrieved, wmsJobType &jobtype){
	jobtype = WMS_JOB;
	toBretrieved =true ;
	glite::wms::common::configuration::WMCConfiguration* wmcConf =wmcUtils->getConf();
	if (collectOpt) {
		logInfo->print (WMS_DEBUG, "A collection of jobs is being submitted; JDL files in:",
			Utils::getAbsolutePath( *collectOpt));
		jobtype = WMS_COLLECTION ;
		try {
			fs::path cp ( Utils::normalizePath(*collectOpt), fs::system_specific);
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
		// JDL string
		collectAd = collectAd->check();
		toBretrieved =collectAd->gettoBretrieved();
/*		if (toBretrieved){
			// ISB ZIP management
			collectAd->setAttribute(JDL::ZIPPED_ISB , *tarFileName+Utils::getZipExtension());
		}
*/
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
		// checks if the file archiving and compression is denied (if ALLOW_ZIPPED_ISB is not present, default value is true)
		zipAllowed = ( ! jobAd->hasAttribute(JDL::ALLOW_ZIPPED_ISB)) || jobAd->getBool(JDL::ALLOW_ZIPPED_ISB) ;
		if (!zipAllowed){
			logInfo->print (WMS_DEBUG, "File compression is not allowed by user", "");
		}
		// Adds ExpireTime JDL attribute
		if ((int)expireTime>0) {
			jobAd->addAttribute (JDL::EXPIRY_TIME, (double)expireTime);
		}
		// Simple Ad manipulation (common)
		AdUtils::setDefaultValuesAd(jobAd,wmcConf);
		if ( jobAd->hasAttribute(JDL::TYPE , JDL_TYPE_COLLECTION) ) {
			logInfo->print (WMS_DEBUG, "A collection of jobs is being submitted", "");
			jobtype = WMS_COLLECTION ;
			try{
				collectAd = new CollectionAd(*(jobAd->ad()));
				collectAd->setLocalAccess(true);
				// Collect Ad manipulation
				AdUtils::setDefaultValues(collectAd,wmcConf);
				// JDL string
				collectAd = collectAd->check();
				toBretrieved =collectAd->gettoBretrieved();
/*				if (toBretrieved){
					// ISB ZIP management
					collectAd->setAttribute(JDL::ZIPPED_ISB , *tarFileName+Utils::getZipExtension());
				}
*/
				jdlString = new string(collectAd->toString());
			}catch (Exception &ex){
				throw WmsClientException(__FILE__,__LINE__,
					"submission",  DEFAULT_ERR_CODE,
					"Invalid JDL collection",
					ex.what()   );
			}
		}  else if ( jobAd->hasAttribute(JDL::TYPE , JDL_TYPE_DAG) ) {  // DAG
				logInfo->print (WMS_DEBUG, "A DAG job is being submitted.", "");
				jobtype = WMS_DAG ;
				dagAd = new ExpDagAd (jobAd->toString());
				dagAd->setLocalAccess(true);
				AdUtils::setDefaultValues(dagAd,wmcConf);
				// expands the DAG loading all JDL files
				dagAd->getSubmissionStrings();
				toBretrieved=dagAd->gettoBretrieved();
/*
				if (toBretrieved){
					// ISB ZIP management
					dagAd->setAttribute(JDL::ZIPPED_ISB , *tarFileName+Utils::getZipExtension());
				}
*/
				// JDL string for the DAG
				jdlString = new string(dagAd->toString()) ;
		}else{   // JOB
			// WMS_JOB
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
			toBretrieved=pass->gettoBretrieved();
/*
			if (toBretrieved){
				// ISB ZIP management
				pass->setAttribute(JDL::ZIPPED_ISB , *tarFileName+Utils::getZipExtension());
			}
*/
			jdlString = new string(pass->toString());
			// Parametric support
			if (  jobAd->hasAttribute(JDL::JOBTYPE,JDL_JOBTYPE_PARAMETRIC)){
				jobtype = WMS_PARAMETRIC;
				dagAd=AdConverter::bulk2dag(pass);
				AdUtils::setDefaultValues(dagAd,wmcConf);
				dagAd->getSubmissionStrings();
				toBretrieved=dagAd->gettoBretrieved();
			}
			delete(pass);
		}
	}
	// --resource : incompatible argument
	if( (resourceOpt) && (jobtype!= WMS_JOB)){
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
			"Incompatible Argument: " + wmcOpts->getAttributeUsage(Options::RESOURCE),
			"cannot be used for  DAG, collection, partitionable and parametric jobs");
	}
}

std::string JobSubmit::jobRegOrSub(bool submit ) {
	vector<string> urls ;
	int index = 0;
	// flag to stop while-loop
	bool success = false;
	// number of enpoint URL's
	int n = 0;
	// checks if jdlstring is not null
	if (!jdlString){
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE ,
			"Null Pointer Error",
			"null pointer to JDL string");
	}
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
		// jobRegister
		logInfo->print(WMS_INFO, "Connecting to the service", *endPoint);
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
			success = true;
		} catch (BaseException &exc) {
			if (n==1) {
				ostringstream err ;
				err << "Unable to "<< method << " the job to the service: " << *endPoint << "\n";
				err << errMsg(exc) ;
				// in case of any error on the only specified endpoint
				throw WmsClientException(__FILE__,__LINE__,
					"job"+method, ECONNABORTED,
					"Operation failed", err.str());
			} else {
				logInfo->print  (WMS_INFO, "Connection failed:", errMsg(exc));
				sleep(1);
				if (urls.empty( )){
					throw WmsClientException(__FILE__,__LINE__,
					"job"+method, ECONNABORTED,
					"Operation failed",
					"Unable to " +method +" the job to any specified endpoint");
				}
			}
		}
		// exits from the loop in case of success
		if (success){ break;}
	}
	return jobIds.jobid;
}

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
/*
*       contacts the endpoint configurated in the context
*       in order to retrieve the destionationURIs of the job
*       identified by the jobid
*/
std::string* JobSubmit::getInputSBDestURI(const std::string &jobid, const std::string &child, std::string &zipURI) {
        string *dest_uri = NULL;
        string look_for = "";
        vector<string> jobids;
        vector< pair<string ,vector<string > > >::iterator it1 ;
        vector<string>::iterator it2;
        bool found = false;
        // The destinationURI's vector is empty: the WMProxy service is called
        if (dsURIs.empty( )){
                try{
                        dsURIs = getSandboxBulkDestURI(jobid, (ConfigContext *)cfgCxt);
                } catch (BaseException &exc){
                        throw WmsClientException(__FILE__,__LINE__,
                                "getSandboxDestURI ", ECONNABORTED,
                                "WMProxy Server Error", errMsg(exc));
                }
                if (dsURIs.empty( )){
                        throw WmsClientException(__FILE__,__LINE__,
                                "getSandboxDestURI ", ECONNABORTED,
                                "WMProxy Server Error",
                                "The server doesn't have information on InputSBDestURI for :" + jobid );
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
                                        // loop-exit if "TAR/ZIP-destURI" has been already found
                                       if (found){
                                                break;
                                        } else {
                                                found = true ;
                                                logInfo->print(WMS_DEBUG, "DestinationURI:", *dest_uri);
                                        }
                                }
                                // Looks for the destURI for TAR/ZIP file creation
                                if ( it2->substr (0, (Options::DESTURI_ZIP_PROTO.size()) ) == Options::DESTURI_ZIP_PROTO){
                                        zipURI = string (*it2);
                                        // loop-exit if "fileTransfer-destURI" has been already found
                                        if (found){
                                                break;
                                        } else {
                                                found = true ;
                                        }
                                }
                        }
                }
		if (found) break;
        }
        return dest_uri ;
}
/*
*	checks if the user free quota (on the server endpoint) could support (in size) the transferring of a set of files
*/
bool JobSubmit::checkFreeQuota ( std::vector<std::pair<std::string,std::string> > files, long &limit )
{
	FILE* fptr = NULL;
	vector<pair<string,string> >::iterator it ;
	bool not_exceed = true;
	// results of getFrewQuota
	pair<long, long> free_quota ;
	long size = 0;
	long max_isbsize = 0;
	ostringstream lim;
	ostringstream s;
	if (cfgCxt) {
		// gets the user free quota from the endpoint
		try{
			logInfo->print(WMS_DEBUG, "Requesting for user free quota on the server", "");
			free_quota = getFreeQuota(cfgCxt);
		} catch (BaseException &exc){
				throw WmsClientException(__FILE__,__LINE__,
					"getFreeQuota", ECONNABORTED,
					"WMProxy Server Error", errMsg(exc));
		}
		// soft limit
		limit = free_quota.first;
		// if the previous function gets back a negative number
		// the user free quota is not set on the server and
		// no check is performed (this functions returns not exceed=true)
		if ( limit  > 0 ) {
			lim << limit ;
			logInfo->print (WMS_DEBUG, "User free quota (bytes): ", lim.str());
			// number of bytes to be transferred
			for ( it = files.begin() ;  it != files.end() ; it++ ) {
				// adds up the size of the size
				size += Utils::getFileSize( (it->first).c_str());
				s << size ;
				logInfo->print (WMS_DEBUG, "Size of the InputSanbox files (bytes): ", s.str() );
				if ( size > limit) {
					not_exceed = false;
					fclose (fptr);
					break;
				}
				fclose (fptr);
			} // for
			if (not_exceed) {
				logInfo->print (WMS_DEBUG, "The InputSandbox size doesn't exceed the user free quota:", "file transferring is allowed" );
			} else {
				logInfo->print (WMS_DEBUG, "The InputSandbox size exceeds the user free quota:", "file transferring is not allowed" );
			}
		} else {
			// User quota is not set on the server: check of Max InputSB size
			logInfo->print (WMS_DEBUG, "User free quota is not set on the server:", " requesting for max InputSandbox size on the server" );
			try{
				max_isbsize = getMaxInputSandboxSize(cfgCxt);
			} catch (BaseException &exc){
					throw WmsClientException(__FILE__,__LINE__,
						"getMaxInputSandboxSize", ECONNABORTED,
						"WMProxy Server Error", errMsg(exc));
			}
			if (max_isbsize>0 ) {
				if (size < max_isbsize) {
					logInfo->print (WMS_DEBUG, "The InputSandbox size doesn't exceed the max size allowed on the server:", "file transferring is allowed" );
					not_exceed = true;
				} else  {
					logInfo->print (WMS_DEBUG, "The InputSandbox size exceeds the max size allowed on the server:", "file transferring is not allowed" );
					not_exceed = false;;
				}
			}
		}
	} // if cfgCxt
	return not_exceed ;
}

// use globus-url-copy
void JobSubmit::gsiFtpTransfer(std::vector<std::pair<std::string,std::string> > &paths) {
	string protocol = "";
	pair<string,string> it ;
	//TBDMS: globus-url-copy searched several times
	while (!paths.empty()) {
		it = paths[0];
		// Protocol has to be added only if not yet present
		protocol = (it.first.find("://")==string::npos)?"file://":"";
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
			paths.erase(paths.begin());
			logInfo->print(WMS_DEBUG, "File Transferring (gsiftp)", "TRANSFER DONE");
		}
	}
}
// use CURL
/*
* Performs the InputSB file transferring by curl
*/
void JobSubmit::curlTransfer (std::vector<std::pair<std::string,std::string> > paths)
{
	// curl struct
	CURL *curl = NULL;
	// curl result code
	CURLcode res;
	// file struct
	FILE * hd_src  = NULL;
	struct stat file_info;
	int fsize = 0;
	int hd = 0;
	// vector iterator
	vector <pair<string , string> >::iterator it ;
	// local filepath
	string file = "" ;
	// result message
	string result = "";
	long	httpcode = 0;
	if ( !paths.empty() ){
		// checks the user proxy pathname
		if (!proxyFile){
			throw WmsClientException(__FILE__,__LINE__,
				"transferFiles", DEFAULT_ERR_CODE,
				"Missing Proxy",
				"unable to determine the proxy file" );
		}
		// checks the trusted cert dir
		if (!trustedCert){
			throw WmsClientException(__FILE__,__LINE__,
				"transferFiles", DEFAULT_ERR_CODE,
				"Directory Not Found",
				"unable to determine the trusted certificate directory" );
		}
		logInfo->print (WMS_DEBUG, "curl SSL option - Proxy File", string( proxyFile)  );
		logInfo->print (WMS_DEBUG, "curl SSL option - Trusted Cert Path", string( trustedCert)  );
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
			for ( it = paths.begin() ; it != paths.end() ; it++ ){
				// curl options: source (first element of the vector)
				hd_src = fopen((it->first).c_str(), "rb");
				if (hd_src == NULL) {
					throw WmsClientException(__FILE__,__LINE__,
						"transferFiles",  DEFAULT_ERR_CODE,
						"File Not Found", "no such file : " + it->first  );
				}
				// Reads the local file
				curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
				// curl options: file size
				hd = open((it->first).c_str(), O_RDONLY) ;
				fstat(hd, &file_info);
				close(hd) ;
				fsize = file_info.st_size;
				if (fsize >0){
					curl_easy_setopt(curl,CURLOPT_INFILESIZE, file_info.st_size);
					// curl options: destination (the 2nd elemnt of the vector)
					curl_easy_setopt(curl,CURLOPT_URL, (it->second).c_str());
					// log-debug message
					ostringstream info ;
					info << "\nInputSandbox file : " << it->first<< "\n";
					info << "destination URI : " << it->second ;
					info << "\nsize : " << fsize << " byte(s)"<< "\n";
					logInfo->print(WMS_DEBUG, "InputSandbox File Transferring", info.str());
					// transferring ......
					res = curl_easy_perform(curl);
					// result
					if ( res != 0 ){
						ostringstream err;
						curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &httpcode);
						err << "couldn't transfer the InputSandbox file : " << it->first ;
						err << "to " << it->second << "\nhttp error code: " << httpcode ;
						throw WmsClientException(__FILE__,__LINE__,
								"transferFiles",  DEFAULT_ERR_CODE,
								"File Transfer Error", err.str()  );
					}
				} else {
					ostringstream warn;
					warn << it->first << ": Invalid  File Size (" << fsize << "bytes)";
					logInfo->print(WMS_WARNING, warn.str(), it->first);
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
		dest_uri = getInputSBDestURI(jobid, child, zip_uri);
		if (!dest_uri){
			throw WmsClientException(__FILE__,__LINE__,"getInputSBDestURI",DEFAULT_ERR_CODE,
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
std::string JobSubmit::createZipFile (std::vector <std::pair<std::string, std::string> > &to_bcopied,
							const std::string &destURI){
	string file = Utils::archiveFiles(to_bcopied, "/tmp", *tarFileName) ;
	// Compress the tar file
	file = Utils::compressFile (file) ;
	to_bcopied.clear( );
	// Inserts in the vector the single zip file to be transferred to the server
	to_bcopied.push_back(make_pair(file, string(destURI + "/" + Utils::getFileName(file)) ) );
	return file ;
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
*  reads the JobRegister results for a normal job and performs the transferring of the
*  local InputSandbox files (called by the main method: submission)
*/
std::string JobSubmit::normalJob(){
	// jobid and nodename
	string jobid = "";
	string node = "";
	// DestinationURI string (for file transferring)
	string *destURI = NULL;
	// InputSB files
	long freequota ;
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
		// gets the InputSandbox files
		string isb_uri= jobAd->hasAttribute(JDL::ISB_BASE_URI)?jobAd->getString(JDL::ISB_BASE_URI):"";
		paths = jobAd->getStringValue  (JDL::INPUTSB) ;
		// InputSandbox file transferring
		destURI = this->toBCopiedFileList(jobid, "", isb_uri, paths, to_bcopied);
		// if the vector is not empty, file transferring is performed
		if (! to_bcopied.empty( ) ){
			// checks user free quota
			if (  ! checkFreeQuota (to_bcopied, freequota) ){
				ostringstream err ;
				err << "not enough free quota on the server for the InputSandbox files (freequota=" ;
				err << freequota << " bytes)";
				throw WmsClientException( __FILE__,__LINE__,
						"transferFiles",  DEFAULT_ERR_CODE,
						"User FreeQuota Error" ,
						err.str());
			}
			if (registerOnly) {
				// If --register-only: message with ISB files list to be printed out
				transferMsg = transferFilesList (to_bcopied, jobid) + "\n";
			} else {
				// Creates a zip file with the ISB files to be transferred if file compression is allowed
				if (zipAllowed) {
					gzip = createZipFile(to_bcopied, *destURI);
					logInfo->print (WMS_DEBUG, "InputSandbox files in: " + gzip, "");
				}
				try {
					// File Transferring according to the chosen protocol
					if (fileProto!= NULL && *fileProto== Options::TRANSFER_FILES_CURL_PROTO ) {
						this->curlTransfer (to_bcopied);
					} else {
						this->gsiFtpTransfer (to_bcopied);
					}

				} catch (WmsClientException &exc) {
					ostringstream err ;
					err << exc.what() << "\n\n";
					err << transferFilesList(to_bcopied, jobid) << "\n";
					throw WmsClientException( __FILE__,__LINE__,
							"transferFiles",  DEFAULT_ERR_CODE,
							"File Transferring Error" ,
							err.str());
				}
				// Removes the temporary zip file if it exists
				if (gzip.size()>0 && Utils::isFile(gzip)) {
					try {
						//Utils::removeFile(gzip);
						logInfo->print (WMS_DEBUG, "Temporary archive for InputSandbox files removed", "(" + gzip + ")");
					} catch (WmsClientException &exc) {
						logInfo->print(WMS_WARNING,"Unable to remove temporary file created for the InputSandbox files", exc.what( ) );
					}
				}
				if (!startJob){
					transferMsg = "To complete the operation start the job by issuing a submission with the option:\n --start "
					+ jobid + "\n";
				}
			}
		} else {
			logInfo->print (WMS_DEBUG, "No local files in the InputSandbox to be transferred", "");
			if (!startJob){
				transferMsg = "To complete the operation start the job by issuing a submission with the option:\n --start "
				+ jobid + "\n";
			}
		}
	}
	return jobid;
}
/*
*  reads the JobRegister results for a normal job and performs the transferring of the
*  local InputSandbox files (called by the main method: submission)
*/
std::string  JobSubmit::dagJob(){
	// jobid and node-name
	string jobid = "";
	string child = "";
	string node = "";
	string *destURI = NULL;
	string zip_uri = "";
	// InputSB files
	long freequota ;
	vector <string> paths ;
	vector <pair<string, string> > to_bcopied ;
	std::vector<std::pair<std::string,std::string> > copied ;
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
		// InputSB files to be transferred to the DestURI
		isb_uri=dagAd->hasAttribute(JDL::ISB_BASE_URI)?dagAd->getAttribute(ExpDagAd::ISB_DEST_URI ):"";
		paths = dagAd->getInputSandbox();
		if (!paths.empty()) {
			destURI = this-> toBCopiedFileList(jobid, "", isb_uri, paths, to_bcopied) ;
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
				info << "JobId node : " << child << "\n";
				info << "Node name : " << node << "\n";
				logInfo->print(WMS_DEBUG, "DAG_CHILD_NODE",  info.str(), false);
				// child: InputSandbox files
				if (dagAd->hasNodeAttribute(node, JDL::INPUTSB) ){
					if (dagAd->hasNodeAttribute(node, JDL::ISB_BASE_URI)){
						isb_uri=dagAd->getNodeAttribute(node, JDL::ISB_BASE_URI);
					}else {
						isb_uri="";
					}
					paths = dagAd->getNodeStringValue(node, JDL::INPUTSB);
					this->toBCopiedFileList(jobid, child, isb_uri, paths, to_bcopied );
				}
			}
		}
	}
	if (! to_bcopied.empty( ) ){
		// checks user free quota
		if (  ! checkFreeQuota (to_bcopied, freequota) ){
			ostringstream err ;
			err << "not enough free quota on the server for the InputSandbox files (freequota=" ;
			err << freequota << " bytes)";
			throw WmsClientException( __FILE__,__LINE__,
					"transferFiles",  DEFAULT_ERR_CODE,
					"User FreeQuota Error" ,
					err.str());
		}
		if (registerOnly) {
			// If --register-only: message with ISB files list to be printed out
			transferMsg = transferFilesList (to_bcopied, jobid) + "\n";
		} else {
			// Destination URI (in case this info has not been retrieved before)
			if (!destURI) {
				destURI = getInputSBDestURI (jobid, child, zip_uri);
				if (!destURI){
					throw WmsClientException(__FILE__,__LINE__,"getInputSBDestURI",DEFAULT_ERR_CODE,
					"Missing Information","unable to retrieve the InputSB DestinationURI for the job: " +jobid  );
				}
			}
			// Creates a zip file with the ISB files to be transferred if file compression is allowed
			if (zipAllowed) {
				gzip = createZipFile(to_bcopied,*destURI);
				logInfo->print (WMS_DEBUG, "InputSandbox files in: " + gzip, "");
			}
			// File Transferring according to the chosen protocol
			try {

				if (fileProto!= NULL && *fileProto== Options::TRANSFER_FILES_CURL_PROTO ) {
					this->curlTransfer (to_bcopied);
				} else {
					this->gsiFtpTransfer (to_bcopied);
				}
			} catch (WmsClientException &exc) {
				ostringstream err ;
				err << exc.what() << "\n\n";
				err << transferFilesList(to_bcopied, jobid) << "\n";
				throw WmsClientException( __FILE__,__LINE__,
					"transferFiles",  DEFAULT_ERR_CODE,
					"File Transferring Error" ,
					err.str());
			}
			// Removes the temporary Zip file if it exists
			if (gzip.size()>0 && Utils::isFile(gzip)) {
				try {
					//Utils::removeFile(gzip);
					logInfo->print (WMS_DEBUG, "Temporary archive for InputSandbox files removed", "(" + gzip + ")");
				} catch (WmsClientException &exc) {
					logInfo->print(WMS_WARNING,"Unable to remove temporary file created for the InputSandbox files", exc.what( ) );
				}
			}
			// Info message for start in case of register only
			if (!startJob){
				transferMsg = "To complete the operation start the job by issuing a submission with the option:\n --start "
				+ jobid + "\n";
			}
		}
	} else {
		logInfo->print (WMS_DEBUG, "No local files in the InputSandbox to be transferred", "");
		// Info message for start in case of register only
		if (!startJob){
			transferMsg += "To complete the operation start the job by issuing a submission with the option:\n --start "
			+ jobid + "\n";
		}
	}
	return jobid;
}

/*
* Gets the list of InputSB in the "list" vector for "node"
*/
vector<string> JobSubmit::getNodeFileList (const string &node, vector< pair<string ,vector<string > > > &list){
	const vector<string> empty;
	vector< pair<string ,vector<string > > >::iterator it ;
	for ( it = list.begin() ; it != list.end( ) ; it++ ){
		if (node == it->first){
			return it->second ;
		}
	}
	return empty;
}
/*
*  reads the JobRegister results for a normal job and performs the transferring of the
*  local InputSandbox files (called by the main method: submission)
*/
std::string JobSubmit::collectionJob() {
	// jobid and node-name
	string jobid = "";
	string child = "";
	string node = "";
	string* destURI = NULL;
	string gzip = "";
	// InputSB files
	long freequota ;
	vector<string> paths;
	vector <pair<string, string> > to_bcopied ;
	if (!collectAd){
		throw WmsClientException(__FILE__,__LINE__,
		"JobSubmit::collectionJob",  DEFAULT_ERR_CODE,
		"Null Pointer Error",
		"null pointer to dagAd object" );
	}
	// MAIN JOB ====================
	// jobid
	jobid = jobIds.jobid;
	Utils::checkJobId(jobid);
	// Main InputSandbox
	if (collectAd->hasAttribute(JDL::INPUTSB)){
		// InputSB files to be transferred to the DestURI
		paths = collectAd->getStringValue(JDL::INPUTSB);
		string isb_uri= collectAd->hasAttribute(JDL::ISB_BASE_URI)?collectAd->getString(JDL::ISB_BASE_URI):"";
		destURI = this->toBCopiedFileList(jobid, "", isb_uri, paths, to_bcopied);
		if (  ! to_bcopied.empty( ) ){
			// checks user free quota
			if (  ! checkFreeQuota (to_bcopied, freequota) ){
				ostringstream err ;
				err << "not enough free quota on the server for the InputSandbox files (freequota=" ;
				err << freequota << " bytes)";
				throw WmsClientException( __FILE__,__LINE__,
						"transferFiles",  DEFAULT_ERR_CODE,
						"User FreeQuota Error" ,
						err.str());
			}
			if (registerOnly) {
				// If --register-only: message with ISB files list to be printed out
				transferMsg += transferFilesList (to_bcopied, jobid) + "\n";
			} else {
				// Creates a zip file with the ISB files to be transferred if file compression is allowed
				if (zipAllowed) {
					gzip = createZipFile(to_bcopied, *destURI);
					logInfo->print (WMS_DEBUG, "InputSandbox files in: " + gzip, "");
				}
				// File Transferring according to the chosen protocol
				try {
					if (fileProto!= NULL && *fileProto== Options::TRANSFER_FILES_CURL_PROTO ) {
						this->curlTransfer (to_bcopied);
					} else {
						this->gsiFtpTransfer (to_bcopied);
					}
				} catch (WmsClientException &exc) {
					ostringstream err ;
					err << exc.what() << "\n\n";
					err << transferFilesList(to_bcopied, jobid) << "\n";
					throw WmsClientException( __FILE__,__LINE__,
							"transferFiles",  DEFAULT_ERR_CODE,
							"File Transferring Error" ,
							err.str());
				}
				// Removes the temporary Zip file if it exists
				if (gzip.size()>0 && Utils::isFile(gzip)) {
					try {
						//Utils::removeFile(gzip);
						logInfo->print (WMS_DEBUG, "Temporary archive for InputSandbox files removed", "(" + gzip + ")");
					} catch (WmsClientException &exc) {
						logInfo->print(WMS_WARNING,"Unable to remove temporary file created for the InputSandbox files", exc.what( ) );
					}
				}
				// Info message for start in case of register only
				if (!startJob){
					transferMsg = "To complete the submission start the job running this command with the option --start " + jobid + "\n";
				}
			}
		} else {
			logInfo->print (WMS_DEBUG, "No local files in the InputSandbox to be transferred", "");
			// Info message for start in case of register only
			if (!startJob){
				transferMsg = "To complete the submission start the job running this command with the option --start " + jobid + "\n";
			}
		}
	}

	return jobid;
}

}}}} // ending namespaces
