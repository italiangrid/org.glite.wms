/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpoperations.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include <fstream>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h> // flock

// Boost
#include <boost/lexical_cast.hpp>

#include "wmpstructconverter.h"

#include "wmpoperations.h"
#include "wmpconfiguration.h"

#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

// Utilities
#include "utilities/wmputils.h"

// Eventlogger
#include "eventlogger/wmpexpdagad.h"
#include "eventlogger/wmpeventlogger.h"

// Authorizer
#include "authorizer/wmpauthorizer.h"
#include "authorizer/wmpgaclmanager.h"
#include "authorizer/wmpvomsauthz.h"

// WMPManager
#include "wmpmanager.h"

//Logger
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "utilities/logging.h"

#include "commands/listfiles.h"

// Delegation
#include "wmpdelegation.h"

// Exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"
#include "glite/wms/jdl/RequestAdExceptions.h"

// RequestAd
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wms/jdl/PrivateAttributes.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/jdl_attributes.h"
#include "glite/wms/jdl/DAGAdManipulation.h"

// Logging and Bookkeeping
#include "glite/lb/Job.h"
#include "glite/lb/JobStatus.h"

// AdConverter class for jdl convertion and templates
#include "glite/wms/jdl/adconverter.h"

// Configuration
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"

#include "glite/wms/common/utilities/edgstrstream.h"

#include "glite/wms/partitioner/Partitioner.h"

#include "authorizer/wmpgaclmanager.h"

#include "libtar.h"

//namespace glite {
//namespace wms {
//namespace wmproxy {
//namespace server {
	
// Global variables for configuration
extern WMProxyConfiguration conf;
extern std::string filelist_global;

// Global variables for configuration attributes (ENV dependant)
extern std::string sandboxdir_global;

// WMProxy software version
const std::string WMP_MAJOR_VERSION = "2";
const std::string WMP_MINOR_VERSION = "1";
const std::string WMP_RELEASE_VERSION = "0";
const std::string WMP_POINT_VERSION = ".";
const std::string WMP_VERSION = WMP_MAJOR_VERSION
	+ WMP_POINT_VERSION + WMP_MINOR_VERSION
	+ WMP_POINT_VERSION + WMP_RELEASE_VERSION;

// DONE job output file
const std::string MARADONA_FILE = "Maradona.output";

// Perusal functionality
const std::string PERUSAL_FILE_2_PEEK_NAME = "files2peek";
const std::string TEMP_PERUSAL_FILE_NAME = "tempperusalfile";
const std::string PERUSAL_DATE_INFO_SEPARATOR = "-";
const int DEFAULT_PERUSAL_TIME_INTERVAL = 10; // seconds

// File size limit of globus URL copy
const long FILE_TRANSFER_SIZE_LIMIT = 2147000000; 
// 2 Giga = 2 * 1.073.741.824 = 2.147.483.648

// Document root variable
const char * DOCUMENT_ROOT = "DOCUMENT_ROOT";

// Listener environment variables
const char * BYPASS_SHADOW_HOST = "BYPASS_SHADOW_HOST";
const char * BYPASS_SHADOW_PORT = "BYPASS_SHADOW_PORT";
const char * GRID_CONSOLE_STDIN = "GRID_CONSOLE_STDIN";
const char * GRID_CONSOLE_STDOUT = "GRID_CONSOLE_STDOUT";

// cert & key environment variables
const char * X509_USER_CERT = "X509_USER_CERT";
const char * X509_USER_KEY = "X509_USER_KEY";
const char * GLITE_HOST_CERT = "GLITE_HOST_CERT";
const char * GLITE_HOST_KEY = "GLITE_HOST_KEY";

const int CURRENT_STEP_DEF_VALUE = 1;

const char * ABORT_SEQ_CODE = "UI=000000:NS=0000096660:WM=000000:"
	"BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000";

// Defining File Separator
#ifdef WIN 
   // Windows File Separator 
   const std::string FILE_SEPARATOR = "\\"; 
#else 
   // Linux File Separator 
   const std::string FILE_SEPARATOR = "/";
#endif

// Flag file to check for start operation fault causes
const std::string FLAG_FILE_UNZIP = ".unzipok";
const std::string FLAG_FILE_REGISTER_SUBJOBS = ".registersubjobsok";
const std::string FLAG_FILE_LOG_CHECKPOINTABLE = ".logcheckpointableok";

using namespace std;
using namespace glite::lb; // JobStatus
using namespace glite::wms::wmproxy::server;  //Exception codes
using namespace glite::wms::jdl; // DagAd, AdConverter
using namespace glite::wmsutils::jobid; //JobId
using namespace glite::wmsutils::exception; //Exception
using namespace glite::wms::common::configuration; // Configuration
using namespace boost::details::pool; //singleton
using namespace glite::wms::wmproxy::eventlogger;
using namespace glite::wms::wmproxy::utilities; //Exception

namespace logger         = glite::wms::common::logger;
namespace configuration  = glite::wms::common::configuration;
namespace wmpmanager	 = glite::wms::wmproxy::server;
namespace wmputilities	 = glite::wms::wmproxy::utilities;
namespace eventlogger    = glite::wms::wmproxy::eventlogger;
namespace authorizer 	 = glite::wms::wmproxy::authorizer;

// Possible values for jdl type attribute
enum type {
	TYPE_JOB,
	TYPE_DAG,
	TYPE_COLLECTION,
};

// "private" methods prototypes
pair<string, string> jobregister(jobRegisterResponse &jobRegister_response,
	const string &jdl, const string &delegation_id,
	const string &delegatedproxy, const string &vo, 
	authorizer::WMPAuthorizer *auth);
	
string regist(jobRegisterResponse &jobRegister_response, 
	authorizer::WMPAuthorizer *auth, const string &delegation_id,
	const string &delegatedproxy, const string &jdl, JobAd *jad);

pair<string, string> regist(jobRegisterResponse &jobRegister_response, 
	authorizer::WMPAuthorizer *auth, const string &delegation_id,
	const string &delegatedproxy, const string &jdl, WMPExpDagAd *dag,
	JobAd *jad = NULL);

void submit(const string &jdl, JobId *jid, authorizer::WMPAuthorizer *auth,
	eventlogger::WMPEventLogger &wmplogger, bool issubmit = false);

int listmatch(jobListMatchResponse &jobListMatch_response, const string &jdl,
	const string &delegation_id);

void jobpurge(jobPurgeResponse &jobPurge_response, JobId *jid, bool checkstate
	= true);

char ** 
copyEnvironment(char** sourceEnv) 
{
	extern char **environ;
	
	// Vars count
	char **oldEnv;
    for (oldEnv = environ; *oldEnv; oldEnv++);
	int nenvvars = (oldEnv - environ);
	
	// Memory allocation
    char ** targetEnv = (char **) malloc ((nenvvars + 1) * sizeof(char **)); 
   	char ** tmp = targetEnv;
    //if (!targetEnv) {
		// ERROR
    //}
    
    // Copying vars
    for (oldEnv = sourceEnv; *oldEnv; oldEnv++) {
	    *targetEnv++ = strdup(*oldEnv);
    }
    *targetEnv = NULL;
    
    return tmp;
}

void
setGlobalSandboxDir()
{
	GLITE_STACK_TRY("setGlobalSandboxDir()");
	
	string sandboxstagingpath = conf.getSandboxStagingPath();
	
	if (!getenv(DOCUMENT_ROOT)) {
		edglog(fatal)<<"DOCUMENT_ROOT variable not set"<<endl;
		throw FileSystemException( __FILE__, __LINE__,
  			"setGlobalSandboxDir()", wmputilities::WMS_FILE_SYSTEM_ERROR,
   			"DOCUMENT_ROOT variable not set");
	}
	string documentroot = getenv(DOCUMENT_ROOT);
	if (documentroot == "") {
		edglog(fatal)<<"DOCUMENT_ROOT is an empty string"<<endl;
		throw FileSystemException( __FILE__, __LINE__,
  			"setGlobalSandboxDir()", wmputilities::WMS_FILE_SYSTEM_ERROR,
   			"DOCUMENT_ROOT is an empty string"
   			"\n(please contact server administrator)");
	}
	
	// Checking if sandbox staging path starts with $DOCUMENT_ROOT
	if (sandboxstagingpath.find(documentroot) != 0) {
		edglog(severe)<<"ERROR: SandboxStagingPath inside "
			"configuration file MUST start with $DOCUMENT_ROOT"<<endl;
		throw FileSystemException( __FILE__, __LINE__,
  			"setGlobalSandboxDir()", wmputilities::WMS_FILE_SYSTEM_ERROR,
   			"SandboxStagingPath inside configuration file MUST start "
   			"with $DOCUMENT_ROOT\n(please contact server administrator)");
	}
	
	if (sandboxstagingpath != documentroot) {
		sandboxdir_global = sandboxstagingpath.substr(documentroot.length() + 1,
			sandboxstagingpath.length() - 1);
		if (sandboxdir_global.find(FILE_SEPARATOR) != string::npos) {
			edglog(fatal)<<"Sandbox directory name MUST be a single directory "
				"name (check configuration file SandboxStagingPath attribute)"
				<<endl;
			throw FileSystemException( __FILE__, __LINE__,
	  			"setGlobalSandboxDir()", wmputilities::WMS_FILE_SYSTEM_ERROR,
	   			"Sandbox directory name MUST be a single directory name"
	   			"\n(please contact server administrator)");
			}
	} else {
		sandboxdir_global = documentroot;
	}
	edglog(debug)<<"Sandbox directory: "<<sandboxdir_global<<endl;
	
	GLITE_STACK_CATCH();
}


int
logRemoteHostInfo()
{
	try {
		string msg = "Remote Host IP: ";
		string msg2 = "Remote CLIENT S DN: ";
		string msg3 = "Remote GRST CRED: ";
		edglog(info)
			<<"-------------------------------- Incoming Request "
				"--------------------------------"
			<<endl;
		
		if (getenv("REMOTE_ADDR")) {
			msg += string(getenv("REMOTE_ADDR"));
			if (getenv("REMOTE_PORT")) {
				msg += ":" + string(getenv("REMOTE_PORT"));
			}
		} else {
			msg += "Not Available";
		}
		msg += " - Remote Host Name: ";
		if (getenv("REMOTE_HOST")) {
			msg += string(getenv("REMOTE_HOST"));
		} else {
			msg += "Not Available";
		}
		if (getenv("SSL_CLIENT_S_DN")) {
			msg2 += string(getenv("SSL_CLIENT_S_DN"));
		} else {
			msg2 += "Not Available";
		}
		if (getenv("GRST_CRED_2")) {
			msg3 += string(getenv("GRST_CRED_2"));
		} else {
			msg3 += "Not Available";
		}
		
		edglog(info)<<msg<<endl;
	    edglog(info)<<msg2<<endl;
		edglog(info)<<msg3<<endl;
		edglog(info)
			<<"----------------------------------------"
				"------------------------------------------"
		<<endl;

		setGlobalSandboxDir();
		
		return 0;
	} catch (exception &ex) {
		edglog(fatal)<<"Exception caught: "<<ex.what()<<endl;
		return -1;
	}
}

/**
 * Returns an int value representing the type of the job described by the jdl
 * If type is not specified -> Job  // TBC change it in the future?
 */
int
getType(string jdl, Ad * ad = NULL)
{
	GLITE_STACK_TRY("getType()");
	edglog_fn("wmpoperations::getType");
	
	edglog(debug)<<"Getting type"<<endl;
	// Default type is normal
	int return_value = TYPE_JOB;
	
	Ad *in_ad = new Ad(jdl);
	
	if (in_ad->hasAttribute(JDL::TYPE)) {
		string type = 
			glite_wms_jdl_toLower((in_ad->getStringValue(JDL::TYPE))[0]);
		edglog(info) <<"Type: "<<type<<endl;
		if (type == JDL_TYPE_DAG) {
			return_value = TYPE_DAG;
		} else if (type == JDL_TYPE_JOB) {
			return_value = TYPE_JOB;
		} else if (type == JDL_TYPE_COLLECTION) {
			return_value = TYPE_COLLECTION;
		}
	}
	if (ad) {
		*ad = *in_ad;
	}
	delete in_ad;
	return return_value;

	GLITE_STACK_CATCH();
}

//
// WM Web Service available operations
//

// To get more information see WMProxy service wsdl file
void
getVersion(getVersionResponse &getVersion_response)
{
	GLITE_STACK_TRY("getVersion()");
	edglog_fn("wmpoperations::getVersion");
	logRemoteHostInfo();
	
	getVersion_response.version = WMP_VERSION;
	edglog(info)<<"Version retrieved: "<<getVersion_response.version<<endl;
	
	GLITE_STACK_CATCH();
}

void
getJDL(const std::string &job_id, JdlType jdltype,
	getJDLResponse &getJDL_response)
{
	GLITE_STACK_TRY("getJDL()");
	edglog_fn("wmpoperations::getJDL");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::getJDL()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	getJDL_response.jdl = "";
	switch (jdltype) {
		case WMS_JDL_ORIGINAL:
			getJDL_response.jdl = wmputilities::readTextFile(
				wmputilities::getJobJDLOriginalPath(*jid));
			break;
		case WMS_JDL_REGISTERED:
			getJDL_response.jdl = wmputilities::readTextFile(
				wmputilities::getJobJDLToStartPath(*jid));
			break;
		default:
			break;
	}
	
	delete jid;
	
	edglog(info)<<"JDL retrieved: "<<getJDL_response.jdl<<endl;
	
	GLITE_STACK_CATCH();
}

