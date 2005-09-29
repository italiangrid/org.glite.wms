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
#include <sys/stat.h>
#include <sys/types.h>

// Boost
#include <boost/lexical_cast.hpp>

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
	

// WMProxy software version
const std::string WMP_MAJOR_VERSION = "2";
const std::string WMP_MINOR_VERSION = "0";
const std::string WMP_RELEASE_VERSION = "0";
const std::string WMP_POINT_VERSION = ".";
const std::string WMP_VERSION = WMP_MAJOR_VERSION
	+ WMP_POINT_VERSION + WMP_MINOR_VERSION
	+ WMP_POINT_VERSION + WMP_RELEASE_VERSION;


const std::string MARADONA_FILE = "Maradona.output";
const std::string PERUSAL_FILE_2_PEEK_NAME = "files2peek";
const std::string TEMP_PERUSAL_FILE_NAME = "tempperusalfile";
const std::string DISABLED_PEEK_FLAG_FILE = ".disabledpeek";
const std::string EXTERNAL_PEEK_FLAG_FILE = ".externalpeek";
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

const std::string CURRENT_STEP_DEF_VALUE = "1";

// Defining File Separator
#ifdef WIN 
   // Windows File Separator 
   const std::string FILE_SEPARATOR = "\\"; 
#else 
   // Linux File Separator 
   const std::string FILE_SEPARATOR = "/";
#endif 

const std::string NOT_SAME_WMPROXY = "The job has not been registered from this "
	"Workload Manager Proxy server (or it has been purged)";


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

void submit(const string &jdl, JobId *jid);

int listmatch(jobListMatchResponse &jobListMatch_response, const string &jdl,
	const string &delegation_id);

void jobpurge(jobPurgeResponse &jobPurge_response, JobId *jid, bool checkstate
	= true);



/**
 * Converts JobIdStruct vector into a JobIdStructType vector pointer
 */
vector<JobIdStructType*> *
convertJobIdStruct(vector<JobIdStruct*> &job_struct)
{
	GLITE_STACK_TRY("convertJobIdStruct()");
	
	vector<JobIdStructType*> *graph_struct_type = new vector<JobIdStructType*>;
	JobIdStructType *graph_struct = NULL;
	for (unsigned int i = 0; i < job_struct.size(); i++) {
		graph_struct = new JobIdStructType();
		graph_struct->id = job_struct[i]->jobid.toString(); // should be equal to: jid->toString()
		graph_struct->name = job_struct[i]->nodeName;
		graph_struct->childrenJob = convertJobIdStruct(job_struct[i]->children);
		graph_struct_type->push_back(graph_struct);
	}
	return graph_struct_type;
	
	GLITE_STACK_CATCH();
}

/**
 * Converts GraphStructType vector pointer into a NodeStruct vector pointer
 */
vector<NodeStruct*> 
convertGraphStructTypeToNodeStructVector(vector<GraphStructType*> *graph_struct)
{
	GLITE_STACK_TRY("convertGraphStructTypeToNodeStructVector()");
	
	vector<NodeStruct*> return_node_struct;
	NodeStruct *element = NULL;
	for (unsigned int i = 0; i < graph_struct->size(); i++) {
		element = new NodeStruct();
		element->name = (*graph_struct)[i]->name;
		if ((*graph_struct)[i]->childrenJob) {
			element->childrenNodes = convertGraphStructTypeToNodeStructVector(
				(*graph_struct)[i]->childrenJob);
		} else {
			element->childrenNodes = *(new vector<NodeStruct*>);
		}
		return_node_struct.push_back(element);
	}
	return return_node_struct;
	
	GLITE_STACK_CATCH();
}

/**
 * Converts GraphStructType into a NodeStruct
 */
