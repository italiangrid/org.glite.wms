/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the copyright
holders.

you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

//
// File: wmpcoreoperations.cpp
// Author: Giuseppe Avellino <egee@datamat.it>
//

// Boost
#include <boost/lexical_cast.hpp>
#include <boost/pool/detail/singleton.hpp>
#include <boost/scoped_ptr.hpp>  // smart pointers

#include <fstream>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "wmp2wm.h"
#include "wmpcommon.h"
#include "wmpcoreoperations.h"
#include "wmpconfiguration.h"
#include "wmpstructconverter.h"

#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

// Utilities
#include "utilities/wmputils.h"

// Eventlogger
#include "eventlogger/wmpexpdagad.h"
#include "eventlogger/wmpeventlogger.h"
#include "eventlogger/wmplbselector.h"	// lbselector

// Authorizer
#include "authorizer/wmpauthorizer.h"
#include "authorizer/wmpgaclmanager.h"
#include "authorizer/wmpvomsauthz.h"

// Logger
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"

#include "commands/listfiles.h"

// Delegation
#include "wmpdelegation.h"

// Exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"
#include "glite/jdl/RequestAdExceptions.h"

// RequestAd
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/jdl/PrivateAttributes.h"
#include "glite/jdl/JDLAttributes.h"
#include "glite/jdl/jdl_attributes.h"
#include "glite/jdl/DAGAdManipulation.h"
#include "glite/jdl/extractfiles.h" // hasWildCards()

// Logging and Bookkeeping
#include "glite/lb/Job.h"
#include "glite/lb/JobStatus.h"

// AdConverter class for jdl convertion and templates
#include "glite/jdl/adconverter.h"

// Configuration
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"

#include "glite/wms/common/utilities/edgstrstream.h"

// Global variables for configuration
extern WMProxyConfiguration conf;
extern std::string filelist_global;
extern glite::wms::wmproxy::eventlogger::WMPLBSelector lbselector;

// DONE job output file
//const std::string MARADONA_FILE = "Maradona.output";

// Perusal functionality
const std::string PERUSAL_FILE_2_PEEK_NAME = "files2peek";
const int DEFAULT_PERUSAL_TIME_INTERVAL = 1000; // seconds

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

using namespace std;
using namespace glite::lb; // JobStatus
using namespace glite::jdl; // DagAd, AdConverter
using namespace boost::details::pool; //singleton
using namespace glite::wmsutils::jobid; //JobId
using namespace glite::wmsutils::exception; //Exception

using namespace glite::wms::wmproxy::server;  //Exception codes
using namespace glite::wms::wmproxy::utilities; //Exception
using namespace glite::wms::wmproxy::eventlogger;
using namespace glite::wms::common::configuration; // Configuration

namespace logger         = glite::wms::common::logger;
namespace wmpmanager	 = glite::wms::wmproxy::server;
namespace wmputilities	 = glite::wms::wmproxy::utilities;
namespace authorizer 	 = glite::wms::wmproxy::authorizer;
namespace eventlogger    = glite::wms::wmproxy::eventlogger;
namespace configuration  = glite::wms::common::configuration;
namespace wmsutilities   = glite::wms::common::utilities;

/* Common Methods:  */

/* WMP Logger Initializer
* Perform ALL common operation to WMPEvent Logger
* NB: may me moved to WMPeventLogger::init TODO
*/
void WMPEventLoggerInitializer(WMPEventLogger &wmplogger, std::pair<std::string, int> &lbaddress_port, JobId *jid, const string &delegatedproxy){
	// WMPEventLogger wmplogger(wmputilities::getEndpoint());
	lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
	conf.getDefaultProtocol(), conf.getDefaultPort());
	edglog(debug)<<"LB Address: "<<lbaddress_port.first<<endl;
	edglog(debug)<<"LB Port: "<<
		boost::lexical_cast<std::string>(lbaddress_port.second)<<endl;
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	wmplogger.setUserProxy(delegatedproxy);
}



// "private" methods prototypes
pair<string, string> jobregister(jobRegisterResponse &jobRegister_response,
	const string &jdl, const string &delegation_id,
	const string &delegatedproxy, const string &delegatedproxyfqan,
	authorizer::WMPAuthorizer *auth);

string regist(jobRegisterResponse &jobRegister_response,
	authorizer::WMPAuthorizer *auth, const string &delegation_id,
	const string &delegatedproxy, const string &delegatedproxyfqan,
	const string &jdl, JobAd *jad);

pair<string, string> regist(jobRegisterResponse &jobRegister_response,
	authorizer::WMPAuthorizer *auth, const string &delegation_id,
	const string &delegatedproxy, const string &delegatedproxyfqan,
	const string &jdl, WMPExpDagAd *dag, JobAd *jad = NULL);

void submit(const string &jdl, JobId *jid, authorizer::WMPAuthorizer *auth,
	eventlogger::WMPEventLogger &wmplogger, bool issubmit = false);
	
int listmatch(jobListMatchResponse &jobListMatch_response, const string &jdl,
	const string &delegation_id, const string &delegatedproxyfqan);

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

/**
Append a tail to an existing ExprTree and build a new exprTree
*/
classad::ExprTree *
appendExprTree(classad::ExprTree * src, classad::ExprTree * tail)
{
        classad::PrettyPrint unp;
        unp.SetClassAdIndentation(0);
        unp.SetListIndentation(0);
        string src_s, tail_s;
	if (src){ 
		unp.Unparse(src_s, src);
		// if a Tail exists then modify the src properly
		if (tail) { src_s="("+src_s+")"+ " && "; }
	}
	if (tail){unp.Unparse(tail_s, tail);}
        src_s = src_s + tail_s;
        classad::ClassAdParser parser;
        return parser.ParseExpression(src_s, true);
}

// make NOT an expression
classad::ExprTree *
notExprTree(classad::ExprTree * expr)
{
	if (expr){
		classad::PrettyPrint unp;
		unp.SetClassAdIndentation(0);
		unp.SetListIndentation(0);
		string buffer;
		unp.Unparse(buffer, expr);
		buffer = "!(" + buffer + ")";
		classad::ClassAdParser parser;
		return parser.ParseExpression(buffer, true);
	}else { return NULL;}
}

/*
* Method  jobregister  (lowcase "r")
* called by wmpcoreoperations::jobRegister (Upcase "R", below)
* calls regist (below)
*/

pair<string, string>
jobregister (jobRegisterResponse &jobRegister_response, const string &jdl,
	const string &delegation_id, const string &delegatedproxy,
	const string &delegatedproxyfqan, authorizer::WMPAuthorizer *auth)
{
	GLITE_STACK_TRY("jobregister()");
	edglog_fn("wmpcoreoperations::jobregister");

	// Checking for VO in jdl file
	string vo = wmputilities::getEnvVO();
	Ad ad;
	int type = getType(jdl, &ad);
	if (ad.hasAttribute(JDL::VIRTUAL_ORGANISATION)) {
		if (vo != "") {
			string jdlvo = ad.getString(JDL::VIRTUAL_ORGANISATION);
			if (glite_wms_jdl_toLower(jdlvo) != glite_wms_jdl_toLower(vo)) {
				string msg = "Jdl " + JDL::VIRTUAL_ORGANISATION
					+ " attribute (" + jdlvo + ") is different from delegated Proxy Virtual "
					"Organisation ("+ vo +")";
				edglog(error)<<msg<<endl;
				throw JobOperationException(__FILE__, __LINE__,
			    	"wmpcoreoperations::jobregister()",
			    	wmputilities::WMS_INVALID_JDL_ATTRIBUTE, msg);
			}
		}
	}
	pair<string, string> returnpair;
	// Checking TYPE/JOBTYPE attributes and convert JDL when needed
	if (type == TYPE_DAG) {
		edglog(debug)<<"Type DAG"<<endl;
		WMPExpDagAd dag(jdl);
		dag.setLocalAccess(false);
		returnpair = regist(jobRegister_response, auth, delegation_id,
			delegatedproxy, delegatedproxyfqan, jdl, &dag);
	} else if (type == TYPE_JOB) {
		edglog(debug)<<"Type Job"<<endl;
		JobAd jad (*(ad.ad()));
		jad.setLocalAccess(false);
		// Checking for multiple job type (not yet supported)
		if (jad.hasAttribute(JDL::JOBTYPE)) {
			if (jad.getStringValue(JDL::JOBTYPE).size() > 1) {
			edglog(error)<<"Composite Job Type not yet supported"<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"wmpcoreoperations::jobregister()",
				wmputilities::WMS_INVALID_JDL_ATTRIBUTE,
				"Composite Job Type not yet supported");
			}
        	}
        	if (jad.hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_PARAMETRIC)) {
			edglog(info)<<"Converting Parametric job to DAG..."<<endl;
			WMPExpDagAd dag (*(AdConverter::bulk2dag(&ad)));
			dag.setLocalAccess(false);
			returnpair = regist(jobRegister_response, auth, delegation_id,
				delegatedproxy, delegatedproxyfqan, jdl, &dag);
		} else if (jad.hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_PARTITIONABLE)) {
			// PARTITIONABLE Jobs ARE DEPRECATED!!!
			string msg = "Partitionable Jobs are Deprecated";
			edglog(error)<< msg << ": Exiting" << endl;
			throw JobOperationException(__FILE__, __LINE__,
				"jobregister()", wmputilities::WMS_JDL_PARSING,msg);
		} else {
			// Normal Jobs (neither PARAMETRIC nor COLLECTION)
			returnpair.first = regist(jobRegister_response, auth, delegation_id,
				delegatedproxy, delegatedproxyfqan, jdl, &jad);
			returnpair.second = jad.toSubmissionString();
		}
	} else if (type == TYPE_COLLECTION) {
		edglog(debug)<<"Type Collection"<<endl;
		WMPExpDagAd dag(*(AdConverter::collection2dag(&ad)));
		dag.setLocalAccess(false);
		returnpair = regist(jobRegister_response, auth, delegation_id,
			delegatedproxy, delegatedproxyfqan, jdl, &dag);
	} else {
		string invalidJobType = ad.hasAttribute(JDL::TYPE)  ?
			ad.getString(JDL::TYPE) :
			"(empty value)" ;
		throw JobOperationException(__FILE__, __LINE__,
			"wmpcoreoperations::jobregister()",
			wmputilities::WMS_INVALID_JDL_ATTRIBUTE,
			"Invalid Job Type: " +invalidJobType);
	}
	return returnpair;
	GLITE_STACK_CATCH();
}
/*
* Method  jobRegister  (upcase "R")
* called by wmpgsoapoperations::ns1__jobRegister
* calls wmpcoreoperations::jobregister (lowcase "R", above)
*/