pair<string, string>
jobregister(jobRegisterResponse &jobRegister_response, const string &jdl,
	const string &delegation_id, const string &delegatedproxy, const string &vo,
	authorizer::WMPAuthorizer *auth)
{
	GLITE_STACK_TRY("jobregister()");
	edglog_fn("wmpoperations::jobregister");
	
	// Checking for VO in jdl file
	Ad *ad = new Ad();
	int type = getType(jdl, ad);
	if (ad->hasAttribute(JDL::VIRTUAL_ORGANISATION)) {
		if (vo != "") {
			string jdlvo = ad->getStringValue(JDL::VIRTUAL_ORGANISATION)[0];
			if (glite_wms_jdl_toLower(jdlvo) != glite_wms_jdl_toLower(vo)) {
				string msg = "Jdl " + JDL::VIRTUAL_ORGANISATION
					+ " attribute is different from delegated Proxy Virtual "
					"Organisation";
				edglog(error)<<msg<<endl;
				throw JobOperationException(__FILE__, __LINE__,
			    	"wmpoperations::jobregister()",
			    	wmputilities::WMS_OPERATION_NOT_ALLOWED, 
			    	msg);
			}
		}
	}
	delete ad;
	
	pair<string, string> returnpair;
	// Checking TYPE/JOBTYPE attributes and convert JDL when needed
	if (type == TYPE_DAG) {
		edglog(debug)<<"Type DAG"<<endl;
		WMPExpDagAd *dag = new WMPExpDagAd(jdl);
		dag->setLocalAccess(false);
		//TBD check()??
		returnpair = regist(jobRegister_response, auth, delegation_id, delegatedproxy,
			jdl, dag);
		delete dag;
	} else if (type == TYPE_JOB) {
		edglog(debug)<<"Type Job"<<endl;
		JobAd *jad = new JobAd(jdl);
		jad->setLocalAccess(false);
		//TBD jad->check();
        // Checking for multiple job type (not yet supported)
		if (jad->hasAttribute(JDL::JOBTYPE)) {
        	if (jad->getStringValue(JDL::JOBTYPE).size() > 1) {
        		edglog(error)<<"Composite Job Type not yet supported"<<endl;
            	throw JobOperationException(__FILE__, __LINE__,
        			"wmpoperations::jobregister()",
        			wmputilities::WMS_OPERATION_NOT_ALLOWED,
        			"Composite Job Type not yet supported");
           }
        }
        if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_PARAMETRIC)) {
			edglog(info)<<"Converting Parametric job to DAG..."<<endl;
			WMPExpDagAd *dag = new WMPExpDagAd(*(AdConverter::bulk2dag(jdl)));
			delete jad;
			dag->setLocalAccess(false);
			returnpair = regist(jobRegister_response, auth, delegation_id, delegatedproxy,
				jdl, dag);
			delete dag;
		} else if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_PARTITIONABLE)) {
			WMPExpDagAd *dag = NULL;
			returnpair = regist(jobRegister_response, auth, delegation_id, delegatedproxy,
				jdl, dag, jad);
			delete dag;
			delete jad;
		} else {
			returnpair.first = regist(jobRegister_response, auth, delegation_id, delegatedproxy,
				jdl, jad);
			returnpair.second = jad->toSubmissionString();
			delete jad;
		}
	} else if (type == TYPE_COLLECTION) {
		edglog(debug)<<"Type Collection"<<endl;
		WMPExpDagAd *dag = new WMPExpDagAd(*(AdConverter::collection2dag(jdl)));
		//TBD check()??
		dag->setLocalAccess(false);
		returnpair = regist(jobRegister_response, auth, delegation_id, delegatedproxy,
			jdl, dag);
		delete dag;
	} else {
		edglog(warning)<<"Type should have been Job!!"<<endl;
	}
	return returnpair;
	
	GLITE_STACK_CATCH();
}

void
jobRegister(jobRegisterResponse &jobRegister_response, const string &jdl,
	const string &delegation_id)
{
	GLITE_STACK_TRY("jobRegister()");
	edglog_fn("wmpoperations::jobRegister");
	logRemoteHostInfo();
	
	// Checking delegation id
	edglog(info)<<"Delegation ID: "<<delegation_id<<endl;
	if (delegation_id == "") {
		edglog(error)<<"Provided delegation ID not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"jobRegister()", wmputilities::WMS_DELEGATION_ERROR,
			"Delegation id not valid");
	}
	
	edglog(debug)<<"JDL to Register:\n"<<jdl<<endl;
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy from SSL Proxy cache
	string delegatedproxy = WMPDelegation::getDelegatedProxyPath(delegation_id);
	edglog(debug)<<"Delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN());
	} else {
		auth->authorize();
	}
	
	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"
			<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::jobRegister()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
		
	// Checking proxy validity
	authorizer::WMPAuthorizer::checkProxy(delegatedproxy);
	
	jobregister(jobRegister_response, jdl, delegation_id, delegatedproxy,
		wmputilities::getEnvVO(), auth);
	
	// Release Memory
	if (auth) {
		delete auth;
	}
	edglog(info)<<"Registered successfully"<<endl;

	GLITE_STACK_CATCH();
}

void
setSubjobFileSystem(authorizer::WMPAuthorizer *auth, 
	const string &jobid, vector<string> &jobids)
{
	GLITE_STACK_TRY("setSubjobFileSystem()");
	edglog_fn("wmpoperations::setSubjobFileSystem");
	
	// Getting LCMAPS mapped User ID
	uid_t jobdiruserid = auth->getUserId();
	edglog(debug)<<"User Id: "<<jobdiruserid<<endl;
	
	// Getting WMP Server User ID
	uid_t userid = getuid();

	string document_root = getenv(DOCUMENT_ROOT);
	
	// Getting delegated proxy inside job directory
	string proxy(wmputilities::getJobDelegatedProxyPath(jobid));
	edglog(debug)<<"Job delegated proxy: "<<proxy<<endl;
	
	// Creating a copy of the Proxy
	string proxybak = wmputilities::getJobDelegatedProxyPathBak(jobid);
	
	// Creating sub jobs directories
	if (jobids.size()) {
		edglog(debug)<<"Creating sub job directories for job:\n"<<jobid<<endl;
		int outcome = wmputilities::managedir(document_root, userid, jobdiruserid, jobids);
		if (outcome) {
			edglog(critical)
				<<"Unable to create sub jobs local directories for job:\n\t"
				<<jobid<<"\n"<<strerror(errno)<<" code: "<<errno<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"setSubjobFileSystem()", wmputilities::WMS_FATAL,
				"Unable to create sub jobs local directories\n(please contact "
				"server administrator)");
		} else {
			string link;
			string linkbak;
			
			vector<string>::iterator iter = jobids.begin();
			vector<string>::iterator const end = jobids.end();
			for (; iter != end; ++iter) {
				link = wmputilities::getJobDelegatedProxyPath(*iter);
				edglog(debug)<<"Creating proxy symbolic link for: "
					<<*iter<<endl;
				if (symlink(proxy.c_str(), link.c_str())) {
					if (errno != EEXIST) {
				      	edglog(critical)
				      		<<"Unable to create symbolic link to proxy file:\n\t"
				      		<<link<<"\n"<<strerror(errno)<<endl;
				      
				      	throw FileSystemException(__FILE__, __LINE__,
							"setSubjobFileSystem()", wmputilities::WMS_FATAL,
							"Unable to create symbolic link to proxy file\n"
								"(please contact server administrator)");
					}
			    }
			    
			    linkbak = wmputilities::getJobDelegatedProxyPathBak(*iter);
			    edglog(debug)<<"Creating backup proxy symbolic link for: "
					<<*iter<<endl;
			    if (symlink(proxybak.c_str(), linkbak.c_str())) {
			    	if (errno != EEXIST) {
				      	edglog(critical)
				      		<<"Unable to create symbolic link to backup proxy file:\n\t"
				      		<<linkbak<<"\n"<<strerror(errno)<<endl;
				      
				      	throw FileSystemException(__FILE__, __LINE__,
							"setSubjobFileSystem()", wmputilities::WMS_FATAL,
							"Unable to create symbolic link to backup proxy file\n"
								"(please contact server administrator)");
			    	}
			    }
			}
		}
		// Creating gacl file in the private job directory
		authorizer::WMPAuthorizer::setJobGacl(jobids);
	}
	
	GLITE_STACK_CATCH();
}

void
setJobFileSystem(authorizer::WMPAuthorizer *auth, const string &delegatedproxy,
	const string &jobid, vector<string> &jobids, const string &jdl,
	char * renewalproxy = NULL)
{
	GLITE_STACK_TRY("setJobFileSystem()");
	edglog_fn("wmpoperations::setJobFileSystem");
	
	// Getting LCMAPS mapped User ID
	uid_t jobdiruserid = auth->getUserId();
	edglog(debug)<<"User Id: "<<jobdiruserid<<endl;
	
	// Getting WMP Server User ID
	uid_t userid = getuid();

	string document_root = getenv(DOCUMENT_ROOT);
	
	// Creating job directory
	vector<string> job;
	job.push_back(jobid);
	edglog(debug)<<"Creating job directories for job:\n"<<jobid<<endl;
	if (wmputilities::managedir(document_root, userid, jobdiruserid, job)) {
		edglog(critical)<<"Unable to create job local directory for job:\n\t"
			<<jobid<<"\n"<<strerror(errno)<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"setJobFileSystem()", wmputilities::WMS_FATAL,
			"Unable to create job local directory\n(please contact server "
			"administrator)");
	}
	
	// Getting delegated proxy inside job directory
	string proxy(wmputilities::getJobDelegatedProxyPath(jobid));
	edglog(debug)<<"Job delegated proxy: "<<proxy<<endl;
	
	if (renewalproxy) {
		// Creating a symbolic link to renewal Proxy temporary file
		if (symlink(renewalproxy, proxy.c_str())) {
			if (errno != EEXIST) {
		      	edglog(critical)
		      		<<"Unable to create symbolic link to renewal proxy:\n\t"
		      		<<proxy<<"\n"<<strerror(errno)<<endl;
		      
		      	throw FileSystemException(__FILE__, __LINE__,
					"setJobFileSystem()", wmputilities::WMS_FATAL,
					"Unable to create symbolic link to renewal proxy\n(please "
					"contact server administrator)");
			}
	    }
	} else {
		// Copying delegated Proxy to destination URI
		wmputilities::fileCopy(delegatedproxy, proxy);
	}
	
	// Creating a copy of the Proxy
	string proxybak = wmputilities::getJobDelegatedProxyPathBak(jobid);
	wmputilities::fileCopy(delegatedproxy, proxybak);
	
	// Creating gacl file in the private job directory
	authorizer::WMPAuthorizer::setJobGacl(jobid);
	
	// Creating sub jobs directories
	if (jobids.size()) {
		setSubjobFileSystem(auth, jobid, jobids);
	}
	
	// Writing original jdl to disk
	JobId jid(jobid);
	Ad ad(jdl);
	edglog(debug)<<"Writing original jdl file: "
		<<wmputilities::getJobJDLOriginalPath(jid)<<endl;
	wmputilities::writeTextFile(wmputilities::getJobJDLOriginalPath(jid),
		ad.toLines());	

	GLITE_STACK_CATCH();
}

void
setAttributes(JobAd *jad, JobId *jid, const string &dest_uri)
{
	GLITE_STACK_TRY("setAttributes()");
	edglog_fn("wmpoperations::setAttributes JOB");
	
	// Inserting Proxy VO if not present in original jdl file
	edglog(debug)<<"Setting attribute JDL::VIRTUAL_ORGANISATION"<<endl;
	if (!jad->hasAttribute(JDL::VIRTUAL_ORGANISATION)) {
		jad->setAttribute(JDL::VIRTUAL_ORGANISATION, wmputilities::getEnvVO());
	}
	
	// Setting job identifier
	edglog(debug)<<"Setting attribute JDL::JOBID"<<endl;
	if (jad->hasAttribute(JDL::JOBID)) {
		jad->delAttribute(JDL::JOBID);
	}
	jad->setAttribute(JDL::JOBID, jid->toString());
	
	// Adding WMPInputSandboxBaseURI attribute
	edglog(debug)<<"Setting attribute JDL::WMPISB_BASE_URI"<<endl;
	if (jad->hasAttribute(JDL::WMPISB_BASE_URI)) {
		jad->delAttribute(JDL::WMPISB_BASE_URI);
	}
	jad->setAttribute(JDL::WMPISB_BASE_URI, dest_uri);
	
	// Adding INPUT_SANDBOX_PATH attribute
	edglog(debug)<<"Setting attribute JDLPrivate::INPUT_SANDBOX_PATH"<<endl;
	if (jad->hasAttribute(JDLPrivate::INPUT_SANDBOX_PATH)) {
		jad->delAttribute(JDLPrivate::INPUT_SANDBOX_PATH);
	}
	jad->setAttribute(JDLPrivate::INPUT_SANDBOX_PATH,
		wmputilities::getInputSBDirectoryPath(*jid));
	
	// Adding OUTPUT_SANDBOX_PATH attribute
	edglog(debug)<<"Setting attribute JDLPrivate::OUTPUT_SANDBOX_PATH"<<endl;
	if (jad->hasAttribute(JDLPrivate::OUTPUT_SANDBOX_PATH)) {
		jad->delAttribute(JDLPrivate::OUTPUT_SANDBOX_PATH);
	}
	jad->setAttribute(JDLPrivate::OUTPUT_SANDBOX_PATH,
		wmputilities::getOutputSBDirectoryPath(*jid));

	// Adding Proxy attributes
	edglog(debug)<<"Setting attribute JDL::CERT_SUBJ"<<endl;
	if (jad->hasAttribute(JDL::CERT_SUBJ)) {
		jad->delAttribute(JDL::CERT_SUBJ);
	}
	jad->setAttribute(JDL::CERT_SUBJ,
		wmputilities::convertDNEMailAddress(wmputilities::getUserDN()));
	
	edglog(debug)<<"Setting attribute JDLPrivate::USERPROXY"<<endl;
	if (jad->hasAttribute(JDLPrivate::USERPROXY)) {
		jad->delAttribute(JDLPrivate::USERPROXY);
	}
	jad->setAttribute(JDLPrivate::USERPROXY,
		wmputilities::getJobDelegatedProxyPath(*jid));
	
	// Checking for empty string value
	if (jad->hasAttribute(JDL::MYPROXY)) {
		if (jad->getString(JDL::MYPROXY) == "") {
			edglog(debug)<<JDL::MYPROXY 
				+ " attribute value is empty string, removing..."<<endl;
			jad->delAttribute(JDL::MYPROXY);
		}
	}
	
	if (jad->hasAttribute(JDL::HLR_LOCATION)) {
		if (jad->getString(JDL::HLR_LOCATION) == "") {
			edglog(debug)<<JDL::HLR_LOCATION 
				+ " attribute value is empty string, removing..."<<endl;
			jad->delAttribute(JDL::HLR_LOCATION);
		}
	}
	
	if (jad->hasAttribute(JDL::JOB_PROVENANCE)) {
		if (jad->getString(JDL::JOB_PROVENANCE) == "") {
			edglog(debug)<<JDL::JOB_PROVENANCE
				+ " attribute value is empty string, removing..."<<endl;
			jad->delAttribute(JDL::JOB_PROVENANCE);
		}
	}

	GLITE_STACK_CATCH();
}