NodeStruct
convertGraphStructTypeToNodeStruct(GraphStructType graph_struct)
{
	GLITE_STACK_TRY("convertGraphStructTypeToNodeStruct()")
	
	NodeStruct return_node_struct;
	return_node_struct.name = graph_struct.name;
	if (graph_struct.childrenJob) {
		return_node_struct.childrenNodes =
			convertGraphStructTypeToNodeStructVector(graph_struct.childrenJob);
	}
	return return_node_struct;
	
	GLITE_STACK_CATCH();
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

/**
 * Converts a JobTypeList type to an int value representing the type of the job
 */
int
convertJobTypeListToInt(JobTypeList job_type_list)
{
	GLITE_STACK_TRY("convertJobTypeListToInt()");
	edglog_fn("wmpoperations::convertJobTypeListToInt");
	
	int type = AdConverter::ADCONV_JOBTYPE_NORMAL;
	for (unsigned int i = 0; i < job_type_list.jobType->size(); i++) {
		switch ((*job_type_list.jobType)[i]) {
			case WMS_PARAMETRIC:
				type |= AdConverter::ADCONV_JOBTYPE_PARAMETRIC;
				edglog(info)<<"Job Type: Parametric"<<endl;
				break;
			case WMS_INTERACTIVE:
				type |= AdConverter::ADCONV_JOBTYPE_INTERACTIVE;
				edglog(info)<<"Job Type: Interactive"<<endl;
				break;
			case WMS_MPI:
				type |= AdConverter::ADCONV_JOBTYPE_MPICH;
				edglog(info)<<"Job Type: MPI"<<endl;
				break;
			case WMS_PARTITIONABLE:
				type |= AdConverter::ADCONV_JOBTYPE_PARTITIONABLE;
				edglog(info)<<"Job Type: Partitionable"<<endl;
				break;
			case WMS_CHECKPOINTABLE:
				type |= AdConverter::ADCONV_JOBTYPE_CHECKPOINTABLE;
				edglog(info)<<"Job Type: Checkpointable"<<endl;
				break;
			default:
				break;
		}
	}
	edglog(debug)<<"Final type value: "<<type<<endl;
	return type;
	
	GLITE_STACK_CATCH();
}



//
// WM Web Service available operations
//

// To get more infomation see WM service wsdl file
void
getVersion(getVersionResponse &getVersion_response)
{
	GLITE_STACK_TRY("getVersion()");
	edglog_fn("wmpoperations::getVersion");
	
	getVersion_response.version = WMP_VERSION;
	edglog(info)<<"Version retrieved: "<<getVersion_response.version<<endl;
	
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
	
	try {
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN());
	} catch (NotAVOMSProxyException &navp) {
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
setJobFileSystem(authorizer::WMPAuthorizer *auth, const string &delegatedproxy,
	const string &jobid, vector<string> &jobids, char * renewalproxy = NULL)
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
	      	edglog(critical)
	      		<<"Unable to create symbolic link to renewal proxy:\n\t"
	      		<<proxy<<"\n"<<strerror(errno)<<endl;
	      
	      	throw FileSystemException(__FILE__, __LINE__,
				"setJobFileSystem()", wmputilities::WMS_FATAL,
				"Unable to create symbolic link to renewal proxy\n(please "
				"contact server administrator)");
	    }
	} else {
		// Copying delegated Proxy to destination URI
		wmputilities::fileCopy(delegatedproxy, proxy);
	}
	
	// Creating a copy of the Proxy
	string proxybak = wmputilities::getJobDelegatedProxyPathBak(jobid);
	wmputilities::fileCopy(delegatedproxy, proxybak);
	
	// Creating sub jobs directories
	if (jobids.size() != 0) {
		edglog(debug)<<"Creating sub job directories for job:\n"<<jobid<<endl;
		int outcome = wmputilities::managedir(document_root, userid, jobdiruserid, jobids);
		if (outcome) {
			edglog(critical)
				<<"Unable to create sub jobs local directories for job:\n\t"
				<<jobid<<"\n"<<strerror(errno)<<" code: "<<errno<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"setJobFileSystem()", wmputilities::WMS_FATAL,
				"Unable to create sub jobs local directories\n(please contact "
				"server administrator)");
		} else {
			string link;
			string linkbak;
			for (unsigned int i = 0; i < jobids.size(); i++) {
				link = wmputilities::getJobDelegatedProxyPath(jobids[i]);
				edglog(debug)<<"Creating proxy symbolic link for: "
					<<jobids[i]<<endl;
				if (symlink(proxy.c_str(), link.c_str())) {
			      	edglog(critical)
			      		<<"Unable to create symbolic link to proxy file:\n\t"
			      		<<link<<"\n"<<strerror(errno)<<endl;
			      
			      	throw FileSystemException(__FILE__, __LINE__,
						"setJobFileSystem()", wmputilities::WMS_FATAL,
						"Unable to create symbolic link to proxy file\n"
							"(please contact server administrator)");
			    }
			    
			    linkbak = wmputilities::getJobDelegatedProxyPathBak(jobids[i]);
			    edglog(debug)<<"Creating backup proxy symbolic link for: "
					<<jobids[i]<<endl;
			    if (symlink(proxybak.c_str(), linkbak.c_str())) {
			      	edglog(critical)
			      		<<"Unable to create symbolic link to backup proxy file:\n\t"
			      		<<linkbak<<"\n"<<strerror(errno)<<endl;
			      
			      	throw FileSystemException(__FILE__, __LINE__,
						"setJobFileSystem()", wmputilities::WMS_FATAL,
						"Unable to create symbolic link to backup proxy file\n"
							"(please contact server administrator)");
			    }
			}
		}
		for (unsigned int i = 0; i < jobids.size(); i++) {
			authorizer::WMPAuthorizer::setJobGacl(jobids[i]);
		}
	}
	// Creating gacl file in the private job directory
	authorizer::WMPAuthorizer::setJobGacl(jobid);

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
		
		for (unsigned int i = 0; i < osbdesturi.size(); i++) {
			edglog(debug)<<"OutputSBDestURI: "<<osbdesturi[i]<<endl;
			jad->addAttribute(JDL::OSB_DEST_URI, osbdesturi[i]);
		}
	}

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
	
	WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
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
	string defaultprotocol = conf.getDefaultProtocol();
	int defaultport = conf.getDefaultPort();
	string dest_uri = wmputilities::getDestURI(stringjid, defaultprotocol,
		defaultport);
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
		defaultprotocol, defaultport);
	wmplogger.setLBProxy(conf.isLBProxyAvailable());
			
	// Setting user proxy
	if (wmplogger.setUserProxy(delegatedproxy)) {
			edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
			throw AuthenticationException(__FILE__, __LINE__,
				"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
				"Unable to set User Proxy for LB context");
	}
	
	// Registering the job
	jad->check();
	wmplogger.registerJob(jad);
	
	// Registering for Proxy renewal
	char * renewalproxy = NULL;
	if (jad->hasAttribute(JDL::MYPROXY)) {
		edglog(debug)<<"Registering Proxy renewal..."<<endl;
		renewalproxy = wmplogger.registerProxyRenewal(delegatedproxy,
			(jad->getStringValue(JDL::MYPROXY))[0]);
	}
	
	// Creating private job directory with delegated Proxy
	vector<string> jobids;
	setJobFileSystem(auth, delegatedproxy, stringjid, jobids, renewalproxy);
	
	delete jid;

	// Logging delegation id & original jdl
	edglog(debug)<<"Logging user tag JDL::DELEGATION_ID..."<<endl;
	wmplogger.logUserTag(JDL::DELEGATION_ID, delegation_id);
	edglog(debug)<<"Logging user tag JDL::JDL_ORIGINAL..."<<endl;
	wmplogger.logUserTag(JDL::JDL_ORIGINAL, jdl);
	
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
	/*if (dag->hasAttribute(JDL::WMPISB_BASE_URI)) {
		dag->removeAttribute(JDL::WMPISB_BASE_URI);
	}*/
	dag->setReserved(JDL::WMPISB_BASE_URI, dest_uri);
	
	// Adding INPUT_SANDBOX_PATH attribute
	edglog(debug)<<"Setting attribute JDLPrivate::INPUT_SANDBOX_PATH"<<endl;
	/*if (dag->hasAttribute(JDLPrivate::INPUT_SANDBOX_PATH)) {
		dag->removeAttribute(JDLPrivate::INPUT_SANDBOX_PATH);
	}*/
	dag->setReserved(JDLPrivate::INPUT_SANDBOX_PATH,
		wmputilities::getInputSBDirectoryPath(*jid));
	
	// Adding Proxy attributes
	edglog(debug)<<"Setting attribute JDL::CERT_SUBJ"<<endl;
	/*if (dag->hasAttribute(JDL::CERT_SUBJ)) {
		dag->removeAttribute(JDL::CERT_SUBJ);
	}*/
	dag->setReserved(JDL::CERT_SUBJ,
		wmputilities::convertDNEMailAddress(wmputilities::getUserDN()));
	
	edglog(debug)<<"Setting attribute JDLPrivate::USERPROXY"<<endl;
	/*if (dag->hasAttribute(JDLPrivate::USERPROXY)) {
		dag->removeAttribute(JDLPrivate::USERPROXY);
	}*/
	dag->setReserved(JDLPrivate::USERPROXY,
		wmputilities::getJobDelegatedProxyPath(*jid));
		
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
	
	WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
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
	
	string defaultprotocol = conf.getDefaultProtocol();
	int defaultport = conf.getDefaultPort();
	
	// Initializing logger
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid, 
		defaultprotocol, defaultport);
	wmplogger.setLBProxy(conf.isLBProxyAvailable());
	
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
	
	// Setting internal attributes for sub jobs
	string jobidstring;
	string dest_uri;
	for (unsigned int i = 0; i < jobids.size(); i++) {
		jobidstring = jobids[i];
		JobId subjobid(jobidstring);
		dest_uri = getDestURI(jobids[i], defaultprotocol, defaultport);
		edglog(debug)<<"Setting internal attributes for sub job: "<<jobids[i]<<endl;
		edglog(debug)<<"Destination URI: "<<dest_uri<<endl;
		
		// Adding WMPISB_BASE_URI & INPUT_SANDBOX_PATH
        edglog(debug)<<"Setting attribute JDL::WMPISB_BASE_URI"<<endl;
		dag->setNodeAttribute(subjobid, JDL::WMPISB_BASE_URI, dest_uri);
        edglog(debug)<<"Setting attribute JDLPrivate::INPUT_SANDBOX_PATH"<<endl;
		dag->setNodeAttribute(subjobid, JDLPrivate::INPUT_SANDBOX_PATH,
			getInputSBDirectoryPath(jobidstring));
        edglog(debug)<<"Setting attribute JDLPrivate::OUTPUT_SANDBOX_PATH"<<endl; 
		dag->setNodeAttribute(subjobid, JDLPrivate::OUTPUT_SANDBOX_PATH,
			getOutputSBDirectoryPath(jobidstring));
        edglog(debug)<<"Setting attribute JDLPrivate::USERPROXY"<<endl;
        dag->setNodeAttribute(subjobid, JDLPrivate::USERPROXY,
            getJobDelegatedProxyPath(jobids[i]));
            
        // Adding OutputSandboxDestURI attribute
        if (dag->hasNodeAttribute(subjobid, JDL::OUTPUTSB)) {
			vector<string> osbdesturi;
			if (dag->hasNodeAttribute(subjobid, JDL::OSB_DEST_URI)) {
				osbdesturi = wmputilities::computeOutputSBDestURI(
					dag->getNodeStringValue(subjobid, JDL::OSB_DEST_URI),
					dest_uri);
				dag->delNodeAttribute(subjobid, JDL::OSB_DEST_URI);
			} else if (dag->hasNodeAttribute(subjobid, JDL::OSB_BASE_DEST_URI)) {
            	osbdesturi = wmputilities::computeOutputSBDestURIBase(
					dag->getNodeStringValue(subjobid, JDL::OUTPUTSB),
					dag->getNodeStringValue(subjobid, JDL::OSB_BASE_DEST_URI)[0]);
			} else {
            	osbdesturi = wmputilities::computeOutputSBDestURIBase(
					dag->getNodeStringValue(subjobid, JDL::OUTPUTSB),
					dest_uri + "/output");
			}
			if (osbdesturi.size() != 0) {
                edglog(debug)<<"Setting attribute JDL::OSB_DEST_URI"<<endl;
				dag->setNodeAttribute(subjobid, JDL::OSB_DEST_URI, osbdesturi);
			}
		}
	}
	
	// Getting Input Sandbox Destination URI
	dest_uri = wmputilities::getDestURI(stringjid, defaultprotocol,
		defaultport);
	edglog(debug)<<"Destination uri: "<<dest_uri<<endl;
	setAttributes(dag, jid, dest_uri);
		
	// Setting user proxy
	if (wmplogger.setUserProxy(delegatedproxy)) {
			edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
			throw AuthenticationException(__FILE__, __LINE__,
				"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
				"Unable to set User Proxy for LB context");
	}
	
	dag->getSubmissionStrings();
	//dag->toString(ExpDagAd::SUBMISSION);
	wmplogger.registerDag(dag);
	
	// Registering for Proxy renewal
	char * renewalproxy = NULL;
	if (dag->hasAttribute(JDL::MYPROXY)) {
		edglog(debug)<<"Registering Proxy renewal..."<<endl;
		renewalproxy = wmplogger.registerProxyRenewal(delegatedproxy,
			(dag->getAttribute(WMPExpDagAd::MYPROXY_SERVER)));
	}
	
	// Creating private job directory with delegated Proxy
	setJobFileSystem(auth, delegatedproxy, stringjid, jobids, renewalproxy);
	
	delete jid;
	
	pair<string, string> returnpair;
	returnpair.first = stringjid;
	returnpair.second = dag->toString();

	// Logging delegation id & original jdl
	edglog(debug)<<"Logging user tag JDL::DELEGATION_ID..."<<endl;
	wmplogger.logUserTag(JDL::DELEGATION_ID, delegation_id);
	edglog(debug)<<"Logging user tag JDL::JDL_ORIGINAL..."<<endl;
	wmplogger.logUserTag(JDL::JDL_ORIGINAL, jdl);
	
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


