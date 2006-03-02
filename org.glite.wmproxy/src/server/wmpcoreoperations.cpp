/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpcoreoperations.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

// Boost
#include <boost/pool/detail/singleton.hpp>

#include "wmp2wm.h"

#include <fstream>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h> // flock

// Boost
#include <boost/lexical_cast.hpp>

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

// Authorizer
#include "authorizer/wmpauthorizer.h"
#include "authorizer/wmpgaclmanager.h"
#include "authorizer/wmpvomsauthz.h"

// WMPManager
//#include "wmpmanager.h"
//#include "wmp2wm.h"

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
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"

#include "glite/wms/common/utilities/edgstrstream.h"

#include "glite/wms/partitioner/Partitioner.h"

#include "libtar.h"

// Global variables for configuration
extern WMProxyConfiguration conf;
extern std::string filelist_global;

// DONE job output file
const std::string MARADONA_FILE = "Maradona.output";

// Perusal functionality
const std::string PERUSAL_FILE_2_PEEK_NAME = "files2peek";
const int DEFAULT_PERUSAL_TIME_INTERVAL = 10; // seconds

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
using namespace glite::wms::jdl; // DagAd, AdConverter
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
	job_id_struct->path = new string(getJobInputSBRelativePath(stringjid));
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
	
	wmplogger.logUserTag(JDL::LB_SEQUENCE_CODE, string(wmplogger.getSequence()));

	// Creating job identifier structure to return to the caller
	JobIdStructType *job_id_struct = new JobIdStructType();
	JobIdStruct job_struct = dag->getJobIdStruct();
	job_id_struct->id = job_struct.jobid.toString(); // should be equal to: jid->toString()
	job_id_struct->name = job_struct.nodeName;
	job_id_struct->path = new string(getJobInputSBRelativePath(job_id_struct->id));
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
			// Starting the job
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
			
			// Inserting sequence code
			edglog(debug)<<"Setting attribute JDL::LB_SEQUENCE_CODE"<<endl;
			if (jad->hasAttribute(JDL::LB_SEQUENCE_CODE)) {
				jad->delAttribute(JDL::LB_SEQUENCE_CODE);
			}
			jad->addAttribute(JDL::LB_SEQUENCE_CODE, string(wmplogger.getSequence()));
		
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
					} else if (nodead.hasAttribute(JDL::OSB_BASE_DEST_URI)) {
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
			
			// Inserting sequence code
			edglog(debug)<<"Setting attribute JDL::LB_SEQUENCE_CODE"<<endl;
			if (dag->hasAttribute(JDL::LB_SEQUENCE_CODE)) {
				dag->removeAttribute(JDL::LB_SEQUENCE_CODE);
			}
			dag->setReserved(JDL::LB_SEQUENCE_CODE, string(wmplogger.getSequence()));
			
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
		
		string filequeue = configuration::Configuration::instance()->wm()->input();
		boost::details::pool::singleton_default<WMP2WM>::instance()
			.init(filequeue, &wmplogger);
		
		boost::details::pool::singleton_default<WMP2WM>::instance()
			.submit(jdltostart);
		
		/*if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
			edglog(severe)<<"Error in runCommand: "<<wmp_fault.message<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"submit()", wmp_fault.code, wmp_fault.message);
		}*/
		
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
	//Ad *ad = new Ad();
	//ad->fromFile(wmputilities::getJobJDLToStartPath(*jid));
	Ad *ad = new Ad(status.getValString(JobStatus::JDL));
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
	
	edglog(debug)<<"Seqcode: "<<seqcode<<endl;
	if (seqcode != "") {
		wmplogger.setSequenceCode(seqcode);
		wmplogger.incrementSequenceCode();
	}
	
	string filequeue = configuration::Configuration::instance()->wm()->input();
	boost::details::pool::singleton_default<WMP2WM>::instance()
		.init(filequeue, &wmplogger);
					
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
			
			boost::details::pool::singleton_default<WMP2WM>::instance()
				.cancel(jid->toString(), string(wmplogger.getSequence()));
			
			/*if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
				edglog(severe)<<"Error in runCommand: " + wmp_fault.message<<endl;
				throw JobOperationException(__FILE__, __LINE__,
					"jobCancel()", wmp_fault.code, wmp_fault.message);
			}*/
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
				
				/*if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
					edglog(severe)<<"Error in runCommand: " + wmp_fault.message<<endl;
					throw JobOperationException(__FILE__, __LINE__,
						"jobCancel()", wmp_fault.code, wmp_fault.message);
				}*/
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
		
		string filequeue = configuration::Configuration::instance()->wm()->input();
		boost::details::pool::singleton_default<WMP2WM>::instance()
			.init(filequeue, &wmplogger);
		
		boost::details::pool::singleton_default<WMP2WM>::instance()
			.match(ad->toString(), conf.getListMatchRootPath(), delegatedproxy);
		
		delete ad;
		
		/*if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
			edglog(severe)<<"Error in runCommand: "<<wmp_fault.message<<endl;
			throw JobOperationException(__FILE__, __LINE__, "listmatch()",
				wmp_fault.code, wmp_fault.message);
		}*/
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