string
regist(jobRegisterResponse &jobRegister_response, authorizer::WMPAuthorizer *auth,
	const string &delegation_id, const string &delegatedproxy, const string &jdl,
	JobAd *jad)
{
	GLITE_STACK_TRY("regist()");
	edglog_fn("wmpoperations::regist JOB");
	
	if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_INTERACTIVE)) {
		if (!jad->hasAttribute(JDL::SHHOST)) {
			string msg = "Mandatory attribute " + JDL::SHHOST + " not set";
			edglog(error)<<msg<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"regist()", wmputilities::WMS_JDL_PARSING,
				msg);
		}
		if (!jad->hasAttribute(JDL::SHPORT)) {
			string msg = "Mandatory attribute " + JDL::SHPORT + " not set";
			edglog(error)<<msg<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"regist()", wmputilities::WMS_JDL_PARSING,
				msg);
		}
		if (!jad->hasAttribute(JDL::SHPIPEPATH)) {
			string msg = "Mandatory attribute " + JDL::SHPIPEPATH + " not set";
			edglog(error)<<msg<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"regist()", wmputilities::WMS_JDL_PARSING,
				msg);
		}
		jad->addAttribute(JDL::ENVIRONMENT, string(BYPASS_SHADOW_HOST) + "=" 
			+ jad->getString(JDL::SHHOST));
		jad->addAttribute(JDL::ENVIRONMENT, string(BYPASS_SHADOW_PORT) + "=" 
			+ boost::lexical_cast<std::string>(jad->getInt(JDL::SHPORT)));
		string pipepath = jad->getString(JDL::SHPIPEPATH);
		jad->addAttribute(JDL::ENVIRONMENT, string(GRID_CONSOLE_STDIN) + "=" 
			+ pipepath + ".in");
		jad->addAttribute(JDL::ENVIRONMENT, string(GRID_CONSOLE_STDOUT) + "=" 
			+ pipepath + ".out");
	} else if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_CHECKPOINTABLE)) {
		if (!jad->hasAttribute(JDL::CHKPT_STEPS)) {
			string msg = "Mandatory attribute " + JDL::CHKPT_STEPS + " not set";
			edglog(error)<<msg<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"regist()", wmputilities::WMS_JDL_PARSING,
				msg);
		}
	}
	
	// Creating unique identifier
	JobId *jid = new JobId();
	
	//WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	std::pair<std::string, int> lbaddress_port;
	
	// Checking for attribute JDL::LB_ADDRESS
	if (jad->hasAttribute(JDL::LB_ADDRESS)) {
		string lbaddressport = jad->getString(JDL::LB_ADDRESS);
		wmputilities::parseAddressPort(lbaddressport, lbaddress_port);
	} else {
	 	lbaddress_port = conf.getLBServerAddressPort();
	}
	edglog(debug)<<"LB Address: "<<lbaddress_port.first<<endl;
	edglog(debug)<<"LB Port: "
		<<boost::lexical_cast<std::string>(lbaddress_port.second)<<endl;
	if (lbaddress_port.second == 0) {
		jid->setJobId(lbaddress_port.first);
	} else {
		jid->setJobId(lbaddress_port.first, lbaddress_port.second);
	}
	string stringjid = jid->toString();
	edglog(info)<<"Register for job id: "<<stringjid<<endl;

	// Getting Input Sandbox Destination URI
	//string defaultprotocol = conf.getDefaultProtocol();
	//int defaultport = conf.getDefaultPort();
	string dest_uri = wmputilities::getDestURI(stringjid, conf.getDefaultProtocol(),
		conf.getDefaultPort());
	edglog(debug)<<"Destination URI: "<<dest_uri<<endl;
	
	setAttributes(jad, jid, dest_uri);
	
	// Initializing logger
	edglog(debug)<<"Endpoint: "<<wmputilities::getEndpoint()<<endl;
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	lbaddress_port = conf.getLBLocalLoggerAddressPort();
	edglog(debug)<<"LB Address: "<<lbaddress_port.first<<endl;
	edglog(debug)<<"LB Port: "
		<<boost::lexical_cast<std::string>(lbaddress_port.second)<<endl;
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid, 
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
			
	// Setting user proxy
	if (wmplogger.setUserProxy(delegatedproxy)) {
			edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
			throw AuthenticationException(__FILE__, __LINE__,
				"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
				"Unable to set User Proxy for LB context");
	}
	
	// Registering the job
	jad->check();
	wmplogger.registerJob(jad, wmputilities::getJobJDLToStartPath(*jid));
	
	// Registering for Proxy renewal
	char * renewalproxy = NULL;
	if (jad->hasAttribute(JDL::MYPROXY)) {
		edglog(debug)<<"Registering Proxy renewal..."<<endl;
		renewalproxy = wmplogger.registerProxyRenewal(delegatedproxy,
			(jad->getStringValue(JDL::MYPROXY))[0]);
	}
	
	// Creating private job directory with delegated Proxy
	vector<string> jobids;
	setJobFileSystem(auth, delegatedproxy, stringjid, jobids,
		jdl, renewalproxy);
	
	// Writing registered JDL (to start)
	edglog(debug)<<"Writing jdl to start file: "
		<<wmputilities::getJobJDLToStartPath(*jid)<<endl;
	//TBC jad->toSubmissionString() = jad->check(false) + jad->toLines()
	jad->check(false);
	wmputilities::writeTextFile(wmputilities::getJobJDLToStartPath(*jid),
		jad->toLines());
	
	delete jid;

	// Logging delegation id & original jdl
	edglog(debug)<<"Logging user tag JDL::DELEGATION_ID..."<<endl;
	wmplogger.logUserTag(JDL::DELEGATION_ID, delegation_id);
	//edglog(debug)<<"Logging user tag JDL::JDL_ORIGINAL..."<<endl;
	//wmplogger.logUserTag(JDL::JDL_ORIGINAL, jdl);
	
	wmplogger.logUserTag(JDL::LB_SEQUENCE_CODE, string(wmplogger.getSequence()));

	// Creating job identifier structure to return to the caller
	JobIdStructType *job_id_struct = new JobIdStructType();
	job_id_struct->id = stringjid;
	job_id_struct->name = new string("");
	job_id_struct->childrenJob = new vector<JobIdStructType*>;
	jobRegister_response.jobIdStruct = job_id_struct;
	
	edglog(info)<<"Job successfully registered: "<<stringjid<<endl;
	
	return stringjid;

	GLITE_STACK_CATCH();
}

void
setAttributes(WMPExpDagAd *dag, JobId *jid, const string &dest_uri)
{
	GLITE_STACK_TRY("setAttributes()");
	edglog_fn("wmpoperations::setAttributes DAG");
	
	// Adding WMProxyDestURI attribute
	edglog(debug)<<"Setting attribute JDL::WMPISB_BASE_URI"<<endl;
	if (dag->hasAttribute(JDL::WMPISB_BASE_URI)) {
		dag->removeAttribute(JDL::WMPISB_BASE_URI);
	}
	dag->setReserved(JDL::WMPISB_BASE_URI, dest_uri);
	
	// Adding INPUT_SANDBOX_PATH attribute
	edglog(debug)<<"Setting attribute JDLPrivate::INPUT_SANDBOX_PATH"<<endl;
	if (dag->hasAttribute(JDLPrivate::INPUT_SANDBOX_PATH)) {
		dag->removeAttribute(JDLPrivate::INPUT_SANDBOX_PATH);
	}
	dag->setReserved(JDLPrivate::INPUT_SANDBOX_PATH,
		wmputilities::getInputSBDirectoryPath(*jid));
	
	// Adding Proxy attributes
	edglog(debug)<<"Setting attribute JDL::CERT_SUBJ"<<endl;
	if (dag->hasAttribute(JDL::CERT_SUBJ)) {
		dag->removeAttribute(JDL::CERT_SUBJ);
	}
	dag->setReserved(JDL::CERT_SUBJ,
		wmputilities::convertDNEMailAddress(wmputilities::getUserDN()));
	
	edglog(debug)<<"Setting attribute JDLPrivate::USERPROXY"<<endl;
	if (dag->hasAttribute(JDLPrivate::USERPROXY)) {
		dag->removeAttribute(JDLPrivate::USERPROXY);
	}
	dag->setReserved(JDLPrivate::USERPROXY,
		wmputilities::getJobDelegatedProxyPath(*jid));
		
	// Checking for empty string value
	if (dag->hasAttribute(JDL::MYPROXY)) {
		if (dag->getString(JDL::MYPROXY) == "") {
			edglog(debug)<<JDL::MYPROXY 
				+ " attribute value is empty string, removing..."<<endl;
			dag->removeAttribute(JDL::MYPROXY);
		}
	}
	
	if (dag->hasAttribute(JDL::HLR_LOCATION)) {
		if (dag->getString(JDL::HLR_LOCATION) == "") {
			edglog(debug)<<JDL::HLR_LOCATION 
				+ " attribute value is empty string, removing..."<<endl;
			dag->removeAttribute(JDL::HLR_LOCATION);
		}
	}
	
	if (dag->hasAttribute(JDL::JOB_PROVENANCE)) {
		if (dag->getString(JDL::JOB_PROVENANCE) == "") {
			edglog(debug)<<JDL::JOB_PROVENANCE 
				+ " attribute value is empty string, removing..."<<endl;
			dag->removeAttribute(JDL::JOB_PROVENANCE);
		}
	}
		
	GLITE_STACK_CATCH();
}


pair<string, string>
regist(jobRegisterResponse &jobRegister_response, authorizer::WMPAuthorizer *auth,
	const string &delegation_id, const string &delegatedproxy, const string &jdl,
	WMPExpDagAd *dag, JobAd *jad)
{
	GLITE_STACK_TRY("regist()");
	edglog_fn("wmpoperations::regist DAG");

	// Creating unique identifier
	JobId *jid = new JobId();
	
	//WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	std::pair<std::string, int> lbaddress_port = conf.getLBServerAddressPort();
	
	// Checking for attribute JDL::LB_ADDRESS
	if (jad) {
		if (jad->hasAttribute(JDL::LB_ADDRESS)) {
			string lbaddressport = jad->getString(JDL::LB_ADDRESS);
			wmputilities::parseAddressPort(lbaddressport, lbaddress_port);
		} else {
		 	lbaddress_port = conf.getLBServerAddressPort();
		}
	} else {
		if (dag->hasAttribute(JDL::LB_ADDRESS)) {
			string lbaddressport = dag->getString(JDL::LB_ADDRESS);
			wmputilities::parseAddressPort(lbaddressport, lbaddress_port);
		} else {
		 	lbaddress_port = conf.getLBServerAddressPort();
		}
	}
	edglog(debug)<<"LB Address: "<<lbaddress_port.first<<endl;
	edglog(debug)<<"LB Port: "
		<<boost::lexical_cast<std::string>(lbaddress_port.second)<<endl;
	if (lbaddress_port.second == 0) {
		jid->setJobId(lbaddress_port.first);
	} else {
		jid->setJobId(lbaddress_port.first, lbaddress_port.second);
	}
	string stringjid = jid->toString();
	edglog(info)<<"Register for job id: "<<stringjid<<endl;
	
	//string defaultprotocol = conf.getDefaultProtocol();
	//int defaultport = conf.getDefaultPort();
	
	// Initializing logger
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid, 
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	
	vector<string> jobids;
	if (jad) {
		// Partitionable job registration
		edglog(info)<<"Partitionable job registration..."<<endl;
		
		// Inserting Proxy VO if not present in original jdl file
		if (!jad->hasAttribute(JDL::VIRTUAL_ORGANISATION)) {
			jad->setAttribute(JDL::VIRTUAL_ORGANISATION, wmputilities::getEnvVO());
		}
		
		// Adding fake JDL::WMPISB_BASE_URI attribute to pass check (toSubmissionString)
		if (jad->hasAttribute(JDL::WMPISB_BASE_URI)) {
			jad->delAttribute(JDL::WMPISB_BASE_URI);	
		}
		jad->setAttribute(JDL::WMPISB_BASE_URI, "protocol://address");
		
		// Checking before call to Partitioner
		jad->check(false);
		
		int res_num;
		if (jad->hasAttribute(JDL::PARTITION_NUMBER)) {
			res_num = jad->getInt(JDL::PARTITION_NUMBER);
		} else {
			jobListMatchResponse jobListMatch_response;
			res_num = listmatch(jobListMatch_response,
				jad->toSubmissionString(), delegation_id);
		}
		
		if (jad->hasAttribute(JDL::CHKPT_STEPS)) {
			int steps = 0;
			if (jad->getType(JDL::CHKPT_STEPS) == Ad::TYPE_INTEGER) {
				steps = jad->getInt(JDL::CHKPT_STEPS);
			} else {
				steps = (int)(jad->getStringValue(JDL::CHKPT_STEPS)).size();
			}
			if (jad->hasAttribute(JDL::CHKPT_CURRENTSTEP)) {
				steps = steps - jad->getInt(JDL::CHKPT_CURRENTSTEP);
			}
			// Number of resources equal to min between res_num and steps
			res_num = (res_num < steps) ? res_num : steps;
		}
		
		if (jad->hasAttribute(JDL::PREJOB)) {
			res_num++;
		}
		if (jad->hasAttribute(JDL::POSTJOB)) {
			res_num++;
		}
		
		// Removing fake
		jad->delAttribute(JDL::WMPISB_BASE_URI);
		
		jobids = wmplogger.generateSubjobsIds(res_num);
		
		// Setting job identifier
		edglog(debug)<<"Setting attribute JDL::JOBID"<<endl;
		jad->setAttribute(JDL::JOBID, stringjid);
		
		// Removing InputSandbox
		vector<string> isb;
		if (jad->hasAttribute(JDL::INPUTSB)) {
			isb = jad->getStringValue(JDL::INPUTSB);
			jad->delAttribute(JDL::INPUTSB);
		}
		
		edglog(debug)<<"Converting Partitionable job to DAG..."<<endl;
		glite::wms::partitioner::Partitioner part(jad->ad(), jobids);
		
		// Adding InputSandbox again
		DAGAd *dagad = part.createDag();
		if (isb.size() != 0) {
			set_input_sandbox(*dagad, isb);
		}
		
		dag = new WMPExpDagAd(dagad);
		dag->setLocalAccess(false);
		
	} else {
		// Setting job identifier
		edglog(debug)<<"Setting attribute WMPExpDagAd::EDG_JOBID"<<endl;
		if (dag->hasAttribute(JDL::JOBID)) {
			//dag->removeAttribute(JDL::JOBID);
		}
		dag->setAttribute(WMPExpDagAd::EDG_JOBID, stringjid);
		
		// Inserting Proxy VO if not present in original jdl file
		edglog(debug)<<"Setting attribute JDL::VIRTUAL_ORGANISATION"<<endl;
		if (!dag->hasAttribute(JDL::VIRTUAL_ORGANISATION)) {
			dag->setReserved(JDL::VIRTUAL_ORGANISATION, wmputilities::getEnvVO());
		}
        jobids = wmplogger.generateSubjobsIds(dag->size());
	}
	
	// Setting generated sub jobs jobids
	dag->setJobIds(jobids);
	
	// Getting Input Sandbox Destination URI
	string dest_uri = wmputilities::getDestURI(stringjid, conf.getDefaultProtocol(),
		conf.getDefaultPort());
	edglog(debug)<<"Destination uri: "<<dest_uri<<endl;
	setAttributes(dag, jid, dest_uri);
		
	// Setting user proxy
	if (wmplogger.setUserProxy(delegatedproxy)) {
			edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
			throw AuthenticationException(__FILE__, __LINE__,
				"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
				"Unable to set User Proxy for LB context");
	}
	
	////////////////////////////////////////////////////
	// If present -> Mandatory attribute not present
	// dag->getSubmissionStrings();
	////////////////////////////////////////////////////
	
	//dag->toString(ExpDagAd::SUBMISSION);
	wmplogger.registerDag(dag, wmputilities::getJobJDLToStartPath(*jid));
	
	// Registering for Proxy renewal
	char * renewalproxy = NULL;
	if (dag->hasAttribute(JDL::MYPROXY)) {
		edglog(debug)<<"Registering Proxy renewal..."<<endl;
		renewalproxy = wmplogger.registerProxyRenewal(delegatedproxy,
			(dag->getAttribute(WMPExpDagAd::MYPROXY_SERVER)));
	}
	
	// Creating private job directory with delegated Proxy
	if (dag->hasAttribute(JDLPrivate::ZIPPED_ISB)) {
		// Creating job directories only for the parent. -> empty vector.
		vector<string> emptyvector;
		setJobFileSystem(auth, delegatedproxy, stringjid, emptyvector,
			jdl, renewalproxy);
	} else {
		// Sub jobs directory MUST be created now
		setJobFileSystem(auth, delegatedproxy, stringjid, jobids,
			jdl, renewalproxy);
	}
	
	string dagjdl = dag->toString(ExpDagAd::MULTI_LINES);
	pair<string, string> returnpair;
	returnpair.first = stringjid;
	returnpair.second = dagjdl;
	
	// Writing registered JDL (to start)
	edglog(debug)<<"Writing jdl to start file: "
		<<wmputilities::getJobJDLToStartPath(*jid)<<endl;
	wmputilities::writeTextFile(wmputilities::getJobJDLToStartPath(*jid),
		dagjdl);
	
	delete jid;

	// Logging delegation id & original jdl
	edglog(debug)<<"Logging user tag JDL::DELEGATION_ID..."<<endl;
	wmplogger.logUserTag(JDL::DELEGATION_ID, delegation_id);
	//edglog(debug)<<"Logging user tag JDL::JDL_ORIGINAL..."<<endl;
	//wmplogger.logUserTag(JDL::JDL_ORIGINAL, jdl);
	
	wmplogger.logUserTag(JDL::LB_SEQUENCE_CODE, string(wmplogger.getSequence()));
	
	// Creating job identifier structure to return to the caller
	JobIdStructType *job_id_struct = new JobIdStructType();
	JobIdStruct job_struct = dag->getJobIdStruct();
	job_id_struct->id = job_struct.jobid.toString(); // should be equal to: jid->toString()
	job_id_struct->name = job_struct.nodeName;
	job_id_struct->childrenJob = convertJobIdStruct(job_struct.children);
	jobRegister_response.jobIdStruct = job_id_struct;
	
	edglog(info)<<"Job successfully registered: "<<stringjid<<endl;

	return returnpair;
	
	GLITE_STACK_CATCH();
}