void
jobStart(jobStartResponse &jobStart_response, const string &job_id)
{
	GLITE_STACK_TRY("jobStart()");
	edglog_fn("wmpoperations::jobStart");
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	try {
		authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} catch (NotAVOMSProxyException &navp) {
		auth->authorize("", job_id);
	}
	delete auth;

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
	WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable());
	
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
			wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ABORT,
				"Job Proxy has expired", true, true);
			jobPurgeResponse jobPurge_response;
			jobpurge(jobPurge_response, jid);	
		}
		throw ex;
	}
	
	if (!wmplogger.isRegisterEventOnly()) {
		edglog(error)<<"The job has already been started"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"The job has already been started");
	}
	
	regJobEvent event = wmplogger.retrieveRegJobEvent(job_id);
	
	if (event.parent != "") {
		string msg = "the job is a DAG subjob. The parent is: "
			+ event.parent;
		edglog(error)<<msg<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart()", wmputilities::WMS_OPERATION_NOT_ALLOWED, msg);
	}
	
	edglog(debug)<<"JDL to Start:\n"<<event.jdl<<endl;
	
	submit(event.jdl, jid);
	
	delete jid;
	
	GLITE_STACK_CATCH();
}


void
logCheckpointable(WMPEventLogger * wmplogger, JobAd * jad,
	const string &stringjid) 
{
	GLITE_STACK_TRY("logCheckpointable()");
	edglog_fn("wmpoperations::logCheckpointable");
	
	string current_step;
	if (jad->hasAttribute(JDL::CHKPT_CURRENTSTEP)) {
		if (jad->getType(JDL::CHKPT_CURRENTSTEP) == Ad::TYPE_INTEGER) {
			current_step = boost::lexical_cast<std::string>(
				jad->getInt(JDL::CHKPT_CURRENTSTEP));
		} else {
			current_step = jad->getString(JDL::CHKPT_CURRENTSTEP);
		}
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
		statead.setAttribute(JDL::CHKPT_DATA, "[]");
		edglog(debug)<<"Setting attribute JDL::CHKPT_CURRENTSTEP"<<endl;
		statead.setAttribute(JDL::CHKPT_CURRENTSTEP, current_step);
	}
	if (wmplogger->logCheckpointable(current_step.c_str(),
			statead.toString().c_str())) {
		edglog(severe)<<"LB logging checkpoint state failed"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"logCheckpointable()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"LB logging checkpoint state failed");
	}
	
	GLITE_STACK_CATCH();
}

