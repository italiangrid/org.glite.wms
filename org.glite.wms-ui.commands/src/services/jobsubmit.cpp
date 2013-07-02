/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


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
// Ad attributes and JDL methods
#include "glite/jdl/jdl_attributes.h"
#include "glite/jdl/JDLAttributes.h"
#include "glite/jdl/PrivateAttributes.h"
#include "glite/jdl/adconverter.h"
// BOOST
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/exception.hpp"
#include <boost/lexical_cast.hpp>
// TAR
//#include "libtar.h"

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace std ;
using namespace glite::wms::client::utilities ;

using namespace glite::wms::wmproxyapiutils;
using namespace glite::jdl;
using namespace glite::wmsutils::exception;
namespace fs = boost::filesystem;
namespace api = glite::wms::wmproxyapi;

namespace glite {
namespace wms{
namespace client {
namespace services {

std::string join( const std::vector<std::string>& array, const std::string& sep)
{
  vector<string>::const_iterator sequence = array.begin( );
  vector<string>::const_iterator end_sequence = array.end( );
  if(sequence == end_sequence) return "";

  string joinstring( "" );
  if (sequence != end_sequence) {
    joinstring += *sequence;
    ++sequence;
  }

  for( ; sequence != end_sequence; ++sequence )
    joinstring += sep + *sequence;
  
  return joinstring;
}

/**
* File structs
*/

JobFileAd::JobFileAd ( ){
	this->jobid = "";
	this->node  = "";
};

ZipFileAd::ZipFileAd( ) {
	this->filename  = "";
};

const int SUCCESS = 0;
const int FORK_FAILURE = -1;
const int TIMEOUT_FAILURE = -3;
const int COREDUMP_FAILURE = -2;
const string FILE_PROTOCOL = "file://" ;
const string ISBFILE_DEFAULT = "ISBfiles";
const string TMP_DEFAULT_LOCATION = "/tmp";
const int HTTP_OK = 200;
const int TRANSFER_OK = 0;
/**
*	Default constructor
*/
JobSubmit::JobSubmit( ){
	// init of the string attributes
	m_collectOpt = "";
	m_jsdlOpt = "";
	m_dagOpt = "";
	m_defJdlOpt = "";
	m_lrmsOpt = "";
	m_toOpt = "";
	m_inOpt = "";
	m_resourceOpt = "";
	m_startOpt = "";
	// init of the valid attribute (long type)
	m_validOpt = "";
	// init of the boolean attributes
	nomsgOpt = false ;
	json = false;
	prettyprint = false;
	nolistenOpt = false ;
	startJob = false ;
	// JDL file
	m_jdlFile = "" ;
	// Ad's
	adObj = NULL;
	jobAd = NULL;
	dagAd = NULL;
	collectAd = NULL;
	extractAd = NULL;
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
	if (adObj){ delete(adObj); }
	if (jobAd){ delete(jobAd); }
	if (dagAd){ delete(dagAd); }
	if (collectAd){ delete(collectAd);  }
}

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
	m_inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	m_resourceOpt = wmcOpts->getStringAttribute(Options::RESOURCE);
	m_nodesresOpt = wmcOpts->getStringAttribute(Options::NODESRES);

	if (!m_inOpt.empty() && (!m_resourceOpt.empty() || !m_nodesresOpt.empty() ) ){
		info << "The following options cannot be specified together:\n" ;
		info << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		info << wmcOpts->getAttributeUsage(Options::RESOURCE) << "\n";
		info << wmcOpts->getAttributeUsage(Options::NODESRES) << "\n";

		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", info.str());
	}
	if (!m_inOpt.empty()){
		// Retrieves and check resources from file
		resources= wmcUtils->getItemsFromFile(m_inOpt);
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
				m_resourceOpt = resources[0];
			} else {
				m_resourceOpt = resources[0];
			}
			logInfo->print(WMS_DEBUG,   "--input option: The job will be submitted to the resource", m_resourceOpt);
		}
	}

	nomsgOpt = wmcOpts->getBoolAttribute (Options::NOMSG);
	json = wmcOpts->getBoolAttribute (Options::JSON);
	prettyprint = wmcOpts->getBoolAttribute (Options::PRETTYPRINT);

	if (!m_outOpt.empty() && json) {
	  ostringstream info ;
	  info << "The following options cannot be specified together:\n" ;
	  info << wmcOpts->getAttributeUsage(Options::OUTPUT) << "\n";
	  info << wmcOpts->getAttributeUsage(Options::JSON) << "\n";
	  
	  throw WmsClientException(__FILE__,__LINE__,
				   "readOptions",DEFAULT_ERR_CODE,
				   "Input Option Error", info.str());
	}

	if (json && nomsgOpt) {
		info << "The following options cannot be specified together:\n" ;
		info << wmcOpts->getAttributeUsage(Options::JSON) << "\n";
		info << wmcOpts->getAttributeUsage(Options::NOMSG) << "\n";

		throw WmsClientException(__FILE__,__LINE__,
			"readOptions",DEFAULT_ERR_CODE,
			"Input Option Error", info.str());
	}

	// --collect
	m_collectOpt = wmcOpts->getStringAttribute(Options::COLLECTION);

	// --jsdl
	m_jsdlOpt = wmcOpts->getStringAttribute(Options::JSDL);

	// --dag
	m_dagOpt = wmcOpts->getStringAttribute(Options::DAG);

	// --default-jdl
	m_defJdlOpt = wmcOpts->getStringAttribute(Options::DEFJDL);

	// register-only & start
	m_startOpt = wmcOpts->getStringAttribute(Options::START);
	registerOnly = wmcOpts->getBoolAttribute(Options::REGISTERONLY);