SOAP_FMAC5 int SOAP_FMAC6 
soap_serve_start(struct soap *soap)
{
	struct ns1__jobStartResponse response;
	soap_serializeheader(soap);
	soap_serialize_ns1__jobStartResponse(soap, &response);
	if (soap_begin_count(soap)) {
		return soap->error; 
	}
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__jobStartResponse(soap, &response,
		 	"ns1:jobStartResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap)) {
			 return soap->error;
		 }
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns1__jobStartResponse(soap, &response,
	 	"ns1:jobStartResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap)) {
		return soap->error;
	}
	
	return soap_closesock(soap); 	
}

void
jobStart(jobStartResponse &jobStart_response, const string &job_id,
	struct soap *soap)
{
	GLITE_STACK_TRY("jobStart()");
	edglog_fn("wmpoperations::jobStart");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::jobStart()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	//WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	
	// Setting user proxy
	if (wmplogger.setUserProxy(delegatedproxy)) {
		edglog(critical)<<"Unable to set User Proxy for LB context"<<endl;
		throw AuthenticationException(__FILE__, __LINE__,
			"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
			"Unable to set User Proxy for LB context");
	}
	
	// Checking proxy validity
	try {
		authorizer::WMPAuthorizer::checkProxy(delegatedproxy);
	} catch (Exception &ex) {
		if (ex.getCode() == wmputilities::WMS_PROXY_EXPIRED) {
			wmplogger.setSequenceCode(ABORT_SEQ_CODE);
			wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ABORT,
				"Job Proxy has expired", true, true);
			jobPurgeResponse jobPurge_response;
			jobpurge(jobPurge_response, jid, false);	
		}
		throw ex;
	}
	
	//if (!wmplogger.isRegisterEventOnly()) {
	string seqcode = wmplogger.isStartAllowed();
	if (seqcode == "") {
		edglog(error)<<"The job has already been started"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"The job has already been started");
	}
	
	wmplogger.setSequenceCode(seqcode);
	wmplogger.incrementSequenceCode();
	
	regJobEvent event = wmplogger.retrieveRegJobEvent(job_id);

	if (event.parent != "") {
		string msg = "the job is a DAG subjob. The parent is: "
			+ event.parent;
		edglog(error)<<msg<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart()", wmputilities::WMS_OPERATION_NOT_ALLOWED, msg);
	}
	/*if (event.jdl == "") {
		edglog(critical)<<"No Register event found quering LB; unable to get "
			"registered jdl"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart()", wmputilities::WMS_IS_FAILURE,
			"Unable to get registered jdl"
			"\n(please contact server administrator)");
	}
	
	edglog(debug)<<"JDL to Start:\n"<<event.jdl<<endl;*/
	
	string jdlpath = wmputilities::getJobJDLToStartPath(*jid);
	if (!wmputilities::fileExists(jdlpath)) {
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"Unable to start job");
	}
	
	if (conf.getAsyncJobStart()) {
		// Creating SOAP answer
		if (soap_serve_start(soap)
				|| (soap->fserveloop && soap->fserveloop(soap))) {
			// if ERROR throw exception??
			soap_send_fault(soap);
		}	
	}
	
	Ad tempad;
	tempad.fromFile(jdlpath);
	submit(tempad.toString(), jid, auth, wmplogger);
	
	delete auth;
	delete jid;
	
	GLITE_STACK_CATCH();
}


void
logCheckpointable(WMPEventLogger * wmplogger, JobAd * jad,
	const string &stringjid) 
{
	GLITE_STACK_TRY("logCheckpointable()");
	edglog_fn("wmpoperations::logCheckpointable");
	
	int current_step;
	if (jad->hasAttribute(JDL::CHKPT_CURRENTSTEP)) {
		current_step = jad->getInt(JDL::CHKPT_CURRENTSTEP);
	} else {
		current_step = CURRENT_STEP_DEF_VALUE;	
	}
	Ad statead;
	if (jad->hasAttribute(JDL::CHKPT_JOBSTATE)) {
		statead = jad->getAd(JDL::CHKPT_JOBSTATE);
		if (statead.hasAttribute(JDL::CHKPT_STATEID)) {
			statead.delAttribute(JDL::CHKPT_STATEID);
		}
		edglog(debug)<<"Setting attribute JDL::CHKPT_STATEID"<<endl;
		statead.addAttribute(JDL::CHKPT_STATEID, stringjid);
	} else {
		edglog(debug)<<"Setting attribute JDL::CHKPT_STATEID"<<endl;
		statead.setAttribute(JDL::CHKPT_STATEID, stringjid);
		edglog(debug)<<"Setting attribute JDL::CHKPT_DATA"<<endl;
		Ad emptyad; // Empty Ad i.e. "[]"
		statead.setAttribute(JDL::CHKPT_DATA, &emptyad);
		
		edglog(debug)<<"Setting attribute JDL::CHKPT_STEPS"<<endl;
		if (jad->getType(JDL::CHKPT_STEPS) == Ad::TYPE_INTEGER) {
			statead.setAttribute(JDL::CHKPT_STEPS,
				jad->getIntValue(JDL::CHKPT_STEPS)[0]);
		} else {
			statead.setAttributeExpr(JDL::CHKPT_STEPS,
				jad->lookUp(JDL::CHKPT_STEPS)->Copy());
		}
		
		edglog(debug)<<"Setting attribute JDL::CHKPT_CURRENTSTEP"<<endl;
		statead.setAttribute(JDL::CHKPT_CURRENTSTEP, current_step);
	}
	if (wmplogger->logCheckpointable(boost::lexical_cast<std::string>(
			current_step).c_str(), statead.toString().c_str())) {
		edglog(severe)<<"LB logging checkpoint state failed"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"logCheckpointable()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"LB logging checkpoint state failed");
	}
	
	GLITE_STACK_CATCH();
}