void
jobRegister(jobRegisterResponse &jobRegister_response, const string &jdl,
	const string &delegation_id)
{
	GLITE_STACK_TRY("jobRegister()");
	edglog_fn("wmpcoreoperations::jobRegister");
	// log Remote host info, call load script file,checkConfiguration, setGlobalSandboxDir
	initWMProxyOperation("jobRegister");
	
	// Checking delegation id
	edglog(info)<<"Delegation ID: "<<delegation_id<<endl;

#ifndef GRST_VERSION	
	if (delegation_id == "") {
		edglog(error)<<"Empty delegation id not allowed with delegation 1"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"jobRegister()", wmputilities::WMS_INVALID_ARGUMENT,
			"Delegation id not valid");
	}
#endif
	edglog(debug)<<"JDL to Register:\n"<<jdl<<endl;

	// complicate checkSecurity: TODO
	// - delegatedproxy, delegatedproxyfqan, auth  needed/reused

	//** Authorizing user
	edglog(debug)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = new authorizer::WMPAuthorizer();

	// Getting delegated proxy from SSL Proxy cache
	string delegatedproxy = WMPDelegation::getDelegatedProxyPath(delegation_id);
	edglog(debug)<<"Delegated proxy: "<<delegatedproxy<<endl;

	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	string delegatedproxyfqan = vomsproxy.getDefaultFQAN();
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(delegatedproxyfqan);
	} else {
		auth->authorize();
	}

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"
			<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpcoreoperations::jobRegister()", wmputilities::WMS_AUTHORIZATION_ERROR,
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}

	// Checking proxy validity
	authorizer::WMPAuthorizer::checkProxy(delegatedproxy);

	jobregister(jobRegister_response, jdl, delegation_id, delegatedproxy,
		delegatedproxyfqan, auth);

	// Release Memory
	if (auth) {
		delete auth;
	}
	edglog(debug)<<"Registered successfully"<<endl;

	GLITE_STACK_CATCH();
}

