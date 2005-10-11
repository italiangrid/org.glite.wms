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

#if defined (WITH_GRID_FTP_API)
#include "glite/wms/common/utilities/globus_ftp_utils.h"
#elif ! defined (WITH_GRID_FTP_API)
// CURL
#include "curl/curl.h"
#endif

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;
#ifdef WITH_GRID_FTP_API
using namespace glite::wms::common::utilities::globus;
#endif
namespace fs = boost::filesystem ;

namespace glite {
namespace wms{
namespace client {
namespace services {

int SUCCESS = 0;
int FAILED = -1;

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
	ostringstream err ;
 	string opts = Job::readOptions  (argc, argv, Options::JOBOUTPUT);
        // writes the information on the specified option in the log file
        logInfo->print(WMS_INFO, "Function Called:", wmcOpts->getApplicationName( ), false);
        logInfo->print(WMS_INFO, "Options specified:", opts, false);
        // --input
        // input file
        inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	// JobId's
        if (inOpt){
        	jobIds = wmcUtils->getItemsFromFile(*inOpt);
        } else {
        	jobIds = wmcOpts->getJobIds();
        }
        jobIds = wmcUtils->checkJobIds (jobIds);
	if ( jobIds.size( ) > 1 && ! wmcOpts->getBoolAttribute(Options::NOINT) ){
        	jobIds = wmcUtils->askMenu(jobIds, Utils::MENU_JOBID);
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
        // checks that the config-context is not null
        if (!cfgCxt){
		throw WmsClientException(__FILE__,__LINE__,
			"getOutput",  DEFAULT_ERR_CODE,
			"Null Pointer Error", "null pointer to ConfigContext object"   );
        }
	// number of jobs
	size = jobIds.size ( );
	// performs output retrieval
	char* environ=getenv("LOGNAME");
	if (environ){logName="/"+string(environ);}
	else{logName="/jobOutput";}
	LbApi lbApi;
	vector<string>::iterator it;
        for (it = jobIds.begin() ; it != jobIds.end() ; it++){
		// JobId
		lbApi.setJobId(*it);
		try{
			Status status=lbApi.getStatus(true,true);
			// Initialize ENDPOINT (start a new (thread of) job (s)
			cfgCxt->endpoint= status.getEndpoint();
			logInfo->print(WMS_DEBUG, "Endpoint set to: ",cfgCxt->endpoint);
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
			logInfo->print(WMS_WARNING, wmsg , "", true);
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
		} else {
			out << msg.str() ;
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
	// Dir Creation Management TBD
	bool createDir=false;
	bool checkChildren = true;
	// JobId
	glite::wmsutils::jobid::JobId jobid= status.getJobId();
	// Check Status (if needed)
	int code = status.checkCodes(Status::OP_OUTPUT, warnings, child);
	if (warnings.size()>0){
		wmsg = jobid.toString() + ": " + warnings ;
		logInfo->print(WMS_WARNING, wmsg, "", true);
		createWarnMsg(wmsg);
	}
	if (!listOnlyOpt &&  (code == 0 || ! child )){
		// It's not a node & retrieval is allowed (code==)
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
	if (code == 0){
		// JDL check
		glite::wms::jdl::Ad ad(status.getJdl());
		if (ad.hasAttribute(glite::wms::jdl::JDL::OUTPUTSB)){
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
			vector <pair<string , long> > empty ;
			this->listResult(empty, jobid.toString(), child);
		}
	}
	// Children (if present) Management
	std::vector<Status> children = status.getChildrenStates();
	if (children.size()){
		ostringstream msgVago ;
		//msg << "   Children files retrieved/purged:" <<endl;
		for (unsigned int i = 0 ;i<children.size();i++){
			retrieveOutput (msgVago,children[i],dirAbs+logName+"_"+ children[i].getJobId().getUnique(), true);
			//msg << " - "<<children[i].getJobId().toString() <<endl;
		}
	}
	bool parent = status.hasParent ( ) ;
	/* Purge logic: Job can be purged when
	* enpoint has been specified (parent has specified)
	* no parent is present */
	bool purge = (!listOnlyOpt) && (cfgCxt->endpoint != "" ) && (! parent) ;

	// checks Children
	if (checkChildren && children.size()>0){
		if (hasFiles){
			msg << "Output sandbox files for the DAG/Collection :\n" << jobid.toString() <<endl ;
			msg << "have been successfully retrieved and stored in the directory:"<<endl<<dirAbs << "\n\n";
		} else{
			msg << "No output files to be retrieved for the DAG/Collection:\n" << jobid.toString() << "\n\n";
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
		msg << "No output files to be retrieved for the job:\n" << jobid.toString() <<" \n\n";
	}
	if (purge){
		try {
			// Check Dir/purge
			logInfo->print(WMS_DEBUG,  jobid.toString() + ": JobPurging", "");
			jobPurge(jobid.toString(),cfgCxt);
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
		string filename = "";
		bool result = true;
		try {
			// gets the list of the out-files from the EndPoint
			logInfo->print(WMS_DEBUG, "getOutputFileList calling for: ",jobid);
			files = getOutputFileList(jobid, cfgCxt );
			hasFiles = hasFiles || (files.size()>0);
		} catch (BaseException &exc) {
			string desc = "";
			if (exc.Description){ desc =" (" + *(exc.Description)+ ")"; }
			if (child) {
				string wmsg = jobid + ": not allowed to retrieve the output" + desc ;
				logInfo->print(WMS_WARNING, wmsg , "" , true );
				if (warnsList){
					*warnsList += "- " + wmsg + "\n";
				} else if (wmsg.size()>0){
					warnsList = new string( );
					*warnsList = "The following errors occurred during the operation:\n";
					*warnsList += "=========================================================\n";
					*warnsList += wmsg + "\n";
				}
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
				for (unsigned int i = 0; i < files.size( ); i++){
					try{
						//fs::path cp (Utils::normalizePath(files[i].first), fs::system_specific); boost 1.29.1
						fs::path cp (Utils::normalizePath(files[i].first), fs::native);
						filename = cp.leaf( );
					} catch (fs::filesystem_error &ex){ }
						paths.push_back( make_pair (files[i].first, string(dirAbs +"/" + filename) ) );
				}
				// actually downloads the files
#if defined(WITH_GRID_FTP_API) || defined(WITH_GRID_FTP)
				this->gsiFtpGetFiles (paths);
#else
				this->curlGetFiles (paths);
#endif
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
	vector <pair<string , long> >::iterator it ;
	string ws = " ";
	ostringstream out ;
	// output message
	if (child){
		out << "\n\t>> child :" << jobid << "\n";
		if (files.size( ) == 0 ){
			out << "\tno output file to be retrieved\n";
		} else{
			for ( it = files.begin() ; it != files.end() ; it++ ){
				out << "\t - file :" << it->first << "\n";
				out << "\t   size (bytes) : " << it->second << "\n";
			}
		}
	} else{
		out << "\nJobId: " << jobid << "\n";
		if (files.size( ) == 0 ){
			out << "no output file to be retrieved\n";
		} else{
			for ( it = files.begin() ; it != files.end() ; it++ ){
				out << " - file :" << it->first << "\n";
				out << "   size (bytes) : " << it->second << "\n";
			}
		}
	}
	if (child){
		childrenFileList += out.str();
	} else{
		parentFileList = out.str();
	}
}
/*
* Creates the final Warning Msg
*/
void JobOutput::createWarnMsg(const std::string &msg ){
	int size = msg.size();
	if (warnsList && size >0 ){
		*warnsList += "- " + msg + "\n";
	} else if (size>0 ){
		warnsList = new string( );
		*warnsList = "The following warnings/errors have been found during the operation:\n";
		*warnsList += "========================================================================\n";
		*warnsList += "- " + msg + "\n";
	}
}

#if defined (WITH_GRID_FTP_API)
/*
*	gsiFtpGetFiles Method   (WITH_GRID_FTP_API)
*/
void JobOutput::gsiFtpGetFiles (std::vector <std::pair<std::string , std::string> > &paths) {
        vector <pair<std::string , string> >::iterator it ;
        if (globus_module_activate(GLOBUS_FTP_CLIENT_MODULE) < 0 ){
                        throw WmsClientException(__FILE__,__LINE__,
                                "globus_module_activate", ECONNABORTED,
                                "File Transferring Error",
                                "unable to activate GLOBUS_FTP_CLIENT_MODULE");
         }
  	 for ( it = paths.begin( ) ; it != paths.end( ) ; it++ ){
         	ostringstream info;
		// log-debug message
                info << "OutputSandbox file :" << it->first << "\n";
                info << "destination : " << it->second << "\n";
                logInfo->print(WMS_DEBUG, "File Transferring (gsiftp)\n", info.str());
		if (get (string(it->first), string(it->second)) ){
                		logInfo->print(WMS_DEBUG, "File Transferring (gsiftp)", "TRANSFER FAILED");
				 if (globus_module_deactivate_all() < 0 ){
					logInfo->print(WMS_WARNING, "File Transferring (gsiftp)", "Deactivation of Globus Modules failed");
         			};
        			throw WmsClientException(__FILE__,__LINE__,
                			"utilities::globus::put", ECONNABORTED,
                        		"File Transferring Error",
                                         "unable to get the file:\n" + info.str());
                   }
         }
         if (globus_module_deactivate_all() < 0 ){
		logInfo->print(WMS_WARNING, "File Transferring (gsiftp)", "Deactivation of Globus Modules failed");
         };
}
#elif defined (WITH_GRID_FTP)
/*
*	gsiFtpGetFiles Method  (WITH_GRID_FTP)
*/
void JobOutput::gsiFtpGetFiles (std::vector <std::pair<std::string , std::string> > &paths) {
        vector <pair<std::string , string> >::iterator it ;
	int code = 0;
        //globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
	 for ( it = paths.begin( ) ; it != paths.end( ) ; it++ ){
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
				"unable to get the file:\n" + info.str());
                   }
         }
}
#else
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
	vector <pair<std::string , string> >::iterator it ;
	string source = "" ;
	string destination = "" ;
	string file = "" ;
	CURLcode res;
	string *proxy = NULL;
	// user proxy
	if ( !paths.empty()){
                // checks the user proxy pathname
                if (!proxyFile){
                        throw WmsClientException(__FILE__,__LINE__,
                                "getFiles", DEFAULT_ERR_CODE,
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
                if ( curl ) {
                        // writing function
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, storegprBody);
                        // user proxy
                        curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE,  "PEM");
                        curl_easy_setopt(curl, CURLOPT_SSLCERT, proxyFile);
                        curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE,   "PEM");
                        curl_easy_setopt(curl, CURLOPT_SSLKEY,      proxy->c_str());
                        curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, NULL);
                        //trusted certificate directory
                        curl_easy_setopt(curl, CURLOPT_CAPATH, trustedCert);
                        //ssl options (no verify)
                        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
                        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
                        // verbose message
                        if ( Job::dbgOpt ){
                                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
                        }
                        // files to be downloaded
                        for ( it = paths.begin( ) ; it != paths.end( ) ; it++ ){
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
                                if (res < 0){
                                	ostringstream err ;
                                        err << "Failed to perform the downloading of the file:\n";
                                        err << "from " << source << "\n";
                                        err << "to " << destination<< "\n";
                                        throw WmsClientException(__FILE__,__LINE__,
                                        	"transferFiles", DEFAULT_ERR_CODE,
                                        	"File Transfer Error",
                                        	err.str() );
                                }
                        }
                }
 	}
}
#endif
}}}} // ending namespaces