void
submit(const string &jdl, JobId *jid, authorizer::WMPAuthorizer *auth,
	eventlogger::WMPEventLogger &wmplogger, bool issubmit)
{
	// File descriptor to control mutual exclusion in start operation
	int fd = -1; 
	
	try {
		edglog_fn("wmpoperations::submit");
		
		if (issubmit) {
			string seqcode =
				wmplogger.getUserTag(eventlogger::WMPEventLogger::QUERY_SEQUENCE_CODE);
			wmplogger.setSequenceCode(const_cast<char*>(seqcode.c_str()));
			wmplogger.incrementSequenceCode();
		} else {
			// Locking lock file to ensure only one start operation is running
			// at any time for a specific job
			string lockfile = wmputilities::getJobStartLockFilePath(*jid);
			edglog(debug)<<"Opening lock file: "<<lockfile<<endl;
			fd = open(lockfile.c_str(), O_CREAT | O_RDONLY, S_IRWXU);
			if (fd == -1) {
				edglog(debug)<<"Unable to open lock file: "<<lockfile<<endl;
				throw JobOperationException( __FILE__, __LINE__,
          			"submit()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
           			"unable to open lock file");
			}
			if (flock(fd, LOCK_EX | LOCK_NB)) {
				edglog(debug)<<"Lock file is locked for job: "<<jid->toString()
					<<endl;
				close(fd);
				throw JobOperationException( __FILE__, __LINE__,
          			"submit()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
           			"start operation already in progress");
			}
		}
		
		// This log is needed as the first step inside submit method
		// WARNING: DO NOT move it in a different location
		/*edglog(debug)<<"Logging LOG_ACCEPT..."<<endl;
		int error = wmplogger.logAcceptEventSync();
		if (error) {
			edglog(debug)<<"LOG_ACCEPT failed, error code: "<<error<<endl;
			throw LBException(__FILE__, __LINE__,
				"submit()", wmputilities::WMS_LOGGING_ERROR,
				"unable to complete operation");
		}*/
			
		if (conf.getAsyncJobStart()) {
			// \/ Copy environment and restore it right after FCGI_Finish
			extern char **environ;
			char ** backupenv = copyEnvironment(environ);
			FCGI_Finish();
			environ = backupenv;
			// /\ From here on, execution is asynchronous
		}
		
		edglog(debug)<<"Logging LOG_ACCEPT..."<<endl;
		int error = wmplogger.logAcceptEventSync();
		if (error) {
			edglog(debug)<<"LOG_ACCEPT failed, error code: "<<error<<endl;
			throw LBException(__FILE__, __LINE__,
				"submit()", wmputilities::WMS_LOGGING_ERROR,
				"unable to complete operation");
		}
		
		edglog(debug)<<"Registering LOG_ENQUEUE_START"<<std::endl;
		wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_START,
			"LOG_ENQUEUE_START", true, true, filelist_global.c_str(),
			wmputilities::getJobJDLToStartPath(*jid).c_str());
	
		// Getting delegated proxy inside job directory
		string proxy(wmputilities::getJobDelegatedProxyPath(*jid));
		edglog(debug)<<"Job delegated proxy: "<<proxy<<endl;
		
		string jdltostart = jdl;
		
		int type = getType(jdl);
		if (type == TYPE_JOB) {
			JobAd * jad = new JobAd(jdl);
			jad->setLocalAccess(false);
			
			// Getting Input Sandbox Destination URI
			string dest_uri = wmputilities::getDestURI(jid->toString(),
				conf.getDefaultProtocol(), conf.getDefaultPort());
			edglog(debug)<<"Destination URI: "<<dest_uri<<endl;
			
			// Adding OutputSandboxDestURI attribute
			if (jad->hasAttribute(JDL::OUTPUTSB)) {
				edglog(debug)<<"Setting attribute JDL::OSB_DEST_URI"<<endl;
				vector<string> osbdesturi;
				if (jad->hasAttribute(JDL::OSB_DEST_URI)) {
					osbdesturi = wmputilities::computeOutputSBDestURI(
						jad->getStringValue(JDL::OSB_DEST_URI),
						dest_uri);
					jad->delAttribute(JDL::OSB_DEST_URI);
				} else if (jad->hasAttribute(JDL::OSB_BASE_DEST_URI)) {
					osbdesturi = wmputilities::computeOutputSBDestURIBase(
						jad->getStringValue(JDL::OUTPUTSB),
						jad->getString(JDL::OSB_BASE_DEST_URI));
				} else {
					osbdesturi = wmputilities::computeOutputSBDestURIBase(
						jad->getStringValue(JDL::OUTPUTSB), dest_uri + "/output");
				}
				
				vector<string>::iterator iter = osbdesturi.begin();
				vector<string>::iterator const end = osbdesturi.end();
				for (; iter != end; ++iter) {
					edglog(debug)<<"OutputSBDestURI: "<<*iter<<endl;
					jad->addAttribute(JDL::OSB_DEST_URI, *iter);
				}
			}
		
			// Adding attribute for perusal functionalities
			string peekdir = wmputilities::getPeekDirectoryPath(*jid);
			if (jad->hasAttribute(JDL::PU_FILE_ENABLE)) {
				if (jad->getBool(JDL::PU_FILE_ENABLE)) {
					string protocol = conf.getDefaultProtocol();
					string port = (conf.getDefaultPort() != 0)
						? (":" + boost::lexical_cast<std::string>(
							conf.getDefaultPort()))
						: "";
					string serverhost = getServerHost();
					
					edglog(debug)<<"Enabling perusal functionalities for job: "
						<<jid->toString()<<endl;
					edglog(debug)<<"Setting attribute JDLPrivate::PU_LIST_FILE_URI"
						<<endl;
					jad->setAttribute(JDLPrivate::PU_LIST_FILE_URI,
						protocol + "://" + serverhost + port + peekdir
						+ FILE_SEPARATOR + PERUSAL_FILE_2_PEEK_NAME);
					if (!jad->hasAttribute(JDL::PU_FILES_DEST_URI)) {
						edglog(debug)<<"Setting attribute JDL::PU_FILES_DEST_URI"
							<<endl;
						jad->setAttribute(JDL::PU_FILES_DEST_URI,
							protocol + "://" + serverhost + port + peekdir);
					}
					
					int time = DEFAULT_PERUSAL_TIME_INTERVAL;
					if (jad->hasAttribute(JDL::PU_TIME_INTERVAL)) {
						time = max(time, jad->getInt(JDL::PU_TIME_INTERVAL));
						jad->delAttribute(JDL::PU_TIME_INTERVAL);
					}
					time = max(time, conf.getMinPerusalTimeInterval());
					edglog(debug)<<"Setting attribute JDL::PU_TIME_INTERVAL"<<endl;
					jad->setAttribute(JDL::PU_TIME_INTERVAL, time);
				}
			}
			
			// Looking for Zipped ISB
			if (jad->hasAttribute(JDLPrivate::ZIPPED_ISB)) {
				string flagfile = wmputilities::getJobDirectoryPath(*jid)
		    			+ FILE_SEPARATOR + FLAG_FILE_UNZIP;
				if (!wmputilities::fileExists(flagfile)) {
			    	vector<string> files = jad->getStringValue(JDLPrivate::ZIPPED_ISB);
					string targetdir = getenv(DOCUMENT_ROOT);
					string jobpath = wmputilities::getInputSBDirectoryPath(*jid)
						+ FILE_SEPARATOR;
					for (unsigned int i = 0; i < files.size(); i++) {
						edglog(debug)<<"Uncompressing zip file: "<<files[i]<<endl;
						wmputilities::uncompressFile(jobpath + files[i], targetdir);
					}
			    	wmputilities::setFlagFile(flagfile, true);
			    } else {
			    	edglog(debug)<<"Flag file "<<flagfile<<" already exists. "
			    		"Skipping uncompressFile..."<<endl;
			    }
			}
			
			if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_INTERACTIVE)) {
				edglog(debug)<<"Logging listener"<<endl;
				if (wmplogger.logListener(jad->getString(JDL::SHHOST).c_str(), 
					jad->getInt(JDL::SHPORT))) {
					edglog(severe)<<"LB logging listener failed"<<endl;
	          		throw JobOperationException( __FILE__, __LINE__,
	          			"submit()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
	           			"LB logging listener failed");
				}	
			} else if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_CHECKPOINTABLE)) {
				string flagfile = wmputilities::getJobDirectoryPath(*jid)
		    		+ FILE_SEPARATOR + FLAG_FILE_LOG_CHECKPOINTABLE;
			    if (!wmputilities::fileExists(flagfile)) {
			    	string jobidstring = jid->toString();
					edglog(debug)<<"Logging checkpointable for job: "
						<<jobidstring<<endl;
					logCheckpointable(&wmplogger, jad, jobidstring);
			    	edglog(debug)<<"logCheckpointable OK, writing flag file: "
			    		<<flagfile<<endl;
			    	wmputilities::setFlagFile(flagfile, true);
			    } else {
			    	edglog(debug)<<"Flag file "<<flagfile<<" already exists. "
			    		"Skipping logCheckpointable..."<<endl;
			    }
			}
			
			jdltostart = jad->toString();
			
			delete jad;
		} else {
			WMPExpDagAd * dag = new WMPExpDagAd(jdl);
			dag->setLocalAccess(false);
			
		    JobIdStruct jobidstruct = dag->getJobIdStruct();
		    JobId parentjobid = jobidstruct.jobid;
		    vector<JobIdStruct*> children = jobidstruct.children;
		    
		    char * seqcode = wmplogger.getSequence();
		    edglog(debug)<<"Storing seqcode: "<<seqcode<<endl;
		    
		    // Setting internal attributes for sub jobs
			string jobidstring;
			string dest_uri;
			string peekdir;
			
			vector<JobIdStruct*>::iterator iter = children.begin();
			vector<JobIdStruct*>::iterator const end = children.end();
			for (; iter != end; ++iter) {
		    	JobId subjobid = (*iter)->jobid;
		    	jobidstring = subjobid.toString();
				
				dest_uri = getDestURI(jobidstring, conf.getDefaultProtocol(),
					conf.getDefaultPort());
				edglog(debug)<<"Setting internal attributes for sub job: "
					<<jobidstring<<endl;
				edglog(debug)<<"Destination URI: "<<dest_uri<<endl;
				
				NodeAd nodead = dag->getNode(subjobid);
		    	
		    	// Adding WMPISB_BASE_URI, INPUT_SANDBOX_PATH, OUTPUT_SANDBOX_PATH
				// & USERPROXY
		    	edglog(debug)<<"Setting attribute JDL::WMPISB_BASE_URI"<<endl;
				nodead.setAttribute(JDL::WMPISB_BASE_URI, dest_uri);
		        edglog(debug)<<"Setting attribute JDLPrivate::INPUT_SANDBOX_PATH"<<endl;
				nodead.setAttribute(JDLPrivate::INPUT_SANDBOX_PATH,
					getInputSBDirectoryPath(jobidstring));
		        edglog(debug)<<"Setting attribute JDLPrivate::OUTPUT_SANDBOX_PATH"<<endl; 
				nodead.setAttribute(JDLPrivate::OUTPUT_SANDBOX_PATH,
					getOutputSBDirectoryPath(jobidstring));
		        edglog(debug)<<"Setting attribute JDLPrivate::USERPROXY"<<endl;
		        nodead.setAttribute(JDLPrivate::USERPROXY,
		            getJobDelegatedProxyPath(jobidstring));
		
				// Adding OutputSandboxDestURI attribute
		        if (nodead.hasAttribute(JDL::OUTPUTSB)) {
					vector<string> osbdesturi;
					if (nodead.hasAttribute(JDL::OSB_DEST_URI)) {
						osbdesturi = wmputilities::computeOutputSBDestURI(
							nodead.getStringValue(JDL::OSB_DEST_URI),
							dest_uri);
						nodead.delAttribute(JDL::OSB_DEST_URI);
					} else if (dag->hasAttribute(JDL::OSB_BASE_DEST_URI)) {
		            	osbdesturi = wmputilities::computeOutputSBDestURIBase(
							nodead.getStringValue(JDL::OUTPUTSB),
							nodead.getStringValue(JDL::OSB_BASE_DEST_URI)[0]);
					} else {
		            	osbdesturi = wmputilities::computeOutputSBDestURIBase(
							nodead.getStringValue(JDL::OUTPUTSB),
							dest_uri + "/output");
					}
					if (osbdesturi.size() != 0) {
		                edglog(debug)<<"Setting attribute JDL::OSB_DEST_URI"<<endl;
		                vector<string>::iterator iter = osbdesturi.begin();
						vector<string>::iterator const end = osbdesturi.end();
						for (; iter != end; ++iter) {
							nodead.addAttribute(JDL::OSB_DEST_URI, *iter);
		                }
					}
				}
				
				// Adding attributes for perusal functionalities
		    	peekdir = wmputilities::getPeekDirectoryPath(subjobid);
				if (nodead.hasAttribute(JDL::PU_FILE_ENABLE)) {
					if (nodead.getBool(JDL::PU_FILE_ENABLE)) {
						string protocol = conf.getDefaultProtocol();
						string port = (conf.getDefaultPort() != 0)
							? (":" + boost::lexical_cast<std::string>(
								conf.getDefaultPort()))
							: "";
						string serverhost = getServerHost();
					
						edglog(debug)<<"Enabling perusal functionalities for job: "
							<<jobidstring<<endl;
						edglog(debug)<<"Setting attribute JDLPrivate::PU_LIST_FILE_URI"
							<<endl;
						nodead.setAttribute(JDLPrivate::PU_LIST_FILE_URI,
							protocol + "://" + serverhost + port + peekdir
							+ FILE_SEPARATOR + PERUSAL_FILE_2_PEEK_NAME);
						if (!nodead.hasAttribute(JDL::PU_FILES_DEST_URI)) {
							edglog(debug)<<"Setting attribute JDL::PU_FILES_DEST_URI"
								<<endl;
							nodead.setAttribute(JDL::PU_FILES_DEST_URI,
								protocol + "://" + serverhost + port + peekdir);
						}
						
						int time = DEFAULT_PERUSAL_TIME_INTERVAL;
						if (nodead.hasAttribute(JDL::PU_TIME_INTERVAL)) {
							time = max(time, nodead.getInt(JDL::PU_TIME_INTERVAL));
							nodead.delAttribute(JDL::PU_TIME_INTERVAL);
						}
						time = max(time, conf.getMinPerusalTimeInterval());
						edglog(debug)<<"Setting attribute JDL::PU_TIME_INTERVAL"
							<<endl;
						nodead.setAttribute(JDL::PU_TIME_INTERVAL, time);
					}
				}
				
				dag->replaceNode(subjobid, nodead);
				
				if (nodead.hasAttribute(JDL::JOBTYPE)) {
		    		string type = nodead.getStringValue(JDL::JOBTYPE)[0];
		    		if (type == JDL_JOBTYPE_CHECKPOINTABLE) {
		    			edglog(debug)<<"Logging checkpointable for subjob: "
		    				<<jobidstring<<endl;
		    			// TBC seqcode to use
		    			wmplogger.setLoggingJob(jobidstring, seqcode);
		    			logCheckpointable(&wmplogger, &nodead, jobidstring);
		    		}
		    	}
			}
			
			//TBD Change getSubmissionStrings() with a better method when coded??
			//Done here, Not done during register any more.
		    dag->getSubmissionStrings();
		    
		    wmplogger.setLoggingJob(parentjobid.toString(), seqcode);
		    
		    //Registering subjobs
		    vector<string> jobids = wmplogger.generateSubjobsIds(dag->size()); //Filling wmplogger subjobs
		    
		    string flagfile = wmputilities::getJobDirectoryPath(*jid)
		    	+ FILE_SEPARATOR + FLAG_FILE_REGISTER_SUBJOBS;
		    if (!wmputilities::fileExists(flagfile)) {
		    	wmplogger.registerSubJobs(dag, wmplogger.subjobs);
		    	edglog(debug)<<"registerSubJobs OK, writing flag file: "<<flagfile<<endl;
		    	wmputilities::setFlagFile(flagfile, true);
		    } else {
		    	edglog(debug)<<"Flag file "<<flagfile<<" already exists. "
		    		"Skipping registerSubJobs..."<<endl;
		    }
		    
			// Looking for Zipped ISB
			if (dag->hasAttribute(JDLPrivate::ZIPPED_ISB)) {
				//Setting file system for subjobs
				setSubjobFileSystem(auth, parentjobid.toString(), jobids);
				
				flagfile = wmputilities::getJobDirectoryPath(*jid) + FILE_SEPARATOR
					+ FLAG_FILE_UNZIP;
				if (!wmputilities::fileExists(flagfile)) {
					//Uncompressing zip file
					vector<string> files = dag->getAttribute(ExpDagAd::ZIPPED_ISB);
					string targetdir = getenv(DOCUMENT_ROOT);
					string jobpath = wmputilities::getInputSBDirectoryPath(*jid)
						+ FILE_SEPARATOR;
					for (unsigned int i = 0; i < files.size(); i++) {
						edglog(debug)<<"Uncompressing zip file: "<<files[i]<<endl;
						wmputilities::uncompressFile(jobpath + files[i], targetdir);
					}
					wmputilities::setFlagFile(flagfile, true);
				} else {
					edglog(debug)<<"Flag file "<<flagfile<<" already exists. "
		    		"Skipping uncompressFile..."<<endl;
				}
		    }
			
			jdltostart = dag->toString();
			
			delete dag;
		}
		
		//TBD Use this line to save jdl written to filelist in a different path
		// e.g. method getJobFileListJDLPath
		//wmputilities::writeTextFile(wmputilities::getJobJDLToStartPath(*jid),
		//	jdltostart);
		
		// \/ To test only, raising an exception
		/*
		string flagfile = wmputilities::getJobDirectoryPath(*jid) + FILE_SEPARATOR
			+ "PROVAFILE";
			
		if (!wmputilities::fileExists(flagfile)) {
			wmputilities::setFlagFile(flagfile, true);
	
			throw AuthenticationException(__FILE__, __LINE__,
				"submit()", wmputilities::WMS_AUTHENTICATION_ERROR,
				"Unable to set User Proxy for LB context");
		}
		*/
		// /\
		
		wmpmanager::WMPManager manager(&wmplogger);
		// Vector of parameters to runCommand()
		vector<string> params;
		params.push_back(jdltostart);
		params.push_back(proxy);
		params.push_back(wmputilities::getJobDirectoryPath(*jid));
		params.push_back(string(getenv(DOCUMENT_ROOT)));
		
		wmp_fault_t wmp_fault = manager.runCommand("JobSubmit", params);
		
		if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
			edglog(severe)<<"Error in runCommand: "<<wmp_fault.message<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"submit()", wmp_fault.code, wmp_fault.message);
		}
		
		/*for (backupenv; *backupenv; backupenv++) {
		    free(*backupenv);
	    }
	    free(*targetEnv);*/
	    
	    if (!issubmit) {
	    	edglog(debug)<<"Removing lock..."<<std::endl;
		    flock(fd, LOCK_UN);
		    close(fd);
	    }

	} catch (Exception &exc) {
		edglog(debug)<<"Logging LOG_ENQUEUE_FAIL"<<std::endl;
		wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_FAIL,
			"LOG_ENQUEUE_FAIL", true, true, filelist_global.c_str(),
			wmputilities::getJobJDLToStartPath(*jid).c_str());
		
		edglog(debug)<<"Removing lock..."<<std::endl;
		flock(fd, LOCK_UN);
	    close(fd);
		
		if (!conf.getAsyncJobStart()) {
			exc.push_back(__FILE__, __LINE__, "submit");
			throw exc;
		}
	} catch (exception &ex) {
		edglog(debug)<<"Logging LOG_ENQUEUE_FAIL"<<std::endl;
		wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_FAIL,
			"LOG_ENQUEUE_FAIL", true, true, filelist_global.c_str(),
			wmputilities::getJobJDLToStartPath(*jid).c_str());
		
		edglog(debug)<<"Removing lock..."<<std::endl;
		flock(fd, LOCK_UN);
	    close(fd);
	    
		if (!conf.getAsyncJobStart()) {
			Exception exc(__FILE__, __LINE__, "submit", 0, "Standard exception: " 
				+ std::string(ex.what())); 
			throw exc;
		}
	}
}


SOAP_FMAC5 int SOAP_FMAC6 
soap_serve_submit(struct soap *soap, struct ns1__jobSubmitResponse response)
{
	//struct ns1__jobSubmitResponse response;
	soap_serializeheader(soap);
	soap_serialize_ns1__jobSubmitResponse(soap, &response);
	if (soap_begin_count(soap)) {
		return soap->error;
	}
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__jobSubmitResponse(soap, &response,
		 	"ns1:jobSubmitResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap)) {
			 return soap->error;
		 }
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns1__jobSubmitResponse(soap, &response,
	 	"ns1:jobSubmitResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap)) {
		return soap->error;
	 }
	return soap_closesock(soap);
}