void
submit(const string &jdl, JobId *jid)
{
	GLITE_STACK_TRY("submit()");
	edglog_fn("wmpoperations::submit");
	
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	edglog(debug)<<"LB Address: "<<lbaddress_port.first<<endl;
	edglog(debug)<<"LB Port: "
		<<boost::lexical_cast<std::string>(lbaddress_port.second)<<endl;
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable());
	
	// Getting delegated proxy inside job directory
	string proxy(wmputilities::getJobDelegatedProxyPath(*jid));
	edglog(debug)<<"Job delegated proxy: "<<proxy<<endl;
	
	// Setting user proxy
	if (wmplogger.setUserProxy(proxy)) {
		edglog(critical)<<"Unable to set User Proxy for LB context"<<endl;
		throw AuthenticationException(__FILE__, __LINE__,
			"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
			"Unable to set User Proxy for LB context");
	}
	
	// Checking if the value is the same as $DOCUMENT_ROOT
	string envsandboxpath = getenv(DOCUMENT_ROOT) + FILE_SEPARATOR
		+ wmputilities::getSandboxDirName();
	edglog(debug)<<"Sandbox Path: "<<conf.getSandboxStagingPath()<<endl;
	edglog(debug)<<"DOCUMENT_ROOT: "<<envsandboxpath<<endl;
	
	// Checking if the value is the same as $DOCUMENT_ROOT
	if (conf.getSandboxStagingPath() != envsandboxpath) {
		edglog(severe)<<"ERROR: SandboxStagingPath value inside "
			"configuration file is different from $DOCUMENT_ROOT/<sandbox dir>"
			<<endl;
		throw FileSystemException( __FILE__, __LINE__,
  			"submit()", wmputilities::WMS_FILE_SYSTEM_ERROR,
   			"SandboxStagingPath value inside configuration file is different "
   			"from $DOCUMENT_ROOT/<sandbox dir>\n(please contact server administrator)");
	}
	
	string jdltostart = jdl;
	
	int type = getType(jdl);
	if (type == TYPE_JOB) {
		JobAd * jad = new JobAd(jdl);
		
		// Looking for Zipped ISB
		if (jad->hasAttribute(JDLPrivate::ZIPPED_ISB)) {
			vector<string> files = jad->getStringValue(JDLPrivate::ZIPPED_ISB);
			string targetdir = getenv(DOCUMENT_ROOT);
			for (unsigned int i = 0; i < files.size(); i++) {
				edglog(debug)<<"Uncompressing zip file: "<<files[i]<<endl;
				wmputilities::uncompressFile(files[i], targetdir);
				remove(files[i].c_str());
			}
		}
		
		if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_INTERACTIVE)) {
			edglog(debug)<<"Logging listener"<<endl;
			if (wmplogger.logListener(jad->getString(JDL::SHHOST).c_str(), 
				jad->getInt(JDL::SHPORT))) {
				edglog(severe)<<"LB logging listener failed"<<endl;
          		throw JobOperationException( __FILE__, __LINE__,
          			"jobStart()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
           			"LB logging listener failed");
			}	
		} else if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_CHECKPOINTABLE)) {
			string jobidstring = jid->toString();
			edglog(debug)<<"Logging checkpointable for job: "<<jobidstring<<endl;
			logCheckpointable(&wmplogger, jad, jobidstring);
		}
		
		// Adding attribute for perusal functionalities
		string peekdir = wmputilities::getPeekDirectoryPath(*jid) + FILE_SEPARATOR;
		if (jad->hasAttribute(JDL::PU_FILE_ENABLE)) {
			if (jad->getBool(JDL::PU_FILE_ENABLE)) {
				edglog(debug)<<"Enabling perusal functionalities for job: "
					<<jid->toString()<<endl;
				edglog(debug)<<"Setting attribute JDLPrivate::PU_LIST_FILE_URI"
					<<endl;
				jad->setAttribute(JDLPrivate::PU_LIST_FILE_URI, peekdir
					+ PERUSAL_FILE_2_PEEK_NAME);
				if (jad->hasAttribute(JDL::PU_FILES_DEST_URI)) {
					edglog(debug)<<"Disabling perusal functionalities..."<<endl;
					wmputilities::setFlagFile(peekdir
					+ EXTERNAL_PEEK_FLAG_FILE, true);
				} else {
					edglog(debug)<<"Setting attribute JDL::PU_FILES_DEST_URI"
						<<endl;
					jad->setAttribute(JDL::PU_FILES_DEST_URI, peekdir);
				}
				
				int time = DEFAULT_PERUSAL_TIME_INTERVAL;
				if (jad->hasAttribute(JDL::PU_TIME_INTERVAL)) {
					time = max(time, jad->getInt(JDL::PU_TIME_INTERVAL));
				}
				time = max(time, conf.getMinPerusalTimeInterval());
				edglog(debug)<<"Setting attribute JDL::PU_TIME_INTERVAL"<<endl;
				jad->setAttribute(JDL::PU_TIME_INTERVAL, time);
				
				jdltostart = jad->toSubmissionString();
			} else {
				edglog(debug)<<"Disabling perusal functionalities..."<<endl;
				wmputilities::setFlagFile(peekdir + DISABLED_PEEK_FLAG_FILE,
					true);
			}
		} else {
			edglog(debug)<<"Disabling perusal functionalities..."<<endl;
			wmputilities::setFlagFile(peekdir + DISABLED_PEEK_FLAG_FILE, true);	
		}
		
		delete jad;
	} else {
		WMPExpDagAd * dag = new WMPExpDagAd(jdl);
		
		// Looking for Zipped ISB
		if (dag->hasAttribute(JDLPrivate::ZIPPED_ISB)) {
			vector<string> files = dag->getAttribute(ExpDagAd::ZIPPED_ISB);
			string targetdir = getenv(DOCUMENT_ROOT);
			for (unsigned int i = 0; i < files.size(); i++) {
				edglog(debug)<<"Uncompressing zip file: "<<files[i]<<endl;
				wmputilities::uncompressFile(files[i], targetdir);
				remove(files[i].c_str());
			}
		}
		
	    JobIdStruct jobidstruct = dag->getJobIdStruct();
	    JobId parentjobid = jobidstruct.jobid;
	    vector<JobIdStruct*> children = jobidstruct.children;
	    //TBD Change getSubmissionStrings() with a better method when coded??
	    vector<string> jdls = dag->getSubmissionStrings();
	    char * seqcode = wmplogger.getSequence();
	    for (unsigned int i = 0; i < children.size(); i++) {
	    	JobId jobid = children[i]->jobid;
	    	string jobidstring = jobid.toString();
	    	if (dag->hasNodeAttribute(jobid, JDL::JOBTYPE)) {
	    		string type = dag->getNodeStringValue(jobid, JDL::JOBTYPE)[0];
	    		if (type == JDL_JOBTYPE_CHECKPOINTABLE) {
	    			edglog(debug)<<"Logging checkpointable for subjob: "
	    				<<jobidstring<<endl;
	    			JobAd *jad = new JobAd(jdls[i]);
	    			wmplogger.setLoggingJob(jobidstring, seqcode);
	    			logCheckpointable(&wmplogger, jad, jobidstring);
	    			delete jad;
	    		}
	    	}
	    	
	    	// Adding attributes for perusal functionalities
	    	string peekdir = wmputilities::getPeekDirectoryPath(jobid)
	    		+ FILE_SEPARATOR;
			if (dag->hasNodeAttribute(jobid, JDL::PU_FILE_ENABLE)) {
				if (dag->getNodeBool(jobid, JDL::PU_FILE_ENABLE)) {
					edglog(debug)<<"Enabling perusal functionalities for job: "
						<<jobidstring<<endl;
					edglog(debug)<<"Setting attribute JDLPrivate::PU_LIST_FILE_URI"
						<<endl;
					dag->setNodeAttribute(jobid, JDLPrivate::PU_LIST_FILE_URI,
						peekdir + PERUSAL_FILE_2_PEEK_NAME);
					if (dag->hasNodeAttribute(jobid, JDL::PU_FILES_DEST_URI)) {
						edglog(debug)<<"Setting external perusal URI..."<<endl;
						wmputilities::setFlagFile(peekdir
							+ EXTERNAL_PEEK_FLAG_FILE, true);
					} else {
						edglog(debug)<<"Setting attribute JDL::PU_FILES_DEST_URI"
							<<endl;
						dag->setNodeAttribute(jobid, JDL::PU_FILES_DEST_URI,
							peekdir);
					}
					
					int time = DEFAULT_PERUSAL_TIME_INTERVAL;
					if (dag->hasNodeAttribute(jobid, JDL::PU_TIME_INTERVAL)) {
						time = max(time, dag->getNodeInt(jobid,
							JDL::PU_TIME_INTERVAL));
					}
					time = max(time, conf.getMinPerusalTimeInterval());
					edglog(debug)<<"Setting attribute JDL::PU_TIME_INTERVAL"
						<<endl;
					dag->setNodeAttribute(jobid, JDL::PU_TIME_INTERVAL, time);
					
					jdltostart = dag->toString();
				} else {
					edglog(debug)<<"Disabling perusal functionalities..."<<endl;
					wmputilities::setFlagFile(peekdir + DISABLED_PEEK_FLAG_FILE,
						true);	
				}
			} else {
				edglog(debug)<<"Disabling perusal functionalities..."<<endl;
				wmputilities::setFlagFile(peekdir + DISABLED_PEEK_FLAG_FILE,
					true);	
			}
		}
	    
	    delete dag;
	    wmplogger.setLoggingJob(parentjobid.toString(), seqcode);
	}
	
	wmpmanager::WMPManager manager(&wmplogger);
	// Vector of parameters to runCommand()
	vector<string> params;
	params.push_back(jdltostart);
	params.push_back(proxy);
	params.push_back(wmputilities::getJobDirectoryPath(*jid));
	params.push_back(string(getenv(DOCUMENT_ROOT)));
	
	wmp_fault_t wmp_fault = manager.runCommand("JobSubmit", params);
	
	if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
		wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ABORT,
			wmp_fault.message.c_str(), true, true);
		edglog(severe)<<"Error in runCommand: "<<wmp_fault.message<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"submit()", wmp_fault.code, wmp_fault.message);
	}
	GLITE_STACK_CATCH();
}