	// --valid & --to
	m_validOpt = wmcOpts->getStringAttribute(Options::VALID);
	m_toOpt = wmcOpts->getStringAttribute(Options::TO);
	// --start: incompatible options
	if (!m_startOpt.empty() &&
		(registerOnly || !m_inOpt.empty() || !m_resourceOpt.empty() || !m_nodesresOpt.empty() || !m_toOpt.empty() ||
			!m_validOpt.empty() ||
			!m_collectOpt.empty() ||
			!m_jsdlOpt.empty() ||
			!m_dagOpt.empty() || !m_defJdlOpt.empty() ||
			!wmcOpts->getStringAttribute(Options::DELEGATION).empty() ||
			wmcOpts->getBoolAttribute(Options::AUTODG) ) ){
		info << "The following options cannot be specified together with --start:\n" ;
		info << wmcOpts->getAttributeUsage(Options::REGISTERONLY) << "\n";
		info << wmcOpts->getAttributeUsage(Options::OUTPUT) << "\n";
		info << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		info << wmcOpts->getAttributeUsage(Options::RESOURCE) << "\n";
		info << wmcOpts->getAttributeUsage(Options::NODESRES) << "\n";
		info << wmcOpts->getAttributeUsage(Options::TO) << "\n";
		info << wmcOpts->getAttributeUsage(Options::VALID) << "\n";
		info << wmcOpts->getAttributeUsage(Options::COLLECTION) << "\n";
		info << wmcOpts->getAttributeUsage(Options::JSDL) << "\n";
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
	if (!m_validOpt.empty() && !m_toOpt.empty()){
		info << "The following options cannot be specified together:\n" ;
		info << wmcOpts->getAttributeUsage(Options::VALID) << "\n";
		info << wmcOpts->getAttributeUsage(Options::TO) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", info.str());
	}
	// lrms has to be used with input o resource
	m_lrmsOpt = wmcOpts->getStringAttribute(Options::LRMS);

	if (!m_lrmsOpt.empty() && !( !m_resourceOpt.empty() || !m_inOpt.empty() ) ){
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
	// check --start option
	// either set or retrieve the ENDPOINT
	if (!m_startOpt.empty()) {
		m_startOpt = string(Utils::checkJobId(m_startOpt));
		// Retrieves the endpoint URL in case of --start
		logInfo->print(WMS_DEBUG, "Getting the enpoint URL");
		LbApi lbApi;
		LbApi lbApi2;

		lbApi.setJobId(m_startOpt);

		string thisEndPoint( lbApi.getStatus(true,true).getEndpoint() );
		if( thisEndPoint.empty() )
		  {
		    lbApi2.setJobId( lbApi.getStatus(true,true).getParent() );
		    thisEndPoint = lbApi2.getStatus(true).getEndpoint();
		  }
		
		//cout << "child  endpoint = " << lbApi.getStatus(true,true).getEndpoint() << endl;
		//cout << "parent endpoint = " << thisEndPoint << endl;
		//exit(1);
		setEndPoint( thisEndPoint /*lbApi.getStatus(true,true).getEndpoint()*/ );
		// checks if --endpoint option has been specified with a different endpoint url
		string endpoint =  wmcOpts->getStringAttribute (Options::ENDPOINT) ;
		if ( !endpoint.empty() && endpoint.compare(getEndPoint( )) !=0 ) {
			logInfo->print(WMS_WARNING, "--endpoint " + endpoint + " : option ignored");
		}
	} else {
		// Normal Behaviour: retrieves the endpoint URL
		retrieveEndPointURL( );
	}
	// file Protocol
	m_fileProto = wmcOpts->getStringAttribute( Options::PROTO) ;

	if (!m_startOpt.empty() && !m_fileProto.empty()) {
		logInfo->print (WMS_WARNING, "--proto: option ignored (start operation doesn't need any file transfer)\n", "", true );
	}  else if (registerOnly && !m_fileProto.empty()) {
		logInfo->print (WMS_WARNING, "--proto: option ignored (register-only operation doesn't need any file transfer)\n", "", true);
	} else {
		// Perform Check File Transfer Protocol Step
		jobPerformStep(STEP_CHECK_FILE_TP);
	}
	// --valid
	if (!m_validOpt.empty()){
		try{
			expireTime =  Utils::checkTime(m_validOpt, d, h, m, Options::TIME_VALID) ;
		} catch (WmsClientException &exc) {
			info << exc.what() << " (use: " << wmcOpts->getAttributeUsage(Options::VALID) << ")\n";
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Wrong Time Value",info.str() );
		}
	}
	// --to
	if (!m_toOpt.empty()) {
		try{
			expireTime= Utils::checkTime(m_toOpt, d, h, m,Options::TIME_TO) ;
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
		if (!m_validOpt.empty()) {
			logInfo->print(WMS_DEBUG,  "--valid option:", info.str( ) );
		} else if (!m_toOpt.empty())  {
			logInfo->print(WMS_DEBUG, "--to option:", info.str( ) );
		}
	}
	// --nolisten
	nolistenOpt =  wmcOpts->getBoolAttribute (Options::NOLISTEN);
	// path to the JDL file
	m_jdlFile = wmcOpts->getPath2Jdl( );

}

/**
* Performs the main operation for the submission
*/
void JobSubmit::submission ( ){
	ostringstream out ;
	toBretrieved = false;
	// in case of --start option
	if (!m_startOpt.empty()){
		jobStarter(m_startOpt);
	} else {

		// Check if a JSDL file has to be managed
		if(m_jsdlOpt.empty()) {
			// Normal management
			this->checkAd(toBretrieved);
		}
		else {
			// JSDL management
			this->checkJSDL();
		}

		// Perform Submission when:
		// (RegisterOnly has not been specified in CLI) && (no file to be transferred)
		// and initialize internal JobId:
		
		submitPerformStep(STEP_REGISTER);
		logInfo->print(WMS_DEBUG, "The JobId is: ", this->getJobId() ) ;
		
		this->checkOutputData( );
		
		// Perform File Transfer when:
		// (Registeronly is NOT specified [or specified with --tranfer-file]) AND (some files are to be transferred)
		if (toBretrieved){
			try{
				this->jobPostProcessing( );
			}catch (Exception &exc){
				string failed = "failed";
				// saves the jobid
				if (!m_outOpt.empty()){
					if ( wmcUtils->saveJobIdToFile(m_outOpt, this->getJobId( ), failed ) < 0){
					logInfo->print (WMS_WARNING, "Unable to write the jobid to the output file ", 						Utils::getAbsolutePath(m_outOpt));
				} else{
					logInfo->print (WMS_DEBUG, "The JobId has been saved in the output file ", 						Utils::getAbsolutePath(m_outOpt));
					out << "\nThe job identifier has been saved in the following file:\n";
					out << Utils::getAbsolutePath(m_outOpt) << "\n";
					}
				}
				throw WmsClientException(__FILE__,__LINE__,
					"submission",  DEFAULT_ERR_CODE ,
					"The job has been successfully registered (the JobId is: " + this->getJobId( ) + "),"+
					" but an error occurred while transferring files:",
					 string (exc.what())+"\n"+
					"To complete the operation be sure that the files have been transferred and start the job by issuing a submission with the option:\n"+
					" --start " + this->getJobId( ) + "\n"
					 );
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
			jobStarter(this->getJobId());
		}

	}  // END startOpt = FALSE branch
	// HEADER OF THE OUTPUT MESSAGE (submission)============================================
	out << "\n" << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " Success") ) << "\n\n";
	if (registerOnly){
		out << "The job has been successfully registered to the WMProxy\n";
	} else if (!m_startOpt.empty()){
		out << "The job has been successfully started to the WMProxy\n";
	} else {
		// OUTPUT MESSAGE (register+start)============================================
		out << "The job has been successfully submitted to the WMProxy\n";
	}
	/// OUTPUT MESSAGE (jobid and other information)============================================
	out << "Your job identifier is:\n\n";
	out << this->getJobId( ) << "\n";

	// saves the result
	if (!m_outOpt.empty()){
		if ( wmcUtils->saveJobIdToFile(m_outOpt, this->getJobId( )) < 0 ){
			logInfo->print (WMS_WARNING, "Unable to write the jobid to the output file " , Utils::getAbsolutePath(m_outOpt));
		} else{
			logInfo->print (WMS_DEBUG, "The JobId has been saved in the output file ", Utils::getAbsolutePath(m_outOpt));
			out << "\nThe job identifier has been saved in the following file:\n";
			out << Utils::getAbsolutePath(m_outOpt) << "\n";
		}
	}

	out << "\n" << wmcUtils->getStripe(74, "=") << "\n\n";

	if (infoMsg.size() > 0) {
		out << infoMsg << "\n";
		logInfo->print (WMS_INFO, infoMsg , "", false);
	}
	out << getLogFileMsg ( ) << "\n";
	// ==============================================================
    if ( !nomsgOpt && !json) {
            // Displays the output message
                cout << out.str() ;
    } else if (nomsgOpt) {
            // Displays only the jobid
        	cout << this->getJobId( ) << "\n";
    } else if (json) {
			//format the output message in json format
			
			vector<string> toJoin, toJoin_children;
			
			string carriage;
			if(prettyprint)
			  carriage="\n";
			else
			  carriage=", ";
			
			string json = "";

			vector<string> jobids = this->getJobIdsAndNodes();
			if(!prettyprint)
			  toJoin.push_back( "\"result\": \"success\"" );
			else
			  toJoin.push_back( "  result: success" );
			//json += "  result: success"+carriage;//\n";
			
			if(!prettyprint)
			  toJoin.push_back( "\"" + jobids[0]+"\": \""+jobids[1]+"\"" );
			else
			  toJoin.push_back( "  " + jobids[0]+": "+jobids[1] );
			//json += "  " + jobids[0]+": "+jobids[1]+carriage;//"\n";
			
			if(!prettyprint)
			  toJoin.push_back( "\"endpoint\": \""+getEndPoint() + "\"" );
			else
			  toJoin.push_back( "  endpoint: "+getEndPoint() );
			//json += "  endpoint: "+getEndPoint()+carriage;//"\n" ;

			int sizeN = jobids.size();
			if (sizeN>2) {
				
				for (int i=2;i<sizeN;i++) {
					if(!prettyprint)
					  toJoin_children.push_back( "\""+jobids[i]+"\": \""+jobids[i+1]+"\"" );
					else
					  toJoin_children.push_back( "      "+jobids[i]+": "+jobids[i+1] );
					//json += "      "+jobids[i]+": "+jobids[i+1]+carriage;//"\n";
					i++;
				}
				
				//cout << boost::algorithm::join( toJoin_children, "," ) << endl<<endl;
				
				//toJoin.push_back( boost::algorithm::join( toJoin_children, "," ) );
				
				if(!prettyprint)
				  toJoin.push_back( "\"children\": {" + join( toJoin_children, carriage ) + "}" );
				else
				  toJoin.push_back( "  children: {" + carriage + join( toJoin_children, carriage ) + carriage + "     }" );
				
			}
			if(prettyprint)
			  json = "{" + carriage + join( toJoin, carriage ) + carriage + "}";
			else
			  json = "{" + join( toJoin, carriage ) + "}";
			cout << json << endl;
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
void JobSubmit::checkUserServerQuota() {
	pair<long, long> free_quota ;
	long maxIsbSize = 0;
	long limit = 0;
	//  (1) User free quota -----------
	logInfo->print (WMS_DEBUG,"Checking the User-FreeQuota on the server", "" );
	try{
		// Gets the user-free quota from the WMProxy server
		logInfo->service(WMP_FREEQUOTA_SERVICE);

		// Set the SOAP timeout
		setSoapTimeout(SOAP_GET_FREE_QUOTA_TIMEOUT);

		free_quota = api::getFreeQuota(getContext( ));
	} catch (api::BaseException &exc){
			throw WmsClientException(__FILE__,__LINE__,
				"checkUserServerQuota", ECONNABORTED,
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
				"checkUserServerQuota",  DEFAULT_ERR_CODE,
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

			// Set the SOAP timeout
			setSoapTimeout(SOAP_GET_MAX_INPUT_SANBOX_SIZE_TIMEOUT);

			maxIsbSize = api::getMaxInputSandboxSize(getContext( ));
		} catch (api::BaseException &exc){
				throw WmsClientException(__FILE__,__LINE__,
					"checkUserServerQuota", ECONNABORTED,
					"WMProxy Server Error", errMsg(exc));
		}
		// (2) MAX ISB size -----------
		if (maxIsbSize>0 ) {
			logInfo->result(WMP_MAXISBSIZE_SERVICE, "Max ISB size information successfully retrieved");
			if (maxJobIsbSize > maxIsbSize) {
				ostringstream err ;
				err << "The max job size of the InputSandbox (" << maxJobIsbSize <<" bytes) ";
				err << "exceeds the MAX InputSandbox size limit on the server (" << maxIsbSize << " bytes)";
				throw WmsClientException( __FILE__,__LINE__,
					"checkUserServerQuota",  DEFAULT_ERR_CODE,
					"InputSandboxSize Error" , err.str());
			} else {
				ostringstream q;
				q << "The max job size (" << maxJobIsbSize << " bytes) doesn't exceed the max size limit of " << maxIsbSize << " bytes:";
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
	string us = wmcUtils -> getUniqueString( );
	if (us.empty()) {
		ostringstream u ;
		u << getpid( ) << "_" << getuid( );
		us = u.str();
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
	zipStruct->filename = ISBFILE_DEFAULT + "_" + us + "_" + boost::lexical_cast<string>(ntar)
		+ Utils::getArchiveExtension( ) + Utils::getZipExtension( ) ;
	fileads = extractAd->getFiles ( );
	n = fileads.size( );
	if (n > 0) {
		// int i = index of the FileAd vector is being filled
		i++;
		jobFiles->node = "";
		while (n != 0){
			filead_size = fileads[0].size ;
			// adding up the size of the first file in the list to the total tar size
			tar_size += filead_size;
			if (tar_size < Options::MAX_TAR_SIZE) {
				// moving the last elment of the FileAd list to the list of the rootFiles
				(jobFiles->files).push_back(fileads.at(0));
				fileads.erase(fileads.begin());
			} else {
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
				zipStruct->filename = ISBFILE_DEFAULT + "_" + us + "_" + boost::lexical_cast<string>(ntar)
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
					zipStruct->filename = ISBFILE_DEFAULT + "_" + us + "_" + boost::lexical_cast<string>(ntar)
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
	vector<ExtractedAd*> children; vector<FileAd> fileads;
	string destURI = ""; string jobid = "";
	int size = 0;
	if (zipAllowed) {
		vector<ZipFileAd>::iterator it1 = zippedFiles.begin() ;
		vector<ZipFileAd>::iterator const end1 = zippedFiles.end();
		for ( ; it1 != end1; it1++){
			createZipFile2(it1->filename, it1->fileads, tob_transferred);
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
		vector<FileAd>::const_iterator it = fileads.begin();
// 		for( ; it != fileads.end(); ++it )
// 		  cout << "FILEADS: " << it->file << endl;

		// JobId (root)
		jobid = this->getJobId( );
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
				destURI = getDestinationURI (jobid, getJobIdFromNode(children[i]->getNodeName()));
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
	isbSize = 0;
	maxJobIsbSize = 0;
	string message = "";
	ostringstream err ;
	logInfo->print (WMS_DEBUG, "Retrieving the list of the local ISB files from the user JDL", "");
	// type of job
	wmsJobType job = this->getJobType() ;
	switch (job) {
		case WMS_JOB : {
			extractAd = jobAdSP.getExtractedAd( );
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
	if (!m_fileProto.empty()) {
		FileAd::setMaxFileSize(Options::getMinimumAllowedFileSize(m_fileProto, zipAllowed));
	} else {
		FileAd::setMaxFileSize(Options::getMinimumAllowedFileSize("", zipAllowed));
	}

	

	// Get the Total of the Input Sandbox
	isbSize = extractAd->getTotalSize ( );

	// Get the highest file size from all the available in the input sandbox
	maxJobIsbSize = extractAd->getMaxJobFileSize();

	if (isbSize > 0) {

                vector<FileAd> files = extractAd->getFiles();
		int sizeF = files.size();
		for (int i=0; i<sizeF ; i++) {
			string cwd = Utils::getAbsolutePath("");
			if (files[i].file.compare(cwd))
				files.erase(files.begin()+i);
		}
			
		logInfo->print (WMS_DEBUG,
			"Total size of the ISB file(s) to be transferred to:",
			boost::lexical_cast<string>(isbSize) );
		logInfo->print (WMS_DEBUG,
			"Max single job size of the ISB file(s) to be transferred to:",
			boost::lexical_cast<string>(maxJobIsbSize) );
		// Checking whether ISB-total_size is supported by either UserFreeQUota or MAX-ISB =============
		submitPerformStep(STEP_CHECK_US_QUOTA);
		if (zipAllowed){
			// Prepares the zipped files
			toBCopiedZippedFileList( );
			// Adding the references to the zipped files to the user JDL =============================
			std::vector<ZipFileAd>::iterator it1 = zippedFiles.begin() ;
			std::vector<ZipFileAd>::iterator const end1 = zippedFiles.end();
			switch (getJobType()) {
				case WMS_JOB:
				case WMS_PARAMETRIC:
				{
					for ( ; it1 != end1; it1++){
						jobAdSP.addAttribute(JDLPrivate::ZIPPED_ISB, it1->filename);
					}
					break;
				}
				case WMS_DAG:{
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

	} else if (extractAd->hasFiles() ){
                vector<FileAd> files = extractAd->getFiles();
		if (files[0].file != "")
			isbSize = 1;
		logInfo->print (WMS_DEBUG,
                        "Total size of the ISB file(s) to be transferred is 0, please check your ISB files whether this is correct, the file(s) will be transferred anyway");
	} else {
		logInfo->print(WMS_DEBUG, "The user JDL does not contain any local ISB file:" ,
			"no ISB-FileTransfer to be performed");
	}
	return isbSize ;
}

/**
*  Checks the user JSDL
*/
void JobSubmit::checkJSDL(){

	string message = "";
	const string JSDL_WARNING_TITLE= "Following Warning(s) found while parsing JDL:";

	// Normalize the JSDL file path
	fs::path cp(Utils::normalizePath(m_jsdlOpt), fs::native);

	// Check if is a file and if it exists
	if (!fs::exists(cp) ) {
		throw WmsClientException(__FILE__,__LINE__,
			"checkJSDL",  DEFAULT_ERR_CODE,
			"Invalid JSDL Path",
			"--jsdl: no valid JSDL file path  (" + m_jsdlOpt + ")"  );
	}
}

void JobSubmit::checkOutputData( void ) {

	//return;
	glite::jdl::Ad *ad;
        
        if(adObj) {
          ad = adObj;
        } else {
          if(collectAd) 
            ad = collectAd;
          else {
	    //cout << "******** ALL NULL!!! " <<endl;  
	    return;
	  }
        }

  	if( /*adObj*/ad->hasAttribute( JDL::OUTPUTDATA ) )
	{
	  //cerr <<" ********* OUTPUT DATA PRESENT !! " << endl;
	  string id( this->getJobId( ) );
	  //string::size_type pos = id.find_last_of( '/', 0 );
	  
	  //cout << "************** pos=" << pos << endl;
	  
	  //id.erase( 0, pos );
	  
	  
	  const boost::regex rgx( "https*://[^/]+/(.+)$" );
          boost::match_results<std::string::const_iterator> what;
        
          boost::regex_match( id, what,  rgx ) ;	  
	  
	  string DS = string("DSUpload_") + what[1] + ".out";
	  
	  
	  
	  classad::ExprTree* osb( /*adObj*/ad->lookUp( "OutputSandbox" ) );
	  if( !osb ) {
	    /*adObj*/ad->setAttributeExpr( "OutputSandbox",  "{ \"" + DS  + "\"}" );
	  } else {
	    vector<string> OSBs( /*adObj*/ad->getStringValue("OutputSandbox") );
	    OSBs.push_back( DS );
	    
	    string newOSB( "{" );
	    
	    for(vector<string>::const_iterator it = OSBs.begin();
	    	it != OSBs.end();
		++it )
		{
		  newOSB +=  "\"" + *it + "\",";
		}
	    boost::trim_if( newOSB, boost::is_any_of(",") );
  	    newOSB += "}";
	    
	    //cout << "************** newOSB=[" << newOSB << "]" << endl;
	    
	    /*adObj*/ad->delAttribute( "OutputSandbox" );
	    /*adObj*/ad->setAttributeExpr( "OutputSandbox", newOSB );
	  }
	  
	}
	
	//cout << endl << "************** NEW AD = " << ad->toString() << endl;
}

/**
*  Checks the user JDL
*/
void JobSubmit::checkAd(bool &toBretrieved){
	string message = "";
	jobType = WMS_JOB;
	const string JDL_WARNING_TITLE= "Following Warning(s) found while parsing JDL:";
	glite::jdl::Ad *wmcConf = wmcUtils->getConf();
	if (!m_collectOpt.empty()) {
		// Collection created with --collection option
		jobType = WMS_COLLECTION ;
		try {
			//fs::path cp ( Utils::normalizePath(*collectOpt), fs::system_specific); // Boost 1.29.1
			fs::path cp ( Utils::normalizePath(m_collectOpt), fs::native);
			if ( fs::is_directory( cp ) ) {
				m_collectOpt= Utils::addStarWildCard2Path(m_collectOpt);
			} else {
				throw WmsClientException(__FILE__,__LINE__,
					"checkAd",  DEFAULT_ERR_CODE,
					"Invalid JDL collection Path",
					"--collection: no valid collection directory (" + m_collectOpt + ")"  );
			}
		} catch ( const fs::filesystem_error & ex ){
			throw WmsClientException(__FILE__,__LINE__,
				"checkAd",  DEFAULT_ERR_CODE,
				"Invalid JDL collection Path",
				ex.what()  );
		}
		logInfo->print (WMS_DEBUG, "A collection of jobs is being submitted; JDL files in:",
					Utils::getAbsolutePath( m_collectOpt));
		collectAd = AdConverter::createCollectionFromPath (m_collectOpt, wmcUtils->getVirtualOrganisation());
		// Simple Ad manipulation
		AdUtils::setDefaultValuesAd(collectAd,wmcConf, m_defJdlOpt);
		if (collectAd->hasWarnings()){printWarnings(JDL_WARNING_TITLE, collectAd->getWarnings() );}
	} else if (!m_dagOpt.empty()) {
		// Dag created with --dag option
		jobType = WMS_DAG ;
		try {
			fs::path cp ( Utils::normalizePath(m_dagOpt), fs::native);
			if ( fs::is_directory( cp ) ) {
				m_dagOpt= Utils::addStarWildCard2Path(m_dagOpt);
			} else {
				throw WmsClientException(__FILE__,__LINE__,
					"checkAd",  DEFAULT_ERR_CODE,
					"Invalid JDL collection Path",
					"--dag: no valid collection directory ("
					+ m_dagOpt + ")"  );
			}
		} catch ( const fs::filesystem_error & ex ){
			throw WmsClientException(__FILE__,__LINE__,
				"checkAd",  DEFAULT_ERR_CODE,
				"Invalid JDL DAG Path",
				ex.what()  );
		}
		logInfo->print (WMS_DEBUG, "A DAG is being submitted; JDL files in:", Utils::getAbsolutePath(m_dagOpt));
		adObj =  AdConverter::createDagAdFromPath(m_dagOpt, wmcUtils->getVirtualOrganisation());
		// Simple Ad manipulation
		AdUtils::setDefaultValuesAd(adObj,wmcConf, m_defJdlOpt);
	} else {
		// ClassAd: can be anything (reading Type)
		adObj = new Ad();
		if (m_jdlFile.empty()){
			throw WmsClientException(__FILE__,__LINE__,
				"",  DEFAULT_ERR_CODE,
				"JDL File Missing",
				"uknown JDL file pathame (Last Argument of the command must be a JDL file)"   );
		}
		logInfo->print (WMS_DEBUG, "The JDL file is:", Utils::getAbsolutePath(m_jdlFile));
		adObj->fromFile (m_jdlFile);
		// Adds ExpireTime JDL attribute
		if ((int)expireTime>0) {
			adObj->addAttribute (JDL::EXPIRY_TIME, (int)expireTime);
		}
		// Simple Ad manipulation (common)
		if (!adObj->hasAttribute (JDL::VIRTUAL_ORGANISATION)){
			adObj->setAttribute(JDL::VIRTUAL_ORGANISATION, wmcUtils->getVirtualOrganisation());
		}
		AdUtils::setDefaultValuesAd(adObj,wmcConf, m_defJdlOpt);
		// Checking the ALLOW_ZIPPED_ISB attribute
		if (adObj->hasAttribute(JDL::ALLOW_ZIPPED_ISB)){
		  zipAllowed = adObj->getBool(JDL::ALLOW_ZIPPED_ISB) ;
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
			}catch (Exception &ex){
				throw WmsClientException(__FILE__,__LINE__,
					"checkAd",  DEFAULT_ERR_CODE,
					"Invalid JDL collection",
					ex.what()   );
			}
		if (collectAd->hasWarnings()){printWarnings(JDL_WARNING_TITLE,collectAd->getWarnings() );}
		}  else
		// DAG  ========================================
		if ( adObj->hasAttribute(JDL::TYPE , JDL_TYPE_DAG) ) {
				logInfo->print (WMS_DEBUG, "A DAG job is being submitted");
				jobType = WMS_DAG ;
		} else {
			// JOB  =========================================
			jobType = WMS_JOB ;
			jobAdSP.fromClassAd (  *(adObj->ad()) ) ;
			AdUtils::setDefaultValues(&jobAdSP,wmcConf);
			// PARAMETRIC ====================================
			if (jobAdSP.hasAttribute(JDL::JOBTYPE,JDL_JOBTYPE_PARAMETRIC)) {
				// check JobAd without restoring attributes
				jobType = WMS_PARAMETRIC;
				jobAdSP.check(false);
			}else{
				jobAdSP.check();
			}
		}
	}
	

	
	switch ( jobType ) {

		case WMS_COLLECTION:
				// Common operations for collections
				collectAd->setLocalAccess(true);
				// Collect Ad manipulation
				AdUtils::setDefaultValues(collectAd,wmcConf);
				if (!m_nodesresOpt.empty()) {
					collectAd->setAttribute(JDL::SUBMIT_TO, m_nodesresOpt);
				}
				// JDL string
				collectAd = collectAd->check();
				if (collectAd->hasAttribute(JDL::ALLOW_ZIPPED_ISB)){
				  zipAllowed = collectAd->getBool(JDL::ALLOW_ZIPPED_ISB) ;
				} else {
					// Default value if the JDL attribute is not present
				  zipAllowed = false;
					logInfo->print (WMS_DEBUG, "The user JDL does not contain the " +
							JDL::ALLOW_ZIPPED_ISB + " attribute: ",
							"adding the attribute to the JDL with the default value (FALSE)");
					collectAd->addAttribute(JDL::ALLOW_ZIPPED_ISB, false);
				}
				// Checks if there are local ISB file(s) to be transferred to
				toBretrieved = (this->checkInputSandbox( ) > 0)?true:false;
				// JDL submission string
				m_jdlString = collectAd->toString();
				if (collectAd->hasWarnings()){printWarnings(JDL_WARNING_TITLE,collectAd->getWarnings() );}
				break;

		case WMS_DAG:
				// Common operations for DAGs
				if (!m_nodesresOpt.empty()) {
					adObj->setAttribute(JDL::SUBMIT_TO, m_nodesresOpt);
				}
				dagAd = new ExpDagAd (adObj);
				dagAd->setLocalAccess(true);
				AdUtils::setDefaultValues(dagAd,wmcConf);
				// expands the DAG loading all JDL files
				dagAd->getSubmissionStrings();
				if (adObj->hasAttribute(JDL::ALLOW_ZIPPED_ISB)){
				  zipAllowed = adObj->getBool(JDL::ALLOW_ZIPPED_ISB) ;
				} else {
					// Default value if the JDL attribute is not present
				  zipAllowed = false;
					logInfo->print (WMS_DEBUG, "The user JDL does not contain the " + JDL::ALLOW_ZIPPED_ISB + " attribute: ",
							"adding the attribute to the JDL with the default value (FALSE)");
					adObj->addAttribute(JDL::ALLOW_ZIPPED_ISB, false);
				}
				// Checks if there are local ISB file(s) to be transferred to
				toBretrieved = (this->checkInputSandbox( ) > 0)?true:false;
				// JDL submission string
				m_jdlString = dagAd->toString();
				if (dagAd->hasWarnings()){ printWarnings(JDL_WARNING_TITLE, dagAd->getWarnings() );}
				break;

		case WMS_JOB:
				// Common operations for normal Jobs
				// Check for DEPRECATED JDL Job Types: Partitionable & Checkpointable
				if (jobAdSP.hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_PARTITIONABLE) ||
				    jobAdSP.hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_CHECKPOINTABLE)){
					throw WmsClientException(__FILE__,__LINE__,
						"checkAd",  DEFAULT_ERR_CODE,
						"Deprecated Job Types",
						"Partitionable and Checkpointable Job Types have been deprecated.");
				}
				// resource <ce_id> ----> SubmitTo JDL attribute
				if (!m_resourceOpt.empty()) {
					jobAdSP.setAttribute(JDL::SUBMIT_TO, m_resourceOpt);
				}
				// MPICH ==================================================
				if (  jobAdSP.hasAttribute(JDL::JOBTYPE,JDL_JOBTYPE_MPICH)){
				// MpiCh Job:
					if (!m_lrmsOpt.empty()){
						// Override previous value (if present)
						if (jobAdSP.hasAttribute(JDL::LRMS_TYPE)){jobAdSP.delAttribute(JDL::LRMS_TYPE);}
							jobAdSP.setAttribute(JDL::LRMS_TYPE, m_lrmsOpt);
						}
					}
				// Checks if there are local ISB file(s) to be transferred to
				toBretrieved = (this->checkInputSandbox( ) > 0)?true:false;
				// JDL submission string
				m_jdlString = jobAdSP.toSubmissionString();
				break;

		case WMS_PARAMETRIC:
				// Common operations for Parametric
				logInfo->print (WMS_DEBUG, "A parametric job is being submitted");
				if (!m_nodesresOpt.empty()) {
					jobAdSP.setAttribute(JDL::SUBMIT_TO, m_nodesresOpt);
				}
				// InputSandbox for the parametric job
				if (jobAdSP.hasAttribute(JDL::INPUTSB)) {
					// InputSandbox Files Check
					dagAd = AdConverter::bulk2dag(&jobAdSP);
					// Print Pre-check Errors
					if (dagAd->hasWarnings()){printWarnings(JDL_WARNING_TITLE, dagAd->getWarnings() );}
					AdUtils::setDefaultValues(dagAd, wmcConf);
					dagAd->getSubmissionStrings();
					// Print Post-check Errors
					if (dagAd->hasWarnings()){printWarnings(JDL_WARNING_TITLE, dagAd->getWarnings() );}
				} else {
					logInfo->print (WMS_DEBUG, "No InputSandbox in the user JDL", "");
					// NO ISB found, convert ONLY first STEP
					// (it is only slight check, the conversion is done on server side)
					dagAd = AdConverter::bulk2dag(&jobAdSP,1);
				}
				// Checks if there are local ISB file(s) to be transferred to
				toBretrieved = (this->checkInputSandbox ( )>0)?true:false;
				// Submission string
				m_jdlString = jobAdSP.toString();

				break;

		default:
				break ;
			}

	if (zipAllowed) { message ="allowed by user in the JDL";}
		else { message = "disabled by user in the JDL"; }
	logInfo->print (WMS_DEBUG, "File archiving and file compression", message);
	// --resource : incompatible argument
	if( (!m_resourceOpt.empty()) && (jobType != WMS_JOB)){
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
			"Incompatible Argument: " + wmcOpts->getAttributeUsage(Options::RESOURCE),
			"cannot be used for  DAG, collection, partitionable and parametric jobs");
	} else if (!m_resourceOpt.empty()) {
		logInfo->print (WMS_DEBUG, "--resource option: The job will be submitted to this resource", m_resourceOpt );
	}else if( (!m_nodesresOpt.empty()) && (jobType == WMS_JOB)){
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
			"Incompatible Argument: " + wmcOpts->getAttributeUsage(Options::NODESRES),
			"cannot be used for jobs");
	}
	
	
}
/**
*Performs:
*	- Job registration when --register-only is selected
*	-  Job submission otherwise
*/
void JobSubmit::jobRegOrSub(const bool &submit) {

	string method  = "";
	// checks if jdlstring is not null
	if (m_jdlString.empty() && m_jsdlOpt.empty()){
		throw WmsClientException(__FILE__,__LINE__,
			"jobRegOrSub",  DEFAULT_ERR_CODE ,
			"Null Pointer Error",
			"null pointer to JDL string\n" + Options::BUG_MSG);
	}
	try{
		if (submit){
			if(m_jsdlOpt.empty())  {
				// jobSubmit
				method = "submit";
				logInfo->print(WMS_DEBUG, "Submitting JDL", m_jdlString);
				logInfo->print(WMS_DEBUG, "Submitting the job to the service", getEndPoint());

				//Submitting....
				logInfo->service(WMP_SUBMIT_SERVICE);

				// Set the SOAP timeout
				setSoapTimeout(SOAP_JOB_SUBMIT_TIMEOUT);

				jobIds = api::jobSubmit(m_jdlString, m_dgOpt, getContext( ));
				logInfo->print(WMS_DEBUG, "The job has been successfully submitted" , "", false);
			}
			else {
				// jobSubmitJSDL
				method = "submit (JSDL)";
				logInfo->print(WMS_DEBUG, "Submitting JSDL", m_jsdlOpt);
				logInfo->print(WMS_DEBUG, "Submitting the job to the service", getEndPoint());

				//Submitting....
				logInfo->service(WMP_SUBMIT_JSDL_SERVICE);

				// Set the SOAP timeout
				setSoapTimeout(SOAP_JOB_SUBMIT_TIMEOUT);

				// Open the JSDL file stream
				ifstream jsdlFile(m_jsdlOpt.c_str());

				jobIds = api::jobSubmitJSDL(jsdlFile, m_dgOpt, getContext( ));
				logInfo->print(WMS_DEBUG, "The job has been successfully submitted" , "", false);
			}
		}
		else {
			if(m_jsdlOpt.empty())  {
				// jobRegister
				method = "register";
				logInfo->print(WMS_DEBUG, "Registering JDL", m_jdlString);
				logInfo->print(WMS_DEBUG, "Registering the job to the service", getEndPoint());

				// registering ...
				logInfo->service(WMP_REGISTER_SERVICE);

				// Set the SOAP timeout
				setSoapTimeout(SOAP_JOB_REGISTER_TIMEOUT);

				jobIds = api::jobRegister(m_jdlString , m_dgOpt, getContext( ));
				logInfo->print(WMS_DEBUG, "The job has been successfully registered" , "", false);
			}
			else {
				// jobRegisterJSDL
				method = "register (JSDL)";
				logInfo->print(WMS_DEBUG, "Registering JSDL", m_jsdlOpt);
				logInfo->print(WMS_DEBUG, "Registering the job to the service", getEndPoint());

				// registering ...
				logInfo->service(WMP_REGISTER_JSDL_SERVICE);

				// Set the SOAP timeout
				setSoapTimeout(SOAP_JOB_REGISTER_TIMEOUT);

				// Open the JSDL file stream
				ifstream jsdlFile(m_jsdlOpt.c_str());

				jobIds = api::jobRegisterJSDL(jsdlFile, m_dgOpt, getContext( ));
				logInfo->print(WMS_DEBUG, "The job has been successfully registered" , "", false);
			}
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

		// Set the SOAP timeout
		setSoapTimeout(SOAP_JOB_START_TIMEOUT);

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
	if (!m_startOpt.empty()) {
		return (m_startOpt);
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
      	string destURI= "";
	string msg = "";
	string look_for = "";
	vector<string> uris ;
	vector<string> jobids;
	string proto = "";
	string service = "";
	bool protoInfo = false;
	bool ch = false; // TRUE=child node
        // The destinationURI's vector is empty: the WMProxy service will be called
        if (dsURIs.empty( )){
		// check the WMP version in order to establish the availability of the getTransferProtocols service
		protoInfo = checkWMProxyRelease(2, 2, 0 );
		if (protoInfo){
			// jobPath is needed for the creation of the Zip files (The path used to archive the files)
			// If this information is not available in the struct returned by the jobReg/jobSubm service,
			// it has to be retrieved with the getBulkDestionationURI service
			if (protocol.size( )> 0){
				proto = protocol;
			} else if (jobIds.jobPath == NULL && zipAllowed) {
				// JobId struct does not contain info on the jobPath
				if (m_fileProto.empty()) {
					// no specified protocol ==> ALL_PROTOCOLS will be requested
					proto = string(Options::WMP_ALL_PROTOCOLS) ;
				} else if  (m_fileProto == Options::JOBPATH_URI_PROTO) {
					// specified protocol: the same protocol as the  jobPath-URI
					proto = m_fileProto ;
				} else {
					// ALL PROTOCOLS
					proto = string(Options::WMP_ALL_PROTOCOLS) ;
				}
			} else
			// JobId struct contains info on the jobPath
			if (m_fileProto.empty()) {
				proto = string(Options::WMP_ALL_PROTOCOLS) ;
			} else {
				proto = m_fileProto;
			}
		} else{
			// getTransferProtocols service is not available
			if (protocol.size (  ) > 0) {
				proto = protocol ;
			} else if (!m_fileProto.empty()) {
				proto = m_fileProto;
			}
		}
                try{
			if (protoInfo) {
				if (this->getJobType() == WMS_JOB){
					// Normal Job ==> using the getSandboxDestURI service
					service = string(WMP_SBDESTURI_SERVICE);
                       			logInfo->print(WMS_DEBUG, "Getting the SandboxDestinationURI from the service" , getEndPoint( ));
					logInfo->print (WMS_DEBUG,
							"Calling the WMProxy " + service + " service with " + proto + " protocol", "" );

					// Set the SOAP timeout
					setSoapTimeout(SOAP_GET_SANDBOX_DEST_URI_TIMEOUT);

					uris = api::getSandboxDestURI(jobid, getContext( ), proto);
					dsURIs.push_back(make_pair(jobid,uris));
				} else {
					// Compound job ==> using the getSandboxBulkDestURI service
					service = string(WMP_BULKDESTURI_SERVICE) ;
					logInfo->print(WMS_DEBUG, "Getting the SandboxBulkDestinationURI from the service" , getEndPoint( ));

					logInfo->print (WMS_DEBUG,
							"Calling the WMProxy " + service  + " service with " + proto + " protocol", "" );

					// Set the SOAP timeout
					setSoapTimeout(SOAP_GET_SANDBOX_BULK_DEST_URI_TIMEOUT);

					dsURIs = api::getSandboxBulkDestURI(jobid, getContext( ), proto);
				}
			} else {
				if (this->getJobType() == WMS_JOB){
					// Normal Job ==> using the getSandboxDestURI service
					service = string(WMP_SBDESTURI_SERVICE);
                       			logInfo->print(WMS_DEBUG, "Getting the SandboxDestinationURI from the service" , getEndPoint( ));
					logInfo->print (WMS_DEBUG,
							"Calling the WMProxy " + service + " service with no request of specific protocol (all available protocols requested)" );

					// Set the SOAP timeout
					setSoapTimeout(SOAP_GET_SANDBOX_DEST_URI_TIMEOUT);

					uris = api::getSandboxDestURI(jobid, getContext( ), proto);
					dsURIs.push_back(make_pair(jobid,uris));
				} else {
					// Compound job ==> using the getSandboxBulkDestURI service
					service = string(WMP_BULKDESTURI_SERVICE) ;
					logInfo->print (WMS_DEBUG,
						"Calling the WMProxy " + service +
							" service with no request of specific protocol (all available protocols requested)");

					// Set the SOAP timeout
					setSoapTimeout(SOAP_GET_SANDBOX_BULK_DEST_URI_TIMEOUT);

					dsURIs = api::getSandboxBulkDestURI(jobid, getContext( ));
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
                } else {
			logInfo->result(service, "Destination URIs sucessfully retrieved");
		}
        } else {
		// The destinationURI's vector is not empty: needed protocol
		if (protocol.size (  ) > 0) {
			proto = protocol ;
		} else if (!m_fileProto.empty()) {
			proto = m_fileProto;
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
	// DestinationURI has not been retrieved yet (because compound may being processed):
	// looking for it in the BulkDestURIs vector
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
					destURI= *it2;
					if (ch) {
						logInfo->print(WMS_DEBUG,  "Child node : " + child, " - DestinationURI : " + destURI, false);
					} else {
						logInfo->print(WMS_DEBUG,  "DestinationURI:", destURI);
					}
					break ;
				}
			}
		}
	}
	if (destURI.empty()) {
		if (ch){
			throw WmsClientException(__FILE__,__LINE__, "getDestinationURI",DEFAULT_ERR_CODE,
			"Missing Information","unable to retrieve the InputSB DestinationURI for the job: " +jobid );
		} else{
			throw WmsClientException(__FILE__,__LINE__, "getDestinationURI",DEFAULT_ERR_CODE,
			"Missing Information","unable to retrieve the InputSB DestinationURI for the child node: " + child  );
		}
	}
        return destURI;
}
/**
* Returns a relative path that is used to archive the ISB local file in the tar files.
* This path is based on the job DestionationURI with the input protocol
*/
std::string JobSubmit::getJobPathFromDestURI(const std::string& jobid, const std::string& protocol) {
	string* jobPath = NULL;
	string msg = "";
	// If DestinationURIs have not been retrieved yet
	if (dsURIs.empty( )){
		this->getDestinationURI (jobid);
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
				jobPath = new string(*it2);
				logInfo->print(WMS_DEBUG,  "JobId : " + jobid, " - JobPath : " + *jobPath, false);
				break;
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
	}
	return *jobPath ;

}



/**
* Retrieve JobId from a specified node name
*/
std::string JobSubmit::getJobIdFromNode(const std::string& node){
	string jobid ="";
	string err = "";
	bool ch = false;
	int len = node.size( );
	// Root Node
	if (len == 0 ) {
		// it's the root node, return root id
  		jobid = jobIds.jobid;
	} else {
		// Looking for the jobid among the children
		ch = true;
		std::vector<api::JobIdApi*>::iterator it = (jobIds.children).begin( );
		std::vector<api::JobIdApi*>::iterator const end = (jobIds.children).end( );
		for ( ; it != end; it++) {
			if ( (*it) ) {
				if ((*it)->nodeName && node.compare(*(*it)->nodeName) ==0 ) {
					jobid= (*it)->jobid ;
				}
			}
		}
	}
	return jobid;
}

/**
* Retrieve all JobIds and its node names
*/
vector<std::string> JobSubmit::getJobIdsAndNodes ( ) {
	vector<string> jobids;
	string err = "";

	if (jobIds.children.size() != 0) {
		// Job is a dag/collection/parametric
		// It's the root node, return root id
		jobids.push_back("parent");
  		jobids.push_back(jobIds.jobid);

		// Storing childrens jobids into the vector of strings
		std::vector<api::JobIdApi*>::iterator it = (jobIds.children).begin( );
		std::vector<api::JobIdApi*>::iterator const end = (jobIds.children).end( );
		for ( ; it != end; it++) {
				jobids.push_back(*(*it)->nodeName) ;
				jobids.push_back((*it)->jobid) ;
		}
	} else {
		// Normal type job
		jobids.push_back("jobid");
		jobids.push_back(jobIds.jobid);
	}
	return jobids;
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
				string proto ;
				logInfo->print(WMS_DEBUG,
					"JobPath: missing information in the struct returned by the jobRegister/jobSubmit service;",
					"Research based on the DestionationURI with protocol: "+ Options::JOBPATH_URI_PROTO, false);
				// jobPath info from BulkDestURI
				if (m_fileProto.empty()) {
					proto = Options::TRANSFER_FILES_GUC_PROTO;
				} else {
					proto = m_fileProto;
				}
				jobPath = new string(this->getJobPathFromDestURI(this->getJobId( ), proto));
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
  void JobSubmit::createZipFile2(
				 const std::string filename,
				 std::vector<JobFileAd> fileads,
				 std::vector<pair<glite::jdl::FileAd, std::string > > &to_btransferred			 
				 )
  {
    vector<string> filesToTAR;
    string jobpath = "";
    string file = "";
    string path = "";
    vector <JobFileAd>::iterator it1 = fileads.begin( );
    vector <JobFileAd>::iterator const end1 = fileads.end( );
    for ( ; it1 != end1; it1++ ) { 
      jobpath = this->getJobPath(it1->node);
      system((string("mkdir -p ")+jobpath).c_str());
      filesToTAR.push_back( jobpath );
      
      vector <FileAd>::iterator it2 = (it1->files).begin( );
      vector <FileAd>::iterator const end2 = (it1->files).end( );
      for ( ; it2 != end2; it2++ ) {
 	file = it2->file;
  	//path = jobpath + "/" + Utils::getFileName(it2->file);

		logInfo->print(WMS_DEBUG, "tar - Copying local file: " + file,
			       " into directory: " + jobpath, false);

	string basenameFile = ::basename( file.c_str() );
	//logInfo->print(WMS_FATAL, "basenameFile="+basenameFile, false);;
	boost::filesystem::path srcPath(file, boost::filesystem::native);
	boost::filesystem::path dstPath(jobpath+"/"+basenameFile, boost::filesystem::native);
	boost::filesystem::copy_file( srcPath, dstPath );
	//filesToTAR.push_back( file );
      }
    }
    string tarfile = TMP_DEFAULT_LOCATION + "/" + Utils::getArchiveFilename (filename);
    string command = string("tar cf ") + tarfile + " ";
    command += join(filesToTAR, " ");
    system(command.c_str());

    boost::uintmax_t tarSize = boost::filesystem::file_size( tarfile );

    string maxISBSize = boost::lexical_cast<string>( api::getMaxInputSandboxSize(getContext( )) );

    if(tarSize > api::getMaxInputSandboxSize(getContext( ))) {
/*      logInfo->print(WMS_FATAL,
		     "ISB tarball size for [" + tarfile + "] is " 
		     + boost::lexical_cast<string>( tarSize ) 
		     + " exceeds the MaxInputSandboxSize specified in the JDL ("
		     + boost::lexical_cast<string>( maxISBSize )
		     + ")", false); */
	
       throw WmsClientException(__FILE__,__LINE__,
                                    "FileSize problem",  DEFAULT_ERR_CODE,
                                    "\n",
                                    "The tar archive size is greater than the maximum allowed by WMS (" + maxISBSize+" bytes)" );	
//      exit(1);
    }

    system((string("gzip -9 ")+tarfile).c_str());
    system((string("\\rm -rf ")+join(filesToTAR, " ")).c_str());

    string gz = tarfile + ".gz";

    FileAd source(FILE_PROTOCOL, gz, Utils::getFileSize(gz));
    string dest = this->getDestinationURI(this->getJobId( )) + "/" + filename;
    logInfo->print(WMS_DEBUG,
		   "ISB Zipped File: " + source.file, "DestURI: " + dest, false);
    to_btransferred.push_back(make_pair(source, dest) );
  }
  
//   void JobSubmit::createZipFile (
// 				 const std::string filename,
// 				 std::vector<JobFileAd> fileads,
// 				 std::vector<pair<glite::jdl::FileAd, std::string > > &to_btransferred
// 				 )
//   {
//     return;
//     int r = 0;
// //    TAR *t =NULL;
// //    tartype_t *type = NULL ;
//     string file = "";
//     string path = "";
//     string jobpath = "";
//     string tar = "";
//     string gz = "";
//     string jobPath = "";
//     string jobid = "";
//     // path of the tar file is being created
//     // zipAd : ZIPPEDFILEAD={std::string filename; FILEAD files;}
//     tar = TMP_DEFAULT_LOCATION + "/" + Utils::getArchiveFilename (filename);
//     logInfo->print(WMS_DEBUG,"Archiving the ISB files:", tar);
//     // opens the tar file
//     r = tar_open ( &t,  (char*)tar.c_str(), type,
// 		   O_CREAT|O_WRONLY,
// 		   S_IRWXU, TAR_GNU |  TAR_NOOVERWRITE  );
//     if ( r != 0 ){
//       throw WmsClientException(__FILE__,__LINE__,
// 			       "tar_open",  DEFAULT_ERR_CODE,
// 			       "File i/o Error",
// 			       "Unable to create tar file for InputSandbox: " + tar );
//     }
//     // files : FILEAD { std::string jobid; std::string node; std::vector<glite::jdl::FileAd> files;};
//     // RootFiles
//     vector <JobFileAd>::iterator it1 = fileads.begin( );
//     vector <JobFileAd>::iterator const end1 = fileads.end( );
//     for ( ; it1 != end1; it1++ ) {
//       jobpath = this->getJobPath(it1->node);
//       vector <FileAd>::iterator it2 = (it1->files).begin( );
//       vector <FileAd>::iterator const end2 = (it1->files).end( );
//       for ( ; it2 != end2; it2++ ) {
// 	file = it2->file;

// 	//	cout << "***** Handling file [" << file << "]" << endl;

// 	path = jobpath + "/" + Utils::getFileName(it2->file);
// 	logInfo->print(WMS_DEBUG, "tar - Archiving the local file: " + file,
// 		       "with the following path: " + path, false);

// 	//cout << "***** tar - Archiving the local file: "<<file<<" with the following path: "<< path<<endl;

// 	r = tar_append_file (t, (char*) file.c_str(), (char*)path.c_str());
// 	if (r!=0){
// 	  string m = "Error in adding the file "+ file+ " to " + tar ;
// 	  char* em = strerror(errno);
// 	  if (em) { m += string("\n(") + string(em) + ")"; }
// 	  throw WmsClientException(__FILE__,__LINE__,
// 				   "archiveFiles",  DEFAULT_ERR_CODE,
// 				   "File i/o Error",
// 				   "Unable to create tar file - " + m);
// 	}
//       }
//     }
//     if (t) {
//       // close the file
//       tar_append_eof(t);
//       tar_close (t);
//       logInfo->print(WMS_DEBUG,
// 		     "This archive file has been successfully created:", tar);
//       logInfo->print(WMS_DEBUG,
// 		     "Compressing the file (" +Utils::getZipExtension() +"):", tar);
//       gz = wmcUtils->fileCompression(tar);
//       logInfo->print(WMS_DEBUG,
// 		     "ISB ZIPPED file successfully created:", gz);
//     }
//     FileAd source(FILE_PROTOCOL, gz, Utils::getFileSize(gz));
//     string dest = this->getDestinationURI(this->getJobId( )) + "/" + filename;
//     logInfo->print(WMS_DEBUG,
// 		   "ISB Zipped File: " + source.file, "DestURI: " + dest, false);
//     to_btransferred.push_back(make_pair(source, dest) );
//     //exit(1);
//   }
  
  
/**
* File transfer by globus-url-copy (gsiftp protocol)
*/
void JobSubmit::gsiFtpTransfer(std::vector <std::pair<glite::jdl::FileAd, std::string> > &paths, std::vector <std::pair<glite::jdl::FileAd, std::string> > &failed, std::string &errors) {
	vector<string> params ;
	ostringstream err;
	string protocol = "";
	string source = "";
	string destination = "";
	char* reason = "";
	// Globus Url Copy Path finder
	string globusUrlCopy="globus-url-copy";
	logInfo->print(WMS_DEBUG, "FileTransfer (gsiftp):",
		"using globus-url-copy to transfer the local InputSandBox file(s) to the submission endpoint");
	if (getenv("GLOBUS_LOCATION") && Utils::isFile(string(getenv("GLOBUS_LOCATION"))+"/bin/"+globusUrlCopy)) {
		globusUrlCopy=string(getenv("GLOBUS_LOCATION"))+"/bin/"+globusUrlCopy;
	}else if (Utils::isFile ("/usr/bin/"+globusUrlCopy)){
		globusUrlCopy="/usr/bin/"+globusUrlCopy;
	}else {
		throw WmsClientException(__FILE__,__LINE__,
			"gsiFtpGetFiles", ECONNABORTED,
			"File Error",
			"Unable to find globus-url-copy executable\n");
	}
	while (paths.empty()==false) {
		// source
		source = (paths[0].first).file ;
		// destination
		destination = paths[0].second ;
		// Protocol has to be added only if not yet present
		protocol = (source.find("://")==string::npos)?FILE_PROTOCOL:"";
		//params
		params.resize(0);
		params.push_back(string (protocol+source));
		params.push_back(destination);
		logInfo->print(WMS_DEBUG, "File Transfer (gsiftp) \n", "Command: "+globusUrlCopy+"\n"+"Source: "+params[0]+"\n"+"Destination: "+params[1]);
		//		cout << "TRANSFERRING: " << params[0] << " TO " << params[1] << endl;
		string errormsg = "";

		// Set the default value;
		int timeout = 0;

		// Check if exists the attribute SystemCallTimeout
		if(wmcUtils->getConf()->hasAttribute(JDL_SYSTEM_CALL_TIMEOUT)) {
			// Retrieve and set the attribute SystemCallTimeout
			timeout = wmcUtils->getConf()->getInt(JDL_SYSTEM_CALL_TIMEOUT);
		}

		// launches the command
		if (int code = wmcUtils->doExecv(globusUrlCopy, params, errormsg, timeout)) {
			if (code > 0 ) {
				// EXIT CODE > 0
				err << " - " <<  source << "\nto: " << destination << " - ErrorCode: " << code << "\n";
				reason = strerror(code);
				if (reason!=NULL) {
					err << "   " << reason << "\n";
					logInfo->print(WMS_DEBUG,
						"FileTransfer (gsiftp) - Transfer Failed (ErrorCode="
						+ boost::lexical_cast<string>(code)+"):",reason );
				}
			} else {
				switch (code) {
					case FORK_FAILURE:
						err << "Fork Failure" << "\n" ;
						logInfo->print(WMS_DEBUG, "File Transfer (gsiftp) - Transfer Failed: ", "Fork Failure");
					case TIMEOUT_FAILURE:
						err << "Timeout Failure" << "\n" ;
						logInfo->print(WMS_DEBUG, "File Transfer (gsfitp) - Transfer Failed: ", "Timeout Failure");
					case COREDUMP_FAILURE:
						err << "Coredump Failure" << "\n" ;
						logInfo->print(WMS_DEBUG, "File Transfer (gsfitp) - Transfer Failed: ", "Coredump Failure");
				}
			}
			failed.push_back(paths[0]);
			errors+=err.str();
		} else{
			logInfo->print(WMS_DEBUG, "File Transfer (gsiftp)", "Transfer successfully done");
			// Removes the zip file just transferred
			if (zipAllowed) {
				try {
					Utils::removeFile(source);
				} catch (WmsClientException &exc) {
					logInfo->print (WMS_WARNING,
						"The following error occured during the removal of the file:",
						exc.what());
				}
			}
		}
		paths.erase(paths.begin());
	}
}

/**
* File transfer by HTCP (https protocol)
*/
void JobSubmit::htcpTransfer(std::vector <std::pair<glite::jdl::FileAd, std::string> > &paths, std::vector <std::pair<glite::jdl::FileAd, std::string> > &failed, std::string &errors) {
	vector<string> params;
	ostringstream err;
	string protocol = "";
	string source = "";
	string destination = "";
	char* reason = "";
	// Globus Url Copy Path finder
	string htcp="htcp";
	logInfo->print(WMS_DEBUG, "FileTransfer (https):",
		"using htcp to transfer the local InputSandBox file(s) to the submission endpoint");
      	if (Utils::isFile("/usr/bin/"+htcp)){
		htcp="/usr/bin/"+htcp;
	}else {
		throw WmsClientException(__FILE__,__LINE__,
			"htcpGetFiles", ECONNABORTED,
			"File Error",
			"Unable to find htcp executable\n");
	}
	while (paths.empty()==false) {
		// source
		source = (paths[0].first).file ;
		// destination
		destination = paths[0].second ;
		// Protocol has to be added only if not yet present
		protocol = (source.find("://")==string::npos)?FILE_PROTOCOL:"";
		//params
		params.resize(0);
		params.push_back(string (protocol+source));
		params.push_back(destination);
		logInfo->print(WMS_DEBUG, "File Transfer (https) \n", "Command: "+htcp+"\n"+"Source: "+params[0]+"\n"+"Destination: "+params[1]);
		string errormsg = "";

		// Set the default value;
		int timeout = 0;

		// Check if exists the attribute SystemCallTimeout
		if(wmcUtils->getConf()->hasAttribute(JDL_SYSTEM_CALL_TIMEOUT)) {
			// Retrieve and set the attribute SystemCallTimeout
			timeout = wmcUtils->getConf()->getInt(JDL_SYSTEM_CALL_TIMEOUT);
		}

		// launches the command
		if (int code = wmcUtils->doExecv(htcp, params, errormsg, timeout)) {
			// EXIT CODE > 0
			if (code > 0) {
				err << " - " <<  source << "\nto: " << destination << " - ErrorCode: " << code << "\n";
				reason = strerror(code);
				if (reason!=NULL) {
					err << "   " << reason << "\n";
					logInfo->print(WMS_DEBUG,
						"FileTransfer (https) - Transfer Failed (ErrorCode="
						+ boost::lexical_cast<string>(code)+"):", reason );
				}
			}else {
				switch (code) {
					case FORK_FAILURE:
						err << "Fork Failure" << "\n" ;
						logInfo->print(WMS_DEBUG, "File Transfer (https) - Transfer Failed: ", "Fork Failure");
					case TIMEOUT_FAILURE:
						err << "Timeout Failure" << "\n" ;
						logInfo->print(WMS_DEBUG, "File Transfer (https) - Transfer Failed: ", "Timeout Failure");
					case COREDUMP_FAILURE:
						err << "Coredump Failure" << "\n" ;
						logInfo->print(WMS_DEBUG, "File Transfer (https) - Transfer Failed: ", "Coredump Failure");
				}
			}
			failed.push_back(paths[0]);
			errors+=err.str();
		} else{
			logInfo->print(WMS_DEBUG, "File Transfer (https)", "Transfer successfully done");
			// Removes the zip file just transferred
			if (zipAllowed) {
				try {
					Utils::removeFile(source);
				} catch (WmsClientException &exc) {
					logInfo->print (WMS_WARNING,
						"The following error occured during the removal of the file:",
						exc.what());
				}
			}
		}
		paths.erase(paths.begin());
	}
}

/**
* Message for InputSB files that need to be transferred
*/
std::string JobSubmit::transferFilesList(const std::vector <std::pair<glite::jdl::FileAd, std::string> > &paths, const std::string& jobid, const bool &zip) {
	ostringstream info;
	string header = "";
	string label = "";
	int size = paths.size( );
	if (size==0) {
		info << "To complete the submission:\n";
		info << "- no local file in the InputSandbox files to be transferred\n";
		info << "- ";
	} else {
		// s a zip file with the ISB files to be transferred if file compression is allowed
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
	info << "start the job by issuing a submission with the option:\n --start " << this->getJobId( ) << "\n";
	return info.str();
}
/*
* Transfers the ISB file to the endpoint
*/
void JobSubmit::transferFiles(std::vector<std::pair<glite::jdl::FileAd,std::string > > &to_bcopied, const std::string &jobid){
	vector<pair<FileAd, string > > failed;
	string errors = "";
		// File Transfer according to the chosen protocol
		if (m_fileProto == Options::TRANSFER_FILES_HTCP_PROTO ) {
			this->htcpTransfer (to_bcopied, failed, errors);
		} else {
			this->gsiFtpTransfer (to_bcopied, failed, errors);
		}
	if (errors.size()>0) {
		ostringstream err ;
		err << "The following error(s) occured while transferring the ISB file(s):\n";
		err << errors << "\n\n";
		err << transferFilesList (failed, jobid, false) << "\n";
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
	std::vector<glite::jdl::FileAd> files;
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
		vector <pair<FileAd, string> >::const_iterator it = to_bcopied.begin();
// 		for( ; it != to_bcopied.end( ); ++it )
// 		  cout << "TO BE COPIED: " << it->first.file << " - " 
// 		       << it->second << endl;

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
/** Perform a certain operation and, if any problem arises, try and recover all the previous steps */
void JobSubmit::submitPerformStep(submitRecoveryStep step){
	switch (step){
		case STEP_CHECK_US_QUOTA:
			// logInfo->print(WMS_DEBUG, "JobSubmit Performing", "STEP_CHECK_US_QUOTA");
			try{
                            checkUserServerQuota();
                            }
			catch (WmsClientException &exc) {
				logInfo->print(WMS_WARNING, string(exc.what()), "");
				submitRecoverStep(step);
			}
			break;
		case STEP_REGISTER:
			// logInfo->print(WMS_DEBUG, "JobSubmit: Performing", "STEP_REGISTER");
			try{
                            jobRegOrSub(startJob && !toBretrieved);
                            }
			catch (WmsClientException &exc) {
				logInfo->print(WMS_WARNING, string(exc.what()), "");
				submitRecoverStep(step);
			}
			break;
		default:
			throw WmsClientException(__FILE__,__LINE__,
				"submitPerformStep", ECONNABORTED,
				"Fatal Recovery",
				"Unable to recover from specified step");
	}
}
void JobSubmit::submitRecoverStep(submitRecoveryStep step){
	doneUrls.push_back(m_endPoint);
	this->m_endPoint = "";
	jobPerformStep(STEP_GET_ENDPOINT);
	jobPerformStep(STEP_DELEGATE_PROXY);
	jobPerformStep(STEP_CHECK_FILE_TP);
	// PERFORM STEP_CHECK_US_QUOTA
	submitPerformStep(STEP_CHECK_US_QUOTA);
	if (step==STEP_CHECK_US_QUOTA){return;}
	// PERFORM STEP_REGISTER
	submitPerformStep(STEP_REGISTER);
	if (step==STEP_REGISTER){return;}
	// no return reached: Unknown STEP
	throw WmsClientException(__FILE__,__LINE__,
		"submitRecoverStep", ECONNABORTED,
		"Fatal Recovery",
		"Unable to recover from specified step");
}


}}}} // ending namespaces
