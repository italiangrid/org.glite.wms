/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#include "joboutput.h"
#include "lbapi.h"
#include <string>
#include <sys/stat.h> //mkdir
// streams
#include<sstream>
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
// BOOST
#include "boost/filesystem/path.hpp" // path
#include "boost/filesystem/exception.hpp" //managing boost errors
// CURL
#include "curl/curl.h"


using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;
namespace fs = boost::filesystem ;

namespace glite {
namespace wms{
namespace client {
namespace services {

int SUCCESS = 0;
int FAILED = -1;
const bool GENERATE_NODE_NAME =false;

/*
* Default constructor
*/
JobOutput::JobOutput () : Job() {
	// init of the string  attributes
	inOpt = NULL;
	dirOpt = NULL;
	// init of the output storage location
	dirCfg = "/tmp";
	// init of the boolean attributes
	listOnlyOpt = false;
	// list of files
	childrenFileList = "";
	parentFileList = "";
	hasFiles = false ;
	successRt = false;
	warnsList = NULL;
}
/*
* Default Destructor
*/
JobOutput::~JobOutput ()  {
	// "free memory" for the string  attributes
	if (inOpt) { free(inOpt);}
	if (dirOpt ) { free(dirOpt);}
}
void JobOutput::readOptions ( int argc,char **argv)  {
	unsigned int njobs = 0;
	ostringstream err ;
 	Job::readOptions  (argc, argv, Options::JOBOUTPUT);
        // --input
        // input file
        inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	// JobId's
        if (inOpt){
		// From input file
		logInfo->print (WMS_DEBUG, "Reading JobId(s) from the input file:", Utils::getAbsolutePath(*inOpt));
        	jobIds = wmcUtils->getItemsFromFile(*inOpt);
		logInfo->print (WMS_DEBUG, "JobId(s) in the input file:", Utils::getList (jobIds), false);
        } else {
        	jobIds = wmcOpts->getJobIds();
        }
        jobIds = wmcUtils->checkJobIds (jobIds);
	njobs = jobIds.size( ) ;
	if ( njobs > 1 && ! wmcOpts->getBoolAttribute(Options::NOINT) ){
		logInfo->print (WMS_DEBUG, "Multiple JobIds found:", "asking for choosing one or more id(s) in the list ", false);
        	jobIds = wmcUtils->askMenu(jobIds, Utils::MENU_JOBID);
		if (jobIds.size() != njobs) {
			logInfo->print (WMS_DEBUG, "Chosen JobId(s):", Utils::getList (jobIds), false);
		}
         }
        // --dir , OutputStorage or DEFAULT_OUTPUT value
        dirOpt = wmcOpts->getStringAttribute(Options::DIR);
	  // --listonly
        listOnlyOpt = wmcOpts->getBoolAttribute( Options::LISTONLY ) ;
	if (listOnlyOpt &&dirOpt) {
		ostringstream info ;
		info << "the following options cannot be specified together:\n" ;
		info << wmcOpts->getAttributeUsage(Options::DIR) << "\n";
		info << wmcOpts->getAttributeUsage(Options::LISTONLY) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", info.str());
	}
	if (!listOnlyOpt){
		if (!dirOpt){
			dirCfg =Utils::getAbsolutePath(wmcUtils->getOutputStorage()) ;
			logInfo->print(WMS_DEBUG, "Output Storage (by configuration file):", dirCfg);
		} else {
			*dirOpt = Utils::getAbsolutePath(*dirOpt);
			logInfo->print(WMS_DEBUG, "Output Storage (by --dir option):", *dirOpt);
		}
	}

}
/******************************
*	getOutput method
******************************/
void JobOutput::getOutput ( ){
	postOptionchecks();
	int result = FAILED;
	ostringstream out ;
	ostringstream msg;
	int size = 0;
	// checks that the jobids vector is not empty
	if (jobIds.empty()){
		throw WmsClientException(__FILE__,__LINE__,
			"getOutput", DEFAULT_ERR_CODE,
			"JobId Error",
			"No valid JobId for which the output can be retrieved" );
	}
	// number of jobs
	size = jobIds.size ( );
	// performs output retrieval
	char* environ=getenv("LOGNAME");
	if (environ){logName="/"+string(environ);}
	else{logName="/jobOutput";}
	LbApi lbApi;
	// Jobid's loop
	vector<string>::iterator it = jobIds.begin() ;
	vector<string>::iterator const end = jobIds.end();
        for ( ; it != end ; it++){
		// JobId
		lbApi.setJobId(*it);
		try{
			Status status=lbApi.getStatus(true,true);
			// Initialize ENDPOINT (start a new (thread of) job (s)
			setEndPoint (status.getEndpoint());
			// Properly set destination Directory
			if (dirOpt){
				if ( size == 1 ){
					retrieveOutput (msg,status,Utils::getAbsolutePath(*dirOpt));
				} else {
					retrieveOutput (msg,status,Utils::getAbsolutePath(*dirOpt)+logName+"_"+Utils::getUnique(*it));
				}
			}else{
				retrieveOutput (msg,status,dirCfg+logName+"_"+Utils::getUnique(*it));
			}
			// if the output has been successfully retrieved for at least one job
			result = SUCCESS;

		} catch (WmsClientException &exc){
			// cancellation not allowed due to its current status
			// If the request contains only one jobid, an exception is thrown
			if (size == 1){ throw exc ;}
			string wmsg = (*it) + ": not allowed to retrieve the output" + exc.what( );
			// if the request is for multiple jobs, a failed-string is built for the final message
			createWarnMsg(wmsg);
			// goes on with the following job
			continue ;
		}
	}
	if (result == SUCCESS && successRt){
		out << "\n" << wmcUtils->getStripe(80, "=" , "") << "\n\n";
		out << "\t\t\tJOB GET OUTPUT OUTCOME\n\n";
		if (listOnlyOpt && hasFiles){
			out << parentFileList  << childrenFileList ;
			// Prints the results into the log file
			logInfo->print(WMS_INFO,  string(parentFileList+childrenFileList), "", false );
		} else {
			out << msg.str() ;
			// Prints the results into the log file
			logInfo->print (WMS_INFO,  msg.str() , "", false );
		}
		out << wmcUtils->getStripe(80, "=" , "" ) << "\n\n";
		// Warnings/errors messages
		if (  wmcOpts->getBoolAttribute(Options::DBG) && warnsList) {
			out << *warnsList << "\n";
		}

	} else {
		string err = "";
		if (size==1) { err ="Unable to retrieve the output"; }
		else { err ="Unable to retrieve the output for any job"; }
		throw WmsClientException(__FILE__,__LINE__,
				"output", ECONNABORTED,
				"Operation Failed",
				err  );
	}
	// logfile
	out << getLogFileMsg ( ) << "\n";
	// STD-OUT
	cout << out.str ( );
}
/*********************************************
*	PRIVATE METHODS:
**********************************************/
int JobOutput::retrieveOutput (ostringstream &msg,Status& status, const std::string& dirAbs, const bool &child){
	string warnings = "";
	string wmsg = "" ;
	string id = "" ;
	// Dir Creation Management 
	bool createDir=false;
	bool checkChildren = true;
	// JobId
	glite::wmsutils::jobid::JobId jobid = status.getJobId();
	logInfo->print(WMS_DEBUG,"Checking the status of the job:" , jobid.toString());
	// Check Status (if needed)
	int code = status.checkCodes(Status::OP_OUTPUT, warnings, child);
	if (warnings.size()>0){
		wmsg = jobid.toString() + ": " + warnings ;
		createWarnMsg(wmsg);
	}
	if (!listOnlyOpt &&  (code == 0 || ! child )){
		// It's not a node & retrieval is allowed (code==0)
		if (Utils::isDirectory(dirAbs)){
			logInfo->print(WMS_WARNING, "Directory already exists: ", dirAbs);
			if (!wmcUtils->answerYes ("Do you wish to overwrite it ?", false, true)){
				cout << "bye" << endl ;
				wmcUtils->ending(ECONNABORTED);
			}
		}else {
			createDir=true;
			if (mkdir(dirAbs.c_str(), 0755)){
				// Error while creating directory
				throw WmsClientException(__FILE__,__LINE__,
					"retrieveOutput", ECONNABORTED,
					"Unable create dir",dirAbs);
			}
		}
	}
	std::vector<Status> children = status.getChildrenStates();
	// Retrieve Output Files management
	if (code == 0){
		// OutputFiles Retrieval NOT allowed for DAGS (collection, partitionables, parametrics...)
		// i.e. ANY job that owns children
		if (children.size()==0){
			// actually retrieve files
			if (retrieveFiles (msg,jobid.toString(),dirAbs, child)){
				// Something has gone bad, no output files stored then purge directory
				if (createDir) {
					rmdir(dirAbs.c_str());
				}
			}
			successRt = true;
			checkChildren = false;
		}else if (listOnlyOpt){
			// IT is a DAG, no output files to be retrieved.
			// Print a simple output
			ostringstream out ;
			out << "\nJobId: " << jobid.toString() << "\n";
			parentFileList = out.str();
		}
	}
	// Children (if present) Management
	if (children.size()){
		ostringstream msgVago ;
		unsigned int size = children.size();
		if (GENERATE_NODE_NAME){
			std::map< std::string, std::string > map= AdUtils::getJobIdMap(status.getJdl());
			for (unsigned int i = 0 ; i < size ;i++){
				retrieveOutput (msgVago,children[i],
					dirAbs+logName+"_"+
					AdUtils::JobId2Node(map,children[i].getJobId()),true);
			}
		}else{
			for (unsigned int i = 0 ; i < size ;i++){
				retrieveOutput (msgVago,children[i],
					dirAbs+logName+"_"+
					children[i].getJobId().getUnique(),true);
			}
		}
	}
	bool parent = status.hasParent ( ) ;
	/* Purge logic: Job can be purged when
	* enpoint has been specified (parent has specified)
	* no parent is present
	* retrieve output successfully done */
	bool purge = (!listOnlyOpt) && ( getEndPoint() != "" ) && (! parent) && (code==0);
	id = jobid.toString() ;
	// checks Children
	if (checkChildren && children.size()>0){
		if (hasFiles){
			msg << "Output sandbox files for the DAG/Collection :\n" << id << endl ;
			msg << "have been successfully retrieved and stored in the directory:"<<endl<<dirAbs << "\n\n";
		} else{
			msg << "No output files to be retrieved for the DAG/Collection:\n" << id << "\n\n";
			if (createDir) {
				// remove created empty dir
				rmdir(dirAbs.c_str());
			}
		}
		// It's a dag, no outputfiles to be retrieved
	}else if  (!hasFiles){ //It's a job with no output files (and no children)
		if (createDir) {
			// remove created empty dir
			rmdir(dirAbs.c_str());
		}
		msg << "No output files to be retrieved for the job:\n" << id <<" \n\n";
	}
	if (purge){
		try {
			// Check Dir/purge
			logInfo->service(WMP_PURGE_SERVICE, id);
			jobPurge(jobid.toString(),getContext());
			logInfo->result(WMP_PURGE_SERVICE, "The purging request has been successfully sent");
		} catch (BaseException &exc) {
			string wmsg =  "";
			if (exc.Description){ wmsg +=" (" + *(exc.Description)+ ")"; }
			logInfo->print (WMS_WARNING, "JobPurging not allowed",wmsg, true );
		}
	}
	return 0;
}
bool JobOutput::retrieveFiles (ostringstream &msg, const std::string& jobid, const std::string& dirAbs, const bool &child){
		vector <pair<string , long> > files ;
		vector <pair<string,string> > paths ;
		string file_proto = "";
		string filename = "";
		bool result = true;
		try {
			// gets the list of the out-files from the EndPoint
			logInfo->print(WMS_DEBUG, "Connecting to the service", this->getEndPoint());
			logInfo->service(WMP_OUTPUT_SERVICE, jobid);
			files = getOutputFileList(jobid, getContext() );
			logInfo->result(WMP_OUTPUT_SERVICE, "The list of output files has been successfully retrieved");
			hasFiles = hasFiles || (files.size()>0);
		} catch (BaseException &exc) {
			string desc = "";
			if (exc.Description){ desc =" (" + *(exc.Description)+ ")"; }
			if (child) {
				string wmsg = jobid + ": not allowed to retrieve the output" + desc ;
				createWarnMsg (wmsg);
				result = false ;
			} else {
				throw WmsClientException(__FILE__,__LINE__,
					"jobOutput", ECONNABORTED,
					"getOutputFileList Error", desc);
			}
		}
		// files successfully retrieved
		if (files.size() == 0){
			if (listOnlyOpt){
				this->listResult(files, jobid, child);
			}
		}else{
			// match the file to be downloaded to a local directory pathname
			if (!listOnlyOpt){
				// Actually retrieving files
				logInfo->print(WMS_DEBUG, "Retrieving Files for: ", jobid);
				unsigned int size = files.size( );
				for (unsigned int i = 0; i < size; i++){
					if (file_proto.size()==0){
						file_proto = Utils::getProtocol (files[i].first);
					}
					filename = Utils::getFileName(files[i].first);
					paths.push_back( make_pair (files[i].first, string(dirAbs +"/" + filename) ) );
				}
				if (file_proto == Options::TRANSFER_FILES_GUC_PROTO){
					// Downloads the files
					this->gsiFtpGetFiles (paths);
				} else if (file_proto == Options::TRANSFER_FILES_CURL_PROTO){
					this->curlGetFiles (paths);
				} else {
					throw WmsClientException(__FILE__,__LINE__,
						"jobOutput", ECONNABORTED,
						"File Protocol Error", "protocol not supported: " + file_proto);
				}
				msg << "Output sandbox files for the job:" << endl << jobid<<endl ;
				msg << "have been successfully retrieved and stored in the directory:"<<endl<<dirAbs << "\n\n";
			} else {
				// Prints file list (only verbose result)
				this->listResult(files, jobid, child);
			}
		}
		return result;
}
/*
*	prints file list on std output
*/
void JobOutput::listResult(std::vector <std::pair<std::string , long> > &files, const std::string jobid, const bool &child){
	string ws = " ";
	ostringstream out ;
	// output message
	if (child){
		out << "\n\t>> child: " << jobid << "\n";
		if (files.size( ) == 0 ){
			out << "\tno output file to be retrieved\n";
		} else{
			vector <pair<string , long> >::iterator it1  = files.begin();
			vector <pair<string , long> >::iterator const end1 = files.end() ;
			for (  ; it1 != end1 ; it1++ ){
				out << "\t - file: " << it1->first << "\n";
				out << "\t   size (bytes): " << it1->second << "\n";
			}
		}
	} else{
		out << "\nJobId: " << jobid << "\n";
		if (files.size( ) == 0 ){
			out << "no output file to be retrieved\n";
		} else{
			vector <pair<string , long> >::iterator it2  = files.begin();
			vector <pair<string , long> >::iterator const end2 = files.end() ;
			for (  ; it2 != end2 ; it2++ ){
				out << " - file: " << it2->first << "\n";
				out << "   size (bytes): " << it2->second << "\n";
			}
		}
	}
	if (child){
		childrenFileList += out.str();
	} else{
		
	}
}
/*
* Creates the final Warning Msg
*/
void JobOutput::createWarnMsg(const std::string &msg ){
	int size = msg.size();
	if (size>0) {
		logInfo->print(WMS_WARNING, msg , "" , true );
		if (warnsList ){
			*warnsList += "- " + msg + "\n";
		} else if (size>0 ){
			warnsList = new string( );
			*warnsList = "The following warnings/errors have been found during the operation:\n";
			*warnsList += "========================================================================\n";
			*warnsList += "- " + msg + "\n";
		}
	}
}

/*
*	gsiFtpGetFiles Method  (WITH_GRID_FTP)
*/
void JobOutput::gsiFtpGetFiles (std::vector <std::pair<std::string , std::string> > &paths) {
	int code = 0;
 	vector <pair<std::string , string> >::iterator it = paths.begin( ) ;
	vector <pair<std::string , string> >::iterator const end = paths.end( ) ;
	 for (  ; it != end ; it++ ){
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
		cmd+=string( it->first) + " file://"  +string( it->second );
		logInfo->print(WMS_DEBUG, "File Transferring (gsiftp command)\n", cmd);
		// launches the command
		code = system( cmd.c_str() ) ;
		if ( code != 0 ){
			ostringstream info ;
			info << code ;
			logInfo->print(WMS_DEBUG, "File Transferring (gsiftp) - TRANSFER FAILED", "return code=" + info.str());
			throw WmsClientException(__FILE__,__LINE__,
				"utilities::globus::put", ECONNABORTED,
				"File Transferring Error",
				"unable to download the file: " +  it->first +"\nto: "+ it->second  );
                   }
         }
}

/*
* writing callback for curl operations
*/
int JobOutput::storegprBody(void *buffer, size_t size, size_t nmemb, void *stream)
 {
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
/*
*	File downloading by CURL
*/
void JobOutput::curlGetFiles (std::vector <std::pair<std::string , std::string> > &paths) {
	CURL *curl = NULL;
	string source = "" ;
	string destination = "" ;
	string file = "" ;
	CURLcode res;
	char curl_errorstr[CURL_ERROR_SIZE];
	// user proxy
	if ( !paths.empty()){
                // curl init
                curl_global_init(CURL_GLOBAL_ALL);
                curl = curl_easy_init();
                if ( curl ) {
                        // writing function
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, storegprBody);
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
                        // verbose message
                        if ( wmcOpts->getBoolAttribute(Options::DBG)){
                                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
                        }
			// enables error message buffer
			curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errorstr);
                        // files to be downloaded
			vector <pair<std::string , string> >::iterator it = paths.begin( ) ;
			vector <pair<std::string , string> >::iterator const end = paths.end( );
                        for ( ; it != end ; it++ ){
				// source
                                source = it->first ;
                                curl_easy_setopt(curl, CURLOPT_URL, source.c_str());
				// destination
                                destination = it->second;
                                ostringstream info ;
                                info << "File:\t" << source << "\n";
                                info << "Destination:\t" << destination <<"\n";
                                logInfo->print(WMS_DEBUG, "Ouput File Transferring", info.str());
                                struct httpfile params={
                                        (char*)destination.c_str() ,
                                        NULL
                                };
                                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &params);
                                // downloading
                                res = curl_easy_perform(curl);
                                if (res != 0){
                                	ostringstream err ;
                                        err << "Failed to perform the downloading of the file:\n";
                                        err << "from " << source << "\n";
                                        err << "to " << destination<< "\n";
					err << curl_errorstr << "\n";
                                        throw WmsClientException(__FILE__,__LINE__,
                                        	"transferFiles", DEFAULT_ERR_CODE,
                                        	"File Transfer Error",
                                        	err.str() );
                                }
                        }
                }
 	}
}
}}}} // ending namespaces