void
jobSubmit(jobSubmitResponse &jobSubmit_response,
	const string &jdl, const string &delegation_id)
{
	GLITE_STACK_TRY("jobSubmit()");
	edglog_fn("wmpoperations::jobSubmit");
	
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
	
	try {
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN());
	} catch (NotAVOMSProxyException &navp) {
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
		
	if (auth) {
		delete auth;	
	}
	
	// Getting job identifier from register response
	//string jobid = jobRegister_response.jobIdStruct->id;
	string jobid = reginfo.first;
	edglog(debug)<<"Starting registered job: "<<jobid<<endl;

	// Starting job submission
	JobId *jid = new JobId(jobid);
	
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable());
	
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
	
	submit(reginfo.second, jid);
	delete jid;
	
	jobSubmit_response.jobIdStruct = jobRegister_response.jobIdStruct;

	GLITE_STACK_CATCH();
}

void
jobCancel(jobCancelResponse &jobCancel_response, const string &job_id)
{
	GLITE_STACK_TRY("jobCancel()");
	edglog_fn("wmpoperations::jobCancel");
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	try {
		authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} catch (NotAVOMSProxyException &navp) {
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
		WMPEventLogger wmplogger(wmputilities::getEndpoint());
		WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
		std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
		wmplogger.init(lbaddress_port.first, lbaddress_port.second, parentjid,
			conf.getDefaultProtocol(), conf.getDefaultPort());
		wmplogger.setLBProxy(conf.isLBProxyAvailable());
		
		// Setting user proxy
		if (wmplogger.setUserProxy(delegatedproxy)) {
			edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
			throw AuthenticationException(__FILE__, __LINE__,
				"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
				"Unable to set User Proxy for LB context");
		}
		string parentjdl = wmplogger.getUserTagJDLOriginal();
		
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
	WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable());
	
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
			wmplogger.setSequenceCode("UI=000000:NS=0000096660:WM=000000:"
				"BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000");
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
			
			// Checking if the value is the same as $DOCUMENT_ROOT
			envsandboxpath = getenv(DOCUMENT_ROOT) + FILE_SEPARATOR
				+ wmputilities::getSandboxDirName();
			edglog(debug)<<"Sandbox Path: "<<conf.getSandboxStagingPath()<<endl;
			edglog(debug)<<"DOCUMENT_ROOT: "<<envsandboxpath<<endl;
			
			// Checking if the value is the same as $DOCUMENT_ROOT
			if (conf.getSandboxStagingPath() != envsandboxpath) {
				edglog(severe)<<"ERROR: SandboxStagingPath value inside "
					"configuration file is different from $DOCUMENT_ROOT/<sandbox dir>"
					<<endl;
				throw FileSystemException( __FILE__, __LINE__,
		  			"jobCancel()", wmputilities::WMS_FILE_SYSTEM_ERROR,
		   			"SandboxStagingPath value inside configuration file is different "
		   			"from $DOCUMENT_ROOT/<sandbox dir>\n(please contact server administrator)");
			}
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
			// DONE_CODE = Failed (1)
			if (status.getValInt(JobStatus::DONE_CODE) != 0) {
				params.push_back(jid->toString());
				params.push_back(jobpath);
				
				// Checking if the value is the same as $DOCUMENT_ROOT
				envsandboxpath = getenv(DOCUMENT_ROOT) + FILE_SEPARATOR
					+ wmputilities::getSandboxDirName();
				edglog(debug)<<"Sandbox Path: "<<conf.getSandboxStagingPath()<<endl;
				edglog(debug)<<"DOCUMENT_ROOT: "<<envsandboxpath<<endl;
				
				// Checking if the value is the same as $DOCUMENT_ROOT
				if (conf.getSandboxStagingPath() != envsandboxpath) {
					edglog(warning)<<"ERROR: SandboxStagingPath value inside "
						"configuration file is different from $DOCUMENT_ROOT/<sandbox dir>"
						<<endl;
					throw FileSystemException( __FILE__, __LINE__,
			  			"jobCancel()", wmputilities::WMS_FILE_SYSTEM_ERROR,
			   			"SandboxStagingPath value inside configuration file is different "
			   			"from $DOCUMENT_ROOT/<sandbox dir>\n(please contact server administrator)");
				}
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
				edglog(debug)<<"Job status (DONE - DONE_CODE = 0) doesn't allow job cancel"<<endl;
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
	edglog(info)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	try {
		authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, jid);
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN(), jid);
	} catch (NotAVOMSProxyException &navp) {
		auth->authorize("", jid);
	}
	delete auth;
	
	getSandboxDestURI_response.path = new StringList;
	getSandboxDestURI_response.path->Item = new vector<string>(0);
	
	WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	vector<pair<std::string, int> > protocols = conf.getProtocols();
	int httpsport = conf.getHTTPSPort();
	
	vector<string> *uris = wmputilities::getDestURIsVector(protocols, httpsport, jid);
	getSandboxDestURI_response.path->Item = uris;
	
	GLITE_STACK_CATCH();
}