void
jobSubmit(struct ns1__jobSubmitResponse &response,
	jobSubmitResponse &jobSubmit_response, const string &jdl,
	const string &delegation_id, struct soap *soap)
{
	GLITE_STACK_TRY("jobSubmit()");
	edglog_fn("wmpoperations::jobSubmit");
	logRemoteHostInfo();
	
	// Checking delegation id
	edglog(debug)<<"Delegation ID: "<<delegation_id<<endl;
	if (delegation_id == "") {
		edglog(error)<<"Provided delegation ID not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"jobRegister()", wmputilities::WMS_DELEGATION_ERROR,
			"Delegation id not valid");
	}
	
	edglog(debug)<<"JDL to Submit:\n"<<jdl<<endl;
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = WMPDelegation::getDelegatedProxyPath(delegation_id);
	edglog(debug)<<"Delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN());
	} else {
		auth->authorize();
	}
	
	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"
			<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::jobRegister()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	
	// Checking proxy validity
	authorizer::WMPAuthorizer::checkProxy(delegatedproxy);
	
	// Registering the job for submission
	jobRegisterResponse jobRegister_response;
	pair<string, string> reginfo = jobregister(jobRegister_response, jdl,
		delegation_id, delegatedproxy, wmputilities::getEnvVO(), auth);
		
	/*if (auth) {
		delete auth;	
	}*/
	
	// Getting job identifier from register response
	//string jobid = jobRegister_response.jobIdStruct->id;
	string jobid = reginfo.first;
	edglog(debug)<<"Starting registered job: "<<jobid<<endl;

	// Starting job submission
	JobId *jid = new JobId(jobid);
	
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	//WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	
	// Getting delegated proxy inside job directory
	string proxy(wmputilities::getJobDelegatedProxyPath(*jid));
	edglog(debug)<<"Job delegated proxy: "<<proxy<<endl;
	
	// Setting user proxy
	if (wmplogger.setUserProxy(proxy)) {
		edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
		throw AuthenticationException(__FILE__, __LINE__,
			"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
			"Unable to set User Proxy for LB context");
	}
	
	jobSubmit_response.jobIdStruct = jobRegister_response.jobIdStruct;
	
	// Filling answer structure to return to user
	ns1__JobIdStructType *job_id_struct = new ns1__JobIdStructType();
	job_id_struct->id = jobSubmit_response.jobIdStruct->id;
	job_id_struct->name = jobSubmit_response.jobIdStruct->name;
	if (jobSubmit_response.jobIdStruct->childrenJob) {
		job_id_struct->childrenJob =
			*convertToGSOAPJobIdStructTypeVector(jobSubmit_response
			.jobIdStruct->childrenJob);
	} else {
		job_id_struct->childrenJob = *(new vector<ns1__JobIdStructType*>);
	}
	response._jobIdStruct = job_id_struct;
	
	if (conf.getAsyncJobStart()) {
		// Creating SOAP answer
		if (soap_serve_submit(soap, response)
				|| (soap->fserveloop && soap->fserveloop(soap))) {
			// if ERROR throw exception??
			soap_send_fault(soap);
		}
	}
	
	submit(reginfo.second, jid, auth, wmplogger, true);
	
	if (auth) {
		delete auth;	
	}
	delete jid;

	GLITE_STACK_CATCH();
}

void
jobCancel(jobCancelResponse &jobCancel_response, const string &job_id)
{
	GLITE_STACK_TRY("jobCancel()");
	edglog_fn("wmpoperations::jobCancel");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;
	
	string jobpath = wmputilities::getJobDirectoryPath(*jid);
  
	// Checking proxy validity
	//authorizer::WMPAuthorizer::checkProxy(delegatedproxy);
	
	// Getting job status to check if cancellation is possible
	JobStatus status = WMPEventLogger::getStatus(jid, delegatedproxy);
	
	// Getting type from jdl
	JobId * parentjid = new JobId(status.getValJobId(JobStatus::PARENT_JOB));
	if (((JobId) status.getValJobId(JobStatus::PARENT_JOB)).isSet()) {
		/*
		WMPEventLogger wmplogger(wmputilities::getEndpoint());
		std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
		wmplogger.init(lbaddress_port.first, lbaddress_port.second, parentjid,
			conf.getDefaultProtocol(), conf.getDefaultPort());
		wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
		
		// Setting user proxy
		if (wmplogger.setUserProxy(delegatedproxy)) {
			edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
			throw AuthenticationException(__FILE__, __LINE__,
				"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
				"Unable to set User Proxy for LB context");
		}
		string parentjdl = wmplogger.getUserTag(WMPEventLogger::QUERY_JDL_ORIGINAL);
		*/
		
		string parentjdl = wmputilities::readTextFile(
			wmputilities::getJobJDLToStartPath(*parentjid));
		
		Ad * parentad = new Ad();
		int type = getType(parentjdl, parentad);
		if ((type != TYPE_COLLECTION)
				&& !parentad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_PARAMETRIC)) {
			string msg = "the job is a DAG subjob. The parent is: "
				+ status.getValJobId(JobStatus::PARENT_JOB).toString();
			edglog(error)<<msg<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"jobCancel()", wmputilities::WMS_OPERATION_NOT_ALLOWED, msg);
		}
		delete parentad;
	}
	
	// Getting sequence code from jdl
	//Ad *ad = new Ad(status.getValString(JobStatus::JDL));
	Ad *ad = new Ad();
	ad->fromFile(wmputilities::getJobJDLToStartPath(*jid));
	string seqcode = "";
	if (ad->hasAttribute(JDL::LB_SEQUENCE_CODE)) {
		seqcode = ad->getStringValue(JDL::LB_SEQUENCE_CODE)[0];
	}
	delete ad;
	
	if (status.getValBool(JobStatus::CANCELLING)) {
		edglog(error)<<"Cancel has already been requested"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobCancel()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"Cancel has already been requested");
	}
	
	// Initializing logger
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	//WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	
	// Setting user proxy
	if (wmplogger.setUserProxy(delegatedproxy)) {
		edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
		throw AuthenticationException(__FILE__, __LINE__,
			"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
			"Unable to set User Proxy for LB context");
	}
	
	// Vector of parameters to pass to runCommand()
	vector<string> params;
	
	// TBC
	wmpmanager::WMPManager manager(&wmplogger);
	wmp_fault_t wmp_fault;
	string envsandboxpath;
	
	switch (status.status) {
		case JobStatus::SUBMITTED: //TBD check this state with call to LBProxy (use events)
			// The register of the job has been done
			
			//wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ABORT,
			//	"Cancelled by user", true, true);
			edglog(debug)<<"Trying to log sync ABORT..."<<endl;
			wmplogger.setSequenceCode(ABORT_SEQ_CODE);
			if (!wmplogger.logAbortEventSync("Cancelled by user")) {
				// If log fails no purge is done
				// purge would find state different from ABORT and will fail
				edglog(debug)<<"Log has succeded, calling purge method..."<<endl;
				jobPurgeResponse jobPurge_response;
				jobpurge(jobPurge_response, jid, false);
			} else {
				edglog(debug)<<"Log has failed, purge method will not be called"<<endl;
			}
			
			//TBC should I check if MyProxyServer was present in jdl??
			edglog(debug)<<"Unregistering Proxy renewal..."<<endl;
			wmplogger.unregisterProxyRenewal();
			break;
		case JobStatus::WAITING:
		case JobStatus::READY:
		case JobStatus::SCHEDULED:
		case JobStatus::RUNNING:
			params.push_back(jid->toString());
			params.push_back(jobpath);
			params.push_back(string(getenv(DOCUMENT_ROOT)));
			params.push_back(seqcode);
			
			wmp_fault = manager.runCommand("JobCancel", params,
				&jobCancel_response);
			
			if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
				edglog(severe)<<"Error in runCommand: " + wmp_fault.message<<endl;
				throw JobOperationException(__FILE__, __LINE__,
					"jobCancel()", wmp_fault.code, wmp_fault.message);
			}
			wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_CANCEL,
				"Cancelled by user", true, true);
			break;
		case JobStatus::DONE:
			// If the job is DONE, then cancellation is allowed only if
			// DONE_CODE = DONE_CODE_FAILED (resubmission is possible)
			if (status.getValInt(JobStatus::DONE_CODE)
					== JobStatus::DONE_CODE_FAILED) {
				params.push_back(jid->toString());
				params.push_back(jobpath);
				params.push_back(string(getenv(DOCUMENT_ROOT)));
				params.push_back(seqcode);
				
				wmp_fault = manager.runCommand("JobCancel", params,
					&jobCancel_response);
				
				if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
					edglog(severe)<<"Error in runCommand: " + wmp_fault.message<<endl;
					throw JobOperationException(__FILE__, __LINE__,
						"jobCancel()", wmp_fault.code, wmp_fault.message);
				}
				wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_CANCEL,
					"Cancelled by user", true, true);
			} else {
				edglog(debug)<<"Job status (DONE - DONE_CODE != DONE_CODE_FAILED)"
					" doesn't allow job cancel"<<endl;
				throw JobOperationException(__FILE__, __LINE__,
					"jobCancel()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
					"job status (DONE) doesn't allow job cancel");
			}
			break;
		default: // Any other value: CLEARED, ABORTED, CANCELLED, PURGED
			edglog(debug)<<"Job status doesn't allow job cancel"<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"jobCancel()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"job status doesn't allow job cancel");
			break;
  	}
  	
  	delete jid;

 	GLITE_STACK_CATCH();
}

void
getMaxInputSandboxSize(getMaxInputSandboxSizeResponse
	&getMaxInputSandboxSize_response)
{
	GLITE_STACK_TRY("getMaxInputSandboxSize()");
	edglog_fn("wmpoperations::getMaxInputSandboxSize");
	logRemoteHostInfo();

	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	try {
		getMaxInputSandboxSize_response.size =
		// WARNING: Temporal cast TBD
		// WARNING: double temporarely casted into long (soon long will be returned directly
		(long)singleton_default<WMProxyConfiguration>::instance()
			.wmp_config->max_input_sandbox_size();
	} catch (exception &ex) {
		edglog(severe)<<"Unable to get max input sandbox size: "
			<<ex.what()<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"getMaxInputSandboxSize()", wmputilities::WMS_IS_FAILURE,
			"Unable to get max input sandbox size");
	}

	GLITE_STACK_CATCH();
}

void
getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response,
	const string &jid)
{
	GLITE_STACK_TRY("getSandboxDestURI()");
	edglog_fn("wmpoperations::getSandboxDestURI");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	

	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, jid);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), jid);
	} else {
		auth->authorize("", jid);
	}
	delete auth;
	
	getSandboxDestURI_response.path = new StringList;
	getSandboxDestURI_response.path->Item = new vector<string>(0);
	
	//WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	//vector<pair<std::string, int> > protocols = conf.getProtocols();
	//int httpsport = conf.getHTTPSPort();
	
	vector<string> *uris = wmputilities::getDestURIsVector(conf.getProtocols(),
		conf.getHTTPSPort(), jid);
	getSandboxDestURI_response.path->Item = uris;
	
	GLITE_STACK_CATCH();
}

void
getSandboxBulkDestURI(getSandboxBulkDestURIResponse &getSandboxBulkDestURI_response,
	const string &jid)
{
	GLITE_STACK_TRY("getSandboxBulkDestURI()");
	edglog_fn("wmpoperations::getSandboxBulkDestURI");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, jid);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), jid);
	} else {
		auth->authorize("", jid);
	}
	delete auth;
	
	// Initializing logger
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	//WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jobid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	
	// Setting user proxy
	if (wmplogger.setUserProxy(delegatedproxy)) {
		edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
		throw AuthenticationException(__FILE__, __LINE__,
			"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
			"Unable to set User Proxy for LB context");
	}
	
	//WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	//vector<pair<std::string, int> > protocols = conf.getProtocols();
	//int httpsport = conf.getHTTPSPort();
	
	DestURIsStructType *destURIsStruct = new DestURIsStructType();
	destURIsStruct->Item = new vector<DestURIStructType*>(0);
	
	// Checking proxy validity
	//authorizer::WMPAuthorizer::checkProxy(delegatedproxy);
	
	// Getting job status to check if cancellation is possible
	JobStatus status = WMPEventLogger::getStatus(jobid, delegatedproxy, true);
	vector<string> jids = status.getValStringList(JobStatus::CHILDREN);
	edglog(debug)<<"Children count: "<<status.getValInt(JobStatus::CHILDREN_NUM)<<endl;
	vector<string>::iterator jidsiterator = jids.begin();
 	jids.insert(jidsiterator, 1, jid);

	vector<string>::iterator iter = jids.begin();
	vector<string>::iterator const end = jids.end();
	for (; iter != end; ++iter) { 	
		vector<string> *uris =
			wmputilities::getDestURIsVector(conf.getProtocols(),
				conf.getHTTPSPort(), *iter);
		
		DestURIStructType *destURIStruct = new DestURIStructType();
		destURIStruct->id = *iter;
		destURIStruct->destURIs = uris;
		destURIsStruct->Item->push_back(destURIStruct);
	}
	getSandboxBulkDestURI_response.destURIsStruct = destURIsStruct;
	
	GLITE_STACK_CATCH();
}

void
getQuota(getQuotaResponse &getQuota_response)
{
	GLITE_STACK_TRY("getQuota()");
	edglog_fn("wmpoperations::getQuota");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize(wmputilities::getEnvFQAN());
	edglog(debug)<<"User Name: "<<auth->getUserName()<<endl;
	
	pair<long, long> quotas;
	if (!wmputilities::getUserQuota(quotas, auth->getUserName())) {
		edglog(severe)<<"Unable to get total quota"<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"getQuota()", wmputilities::WMS_IS_FAILURE,
			"Unable to get total quota");
	}
	delete auth;
	getQuota_response.softLimit = quotas.first;
	getQuota_response.hardLimit = quotas.second;
	
	GLITE_STACK_CATCH();
}

void
getFreeQuota(getFreeQuotaResponse &getFreeQuota_response)
{
	GLITE_STACK_TRY("getFreeQuota()");
	edglog_fn("wmpoperations::getFreeQuota");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize(wmputilities::getEnvFQAN());
	edglog(debug)<<"User Name: "<<auth->getUserName()<<endl;
	
	pair<long, long> quotas;
	if (!wmputilities::getUserFreeQuota(quotas, auth->getUserName())) {
		edglog(severe)<<"Unable to get free quota"<<endl;
		throw FileSystemException(__FILE__, __LINE__, "getFreeQuota()",
			wmputilities::WMS_IS_FAILURE, "Unable to get free quota");
	}
	delete auth;
	getFreeQuota_response.softLimit = quotas.first;
	getFreeQuota_response.hardLimit = quotas.second;
	
	GLITE_STACK_CATCH();
}