void
setSubjobFileSystem(authorizer::WMPAuthorizer *auth, 
	const string &jobid, vector<string> &jobids)
{
	GLITE_STACK_TRY("setSubjobFileSystem()");
	edglog_fn("wmpcoreoperations::setSubjobFileSystem");
	
	// Getting LCMAPS mapped User ID
	uid_t jobdiruserid = auth->getUserId();
	edglog(debug)<<"User Id: "<<jobdiruserid<<endl;

	// Getting WMP Server User ID
	uid_t userid = getuid();

	// Logging Server User ID on syslog

        openlog("glite_wms_wmproxy_server", LOG_PID || LOG_CONS, LOG_DAEMON);

        char time_string[20];
        struct timeval tv;
        struct tm* ptm;
        gettimeofday(&tv, NULL);
        ptm = localtime(&tv.tv_sec);
        strftime(time_string, sizeof (time_string), "%Y-%m-%d.%X%H%M%S", ptm);

        string userid_log = "ts="+std::string(time_string);
        userid_log += " : ";
        userid_log += "event=wms.wmproxyserver.setSubjobFileSystem()";
        userid_log += " : ";
        userid_log += "userid="+boost::lexical_cast<string>(userid);
	userid_log += " ";
	userid_log += "jobid="+jobid;


        syslog(LOG_NOTICE || LOG_INFO || LOG_DEBUG, userid_log.c_str());
        closelog();

	string document_root = getenv(DOCUMENT_ROOT);

	// Getting delegated proxy inside job directory
	string proxy(wmputilities::getJobDelegatedProxyPath(jobid));
	edglog(debug)<<"Job delegated proxy: "<<proxy<<endl;
	
	// Creating a copy of the Proxy
	string proxybak = wmputilities::getJobDelegatedProxyPathBak(jobid);
	
	// Creating sub jobs directories
	if (jobids.size()) {
		edglog(debug)<<"Creating sub job directories for job:\n"<<jobid<<endl;
		wmputilities::managedir(document_root, userid, jobdiruserid, jobids,
			wmputilities::DIRECTORY_INPUT);
		
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
						"setSubjobFileSystem()", wmputilities::WMS_FILE_SYSTEM_ERROR,
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
						"setSubjobFileSystem()", wmputilities::WMS_FILE_SYSTEM_ERROR,
						"Unable to create symbolic link to backup proxy file\n"
							"(please contact server administrator)");
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
	edglog_fn("wmpcoreoperations::setJobFileSystem");
	
	// Getting LCMAPS mapped User ID
	uid_t jobdiruserid = auth->getUserId();
	edglog(debug)<<"User Id: "<<jobdiruserid<<endl;
	
	// Getting WMP Server User ID
	uid_t userid = getuid();

        // Logging Server User ID on syslog

        openlog("glite_wms_wmproxy_server", LOG_PID || LOG_CONS, LOG_DAEMON);

	char time_string[20];
        struct timeval tv;
        struct tm* ptm;
        gettimeofday(&tv, NULL);
        ptm = localtime(&tv.tv_sec);
        strftime(time_string, sizeof (time_string), "%Y-%m-%d.%X%H%M%S", ptm);

        string userid_log = "ts="+std::string(time_string);
        userid_log += " : ";
        userid_log += "event=wms.wmproxyserver.setJobFileSystem()";
        userid_log += " : ";
        userid_log += "userid="+boost::lexical_cast<string>(userid);
        userid_log += " ";
	userid_log += "jobid="+jobid;

        syslog(LOG_NOTICE || LOG_INFO || LOG_DEBUG, userid_log.c_str());
        closelog();

	string document_root = getenv(DOCUMENT_ROOT);
	
	// Creating job directory
	vector<string> job;
	job.push_back(jobid);
	edglog(debug)<<"Creating job directories for job:\n"<<jobid<<endl;
	wmputilities::managedir(document_root, userid, jobdiruserid, job,
		wmputilities::DIRECTORY_ALL);
	
	// Getting delegated proxy inside job directory
	string proxy(wmputilities::getJobDelegatedProxyPath(jobid));
	edglog(debug)<<"Job delegated proxy: "<<proxy<<endl;
	
	if (renewalproxy) {
		// Creating a symbolic link to renewal Proxy temporary file
		edglog(debug)<<"Creating a symbolic link to renewal Proxy temporary file..."
			<<endl;
		if (symlink(renewalproxy, proxy.c_str())) {
			if (errno != EEXIST) {
		      	edglog(critical)
		      		<<"Unable to create symbolic link to renewal proxy:\n\t"
		      		<<proxy<<"\n"<<strerror(errno)<<endl;
		      
		      	throw FileSystemException(__FILE__, __LINE__,
					"setJobFileSystem()", wmputilities::WMS_FILE_SYSTEM_ERROR,
					"Unable to create symbolic link to renewal proxy\n(please "
					"contact server administrator)");
			}
	    }
	} else {
		// Copying delegated Proxy to destination URI
		wmputilities::fileCopy(delegatedproxy, proxy);
	}
	
	// Creating a copy of the Proxy
	edglog(debug)<<"Creating a copy of the Proxy..."<<endl;
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
setAttributes(JobAd *jad, JobId *jid, const string &dest_uri,
	const string &delegatedproxyfqan)
{
	GLITE_STACK_TRY("setAttributes()");
	edglog_fn("wmpcoreoperations::setAttributes JOB");
	
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
	
	if (delegatedproxyfqan != "") {
		edglog(debug)<<"Setting attribute JDLPrivate::VOMS_FQAN"<<endl;
		if (jad->hasAttribute(JDLPrivate::VOMS_FQAN)) {
			jad->delAttribute(JDLPrivate::VOMS_FQAN);
		}
		jad->setAttribute(JDLPrivate::VOMS_FQAN, delegatedproxyfqan);
	}
	
	// Checking for empty string value
	if (jad->hasAttribute(JDL::MYPROXY)) {
		if (jad->getString(JDL::MYPROXY) == "") {
			edglog(debug)<<JDL::MYPROXY
				+ " attribute value is empty string, removing..."<<endl;
			jad->delAttribute(JDL::MYPROXY);
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

/*
* Registering for jobs ( jobtype = Job) , called by jobregister)
* JDL paramters:   string jdl, JobAd* jad
*/

string
regist(jobRegisterResponse &jobRegister_response, authorizer::WMPAuthorizer *auth,
	const string &delegation_id, const string &delegatedproxy,
	const string &delegatedproxyfqan, const string &jdl, JobAd *jad)
{
	GLITE_STACK_TRY("regist()");
	edglog_fn("wmpcoreoperations::regist JOB");
	
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
		// CHECKPOINTABLE Jobs ARE DEPRECATED!!!
		string msg = "Checkpointable Jobs are Deprecated";
		edglog(error)<< msg << endl;
		throw JobOperationException(__FILE__, __LINE__,
			"regist()", wmputilities::WMS_JDL_PARSING,msg);
	}
	
	// Creating unique identifier
	JobId *jid = new JobId();
	
	std::pair<std::string, int> lbaddress_port;
	
	// Checking for attribute JDL::LB_ADDRESS
	if (jad->hasAttribute(JDL::LB_ADDRESS)) {
		string lbaddressport = jad->getString(JDL::LB_ADDRESS);
		wmputilities::parseAddressPort(lbaddressport, lbaddress_port);
	} else {
	 	//lbaddress_port = conf.getLBServerAddressPort();
	 	lbaddress_port = lbselector.selectLBServer();
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
	edglog(info)<<"Registering id: "<<stringjid<<endl;

	// Getting Input Sandbox Destination URI
	string dest_uri = wmputilities::getDestURI(stringjid, conf.getDefaultProtocol(),
		conf.getDefaultPort());
	edglog(debug)<<"Destination URI: "<<dest_uri<<endl;
	
	setAttributes(jad, jid, dest_uri, delegatedproxyfqan);
	
	edglog(debug)<<"Endpoint: "<<wmputilities::getEndpoint()<<endl;
	// Initializing logger
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	lbaddress_port = conf.getLBLocalLoggerAddressPort();
	edglog(debug)<<"LB Address: "<<lbaddress_port.first<<endl;
	edglog(debug)<<"LB Port: "
		<<boost::lexical_cast<std::string>(lbaddress_port.second)<<endl;
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());

	// Setting user proxy
	wmplogger.setUserProxy(delegatedproxy);

	// Registering the job
	jad->check();
	wmplogger.registerJob(jad, wmputilities::getJobJDLToStartPath(*jid, true));
	
	// Registering for Proxy renewal
	char * renewalproxy = NULL;
	if (jad->hasAttribute(JDL::MYPROXY)) {
		edglog(debug)<<"Registering Proxy renewal..."<<endl;
		renewalproxy = wmplogger.registerProxyRenewal(delegatedproxy,
			(jad->getString(JDL::MYPROXY)));
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

	// Creating job identifier structure to return to the caller
	JobIdStructType *job_id_struct = new JobIdStructType();
	job_id_struct->id = stringjid;
	job_id_struct->name = new string("");
	job_id_struct->path = new string(getJobInputSBRelativePath(stringjid));
	job_id_struct->childrenJob = new vector<JobIdStructType*>;
	jobRegister_response.jobIdStruct = job_id_struct;
	
	edglog(debug)<<"Job successfully registered: "<<stringjid<<endl;
	
	return stringjid;

	GLITE_STACK_CATCH();
}





void
setAttributes(WMPExpDagAd *dag, JobId *jid, const string &dest_uri,
	const string &delegatedproxyfqan)
{
	GLITE_STACK_TRY("setAttributes()");
	edglog_fn("wmpcoreoperations::setAttributes DAG");
	
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
	
	if (delegatedproxyfqan != "") {
		edglog(debug)<<"Setting attribute JDLPrivate::VOMS_FQAN"<<endl;
		if (dag->hasAttribute(JDLPrivate::VOMS_FQAN)) {
			dag->removeAttribute(JDLPrivate::VOMS_FQAN);
		}
		dag->setReserved(JDLPrivate::VOMS_FQAN, delegatedproxyfqan);
	}
		
	// Checking for empty string value
	if (dag->hasAttribute(JDL::MYPROXY)) {
		if (dag->getString(JDL::MYPROXY) == "") {
			edglog(debug)<<JDL::MYPROXY
				+ " attribute value is empty string, removing..."<<endl;
			dag->removeAttribute(JDL::MYPROXY);
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


/*
* Registering for dags, collections, parametric and partitionables ( jobtype = Job) , called by jobregister)
* JDL paramters:   string jdl, WMPExpDagAd *dag, JobAd *jad
* calls WMPeventlogger::registerDag
* NB: Partitionable jobs are deprecated
*/
pair<string, string>
regist(jobRegisterResponse &jobRegister_response, authorizer::WMPAuthorizer *auth,
	const string &delegation_id, const string &delegatedproxy,
	const string &delegatedproxyfqan, const string &jdl, WMPExpDagAd *dag,
	JobAd *jad)
{
	GLITE_STACK_TRY("regist()");
	edglog_fn("wmpcoreoperations::regist DAG");

	// Creating unique identifier
	JobId *jid = new JobId();

	std::pair<std::string, int> lbaddress_port;
	
	// Checking for attribute JDL::LB_ADDRESS
	if (jad) {
		if (jad->hasAttribute(JDL::LB_ADDRESS)) {
			string lbaddressport = jad->getString(JDL::LB_ADDRESS);
			wmputilities::parseAddressPort(lbaddressport, lbaddress_port);
		} else {
		 	//lbaddress_port = conf.getLBServerAddressPort();
		 	lbaddress_port = lbselector.selectLBServer();
		}
	} else {
		if (dag->hasAttribute(JDL::LB_ADDRESS)) {
			string lbaddressport = dag->getString(JDL::LB_ADDRESS);
			wmputilities::parseAddressPort(lbaddressport, lbaddress_port);
		} else {
		 	//lbaddress_port = conf.getLBServerAddressPort();
		 	lbaddress_port = lbselector.selectLBServer();
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
	edglog(info)<<"Registering job id: "<<stringjid<<endl;

	// Initializing logger
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	
	vector<string> jobids;
	if (jad) {
		// PArtitionable jobs registration......
		// PARTITIONABLE Jobs ARE DEPRECATED!!!
		string msg = "Partitionable Jobs are Deprecated";
		edglog(error)<< msg << endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobregister()", wmputilities::WMS_JDL_PARSING,msg);
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
	setAttributes(dag, jid, dest_uri, delegatedproxyfqan);
		
	// Setting user proxy
	wmplogger.setUserProxy(delegatedproxy);

	// Setting Bulk MM
	wmplogger.setBulkMM(configuration::Configuration::instance()->wm()->enable_bulk_mm());

	// It is used also for attribute inheritance
	//dag->toString(ExpDagAd::SUBMISSION);
	wmplogger.registerDag(dag, wmputilities::getJobJDLToStartPath(*jid, true));
	
	// Registering for Proxy renewal
	char * renewalproxy = NULL;
	if (dag->hasAttribute(JDL::MYPROXY)) {
		edglog(debug)<<"Registering Proxy renewal..."<<endl;
		renewalproxy = wmplogger.registerProxyRenewal(delegatedproxy,
			(dag->getAttribute(WMPExpDagAd::MYPROXY_SERVER)));
	}
	
	// Creating private job directory with delegated Proxy
	/***if (dag->hasAttribute(JDLPrivate::ZIPPED_ISB)) {
		// Creating job directories only for the parent. -> empty vector.
		vector<string> emptyvector;
		setJobFileSystem(auth, delegatedproxy, stringjid, emptyvector,
			jdl, renewalproxy);
	} else {***/
		// Sub jobs directory MUST be created now
		setJobFileSystem(auth, delegatedproxy, stringjid, jobids,
			jdl, renewalproxy);
	/***}***/
	
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

	// Creating job identifier structure to return to the caller
	JobIdStructType *job_id_struct = new JobIdStructType();
	JobIdStruct job_struct = dag->getJobIdStruct();
	job_id_struct->id = job_struct.jobid.toString(); // should be equal to: jid->toString()
	job_id_struct->name = job_struct.nodeName;
	job_id_struct->path = new string(getJobInputSBRelativePath(job_id_struct->id));
	job_id_struct->childrenJob = convertJobIdStruct(job_struct.children);
	jobRegister_response.jobIdStruct = job_id_struct;
	
	edglog(debug)<<"Job successfully registered: "<<stringjid<<endl;

	return returnpair;
	
	GLITE_STACK_CATCH();
}

/**
* gsoap code message filling wrapper
*/
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
	edglog_fn("wmpcoreoperations::jobStart");
	// log Remote host info, call load script file,checkConfiguration, setGlobalSandboxDir
	initWMProxyOperation("jobStart");
	edglog(debug)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	// checkSecurity(jid, NULL, true);  delegated proxy and auth needed TODO
	// string delegatedproxy= wmputilities::getJobDelegatedProxyPath(*jid);


	//** Authorizing user
	edglog(debug)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = new authorizer::WMPAuthorizer();

	// Checking job existency (if the job directory doesn't exist:
	// The job has not been registered from this Workload Manager Proxy
	// or it has been purged)
	checkJobDirectoryExistence(*jid);

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
	    	"wmpcoreoperations::jobStart()", wmputilities::WMS_AUTHORIZATION_ERROR,
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END


	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	
	// Setting user proxy

	wmplogger.setUserProxy(delegatedproxy); // delegated proxy may be returned by checkSecurity

	// Checking proxy validity
	try {
		authorizer::WMPAuthorizer::checkProxy(delegatedproxy);
	} catch (Exception &ex) {
		if (ex.getCode() == wmputilities::WMS_PROXY_EXPIRED) {
			wmplogger.setSequenceCode(EDG_WLL_SEQ_ABORT);
			wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ABORT,
				"Job Proxy has expired", true, true);
			jobPurgeResponse jobPurge_response;
			jobpurge(jobPurge_response, jid, false);	
		}
		throw ex;
	}
	
	pair<string, regJobEvent> startpair = wmplogger.isStartAllowed();
	if (startpair.first == "") { // seqcode
		edglog(error)<<"The job has already been started"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"The job has already been started");
	}
	if (startpair.second.parent != "") { // event
		string msg = "the job is a DAG subjob. The parent is: "
			+ startpair.second.parent;
		edglog(error)<<msg<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart()", wmputilities::WMS_OPERATION_NOT_ALLOWED, msg);
	}
	
	wmplogger.setSequenceCode(startpair.first);
	wmplogger.incrementSequenceCode();
	
	string jdlpath = wmputilities::getJobJDLToStartPath(*jid);
	if (!wmputilities::fileExists(jdlpath)) {
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart()", wmputilities::WMS_JOB_NOT_FOUND,
			"Unable to start job");
	}

	if (conf.getAsyncJobStart()) {
		// Creating SOAP answer (submit method will finish FCGI)
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
submit(const string &jdl, JobId *jid, authorizer::WMPAuthorizer *auth,
	eventlogger::WMPEventLogger &wmplogger, bool issubmit)
{
	// File descriptor to control mutual exclusion in start operation
	int fd = -1; 
	
	try {
		edglog_fn("wmpcoreoperations::submit");
		
		if (issubmit) {
			// Starting the job. Need to continue from last logged seqcode
			string seqcode = wmplogger.getLastEventSeqCode();
			wmplogger.setSequenceCode(const_cast<char*>(seqcode.c_str()));
			wmplogger.incrementSequenceCode();
		} else {
			// Locking lock file to ensure only one start operation is running
			// at any time for a specific job
			fd = wmputilities::operationLock(
				wmputilities::getJobStartLockFilePath(*jid), "jobStart");
		}
			
		if (conf.getAsyncJobStart()) {
			// \/ Copy environment and restore it right after FCGI_Finish
			extern char **environ;
			char ** backupenv = copyEnvironment(environ);
			// return control to client
			FCGI_Finish();
			environ = backupenv;
			// TODO release memory
			// /_ From here on, execution is asynchronous
		}

		edglog(debug)<<"Logging LOG_ACCEPT..."<<endl;
		char * fromclient = NULL;
		if (getenv("REMOTE_HOST")) {
			fromclient = getenv("REMOTE_HOST");
		} else if (getenv("REMOTE_ADDR")) {
			fromclient = getenv("REMOTE_ADDR");
		}
		
		// SYNCH log
		int error = wmplogger.logAcceptEventSync(fromclient);
		if (error) {
			edglog(debug)<<"LOG_ACCEPT failed, error code: "<<error<<endl;
			
			// Logging event start to begin iter before fail log in above catch
			edglog(debug)<<"Registering LOG_ENQUEUE_START"<<std::endl;
			wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_START,
				"LOG_ENQUEUE_START", true, true, filelist_global.c_str(),
				wmputilities::getJobJDLToStartPath(*jid).c_str());
				
			throw LBException(__FILE__, __LINE__,
				"submit()", wmputilities::WMS_LOGGING_ERROR,
				"unable to complete operation");
		}
		
		// ASYNCH log TODO
		/*try {
			wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ACCEPT,
				fromclient, true, true);
		} catch (LBException lbe) {
			edglog(debug)<<"LOG_ACCEPT failed"<<endl;
			
			// Logging event start to begin iter before fail log in above catch
			edglog(debug)<<"Registering LOG_ENQUEUE_START"<<std::endl;
			wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_START,
				"LOG_ENQUEUE_START", true, true, filelist_global.c_str(),
				wmputilities::getJobJDLToStartPath(*jid).c_str());
			
			lbe.push_back(__FILE__, __LINE__, "submit()");

			throw lbe;
		}*/

		edglog(debug)<<"Registering LOG_ENQUEUE_START"<<std::endl;
		wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_START,
			"LOG_ENQUEUE_START", true, true, filelist_global.c_str(),
			wmputilities::getJobJDLToStartPath(*jid).c_str());

		// Getting delegated proxy inside job directory
		string proxy(wmputilities::getJobDelegatedProxyPath(*jid));
		edglog(debug)<<"Job delegated proxy: "<<proxy<<endl;

		string jdltostart = jdl;
		string jdlpath = "";

		// Retrieving configuration attributes ISB and OSB max files number
		int maxInputSandboxFiles = conf.getMaxInputSandboxFiles();
                int maxOutputSandboxFiles = conf.getMaxOutputSandboxFiles();


		// Add it if registerSubjos is done after FL insertion
		int type = getType(jdl);
		if (type == TYPE_JOB) {
// ==========   TYPE is a JOB   ==========
			JobAd jad(jdl);
			jad.setLocalAccess(false);
			// Getting Input Sandbox Destination URI
			string dest_uri = wmputilities::getDestURI(jid->toString(),
				conf.getDefaultProtocol(), conf.getDefaultPort());
			edglog(debug)<<"Destination URI: "<<dest_uri<<endl;
	
			// Checking files number of inputsandbox	
			if (jad.hasAttribute(JDL::INPUTSB) && (maxInputSandboxFiles > 0)) {
				
				vector<string> inputsburi = jad.getStringValue(JDL::INPUTSB);
				int inputsbSize = inputsburi.size();
			
				if (inputsbSize > maxInputSandboxFiles) {
					edglog(debug)<<"The maximum number of input sandbox files is reached"<<endl;
					throw JobOperationException(__FILE__, __LINE__, "wmpcoreoperations::submit()", 
					  wmputilities::WMS_OPERATION_NOT_ALLOWED, "The maximum number of input sandbox files is reached");	
	                        }
			}	

			// Adding OutputSandboxDestURI attribute
			if (jad.hasAttribute(JDL::OUTPUTSB)) {
				vector<string> outputsb = jad.getStringValue(JDL::OUTPUTSB);
				bool flag = false;
				unsigned int size = outputsb.size();
				for (unsigned int i = 0; (i < size) && !flag; i++) {
					if (hasWildCards(outputsb[i])) {
						flag = true;
					}
				}
				if (!flag) {
					edglog(debug)<<"Setting attribute JDL::OSB_DEST_URI"<<endl;
					vector<string> osbdesturi;
					if (jad.hasAttribute(JDL::OSB_DEST_URI)) {
						osbdesturi = wmputilities::computeOutputSBDestURI(
							jad.getStringValue(JDL::OSB_DEST_URI),
							dest_uri);
						jad.delAttribute(JDL::OSB_DEST_URI);
					} else if (jad.hasAttribute(JDL::OSB_BASE_DEST_URI)) {
						osbdesturi = wmputilities::computeOutputSBDestURIBase(
							jad.getStringValue(JDL::OUTPUTSB),
							jad.getString(JDL::OSB_BASE_DEST_URI));
					} else {
						osbdesturi = wmputilities::computeOutputSBDestURIBase(
							jad.getStringValue(JDL::OUTPUTSB),
							dest_uri + "/output");
					}
					
					vector<string>::iterator iter = osbdesturi.begin();
					vector<string>::iterator const end = osbdesturi.end();
					int counter = 0;
					for (; iter != end; ++iter) {
						counter++;
						if (counter <= maxOutputSandboxFiles) {
							edglog(debug)<<"OutputSBDestURI: "<<*iter<<endl;
							jad.addAttribute(JDL::OSB_DEST_URI, *iter);
						} else if (maxOutputSandboxFiles > 0) {
							edglog(debug)<<"The maximum number of output sandbox files is reached"<<endl;
                                                        throw JobOperationException(__FILE__, __LINE__, "wmpcoreoperations::submit()",
                                                 		wmputilities::WMS_OPERATION_NOT_ALLOWED, "The maximum number of output sandbox files is reached"); 
						}
					}
				} else {
					edglog(debug)<<"Output SB has wildcards: skipping setting "
						"attribute JDL::OSB_DEST_URI"<<endl;
					if (!jad.hasAttribute(JDL::OSB_BASE_DEST_URI)) {
						edglog(debug)<<"Setting attribute JDL::OSB_BASE_DEST_URI"<<endl;
						jad.setAttribute(JDL::OSB_BASE_DEST_URI,
							dest_uri + "/output");
					}
				}
			}
			// Adding attribute for perusal functionalities
			string peekdir = wmputilities::getPeekDirectoryPath(*jid);
			if (jad.hasAttribute(JDL::PU_FILE_ENABLE)) {
				if (jad.getBool(JDL::PU_FILE_ENABLE)) {
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
					if (jad.hasAttribute(JDL::PU_FILES_DEST_URI)) {
						jad.delAttribute(JDLPrivate::PU_LIST_FILE_URI);
					}
					jad.setAttribute(JDLPrivate::PU_LIST_FILE_URI,
						protocol + "://" + serverhost + port + peekdir
						+ FILE_SEPARATOR + PERUSAL_FILE_2_PEEK_NAME);
					if (!jad.hasAttribute(JDL::PU_FILES_DEST_URI)) {
						edglog(debug)<<"Setting attribute JDL::PU_FILES_DEST_URI"
							<<endl;
						jad.setAttribute(JDL::PU_FILES_DEST_URI,
							protocol + "://" + serverhost + port + peekdir);
					}

					int time = DEFAULT_PERUSAL_TIME_INTERVAL;
					if (jad.hasAttribute(JDL::PU_TIME_INTERVAL)) {
						time = max(time, jad.getInt(JDL::PU_TIME_INTERVAL));
						jad.delAttribute(JDL::PU_TIME_INTERVAL);
					}
					time = max(time, conf.getMinPerusalTimeInterval());
					edglog(debug)<<"Setting attribute JDL::PU_TIME_INTERVAL"<<endl;
					jad.setAttribute(JDL::PU_TIME_INTERVAL, time);
				}
			}

			// Looking for Zipped ISB
			if (jad.hasAttribute(JDLPrivate::ZIPPED_ISB)) {
				string flagfile = wmputilities::getJobDirectoryPath(*jid)
		    			+ FILE_SEPARATOR + FLAG_FILE_UNZIP;
				if (!wmputilities::fileExists(flagfile)) {
					vector<string> files = jad.getStringValue(JDLPrivate::ZIPPED_ISB);
					string targetdir = getenv(DOCUMENT_ROOT);
					string jobpath = wmputilities::getInputSBDirectoryPath(*jid)
						+ FILE_SEPARATOR;
					for (unsigned int i = 0; i < files.size(); i++) {
							edglog(debug)<<"Uncompressing zip file: "<<files[i]<<endl;
							edglog(debug)<<"Absolute path: "<<jobpath + files[i]<<endl;
							edglog(debug)<<"Target directory: "<<targetdir<<endl;
							//wmputilities::uncompressFile(jobpath + files[i], targetdir);
							// TBD Call the method with a vector of file
							wmputilities::untarFile(jobpath + files[i],
								targetdir, auth->getUserId(), auth->getUserGroup());
					}
					wmputilities::setFlagFile(flagfile, true);
				} else {
					edglog(debug)<<"Flag file "<<flagfile<<" already exists. "
						"Skipping untarFile..."<<endl;
				}
			}
			if (jad.hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_INTERACTIVE)) {
				edglog(debug)<<"Logging listener"<<endl;
				wmplogger.logListener(jad.getString(JDL::SHHOST).c_str(),
					jad.getInt(JDL::SHPORT));
			} else if (jad.hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_CHECKPOINTABLE)) {
				// CHECKPOINTABLE Jobs ARE DEPRECATED!!!
				string msg = "Checkpointable Jobs are Deprecated";
				edglog(error)<< msg << endl;
				throw JobOperationException(__FILE__, __LINE__,
					"submit()", wmputilities::WMS_JDL_PARSING,msg);
			}
			// SDJ CHECKS/adjust Requirements attribute
			classad::ExprTree * sdjrequirements= conf.getSDJRequirements(); 
			if (sdjrequirements){
				classad::ExprTree * jadREQ= jad.delAttribute(JDL::REQUIREMENTS); 
			        if ( jad.hasAttribute(JDL::SHORT_DEADLINE_JOB)
			                && jad.getBool(JDL::SHORT_DEADLINE_JOB) ){
			                // sdj conf found, sdj requested by user within JDL
			                // Do notihing - accept sdj conf value found
					 edglog(warning)<<"Short Deadline Job selected by user"<< endl;
			        } else{
			                // sdj conf value found NEITHER requested by JDL nor by any other case
			                sdjrequirements=notExprTree(sdjrequirements); // TODO MEMORY allocation
			        }
			        // Write Requirements Attribute:
			        jad.setAttributeExpr(JDL::REQUIREMENTS,appendExprTree (jadREQ,sdjrequirements));
				if (jadREQ)    {delete jadREQ;}
				delete sdjrequirements;
			}else{
				// SDJ conf not found, nothing to change
				edglog(warning)<<"Unable to find SDJRequirements in configuration file"<< endl;
			}
			// \[ Do not separate
			// Inserting sequence code
			edglog(debug)<<"Setting attribute JDL::LB_SEQUENCE_CODE"<<endl;
			if (jad.hasAttribute(JDL::LB_SEQUENCE_CODE)) {
				jad.delAttribute(JDL::LB_SEQUENCE_CODE);
			}
			jad.setAttribute(JDL::LB_SEQUENCE_CODE, string(wmplogger.getSequence()));
			// jdltostart MUST contain last seqcode
			jdltostart = jad.toString();
			// /_
// ==========   END TYPE is a JOB ==========
		} else {
// ==========   TYPE is a DAG   ==========

			WMPExpDagAd dag (jdl);
			dag.setLocalAccess(false);
			JobIdStruct jobidstruct = dag.getJobIdStruct();
			JobId parentjobid = jobidstruct.jobid;
			vector<JobIdStruct*> children = jobidstruct.children;

			// [ Creating 'output' and 'peek' sub job directories
			vector<string> jobids = wmplogger.generateSubjobsIds(dag.size()); //Filling wmplogger subjobs
			// Getting LCMAPS mapped User ID
			uid_t jobdiruserid = auth->getUserId();
			edglog(debug)<<"User Id: "<<jobdiruserid<<endl;
			// Getting WMP Server User ID
			uid_t userid = getuid();

	        	// Logging Server User ID on syslog

		        openlog("glite_wms_wmproxy_server", LOG_PID || LOG_CONS, LOG_DAEMON);

       			char time_string[20];
        		struct timeval tv;
        		struct tm* ptm;
        		gettimeofday(&tv, NULL);
        		ptm = localtime(&tv.tv_sec);
        		strftime(time_string, sizeof (time_string), "%Y-%m-%d.%X%H%M%S", ptm);

        		string userid_log = "ts="+std::string(time_string);
        		userid_log += " : ";
        		userid_log += "event=wms.wmproxyserver.submit()";
        		userid_log += " : ";
        		userid_log += "userid="+boost::lexical_cast<string>(userid);
		        userid_log += " ";
			userid_log += "jobid="+parentjobid.toString();

        		syslog(LOG_NOTICE || LOG_INFO || LOG_DEBUG, userid_log.c_str());
        		closelog();

			string document_root = getenv(DOCUMENT_ROOT);
			edglog(debug)<<"Creating sub job directories for job:\n"
				<<parentjobid.toString()<<endl;
			// N.B. jobids vector is generated by generateSubjobsIds!!
			wmputilities::managedir(document_root, userid, jobdiruserid, jobids,wmputilities::DIRECTORY_OUTPUT);

			char * seqcode = wmplogger.getSequence();
			edglog(debug)<<"Storing seqcode: "<<seqcode<<endl;
			// SDJ needed variables for checking nodes
			classad::ExprTree * sdjrequirements = conf.getSDJRequirements();
			classad::ExprTree * notsdjrequirements = NULL;
			classad::ExprTree * nodeadREQ = NULL;
			if (sdjrequirements) {
				notsdjrequirements = notExprTree(sdjrequirements);
			}else{
				edglog(warning) << "Short Deadline Job not found in configuration file" << endl ;
			}
		        classad::ClassAdParser parser;
			classad::ExprTree *dagREQ=dag.hasAttribute(JDL::REQUIREMENTS)?
					parser.ParseExpression(dag.getString(JDL::REQUIREMENTS), true):
					NULL;
			string jobidstring;
			string dest_uri;
			string peekdir;
			// Storing parent OSB_BASE_DEST_URI
			string parent_dest_uri ="";
			if (dag.hasAttribute(JDL::OSB_BASE_DEST_URI)) {
				parent_dest_uri = dag.getString(JDL::OSB_BASE_DEST_URI);
			}
            
			int parentIsbSize = 0;	
	
			if (dag.hasAttribute(JDL::INPUTSB)) {
				vector<string> inputsburi = dag.getInputSandbox();
                		parentIsbSize = inputsburi.size();
            		}
	
			vector<JobIdStruct*>::iterator iter = children.begin();
			vector<JobIdStruct*>::iterator const end = children.end();
// Iterate over DAG children
			for (; iter != end; ++iter) {
				JobId subjobid = (*iter)->jobid;
				jobidstring = subjobid.toString();
				dest_uri = getDestURI(jobidstring, conf.getDefaultProtocol(),
					conf.getDefaultPort());
				edglog(debug)<<"Setting internal attributes for sub job: "
					<<jobidstring<<endl;
				edglog(debug)<<"Destination URI: "<<dest_uri<<endl;

				NodeAd nodead = dag.getNode(subjobid);
				// Adding WMPISB_BASE_URI, INPUT_SANDBOX_PATH, OUTPUT_SANDBOX_PATH  & USERPROXY
				edglog(debug)<<"Setting attribute JDL::WMPISB_BASE_URI"<<endl;
				nodead.setAttribute(JDL::WMPISB_BASE_URI, dest_uri);
			        edglog(debug)<<"Setting attribute JDLPrivate::INPUT_SANDBOX_PATH"<<endl;
				nodead.setAttribute(JDLPrivate::INPUT_SANDBOX_PATH,getInputSBDirectoryPath(jobidstring));
		        	edglog(debug)<<"Setting attribute JDLPrivate::OUTPUT_SANDBOX_PATH"<<endl;
				nodead.setAttribute(JDLPrivate::OUTPUT_SANDBOX_PATH,getOutputSBDirectoryPath(jobidstring));
		        	edglog(debug)<<"Setting attribute JDLPrivate::USERPROXY"<<endl;
		        	nodead.setAttribute(JDLPrivate::USERPROXY,getJobDelegatedProxyPath(jobidstring));
				
				if (!nodead.hasAttribute(JDL::INPUTSB) && (maxInputSandboxFiles > 0)) {

                                        if ( parentIsbSize > maxInputSandboxFiles) {
						edglog(debug)<<"The maximum number of input sandbox files is reached"<<endl;
                                                throw JobOperationException(__FILE__, __LINE__, "wmpcoreoperations::submit()",
                                                      wmputilities::WMS_OPERATION_NOT_ALLOWED, "The maximum number of input sandbox files is reached");
                                        }
                                }

                		// SDJ CHECKS
				if (sdjrequirements){
					if (nodead.hasAttribute(JDL::REQUIREMENTS)){
						nodeadREQ=nodead.delAttribute(JDL::REQUIREMENTS);
					}else{
						nodeadREQ=dagREQ; // inherit value from father
					}
					if ( nodead.hasAttribute(JDL::SHORT_DEADLINE_JOB)
				                && nodead.getBool(JDL::SHORT_DEADLINE_JOB) ){
				                // sdj conf found, sdj requested by user within JDL
			        		nodead.setAttributeExpr(JDL::REQUIREMENTS,
							appendExprTree (nodeadREQ,sdjrequirements));
				        } else {
				                // sdj conf value found NEITHER requested by JDL nor by any other case
			        		nodead.setAttributeExpr(JDL::REQUIREMENTS,
							appendExprTree (nodeadREQ,notsdjrequirements));
				        }
				}
				// Adding OutputSandboxDestURI attribute
		        	if (nodead.hasAttribute(JDL::OUTPUTSB)) {
					vector<string> osbdesturi;
					if (nodead.hasAttribute(JDL::OSB_DEST_URI)) {
						osbdesturi = wmputilities::computeOutputSBDestURI(
							nodead.getStringValue(JDL::OSB_DEST_URI),
							dest_uri);
						nodead.delAttribute(JDL::OSB_DEST_URI);
					} else if (nodead.hasAttribute(JDL::OSB_BASE_DEST_URI)) {
		        			osbdesturi = wmputilities::computeOutputSBDestURIBase(
							nodead.getStringValue(JDL::OUTPUTSB),
							nodead.getString(JDL::OSB_BASE_DEST_URI));
					} else if (!parent_dest_uri.empty()){
						osbdesturi = wmputilities::computeOutputSBDestURIBase(
							nodead.getStringValue(JDL::OUTPUTSB),
							parent_dest_uri);
					}else{
		        			osbdesturi = wmputilities::computeOutputSBDestURIBase(
							nodead.getStringValue(JDL::OUTPUTSB),
							dest_uri + "/output");
					}
					if (osbdesturi.size() != 0) {
						vector<string>::iterator iter = osbdesturi.begin();
						vector<string>::iterator const end = osbdesturi.end();
						int counter = 0;	
						for (; iter != end; ++iter) {
							counter++;
							if ( counter <= maxOutputSandboxFiles ) {
								edglog(debug)<<"Setting attribute JDL::OSB_DEST_URI"<<endl;
								nodead.addAttribute(JDL::OSB_DEST_URI, *iter);
		                			} else if (maxOutputSandboxFiles > 0) {
								edglog(debug)<<"The maximum number of output sandbox files is reached"<<endl;
								throw JobOperationException(__FILE__, __LINE__, "wmpcoreoperations::submit()",
								 wmputilities::WMS_OPERATION_NOT_ALLOWED, "The maximum number of output sandbox files is reached");
							}
						}
					}
				}
				if (nodead.hasAttribute(JDL::SHORT_DEADLINE_JOB)) {
					if (sdjrequirements) {
						if (nodead.getBool(JDL::SHORT_DEADLINE_JOB)) {
							nodead.setDefaultReq(sdjrequirements);
						} else {
							nodead.setDefaultReq(notsdjrequirements);
						}
					}
				}
				// Adding attributes for perusal functionalities
				peekdir = wmputilities::getPeekDirectoryPath(subjobid);
				if (nodead.hasAttribute(JDL::PU_FILE_ENABLE)) {
					if (nodead.getBool(JDL::PU_FILE_ENABLE)) {
						string protocol = conf.getDefaultProtocol();
						string port = (conf.getDefaultPort() != 0)
							? (":" + boost::lexical_cast<std::string>(conf.getDefaultPort()))
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

				if (nodead.hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_CHECKPOINTABLE)) {
					// CHECKPOINTABLE Jobs ARE DEPRECATED!!!
					string msg = "Checkpointable Jobs are Deprecated";
					edglog(error)<< msg << endl;
					throw JobOperationException(__FILE__, __LINE__,
						"submit()", wmputilities::WMS_JDL_PARSING,msg);
				}
				dag.replaceNode(subjobid, nodead);
			}
// END Iterate over DAG children
			
			//TBD Change getSubmissionStrings() with a better method when coded??
			//Done here, Not done during register any more.
			dag.getSubmissionStrings();
			
 			iter = children.begin();

			if (maxInputSandboxFiles > 0) {
                        	for (; iter != end; ++iter) {
                                	JobId subjobid = (*iter)->jobid;
					NodeAd nodead = dag.getNode(subjobid);
					
					if (nodead.hasAttribute(JDL::INPUTSB)) {
		                        	
						vector<string> inputsburi = nodead.getStringValue(JDL::INPUTSB);
        		                	int inputsbSize = inputsburi.size();
        	                		
						if ( inputsbSize > maxInputSandboxFiles) {
							edglog(debug)<<"The maximum number of input sandbox files is reached"<<endl;
                        				throw JobOperationException(__FILE__, __LINE__, "wmpcoreoperations::submit()",
                	        		      	 wmputilities::WMS_OPERATION_NOT_ALLOWED, "The maximum number of input sandbox files is reached");
        	                		}
	                		}	
				}
			}

			wmplogger.setLoggingJob(parentjobid.toString(), seqcode);
			// TODO moved after operation request written to FL
			//Registering subjobs
			//vector<string> jobids = wmplogger.generateSubjobsIds(dag.size()); //Filling wmplogger subjobs
			string flagfile = wmputilities::getJobDirectoryPath(*jid)
				+ FILE_SEPARATOR + FLAG_FILE_REGISTER_SUBJOBS;
			if (!wmputilities::fileExists(flagfile)) {
				wmplogger.registerSubJobs(&dag, wmplogger.m_subjobs);
				edglog(debug)<<"registerSubJobs OK, writing flag file: "
					<<flagfile<<endl;
				wmputilities::setFlagFile(flagfile, true);
			} else {
				edglog(debug)<<"Flag file "<<flagfile<<" already exists. "
					"Skipping registerSubJobs..."<<endl;
			}
			// Looking for Zipped ISB
			if (dag.hasAttribute(JDLPrivate::ZIPPED_ISB)) {
				//Setting file system for subjobs
				/***setSubjobFileSystem(auth, parentjobid.toString(), jobids);***/
				string flagfile = wmputilities::getJobDirectoryPath(*jid) + FILE_SEPARATOR
					+ FLAG_FILE_UNZIP;
				if (!wmputilities::fileExists(flagfile)) {
					//Uncompressing zip file
					vector<string> files = dag.getAttribute(ExpDagAd::ZIPPED_ISB);
					string targetdir = getenv(DOCUMENT_ROOT);
					string jobpath = wmputilities::getInputSBDirectoryPath(*jid)
						+ FILE_SEPARATOR;
					for (unsigned int i = 0; i < files.size(); i++) {
						edglog(debug)<<"Uncompressing zip file: "<<files[i]<<endl;
						edglog(debug)<<"Absolute path: "<<jobpath + files[i]<<endl;
						edglog(debug)<<"Target directory: "<<targetdir<<endl;
						//wmputilities::uncompressFile(jobpath + files[i], targetdir);
						// TBD Call the method with a vector of file
						wmputilities::untarFile(jobpath + files[i], targetdir,
							auth->getUserId(), auth->getUserGroup());
					}
					wmputilities::setFlagFile(flagfile, true);
				} else {
					edglog(debug)<<"Flag file "<<flagfile
						<<" already exists. Skipping untarFile..."<<endl;
				}
			}
			// \[ Do not separate
			// Inserting sequence code
			edglog(debug)<<"Setting attribute JDL::LB_SEQUENCE_CODE"<<endl;
			if (dag.hasAttribute(JDL::LB_SEQUENCE_CODE)) {
				dag.removeAttribute(JDL::LB_SEQUENCE_CODE);
			}
			dag.setReserved(JDL::LB_SEQUENCE_CODE, string(wmplogger.getSequence()));
			// jdltostart MUST contain last seqcode
			jdltostart = dag.toString();
			jdlpath = wmputilities::getJobJDLToStartPath(*jid);
			// /_
		}
// ==========   END TYPE is a DAG   ==========
		// Perform ADDITIONAL chekcs
		string reason;
		if (wmplogger.isAborted(reason)) {
			throw JobOperationException(__FILE__, __LINE__,
				"submit()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"the job has been aborted: " + reason);
		}
		
		string filequeue = configuration::Configuration::instance()->wm()->input();
		boost::details::pool::singleton_default<WMP2WM>::instance()
			.init(filequeue, &wmplogger);
		
		boost::details::pool::singleton_default<WMP2WM>::instance()
			.submit(jdltostart, jdlpath);
		
		// Writing started jdl
		wmputilities::writeTextFile(wmputilities::getJobJDLToStartPath(*jid),
			jdltostart);
			
		// Moving JDL to start file to JDL started file
		if (rename(wmputilities::getJobJDLToStartPath(*jid).c_str(),
				wmputilities::getJobJDLStartedPath(*jid).c_str())) {
		}
		
		if (!issubmit) {
		edglog(debug)<<"Removing lock..."<<std::endl;
		wmputilities::operationUnlock(fd);
		}

	} catch (RequestAdException &exc) {
		// Something has gone bad!
		edglog(error)<<"Error while checking JDL: "<< exc.what() <<std::endl;
		wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_FAIL,
			exc.what(), true, true, filelist_global.c_str(),
			wmputilities::getJobJDLToStartPath(*jid).c_str());
		wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ABORT,
				exc.what(), true, true);
		if (!issubmit) {
			edglog(debug)<<"Removing lock..."<<std::endl;
			wmputilities::operationUnlock(fd);
		}
		if (!conf.getAsyncJobStart()) {
			exc.push_back(__FILE__, __LINE__, "submit()");
			throw exc;
		}
	} catch (Exception &exc) {
		edglog(debug)<<"Logging LOG_ENQUEUE_FAIL"<<std::endl;
		wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_FAIL,
			exc.what(), true, true, filelist_global.c_str(),
			wmputilities::getJobJDLToStartPath(*jid).c_str());
		
		if (!issubmit) {
			edglog(debug)<<"Removing lock..."<<std::endl;
			wmputilities::operationUnlock(fd);
		}
		
		if (!conf.getAsyncJobStart()) {
			exc.push_back(__FILE__, __LINE__, "submit()");
			throw exc;
		}
	} catch (exception &ex) {
		edglog(debug)<<"Logging LOG_ENQUEUE_FAIL"<<std::endl;
		wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_FAIL,
			ex.what(), true, true, filelist_global.c_str(),
			wmputilities::getJobJDLToStartPath(*jid).c_str());
		
		if (!issubmit) {
			edglog(debug)<<"Removing lock..."<<std::endl;
			wmputilities::operationUnlock(fd);
		}
	    
		if (!conf.getAsyncJobStart()) {
			Exception exc(__FILE__, __LINE__, "submit()", 0,
				"Standard exception: " + std::string(ex.what())); 
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
	edglog_fn("wmpcoreoperations::jobSubmit");
	// log Remote host info, call load script file,checkConfiguration, setGlobalSandboxDir
	initWMProxyOperation("jobSubmit");
	
	// Checking delegation id
	edglog(debug)<<"Delegation ID: "<<delegation_id<<endl;
	
#ifndef GRST_VERSION
	if (delegation_id == "") {
		edglog(error)<<"Empty delegation id not allowed with delegation 1"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"jobRegister()", wmputilities::WMS_INVALID_ARGUMENT,
			"Delegation id not valid");
	}
#endif

	edglog(debug)<<"JDL to Submit:\n"<<jdl<<endl;


	// complicate checkSecurity: TODO
	// - delegatedproxy, delegatedproxyfqan, auth  needed/reused
	//** Authorizing user
	edglog(debug)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = WMPDelegation::getDelegatedProxyPath(delegation_id);
	edglog(debug)<<"Delegated proxy: "<<delegatedproxy<<endl;

	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	string delegatedproxyfqan = vomsproxy.getDefaultFQAN();
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(delegatedproxyfqan);
	} else {
		auth->authorize();
	}

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"
			<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpcoreoperations::jobSubmit()", wmputilities::WMS_AUTHORIZATION_ERROR,
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}

	// Checking proxy validity
	authorizer::WMPAuthorizer::checkProxy(delegatedproxy);
	// Registering the job for submission
	jobRegisterResponse jobRegister_response;
	pair<string, string> reginfo = jobregister(jobRegister_response, jdl,
		delegation_id, delegatedproxy, delegatedproxyfqan, auth);
		

	// Getting job identifier from register response
	string jobid = reginfo.first;
	edglog(debug)<<"Starting registered job: "<<jobid<<endl;

	// Starting job submission
	JobId *jid = new JobId(jobid);
	
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	
	// Getting delegated proxy inside job directory
	string proxy(wmputilities::getJobDelegatedProxyPath(*jid));
	edglog(debug)<<"Job delegated proxy: "<<proxy<<endl;
	
	// Setting user proxy
	wmplogger.setUserProxy(proxy);
	
	jobSubmit_response.jobIdStruct = jobRegister_response.jobIdStruct;

	// Filling answer structure to return to user
	ns1__JobIdStructType *job_id_struct = new ns1__JobIdStructType();
	job_id_struct->id = jobSubmit_response.jobIdStruct->id;
	job_id_struct->name = jobSubmit_response.jobIdStruct->name;
	job_id_struct->path = new string(getJobInputSBRelativePath(job_id_struct->id));
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
	edglog_fn("wmpcoreoperations::jobCancel");
	// log Remote host info, call load script file,checkConfiguration, setGlobalSandboxDir
	initWMProxyOperation("jobCancel");
	edglog(debug)<<"Operation requested for job: "<<job_id<<endl;

	JobId *jid = new JobId(job_id);
	// wmpcommon perform all security checks
	checkSecurity(jid);
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);

	string jobpath = wmputilities::getJobDirectoryPath(*jid);
	// Initializing logger
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());

	// Setting user proxy
	wmplogger.setUserProxy(delegatedproxy);
	// Getting job status to check if cancellation is possible
	JobStatus status = wmplogger.getStatus(false);

	if (status.getValBool(JobStatus::CANCELLING)) {
		edglog(error)<<"Cancel has already been requested"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobCancel()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"Cancel has already been requested");
	}
	// Getting type from jdl
	string seqcode = "";
	JobId parentjid (status.getValJobId(JobStatus::PARENT_JOB));

	if (parentjid.isSet()) {
		string parentjdl = wmputilities::readTextFile(wmputilities::getJobJDLExistingStartPath(parentjid));
		/* bug #19652 fix: WMProxy tries to purge DAG node upon cancellation
		* DAG node cancellation was forbidden in the past
		*/
	} else {
		// Getting sequence code from jdl
		Ad ad;
		ad.fromFile(wmputilities::getJobJDLExistingStartPath(*jid));
		if (ad.hasAttribute(JDL::LB_SEQUENCE_CODE)) {
			seqcode = ad.getString(JDL::LB_SEQUENCE_CODE);
		}
	}
	if (seqcode == "") {
		seqcode = wmplogger.getLastEventSeqCode();
	}
	// Setting user proxy  // TODO why twice? (see 30 lines above) temporarily commented
	// wmplogger.setUserProxy(delegatedproxy);

	edglog(debug)<<"Seqcode: "<<seqcode<<endl;
	if (seqcode != "") {
		wmplogger.setSequenceCode(seqcode);
		wmplogger.incrementSequenceCode();
	}

	string filequeue = configuration::Configuration::instance()->wm()->input();
	boost::details::pool::singleton_default<WMP2WM>::instance()
		.init(filequeue, &wmplogger);
					
	string envsandboxpath;
	
	switch (status.status) {
		case JobStatus::SUBMITTED: //TBD check this state with call to LBProxy (use events instead of status)
			// The register of the job has been done
			if (  !parentjid.isSet()  ) {
				// SUBMITTED dag/collection/parametric/job (not a node)
				// #19652 fix: Dag nodes cannot be purged locally,
				// because the request have not reached WM
				edglog(debug)<<"Trying to log sync ABORT..."<<endl;
				wmplogger.setSequenceCode(EDG_WLL_SEQ_ABORT);
				if (!wmplogger.logAbortEventSync("Cancelled by user")) {
					// If log fails no jobPurge is performed
					// jobPurge would find state different from ABORT and will fail
					if (!wmputilities::isOperationLocked(
							wmputilities::getJobStartLockFilePath(*jid))) {
						// purge is done if jobStart is not in progress
						edglog(debug)<<"jobStart operation is not in progress"<<endl;
						edglog(debug)<<"Log has succeded, calling purge method..."<<endl;
						jobPurgeResponse jobPurge_response;
						jobpurge(jobPurge_response, jid, false);
					} else {
						edglog(debug)<<"jobStart operation is in progress, "
							"skipping jobPurge"<<endl;
					}
				} else {
					edglog(debug)<<"Log has failed, purge method will not be called"<<endl;
				}

				//TBC should I check if MyProxyServer was present in jdl??
				edglog(debug)<<"Unregistering Proxy renewal..."<<endl;
				wmplogger.unregisterProxyRenewal();
				break;
			} else{
				// SUBMITTED node, continuing with
				// Normal procedure (do not perform purge)
				edglog(debug)<<"SUBMITTED node of a dag: purge not performed"<<endl;
			}
		case JobStatus::WAITING:
		case JobStatus::READY:
		case JobStatus::SCHEDULED:
		case JobStatus::RUNNING:
			
			boost::details::pool::singleton_default<WMP2WM>::instance()
				.cancel(jid->toString(), string(wmplogger.getSequence()));
			
			wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_CANCEL,
				"Cancelled by user", true, true);
			break;
		case JobStatus::DONE:
			// If the job is DONE, then cancellation is allowed only if
			// DONE_CODE = DONE_CODE_FAILED (resubmission is possible)
			if (status.getValInt(JobStatus::DONE_CODE)
					== JobStatus::DONE_CODE_FAILED) {
				
				boost::details::pool::singleton_default<WMP2WM>::instance()
					.cancel(jid->toString(), string(wmplogger.getSequence()));

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
			edglog(debug)<<"Job status does not allow job cancel"<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"jobCancel()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"job status doesn't allow job cancel");
			break;
  	}
  	
  	delete jid;

 	GLITE_STACK_CATCH();
}
/** Called by jobListMatch */
int 
listmatch(jobListMatchResponse &jobListMatch_response, const string &jdl,
	const string &delegation_id, const string &delegatedproxyfqan)
{
	GLITE_STACK_TRY("listmatch");
	edglog_fn("wmpcoreoperations::listmatch");

	int result = 0;
	int type = getType(jdl);
	if (type == TYPE_JOB) {
		JobAd *ad = new JobAd(jdl);
		ad->setLocalAccess(false);
	
		// Getting delegated proxy from SSL Proxy cache
		string delegatedproxy = WMPDelegation::getDelegatedProxyPath(delegation_id);
		edglog(debug)<<"Delegated proxy: "<<delegatedproxy<<endl;
	
		// Setting VIRTUAL_ORGANISATION attribute
		if (!ad->hasAttribute(JDL::VIRTUAL_ORGANISATION)) {
			edglog(debug)<<"Setting attribute JDL::VIRTUAL_ORGANISATION"<<endl;
			ad->setAttribute(JDL::VIRTUAL_ORGANISATION, wmputilities::getEnvVO());
		}
		
		if (delegatedproxyfqan != "") {
			edglog(debug)<<"Setting attribute JDLPrivate::VOMS_FQAN"<<endl;
			if (ad->hasAttribute(JDLPrivate::VOMS_FQAN)) {
				ad->delAttribute(JDLPrivate::VOMS_FQAN);
			}
			ad->setAttribute(JDLPrivate::VOMS_FQAN, delegatedproxyfqan);
		}
		
		if (ad->hasAttribute(JDL::CERT_SUBJ)) {
			ad->delAttribute(JDL::CERT_SUBJ);
		}
		edglog(debug)<<"Setting attribute JDL::CERT_SUBJ"<<endl;
		ad->setAttribute(JDL::CERT_SUBJ, 
			wmputilities::convertDNEMailAddress(wmputilities::getUserDN()));
		
		// \/
		// Adding fake JDL::WMPISB_BASE_URI attribute to pass check (toSubmissionString)
		if (ad->hasAttribute(JDL::WMPISB_BASE_URI)) {
			ad->delAttribute(JDL::WMPISB_BASE_URI);	
		}
		edglog(debug)<<"Setting attribute JDL::WMPISB_BASE_URI"<<endl;
		ad->setAttribute(JDL::WMPISB_BASE_URI, "protocol://address");
		
		ad->check();
		
		// Removing fake
		ad->delAttribute(JDL::WMPISB_BASE_URI);
		// /_
		
		WMPEventLogger wmplogger(wmputilities::getEndpoint());
		wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());

		string filequeue = configuration::Configuration::instance()->wm()->input();
		boost::details::pool::singleton_default<WMP2WM>::instance()
			.init(filequeue, &wmplogger);

		wmputilities::createSuidDirectory(conf.getListMatchRootPath());
		boost::details::pool::singleton_default<WMP2WM>::instance()
			.match(ad->toString(), conf.getListMatchRootPath(), delegatedproxy,
				&jobListMatch_response);
		
		delete ad;
		
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
	edglog_fn("wmpcoreoperations::jobListMatch");
	// log Remote host info, call load script file,checkConfiguration, setGlobalSandboxDir
	initWMProxyOperation("jobListMatch");
	
	// Checking delegation id
	edglog(debug)<<"Delegation ID: "<<delegation_id<<endl;
	if (delegation_id == "") {
		edglog(error)<<"Provided delegation ID not valid"<<endl;
		throw ProxyOperationException(__FILE__, __LINE__,
			"jobListMatch()", wmputilities::WMS_INVALID_ARGUMENT,
			"Delegation id not valid");
	}

	edglog(debug)<<"JDL to find Match:\n"<<jdl<<endl;

	// Authorizing user
	edglog(debug)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = new authorizer::WMPAuthorizer();

	// Getting delegated proxy from SSL Proxy cache
	string delegatedproxy = WMPDelegation::getDelegatedProxyPath(delegation_id);
	edglog(debug)<<"Delegated proxy: "<<delegatedproxy<<endl;

	string delegatedproxyfqan = "";
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		delegatedproxyfqan = vomsproxy.getDefaultFQAN();
		auth->authorize(delegatedproxyfqan);
	} else {
		auth->authorize();
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"
			<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpcoreoperations::jobListMatch()", wmputilities::WMS_AUTHORIZATION_ERROR,
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}

	// Checking proxy validity
	authorizer::WMPAuthorizer::checkProxy(delegatedproxy);
	listmatch(jobListMatch_response, jdl, delegation_id, delegatedproxyfqan);
	GLITE_STACK_CATCH();
}