void
getSandboxBulkDestURI(getSandboxBulkDestURIResponse &getSandboxBulkDestURI_response,
	const string &jid)
{
	GLITE_STACK_TRY("getSandboxBulkDestURI()");
	edglog_fn("wmpoperations::getSandboxBulkDestURI");
	edglog(info)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	try {
		authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, jid);
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN(), jid);
	} catch (NotAVOMSProxyException &navp) {
		auth->authorize("", jid);
	}
	delete auth;
	
	// Initializing logger
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jobid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable());
	
	// Setting user proxy
	if (wmplogger.setUserProxy(delegatedproxy)) {
		edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
		throw AuthenticationException(__FILE__, __LINE__,
			"setUserProxy()", wmputilities::WMS_AUTHENTICATION_ERROR,
			"Unable to set User Proxy for LB context");
	}
	
	//WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	vector<pair<std::string, int> > protocols = conf.getProtocols();
	int httpsport = conf.getHTTPSPort();
	
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
 	
	for (unsigned int j = 0; j < jids.size(); j++) {
		vector<string> *uris =
			wmputilities::getDestURIsVector(protocols, httpsport, jids[j]);
		
		DestURIStructType *destURIStruct = new DestURIStructType();
		destURIStruct->id = jids[j];
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
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
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
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
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
	// DONE_CODE = Failed (0)
	if (!checkstate
			|| ((status.status == JobStatus::ABORTED)
			|| ((status.status == JobStatus::DONE)
				&& (status.getValInt(JobStatus::DONE_CODE) == 0)))) {
		
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
						<<string(GLITE_HOST_CERT)<<" e/o "<<string(GLITE_HOST_KEY)<<endl;
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
			edglog(severe)<<"Unable to perform job purge"<<endl;
			if (checkstate) {
				throw FileSystemException(__FILE__, __LINE__,
					"jobpurge()", wmputilities::WMS_IS_FAILURE,
					"Unable to perform job purge");
			}
		}
		
		unsetenv(X509_USER_CERT);
		unsetenv(X509_USER_KEY);
		
		// Removing temporary Proxy file
		if (isproxyfile) {
	    	remove(usercert.c_str());
		}
		
	} else {
		edglog(error)<<"Job state doesn't allow purge operation"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobpurge()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"Job state doesn't allow purge operation");
	}
	
	GLITE_STACK_CATCH();
}