void
jobpurge(jobPurgeResponse &jobPurge_response, JobId *jobid, bool checkstate)
{
	GLITE_STACK_TRY("jobpurge()");
	edglog_fn("wmpoperations::jobpurge");
	
	edglog(debug)<<"CheckState: "<<(checkstate ? "True" : "False")<<endl;
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	// Checking proxy validity
	//authorizer::WMPAuthorizer::checkProxy(delegatedproxy);
	
	// Getting job status to check if purge is possible
	JobStatus status = WMPEventLogger::getStatus(jobid, delegatedproxy);
	
	if (((JobId) status.getValJobId(JobStatus::PARENT_JOB)).isSet()) {
		string msg = "the job is a DAG subjob. The parent is: "
			+ status.getValJobId(JobStatus::PARENT_JOB).toString();
		edglog(error)<<msg<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobpurge()", wmputilities::WMS_OPERATION_NOT_ALLOWED, msg);
	}
	
	// Purge allowed if the job is in ABORTED state or DONE success
	// DONE_COE = DONE_CODE_OK  or DONE_CODE_CANCELLED
	int donecode = status.getValInt(JobStatus::DONE_CODE);
	if (!checkstate
			|| ((status.status == JobStatus::ABORTED)
			|| ((status.status == JobStatus::DONE)
				&& ((donecode == JobStatus::DONE_CODE_OK)
					|| (donecode == JobStatus::DONE_CODE_CANCELLED))))) {
		
		string usercert;
		string userkey;
		bool isproxyfile = false;
		try {
			authorizer::WMPAuthorizer::checkProxy(delegatedproxy);
			
			// Creating temporary Proxy file
			char time_string[20];
			utilities::oedgstrstream s;
			struct timeval tv;
			struct tm* ptm;
			long milliseconds;
			gettimeofday(&tv, NULL);                                                                    
			ptm = localtime(&tv.tv_sec);                                                                                            
			strftime(time_string, sizeof (time_string), "%Y%m%d%H%M%S", ptm); 
		 	milliseconds = tv.tv_usec / 1000;
			s<<"/tmp/"<<std::string(time_string)<<milliseconds<<".pproxy";
			string tempproxy = s.str();
			
			wmputilities::fileCopy(delegatedproxy, tempproxy);
			
			usercert = tempproxy;
			userkey = tempproxy;
			
			isproxyfile = true;
		} catch (Exception &ex) {
			if (ex.getCode() == wmputilities::WMS_PROXY_EXPIRED) {
				if (!getenv(GLITE_HOST_CERT) || ! getenv(GLITE_HOST_KEY)) {
					edglog(severe)<<"Unable to get values for environment variable "
						<<string(GLITE_HOST_CERT)<<" and/or "<<string(GLITE_HOST_KEY)<<endl;
					throw FileSystemException(__FILE__, __LINE__,
						"jobpurge()", wmputilities::WMS_IS_FAILURE,
						"Unable to perform job purge. Server error\n(please "
						"contact server administrator)");
				} else {
					edglog(debug)<<"Reading user cert and user key from "
						"environment variables GLITE_HOST_CERT and "
						"GLITE_HOST_KEY"<<endl;
					usercert = string(getenv(GLITE_HOST_CERT));
					userkey = string(getenv(GLITE_HOST_KEY));
				}
			} else {
				throw ex;
			}
		}
		
		edglog(debug)<<"User cert: "<<usercert<<endl;
		edglog(debug)<<"User key: "<<userkey<<endl;
					
		setenv(X509_USER_CERT, usercert.c_str(), 1);
		setenv(X509_USER_KEY, userkey.c_str(), 1);
		
		if (!wmputilities::doPurge(jobid->toString())) {
			edglog(severe)<<"Unable to complete job purge"<<endl;
			if (checkstate) {
				throw FileSystemException(__FILE__, __LINE__,
					"jobpurge()", wmputilities::WMS_IS_FAILURE,
					"Unable to complete job purge");
			}
		}
		
		unsetenv(X509_USER_CERT);
		unsetenv(X509_USER_KEY);
		
		// Removing temporary Proxy file
		if (isproxyfile) {
	    	remove(usercert.c_str());
		}
		
	} else {
		edglog(error)<<"Job current status doesn't allow purge operation"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobpurge()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"Job current status doesn't allow purge operation");
	}
	
	GLITE_STACK_CATCH();
}

void
jobPurge(jobPurgeResponse &jobPurge_response, const string &jid)
{
	GLITE_STACK_TRY("jobPurge()");
	edglog_fn("wmpoperations::jobPurge");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;

	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, jid);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), jid);
	} else {
		auth->authorize("", jid);
	}
	delete auth;
	
	jobpurge(jobPurge_response, jobid);
	
	delete jobid;
	
	edglog(info) << "Job purged successfully" << endl;
	GLITE_STACK_CATCH();
}

void
getOutputFileList(getOutputFileListResponse &getOutputFileList_response,
	const string &jid)
{
	GLITE_STACK_TRY("getOutputFileList()");
	edglog_fn("wmpoperations::getOutputFileList");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, jid);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), jid);
	} else {
		auth->authorize("", jid);
	}
	delete auth;
	
	string jobdirectory = wmputilities::getJobDirectoryPath(*jobid);

	delete jobid;

	// Checking for maradona file, created if and only if the job is in DONE state
	edglog(debug)<<"Searching for file: "<<jobdirectory + FILE_SEPARATOR
		+ MARADONA_FILE<<endl;
	if (!wmputilities::fileExists(jobdirectory + FILE_SEPARATOR + MARADONA_FILE)) {
		// Getting job status to check if is a job and is done success
		JobStatus status = WMPEventLogger::getStatus(jobid, delegatedproxy);
	
		if (status.getValInt(JobStatus::CHILDREN_NUM) != 0) {
			string msg = "getOutputFileList operation not allowed for dag or "
				"collection type";
			edglog(error)<<msg<<": "<<jobid<<endl;
			throw JobOperationException(__FILE__, __LINE__, "getOutputFileList()", 
				wmputilities::WMS_OPERATION_NOT_ALLOWED, msg);
		}
		
		if ((status.status == JobStatus::DONE)
				&& (status.getValInt(JobStatus::DONE_CODE)
					== JobStatus::DONE_CODE_OK)) {
			edglog(error)<<
				"Job current status doesn't allow getOutputFileList operation"<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"getOutputFileList()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"Job current status doesn't allow getOutputFileList operation");
		}
	}
		
	string output_uri = wmputilities::getDestURI(jid, conf.getDefaultProtocol(),
		conf.getDefaultPort()) + FILE_SEPARATOR + "output";
	string outputpath = wmputilities::getOutputSBDirectoryPath(jid);
	edglog(debug)<<"Output URI: " << output_uri <<endl;
	
	// Searching files inside directory
	const boost::filesystem::path p(outputpath, boost::filesystem::native);
	std::vector<std::string> found;
	glite::wms::wmproxy::commands::list_files(p, found);
	edglog(debug)<<"List size is (hidden files included): "
		<<found.size()<<endl;
	
	// Creating the list
	StringAndLongList *list = new StringAndLongList();
	vector<StringAndLongType*> *file = new vector<StringAndLongType*>;
	StringAndLongType *item = NULL;
	string filename;
	
	vector<string>::iterator iter = found.begin();
	vector<string>::iterator const end = found.end();
	for (; iter != end; ++iter) {
		// Checking for hidden files
		filename = wmputilities::getFileName(string(*iter));
		if ((filename != MARADONA_FILE) && (filename
				!= authorizer::GaclManager::WMPGACL_DEFAULT_FILE)) {
			item = new StringAndLongType();
			item->name = output_uri + FILE_SEPARATOR + filename;
			item->size = wmputilities::computeFileSize(*iter);
			edglog(debug)<<"Inserting file name: " <<item->name<<endl;
			edglog(debug)<<"Inserting file size: " <<item->size<<endl;
			file->push_back(item);
		}
	}
	list->file = file;
	getOutputFileList_response.OutputFileAndSizeList = list;
	
	edglog(info)<<"Successfully retrieved files: "<<found.size()<<endl;
	
	GLITE_STACK_CATCH();
}

int 
listmatch(jobListMatchResponse &jobListMatch_response, const string &jdl,
	const string &delegation_id)
{
	GLITE_STACK_TRY("listmatch");
	edglog_fn("wmpoperations::listmatch");
	
	int result = 0;
	
	//Ad * ad = new Ad();
	//int type = getType(jdl, ad);
	int type = getType(jdl);
	
	if (type == TYPE_JOB) {
		JobAd *ad = new JobAd(jdl);
		ad->setLocalAccess(false);
	
		// Getting delegated proxy from SSL Proxy cache
		string delegatedproxy = WMPDelegation::getDelegatedProxyPath(delegation_id);
		edglog(debug)<<"Delegated proxy: "<<delegatedproxy<<endl;
	
		// Setting VIRTUAL_ORGANISATION attribute
		if (!ad->hasAttribute(JDL::VIRTUAL_ORGANISATION)) {
			ad->setAttribute(JDL::VIRTUAL_ORGANISATION, wmputilities::getEnvVO());
		}
		//ad->check();
		if (ad->hasAttribute(JDL::CERT_SUBJ)) {
			ad->delAttribute(JDL::CERT_SUBJ);
		}
		ad->setAttribute(JDL::CERT_SUBJ, 
				wmputilities::convertDNEMailAddress(wmputilities::getUserDN()));
		
		//TBD do a check() before toString() for instance to add DefaultRank
		//attribute????
		
		//WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
		WMPEventLogger wmplogger(wmputilities::getEndpoint());
		wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
		
		vector<string> params;
		wmpmanager::WMPManager manager(&wmplogger);
		params.push_back(ad->toString());
		params.push_back(singleton_default<WMProxyConfiguration>::instance()
				.getListMatchRootPath());
		params.push_back(delegatedproxy);
		
		delete ad;
		
		wmp_fault_t wmp_fault = manager.runCommand("ListJobMatch", params,
			&jobListMatch_response);
		if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
			edglog(severe)<<"Error in runCommand: "<<wmp_fault.message<<endl;
			throw JobOperationException(__FILE__, __LINE__, "listmatch()",
				wmp_fault.code, wmp_fault.message);
		}
		result = jobListMatch_response.CEIdAndRankList->file->size(); 
	} else {
		edglog(error)<<"Operation permitted only for normal job"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"listmatch()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"Operation permitted only for normal job");
	}

	return result;
	GLITE_STACK_CATCH();
}

void
jobListMatch(jobListMatchResponse &jobListMatch_response, const string &jdl,
	const string &delegation_id)
{
	GLITE_STACK_TRY("jobListMatch(jobListMatchResponse &jobListMatch_response, "
		"const string &jdl, const string &delegation_id)");
	edglog_fn("wmpoperations::jobListMatch");
	logRemoteHostInfo();
	
	// Checking delegation id
	edglog(debug)<<"Delegation ID: "<<delegation_id<<endl;
	if (delegation_id == "") {
		edglog(error)<<"Provided delegation ID not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"jobListMatch()", wmputilities::WMS_DELEGATION_ERROR,
			"Delegation id not valid");
	}
	
	edglog(debug)<<"JDL to find Match:\n"<<jdl<<endl;
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy from SSL Proxy cache
	string delegatedproxy = WMPDelegation::getDelegatedProxyPath(delegation_id);
	edglog(debug)<<"Delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN());
	} else {
		auth->authorize();
	}
	delete auth;
	
	// Checking proxy validity
	authorizer::WMPAuthorizer::checkProxy(delegatedproxy);
	
	listmatch(jobListMatch_response, jdl, delegation_id);
	
	GLITE_STACK_CATCH();
}
	
void
getJobTemplate(getJobTemplateResponse &getJobTemplate_response,
	JobTypeList jobType, const string &executable,
	const string &arguments, const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getJobTemplate()");
	edglog_fn("wmpoperations::getJobTemplate");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	getJobTemplate_response.jdl =
		(AdConverter::createJobTemplate(convertJobTypeListToInt(jobType),
		executable, arguments, requirements, rank))->toString();
		
	edglog(info)<<"Job Template retrieved successfully"<<endl;
	GLITE_STACK_CATCH();
}

void
getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response,
	GraphStructType dependencies, const string &requirements,
	const string &rank)
{
	GLITE_STACK_TRY("getDAGTemplate()");
	edglog_fn("wmpoperations::getDAGTemplate");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	getDAGTemplate_response.jdl = AdConverter::createDAGTemplate(
		convertGraphStructTypeToNodeStruct(dependencies),
		requirements, rank)->toString();
		
	edglog(info)<<"DAG Template retrieved successfully"<<endl;
	GLITE_STACK_CATCH();
}

void
getCollectionTemplate(getCollectionTemplateResponse
	&getCollectionTemplate_response, int jobNumber,
	const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getCollectionTemplate()");
	edglog_fn("wmpoperations::getCollectionTemplate");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;

	getCollectionTemplate_response.jdl =
		AdConverter::createCollectionTemplate(jobNumber, requirements,
			rank)->toString();
			
	edglog(info)<<"Collection Template retrieved successfully"<<endl;
	GLITE_STACK_CATCH();
}