void
jobpurge(jobPurgeResponse &jobPurge_response, JobId *jobid, bool checkstate)
{
	GLITE_STACK_TRY("jobpurge()");
	edglog_fn("wmpcoreoperations::jobpurge");
	
	edglog(debug)<<"CheckState: "<<(checkstate ? "True" : "False")<<endl;
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	// Initializing logger
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jobid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());

	// Setting user proxy
	wmplogger.setUserProxy(delegatedproxy);
	
	// Getting job status to check if purge is possible
	JobStatus status = wmplogger.getStatus(false);
	// dag nodes have to be forcingly purged  (cfr bug #27074)
	bool forcePurge=false ;
        if (((JobId) status.getValJobId(JobStatus::PARENT_JOB)).isSet()) {
		string msg = "Forcing purge of subjob. The parent is: "
			+ status.getValJobId(JobStatus::PARENT_JOB).toString();
		edglog(error)<<msg<<endl;
		forcePurge=true;
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
			wmsutilities::oedgstrstream s;
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
					throw JobOperationException(__FILE__, __LINE__,
						"jobpurge()", wmputilities::WMS_ENVIRONMENT_ERROR,
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

		if (!wmputilities::doPurge(jobid->toString(), forcePurge)) {
			edglog(severe)<<"Unable to complete job purge"<<endl;
			if (checkstate) {
				throw JobOperationException(__FILE__, __LINE__,
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
	edglog_fn("wmpcoreoperations::jobPurge");
	// log Remote host info, call load script file,checkConfiguration, setGlobalSandboxDir
	initWMProxyOperation("jobPurge");
	edglog(debug)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(debug)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Checking job existency (if the job directory doesn't exist:
	// The job has not been registered from this Workload Manager Proxy
	// or it has been purged)
	checkJobDirectoryExistence(*jobid);
	
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
	
	if (wmputilities::isOperationLocked(
			wmputilities::getGetOutputFileListLockFilePath(*jobid))) {
		edglog(debug)<<"operation aborted: a getOutputFileList on the same job "
				"has been requested"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobPurge()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"operation aborted: a getOutputFileList on the same job has been "
				"requested");
	}
	
	jobpurge(jobPurge_response, jobid);
	
	delete jobid;
	
	edglog(debug) << "Job purged successfully" << endl;
	GLITE_STACK_CATCH();
}