void
jobPurge(jobPurgeResponse &jobPurge_response, const string &jid)
{
	GLITE_STACK_TRY("jobPurge()");
	edglog_fn("wmpoperations::jobPurge");
	edglog(info)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;

	try {
		authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, jid);
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN(), jid);
	} catch (NotAVOMSProxyException &navp) {
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
	edglog(info)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	try {
		authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, jid);
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN(), jid);
	} catch (NotAVOMSProxyException &navp) {
		auth->authorize("", jid);
	}
	delete auth;
	
	string jobdirectory = wmputilities::getJobDirectoryPath(*jobid);

	delete jobid;

	// Checking for maradona file, created if and only if the job is in DONE state
	edglog(debug)<<"Searching for file: "<<jobdirectory + FILE_SEPARATOR
		+ MARADONA_FILE<<endl;
	if (wmputilities::fileExists(jobdirectory + FILE_SEPARATOR + MARADONA_FILE)) {
		WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
		string output_uri = wmputilities::getDestURI(jid, conf.getDefaultProtocol(),
			conf.getDefaultPort()) + FILE_SEPARATOR + "output";
		string outputpath = wmputilities::getOutputSBDirectoryPath(jid);
		edglog(debug)<<"Output URI: " << output_uri <<endl;
		
		// Searching files inside directory
		const boost::filesystem::path p(outputpath);
		std::vector<std::string> found;
		glite::wms::wmproxy::commands::list_files(p, found);
		edglog(debug)<<"List size is (hidden files included): "
			<<found.size()<<endl;
		
		// Creating the list
		StringAndLongList *list = new StringAndLongList();
		vector<StringAndLongType*> *file = new vector<StringAndLongType*>;
		StringAndLongType *item = NULL;
		string filename;
		for (unsigned int i = 0; i < found.size(); i++) {
			// Checking for hidden files
			filename = wmputilities::getFileName(string(found[i]));
			//if ((filename != MARADONA_FILE) && (filename.substr(0, 1) != ".")) {
			if ((filename != MARADONA_FILE) && (filename
					!= authorizer::GaclManager::WMPGACL_DEFAULT_FILE)) {
				item = new StringAndLongType();
				item->name = output_uri + FILE_SEPARATOR + filename;
				item->size = wmputilities::computeFileSize(found[i]);
				edglog(debug)<<"Inserting file name: " <<item->name<<endl;
				edglog(debug)<<"Inserting file size: " <<item->size<<endl;
				file->push_back(item);
			}
		}
		list->file = file;
		getOutputFileList_response.OutputFileAndSizeList = list;
		
		edglog(info)<<"Successfully retrieved files: "<<found.size()<<endl;
	} else {
		edglog(error)<<
			"Job state doesn't allow getOutputFileList operation"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"getOutputFileList()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"Job state doesn't allow getOutputFileList operation");
	}
	
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
		
		WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
		WMPEventLogger wmplogger(wmputilities::getEndpoint());
		wmplogger.setLBProxy(conf.isLBProxyAvailable());
		
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
	
	try {
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN());
	} catch (NotAVOMSProxyException &navp) {
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
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	try {
		authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} catch (NotAVOMSProxyException &navp) {
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
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	try {
		authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} catch (NotAVOMSProxyException &navp) {
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
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	try {
		authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} catch (NotAVOMSProxyException &navp) {
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

vector<string>
getDelegatedProxyInfo(getDelegatedProxyInfoResponse 
	&getDelegatedProxyInfo_response, const string &delegation_id)
{
	GLITE_STACK_TRY("getDelegatedProxyInfo()");
	edglog_fn("wmpoperations::getDelegatedProxyInfo");
	
	// Checking delegation id
	edglog(info)<<"Delegation ID: "<<delegation_id<<endl;
	if (delegation_id == "") {
		edglog(error)<<"Provided delegation ID not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"getDelegatedProxyInfo()", wmputilities::WMS_DELEGATION_ERROR,
			"Delegation id not valid");
	}
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy from SSL Proxy cache
	string delegatedproxy = WMPDelegation::getDelegatedProxyPath(delegation_id);
	edglog(debug)<<"Delegated proxy: "<<delegatedproxy<<endl;
	
	try {
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN());
	} catch (NotAVOMSProxyException &navp) {
		auth->authorize();
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"
			<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::getDelegatedProxyInfo()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	long timeleft = authorizer::WMPAuthorizer::getProxyTimeLeft(delegatedproxy);
	if (timeleft < 0) {
		timeleft = 0;
	}
	string time = "Timeleft = " + boost::lexical_cast<std::string>(timeleft);
	edglog(debug)<<time<<endl;
	vector<string> returnvector;
	returnvector.push_back(time);
	
	edglog(info)<<"getDelegatedProxyInfo successfully"<<endl;
	
	return returnvector;
	
	GLITE_STACK_CATCH();
}

void
enableFilePerusal(enableFilePerusalResponse &enableFilePerusal_response,
	const string &job_id, StringList * fileList)
{
	GLITE_STACK_TRY("enableFilePerusal()");
	edglog_fn("wmpoperations::enableFilePerusal");
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	try {
		authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} catch (NotAVOMSProxyException &navp) {
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
	
	string peekdir = wmputilities::getPeekDirectoryPath(*jid) + FILE_SEPARATOR;
	if (wmputilities::fileExists(peekdir + DISABLED_PEEK_FLAG_FILE)) {
		throw JobOperationException(__FILE__, __LINE__,
	    	"wmpoperations::enableFilePerusal()", wmputilities::WMS_OPERATION_NOT_ALLOWED, 
	    	"Perusal not enabled for this job");
	}
	string filename = peekdir + PERUSAL_FILE_2_PEEK_NAME;
	
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
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	try {
		authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
		authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} catch (NotAVOMSProxyException &navp) {
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
	
	string peekdir = wmputilities::getPeekDirectoryPath(*jid) + FILE_SEPARATOR;
	if (wmputilities::fileExists(peekdir + DISABLED_PEEK_FLAG_FILE)) {
		throw JobOperationException(__FILE__, __LINE__,
	    	"wmpoperations::getPerusalFiles()", wmputilities::WMS_OPERATION_NOT_ALLOWED, 
	    	"Perusal not enabled for this job");
	}
	if (wmputilities::fileExists(peekdir + EXTERNAL_PEEK_FLAG_FILE)) {
		throw JobOperationException(__FILE__, __LINE__,
	    	"wmpoperations::getPerusalFiles()", wmputilities::WMS_OPERATION_NOT_ALLOWED, 
	    	"Remote perusal peek directory set");
	}
	
	const boost::filesystem::path p(peekdir);
	vector<string> found;
	glite::wms::wmproxy::commands::list_files(p, found);
	
	WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	string protocol = conf.getDefaultProtocol();
	string port = (conf.getDefaultPort() != 0) ? 
		boost::lexical_cast<std::string>(conf.getDefaultPort()) : "";
	string serverhost = getServerHost();
		
	vector<string> good;
	vector<string> returnvector;
	string currentfilename;
	for (unsigned int i = 0; i < found.size(); i++) {
		currentfilename = wmputilities::getFileName(found[i]);
		if (currentfilename.find(fileName + ".") == 0) {
			edglog(debug)<<"Good perusal file: "<<found[i]<<endl;
			good.push_back(found[i]);
		}
		if (allChunks) {
			if (currentfilename.find(fileName + PERUSAL_DATE_INFO_SEPARATOR)
					== 0) {
				edglog(debug)<<"Good old global perusal file: "<<found[i]<<endl;
				returnvector.push_back(protocol + "://" + serverhost + ":"
					+ port + found[i]);	
			}
		}
	}
	
	unsigned int size = good.size();
	if (size != 0) {
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
		for (unsigned int i = 0; i < size; i++) {
			filesize = wmputilities::computeFileSize(good[i]);
			filetoreturn = peekdir + fileName
				+ PERUSAL_DATE_INFO_SEPARATOR + startdate
				+ PERUSAL_DATE_INFO_SEPARATOR + enddate;
			if ((totalfilesize + filesize) > FILE_TRANSFER_SIZE_LIMIT) {
				outfile.close();
				rename(tempfile.c_str(), filetoreturn.c_str());
				returnvector.push_back(protocol + "://" + serverhost + ":"
					+ port + filetoreturn);
				outfile.open(tempfile.c_str(), ios::out);
				if (!outfile.good()) {
					edglog(severe)<<tempfile<<": !outfile.good()"<<endl;
					throw FileSystemException(__FILE__, __LINE__,
						"getPerusalFiles()", WMS_IS_FAILURE, "Unable to open perusal "
						"temporary file\n(please contact server administrator)");
				}
				totalfilesize = 0;
				enddate = good[i].substr(good[i].rfind(".") + 1,
                	good[i].length() - 1);
				startdate = enddate;
			} else {
				enddate = good[i].substr(good[i].rfind(".") + 1,
                	good[i].length() - 1);
			}
			ifstream infile(good[i].c_str());
			if (!infile.good()) {
				edglog(severe)<<good[i]<<": !infile.good()"<<endl;
				throw FileSystemException(__FILE__, __LINE__,
					"getPerusalFiles()", WMS_IS_FAILURE, "Unable to open perusal "
					"input file\n(please contact server administrator)");
			}
			outfile << infile.rdbuf();
			infile.close();
			remove(good[i].c_str());
			totalfilesize += filesize;
		}
		outfile.close();
		filetoreturn = peekdir + fileName
			+ PERUSAL_DATE_INFO_SEPARATOR + startdate
			+ PERUSAL_DATE_INFO_SEPARATOR + enddate;
		rename(tempfile.c_str(), filetoreturn.c_str());
		returnvector.push_back(protocol + "://" + serverhost + ":" + port
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