void
getIntParametricJobTemplate(getIntParametricJobTemplateResponse 
	&getIntParametricJobTemplate_response, StringList *attributes, int param,
	int parameterStart, int parameterStep, const string &requirements,
	const string &rank)
{
	GLITE_STACK_TRY("getIntParametricJobTemplate()");
	edglog_fn("wmpoperations::getIntParametricJobTemplate");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	getIntParametricJobTemplate_response.jdl =
		AdConverter::createIntParametricTemplate(*(attributes->Item), param,
			parameterStart, parameterStep, requirements, rank)->toString();
			
	edglog(info)<<"Int Parametric Job Template retrieved successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
getStringParametricJobTemplate(getStringParametricJobTemplateResponse
	&getStringParametricJobTemplate_response, StringList *attributes,
	StringList *param, const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getStringParametricJobTemplate()");
	edglog_fn("wmpoperations::getStringParametricJobTemplate");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	getStringParametricJobTemplate_response.jdl =
		AdConverter::createStringParametricTemplate(*(attributes->Item),
		*(param->Item), requirements, rank)->toString();
		
	edglog(info)<<"String Parametric Job Template retrieved successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
getProxyReq(getProxyReqResponse &getProxyReq_response,
	const string &delegation_id)
{
	GLITE_STACK_TRY("getProxyReq()");
	edglog_fn("wmpoperations::getProxyReq");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	if (delegation_id == "") {
		edglog(error)<<"Provided delegation id not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"getProxyReq()", wmputilities::WMS_DELEGATION_ERROR,
			"Provided delegation id not valid");
	}
	
	getProxyReq_response.request =
		WMPDelegation::getProxyRequest(delegation_id);
	edglog(info)<<"Proxy requested successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
putProxy(putProxyResponse &putProxyReq_response, const string &delegation_id,
	const string &proxy)
{
	GLITE_STACK_TRY("putProxy()");
	edglog_fn("wmpoperations::putProxy");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	if (delegation_id == "") {
		edglog(error)<<"Provided delegation id not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"putProxy()", wmputilities::WMS_DELEGATION_ERROR,
			"Provided delegation id not valid");
	}
	
	WMPDelegation::putProxy(delegation_id, proxy);
	edglog(info)<<"Proxy put successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

vector<string>
getACLItems(getACLItemsResponse &getACLItems_response, const string &job_id)
{
	GLITE_STACK_TRY("getACLItems()");
	edglog_fn("wmpoperations::getACLItems");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"
			<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::addACLItems()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	string jobpath = wmputilities::getJobDirectoryPath(*jid);
	
	edglog(debug)<<"GACL File: "<<jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE<<endl;
	authorizer::GaclManager gaclmanager(jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE);
	vector<string> returnvector =
		gaclmanager.getItems(authorizer::GaclManager::WMPGACL_PERSON_TYPE);
	
	edglog(info)<<"getACLItems successfully"<<endl;
	
	return returnvector;
	
	GLITE_STACK_CATCH();
}

void
addACLItems(addACLItemsResponse &addACLItems_response, const string &job_id,
	StringList * dnlist)
{
	GLITE_STACK_TRY("addACLItems()");
	edglog_fn("wmpoperations::addACLItems");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"
			<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::addACLItems()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	string jobpath = wmputilities::getJobDirectoryPath(*jid);

	edglog(debug)<<"GACL File: "<<jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE<<endl;
	authorizer::GaclManager gaclmanager(jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE);
		
	vector<pair<authorizer::GaclManager::WMPgaclCredType, string> > gaclvect;
	pair<authorizer::GaclManager::WMPgaclCredType, string> gaclpair;
	for (unsigned int i = 0; i < dnlist->Item->size(); i++) {
		gaclpair.first = authorizer::GaclManager::WMPGACL_PERSON_TYPE;
		edglog(info)<<"Item to add: "<<(*(dnlist->Item))[i]<<endl;
		gaclpair.second = (*(dnlist->Item))[i];
		gaclvect.push_back(gaclpair);
	}
	gaclmanager.addEntries(gaclvect);
	gaclmanager.saveGacl();
	
	edglog(info)<<"addACLItems successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
removeACLItem(removeACLItemResponse &removeACLItem_response,
	const string &job_id, const string &item)
{
	GLITE_STACK_TRY("removeACLItem()");
	edglog_fn("wmpoperations::removeACLItem");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::removeACLItem()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	// TBD change test in: item == owner
	if (item == wmputilities::getUserDN()) {
		edglog(error)<<"Removal of the item representing user that has "
			"registered the job is not allowed"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"removeACLItem()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"Removal of the item representing user that has registered the "
			"job is not allowed");
	}
	string jobpath = wmputilities::getJobDirectoryPath(*jid);
	
	edglog(debug)<<"GACL File: "<<jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE<<endl;
	authorizer::GaclManager gaclmanager(jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE);
	gaclmanager.removeEntry(authorizer::GaclManager::WMPGACL_PERSON_TYPE,
		item);
	gaclmanager.saveGacl();
	
	edglog(info)<<"removeACLItem successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

// getDelegatedProxyInfo and getJobProxyInfo
void
getProxyInfo(getProxyInfoResponse &getProxyInfo_response, const string &id,
	bool isjobid)
{
	GLITE_STACK_TRY("getProxyInfo()");
	edglog_fn("wmpoperations::getProxyInfo");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	string proxy;
	if (isjobid) {
		edglog(info)<<"Job Id: "<<id<<endl;
		// Checking delegation id
		if (id == "") {
			edglog(error)<<"Provided Job Id not valid"<<endl;
	  		throw JobOperationException(__FILE__, __LINE__,
				"getProxyInfo()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"Provided Job Id not valid");
		}
		auth->authorize("", id);
		proxy = wmputilities::getJobDelegatedProxyPath(id);
		authorizer::WMPAuthorizer::checkProxyExistence(proxy, id);
		edglog(debug)<<"Job proxy: "<<proxy<<endl;
	} else {
		edglog(info)<<"Delegation Id: "<<id<<endl;
		auth->authorize();
		// Checking delegation id
		if (id == "") {
			edglog(error)<<"Provided Delegation Id not valid"<<endl;
	  		throw JobOperationException(__FILE__, __LINE__,
				"getProxyInfo()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"Provided Delegation Id not valid");
		}
		proxy = WMPDelegation::getDelegatedProxyPath(id);
		edglog(debug)<<"Delegated Proxy: "<<proxy<<endl;
	}
	delete auth;
	
	
	authorizer::VOMSAuthZ vomsproxy = authorizer::VOMSAuthZ(proxy);
	getProxyInfo_response.items = vomsproxy.getProxyInfo();
	
	edglog(info)<<"getProxyInfo successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
checkPerusalFlag(JobId *jid, string &delegatedproxy, bool checkremotepeek)
{
	GLITE_STACK_TRY("checkPerusalFlag()");
	edglog_fn("wmpoperations::checkPerusalFlag");
	
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	
	// Setting user proxy
	if (wmplogger.setUserProxy(delegatedproxy)) {
		edglog(critical)<<"Unable to set User Proxy for LB context"<<endl;
		throw AuthenticationException(__FILE__, __LINE__,
			"checkPerusalFlag()", wmputilities::WMS_AUTHENTICATION_ERROR,
			"Unable to set User Proxy for LB context");
	}
	
	string jdlpath = wmplogger.retrieveRegJobEvent(jid->toString()).jdl;
	edglog(debug)<<"jdlpath: "<<jdlpath<<endl;
	if (jdlpath == "") {
		edglog(critical)<<"No Register event found quering LB; unable to get "
			"registered jdl"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"checkPerusalFlag()", wmputilities::WMS_IS_FAILURE,
			"Unable to check perusal availability"
			"\n(please contact server administrator)");
	}
	
	string jdl = wmputilities::readTextFile(jdlpath);
	edglog(debug)<<"Jdl: "<<jdl<<endl;
	
	int type = getType(jdl);
	if (type == TYPE_JOB) {
		JobAd * jad = new JobAd(jdl);
		jad->setLocalAccess(false);
		
		if (jad->hasAttribute(JDL::PU_FILE_ENABLE)) {
			if (!jad->getBool(JDL::PU_FILE_ENABLE)) {
				edglog(debug)<<"Perusal not enabled for this job"<<endl;
				throw JobOperationException(__FILE__, __LINE__,
			    	"checkPerusalFlag()",
			    	wmputilities::WMS_OPERATION_NOT_ALLOWED, 
			    	"Perusal not enabled for this job");
			}
		} else {
			edglog(debug)<<"Perusal not enabled for this job"<<endl;
			throw JobOperationException(__FILE__, __LINE__,
		    	"checkPerusalFlag()",
		    	wmputilities::WMS_OPERATION_NOT_ALLOWED, 
		    	"Perusal not enabled for this job");
		}
		if (checkremotepeek) {
			if (jad->hasAttribute(JDL::PU_FILES_DEST_URI)) {
				string protocol = conf.getDefaultProtocol();
				string port = (conf.getDefaultPort() != 0)
					? (":" + boost::lexical_cast<std::string>(
						conf.getDefaultPort()))
					: "";
				string serverhost = getServerHost();
				string uri = jad->getString(JDL::PU_FILES_DEST_URI);
				
				string tofind = "://" + serverhost;
				unsigned int pos = uri.find(tofind);
				if (pos == string::npos) {
					edglog(debug)<<"Remote perusal peek URI set:\n"
						<<uri<<endl;
					throw JobOperationException(__FILE__, __LINE__,
				    	"checkPerusalFlag()",
				    	wmputilities::WMS_OPERATION_NOT_ALLOWED, 
				    	"Remote perusal peek URI set:\n" + uri);
				}
				string uriprotocol = uri.substr(0, pos);
				edglog(debug)<<"URI protocol: "<<uriprotocol<<endl;
				string uripath = uri.substr(pos + tofind.length(),
					uri.length() - 1);
				edglog(debug)<<"URI path: "<<uripath<<endl;
				pos = uripath.find("/");
				// Looking for port
				if (pos == string::npos) {
					edglog(debug)<<"Remote perusal peek URI set:\n"
						<<uri<<endl;
					throw JobOperationException(__FILE__, __LINE__,
				    	"checkPerusalFlag()",
				    	wmputilities::WMS_OPERATION_NOT_ALLOWED, 
				    	"Remote perusal peek URI set:\n" + uri);
				}
				uripath = uripath.substr(pos, uripath.length() - 1);
				edglog(debug)<<"URI path: "<<uripath<<endl;
				
				string serverpath;
				if ((uriprotocol == "https") || (uriprotocol == "http")) {
					// No document root added
					serverpath = wmputilities::getPeekDirectoryPath(*jid, false);
				} else {
					serverpath = wmputilities::getPeekDirectoryPath(*jid);	
				}
				edglog(debug)<<"Server path: "<<serverpath<<endl;
				if (uripath != serverpath) {
					edglog(debug)<<"Remote perusal peek URI set:\n"
						<<uri<<endl;
					throw JobOperationException(__FILE__, __LINE__,
				    	"checkPerusalFlag()",
				    	wmputilities::WMS_OPERATION_NOT_ALLOWED, 
				    	"Perusal peek URI refers to server host, but the path "
				    		"is not manged by WMProxy:\n" + uri);
				}
			}
		}
		
		delete jad;
	} else {
		edglog(debug)<<"Perusal service not available for dag or collection type"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
	    	"checkPerusalFlag()", wmputilities::WMS_OPERATION_NOT_ALLOWED, 
	    	"Perusal service not available for dag or collection type");
	}
	
	GLITE_STACK_CATCH();
}

void
enableFilePerusal(enableFilePerusalResponse &enableFilePerusal_response,
	const string &job_id, StringList * fileList)
{
	GLITE_STACK_TRY("enableFilePerusal()");
	edglog_fn("wmpoperations::enableFilePerusal");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if (authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::enableFilePerusal()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	checkPerusalFlag(jid, delegatedproxy, false);
	
	string filename = wmputilities::getPeekDirectoryPath(*jid) + FILE_SEPARATOR
		+ PERUSAL_FILE_2_PEEK_NAME;
	
	unsigned int size = fileList->Item->size();
	if (size != 0) {
		fstream file2peek(filename.c_str(), ios::out);
		for (unsigned int i = 0; i < size; i++) {
	    	file2peek << (*(fileList->Item))[i] + "\n";
		}
	    file2peek.close();
	} else {
		// Removing file2peek file if needed
		if (wmputilities::fileExists(filename)) {
			remove(filename.c_str());
		}
	}

	edglog(info)<<"enableFilePerusal successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

vector<string>
getPerusalFiles(getPerusalFilesResponse &getPerusalFiles_response,
	const string &job_id, const string &fileName, bool allChunks)
{
	GLITE_STACK_TRY("getPerusalFiles()");
	edglog_fn("wmpoperations::getPerusalFiles");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::getPerusalFiles()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	if (fileName == "") {
		throw JobOperationException(__FILE__, __LINE__,
	    	"wmpoperations::getPerusalFiles()", wmputilities::WMS_INVALID_ARGUMENT, 
	    	"Provided file name not valid");
	}
	
	checkPerusalFlag(jid, delegatedproxy, true);

	string peekdir = wmputilities::getPeekDirectoryPath(*jid) + FILE_SEPARATOR;
	const boost::filesystem::path p(peekdir, boost::filesystem::native);
	vector<string> found;
	glite::wms::wmproxy::commands::list_files(p, found);
	
	string protocol = conf.getDefaultProtocol();
	string port = (conf.getDefaultPort())
		? (":" + boost::lexical_cast<std::string>(conf.getDefaultPort()))
		: "";
	string serverhost = getServerHost();
		
	vector<string> good;
	vector<string> returnvector;
	string currentfilename;
	
	vector<string>::iterator iter = found.begin();
	vector<string>::iterator const end = found.end();
	for (; iter != end; ++iter) {
		currentfilename = wmputilities::getFileName(*iter);
		if (currentfilename.find(fileName + ".") == 0) {
			edglog(debug)<<"Good perusal file: "<<*iter<<endl;
			good.push_back(*iter);
		}
		if (allChunks) {
			if (currentfilename.find(fileName + PERUSAL_DATE_INFO_SEPARATOR)
					== 0) {
				edglog(debug)<<"Good old global perusal file: "<<*iter<<endl;
				returnvector.push_back(protocol + "://" + serverhost
					+ port + *iter);	
			}
		}
	}
	
	//unsigned int size = good.size();
	if (good.size()) {
		// Sorting vector
		sort(good.begin(), good.end());
		
		string tempfile = peekdir + TEMP_PERUSAL_FILE_NAME;
		
		long filesize = 0;
		long totalfilesize = 0;
		
		string startdate = good[0].substr(good[0].rfind(".") + 1,
			good[0].length() - 1);
		string enddate = startdate;
			
		fstream outfile(tempfile.c_str(), ios::out);
		if (!outfile.good()) {
			edglog(severe)<<tempfile<<": !outfile.good()"<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"getPerusalFiles()", WMS_IS_FAILURE, "Unable to open perusal "
				"temporary file\n(please contact server administrator)");
		}
		
		string filetoreturn;
		vector<string>::iterator iter = good.begin();
		vector<string>::iterator const end = good.end();
		for (; iter != end; ++iter) {
			filesize = wmputilities::computeFileSize(*iter);
			filetoreturn = peekdir + fileName
				+ PERUSAL_DATE_INFO_SEPARATOR + startdate
				+ PERUSAL_DATE_INFO_SEPARATOR + enddate;
			if ((totalfilesize + filesize) > FILE_TRANSFER_SIZE_LIMIT) {
				outfile.close();
				rename(tempfile.c_str(), filetoreturn.c_str());
				returnvector.push_back(protocol + "://" + serverhost
					+ port + filetoreturn);
				outfile.open(tempfile.c_str(), ios::out);
				if (!outfile.good()) {
					edglog(severe)<<tempfile<<": !outfile.good()"<<endl;
					throw FileSystemException(__FILE__, __LINE__,
						"getPerusalFiles()", WMS_IS_FAILURE, "Unable to open perusal "
						"temporary file\n(please contact server administrator)");
				}
				totalfilesize = 0;
				enddate = (*iter).substr((*iter).rfind(".") + 1,
                	(*iter).length() - 1);
				startdate = enddate;
			} else {
				enddate = (*iter).substr((*iter).rfind(".") + 1,
                	(*iter).length() - 1);
			}
			ifstream infile((*iter).c_str());
			if (!infile.good()) {
				edglog(severe)<<*iter<<": !infile.good()"<<endl;
				throw FileSystemException(__FILE__, __LINE__,
					"getPerusalFiles()", WMS_IS_FAILURE, "Unable to open perusal "
					"input file\n(please contact server administrator)");
			}
			outfile << infile.rdbuf();
			infile.close();
			remove((*iter).c_str());
			totalfilesize += filesize;
		}
		outfile.close();
		filetoreturn = peekdir + fileName
			+ PERUSAL_DATE_INFO_SEPARATOR + startdate
			+ PERUSAL_DATE_INFO_SEPARATOR + enddate;
		rename(tempfile.c_str(), filetoreturn.c_str());
		returnvector.push_back(protocol + "://" + serverhost + port
			+ filetoreturn);
	}
	
	return returnvector;
	
	edglog(info)<<"getPerusalFiles successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

//} // server
//} // wmproxy
//} // wms
//} // glite
