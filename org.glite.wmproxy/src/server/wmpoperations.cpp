/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
		
#include <fstream>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

#include "wmproxy.h"
#include "wmpoperations.h"
#include "wmpexpdagad.h"
#include "wmpconfiguration.h"

// Utilities
#include "utilities/wmputils.h"

// Eventlogger
#include "eventlogger/wmpeventlogger.h"

// Authorizer
#include "authorizer/wmpauthorizer.h"


// WMPManager
#include "WMPManager.h"

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

// Logging and Bookkeeping
#include "glite/lb/producer.h"
#include "glite/lb/Event.h"
#include "glite/lb/Job.h"
#include "glite/lb/JobStatus.h"

// AdConverter class for jdl convertion and templates
#include "glite/wms/jdl/adconverter.h"

// Configuration
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"

// WMProxy software version
const std::string WMP_MAJOR_VERSION = "1";
const std::string WMP_MINOR_VERSION = "0";
const std::string WMP_RELEASE_VERSION = "0";
const std::string WMP_POINT_VERSION = ".";
const std::string WMP_VERSION = WMP_MAJOR_VERSION
	+ WMP_POINT_VERSION + WMP_MINOR_VERSION
	+ WMP_POINT_VERSION + WMP_RELEASE_VERSION;

// Default name of the delegated Proxy that is copied inside private job
// directory
const std::string USER_PROXY_NAME = "user.proxy"; 
const char* DOCUMENT_ROOT = "DOCUMENT_ROOT";

// Defining File Separator 
#ifdef WIN 
   // Windows File Separator 
   const std::string FILE_SEPARATOR = "\\"; 
#else 
   // Linux File Separator 
   const std::string FILE_SEPARATOR = "/"; 
#endif 

using namespace std;
using namespace glite::lb; // JobStatus
using namespace glite::wms::wmproxy::server ;  //Exception codes
using namespace glite::wms::jdl; // DagAd, AdConverter
using namespace glite::wmsutils::jobid; //JobId
using namespace glite::wmsutils::exception; //Exception
using namespace glite::wms::common::configuration; // Configuration
using namespace boost::details::pool ; //singleton
using namespace glite::wms::wmproxy::eventlogger;

namespace logger         = glite::wms::common::logger;
namespace configuration  = glite::wms::common::configuration;
namespace wmpmanager	 = glite::wms::wmproxy::server;
namespace wmputilities	 = glite::wms::wmproxy::utilities;
namespace task           = glite::wms::common::task;
namespace eventlogger    = glite::wms::wmproxy::eventlogger;
namespace authorizer 	 = glite::wms::wmproxy::authorizer;


//namespace glite {
//namespace wms {
//namespace wmproxy {

// Possible values for jdl type attribute
enum type {
	TYPE_JOB,
	TYPE_DAG,
	TYPE_COLLECTION,
};

// "private" methods prototypes
glite::lb::JobStatus getStatus(glite::wmsutils::jobid::JobId *jid);

void regist(jobRegisterResponse &jobRegister_response,
	const string &delegation_id, const string &jdl, JobAd *jad);

void regist(jobRegisterResponse &jobRegister_response,
	const string &delegation_id, const string &jdl, WMPExpDagAd *dag);

void regist(jobRegisterResponse &jobRegister_response,
	const string &delegation_id, const string &jdl, WMPExpDagAd *dag,
	JobAd *jad);

void submit(const string &jdl, JobId *jid);


/**
 * Converts JobIdStruct vector into a JobIdStructType vector pointer
 */
vector<JobIdStructType*> *
convertJobIdStruct(vector<JobIdStruct*> &job_struct)
{
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
}

/**
 * Converts GraphStructType vector pointer into a NodeStruct vector pointer
 */
vector<NodeStruct*> 
convertGraphStructTypeToNodeStructVector(vector<GraphStructType*> *graph_struct)
{
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
}

/**
 * Converts GraphStructType into a NodeStruct
 */
NodeStruct
convertGraphStructTypeToNodeStruct(GraphStructType graph_struct)
{
	NodeStruct return_node_struct;
	return_node_struct.name = graph_struct.name;
	if (graph_struct.childrenJob) {
		return_node_struct.childrenNodes =
			convertGraphStructTypeToNodeStructVector(graph_struct.childrenJob);
	}
	return return_node_struct;
}
	
/**
 * Returns an int value representing the type of the job described by the jdl
 * If type is not specified -> Job  // TBC change it in the future?
 */
int
getType(string jdl)
{
	GLITE_STACK_TRY("getType(string jdl)");
	edglog_fn("   wmpoperations::getType");
	
	// Default type is normal
	int return_value = TYPE_JOB;
	try {
		Ad *in_ad = new Ad(jdl);
		if (in_ad->hasAttribute(JDL::TYPE)) {
			string type = (in_ad->getStringValue(JDL::TYPE))[0];
			edglog(info) <<"\ntype: "<<type<<endl;
			if (type == JDL_TYPE_DAG) {
				return_value = TYPE_DAG;
			} else if (type == JDL_TYPE_JOB) {
				return_value = TYPE_JOB;
			} else if (type == JDL_TYPE_COLLECTION) {
				return_value = TYPE_COLLECTION;
			}
		}
		delete in_ad;
	} catch (Exception &exc) {
		edglog(fatal)<<"\ngetType() Exception cought: "<<exc.what()<<endl;
		throw exc;
	} catch (exception &ex) {
		edglog(fatal)<<"\ngetType() exception cought: "<<ex.what()<<endl;
		throw ex;
	}
	return return_value;

	GLITE_STACK_CATCH();
}

/**
 * Converts a JobTypeList type to an int value representing the type of the job
 */
int
convertJobTypeListToInt(JobTypeList job_type_list)
{
	GLITE_STACK_TRY("convertJobTypeListToInt(JobTypeList job_type_list)");
	edglog_fn("   wmpoperations::convertJobTypeListToInt");
	int type = AdConverter::ADCONV_JOBTYPE_NORMAL;
	for (unsigned int i = 0; i < job_type_list.jobType->size(); i++) {
		switch ((*job_type_list.jobType)[i]) {
			case WMS_PARAMETRIC:
				type |= AdConverter::ADCONV_JOBTYPE_PARAMETRIC;
				edglog(info)<<"\nType caught: Parametric"<<endl;
				break;
			case WMS_INTERACTIVE:
				type |= AdConverter::ADCONV_JOBTYPE_INTERACTIVE;
				edglog(info)<<"\nType caught: Interactive"<<endl;
				break;
			case WMS_MPI:
				type |= AdConverter::ADCONV_JOBTYPE_MPICH;
				edglog(info)<<"\nType caught: Mpi"<<endl;
				break;
			case WMS_PARTITIONABLE:
				type |= AdConverter::ADCONV_JOBTYPE_PARTITIONABLE;
				edglog(info)<<"\nType caught: Partitionable"<<endl;
				break;
			case WMS_CHECKPOINTABLE:
				type |= AdConverter::ADCONV_JOBTYPE_CHECKPOINTABLE;
				edglog(info)<<"\nType caught: CHKPT"<<endl;
				break;
			default:
				break;
		}
	}
	// edglog(debug)<<"Final type value = "<<type<<endl;
	return type;
	GLITE_STACK_CATCH();
}

/**
 * Gets the full status of the job represented by the input job id
 */
glite::lb::JobStatus
getStatus(JobId *jid)
{
	GLITE_STACK_TRY("getStatus(JobId *jid)");
	glite::lb::Job lb_job(*jid);
	return lb_job.status(glite::lb::Job::STAT_CLASSADS); // to get also jdl
	// lb_job.status(0) minimal information about the job
	GLITE_STACK_CATCH();
}

//
// WM Web Service available operations
//

// To get more infomation see WM service wsdl file
void
getVersion(getVersionResponse &getVersion_response)
{
	GLITE_STACK_TRY("getVersion(getVersionResponse &getVersion_response)");
	edglog_fn("   wmpoperations::getVersion");
	getVersion_response.version = WMP_VERSION;
	edglog(info)<<"\nVersion retrieved: "<<getVersion_response.version<<endl;
	GLITE_STACK_CATCH();
}

void
jobRegister(jobRegisterResponse &jobRegister_response, const string &jdl,
	const string &delegation_id)
{
	GLITE_STACK_TRY("jobRegister(jobRegisterResponse &jobRegister_response, "
		"const string &jdl, const string &delegation_id)");
	edglog_fn("   wmpoperations::jobRegister");
	
	// Checking delegation id
	if (delegation_id == "") {
  		throw ProxyOperationException(__FILE__, __LINE__,
			"jobRegister(jobRegisterResponse &jobRegister_response, "
			"const string &jdl, const string &delegation_id)",
			wmputilities::WMS_DELEGATION_ERROR, "Delegation id not valid");
	}
	try {
		// Check TYPE/JOBTYPE attributes and convert JDL when needed
		WMPExpDagAd *dag = NULL;
		JobAd *jad = NULL;
		int type = getType(jdl);
		if (type == TYPE_DAG) {
			edglog(info)<<"\nType dag"<<endl;
			dag = new WMPExpDagAd(jdl);
		} else if (type == TYPE_JOB) {
			edglog(info)<<"\nType job"<<endl;
			jad = new JobAd(jdl);
			if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_PARAMETRIC)) {
				dag = new WMPExpDagAd(*(AdConverter::bulk2dag(jdl)));
				delete jad;
			} else if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_PARTITIONABLE)) {
				dag = new WMPExpDagAd(*(AdConverter::part2dag(jdl)));
			}
		} else if (type == TYPE_COLLECTION) {
			edglog(info)<<"\nType collection"<<endl;
			dag = new WMPExpDagAd(*(AdConverter::collection2dag(jdl)));
		}
		// PERFORM THE PROPER REGISTRATION
		if (dag && jad) {
			dag->setLocalAccess(false);
			jad->setLocalAccess(false);
			regist(jobRegister_response, delegation_id, jdl, dag, jad);
		} else if (dag) {
			dag->setLocalAccess(false);
			regist(jobRegister_response, delegation_id, jdl, dag);
		} else if (jad) {
			jad->setLocalAccess(false);
			regist(jobRegister_response, delegation_id, jdl, jad);
		} else {
			throw "FATAL ERROR";  // UNREACHABLE LINE (either dag or jad are initialised)
		}
		// Release Memory
		if (dag) {
			delete dag;
		}
		if (jad) {
			delete jad;
		}
		edglog(info) << "\n...registered successfully" << endl;
	} catch (Exception &exc) {
		edglog(fatal)<<"\njobRegister() Exception: "<<exc.what()<<endl;
		throw exc;
	} catch (exception &ex) {
		edglog(fatal)<<"\njobRegister() exception: "<<ex.what()<<endl;
		throw ex;
	}

	GLITE_STACK_CATCH();
}

void
setJobFileSystem(const string &delegation_id, const string &jobid, 
	vector<string> &jobids)
{
	GLITE_STACK_TRY("setJobFileSystem(const string &delegation_id, const "
		"string &jobid, vector<string> &jobids)");
		
	// Getting delegated Proxy file name
	string delegated_proxy = WMPDelegation::getDelegatedProxyPath(delegation_id);
	edglog_fn("   wmpoperations::seJobFileSystem");
	edglog(info)<<"\nDelegated Proxy file name:\n\t"<<delegated_proxy<<endl;
	
	string document_root = getenv(DOCUMENT_ROOT);
	// TBD check value???
	/*if (document_root ...) {
		edglog(fatal)<<"\nUnable to get DOCUMENT_ROOT environment variable"<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"setJobFileSystem(const string &delegation_id, const string &jobid, "
			"vector<string> &jobids)",
			WMS_FATAL, "Unable to get DOCUMENT_ROOT environment variable");
	}*/
	
	// Setting mode
	mode_t mode(755);
	
	// TBD WARNING! THIS IS SHALL BE PROVIDED BY an LCMAP METHOD
	// FILE *file 
	//int userid = WMPAuthorizer::getUserId();
	int userid = getuid();
	
	// Creating job directory
	vector<string> job;
	job.push_back(jobid);
	if (wmputilities::managedir(document_root, userid, job)) {
		edglog(fatal)<<"\nUnable to create job local directory for job:\n\t"
			<<jobid<<"\n"<<strerror(errno)<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"setJobFileSystem(const string &delegation_id, const string &jobid, "
			"vector<string> &jobids)",
			wmputilities::WMS_FATAL, "Unable to create job local directory");
	}

	// Copying delegated Proxy to destination URI
	//TBD check result of copy
	wmputilities::fileCopy(delegated_proxy, document_root + FILE_SEPARATOR
		+ wmputilities::to_filename(glite::wmsutils::jobid::JobId(jobid))
		+ FILE_SEPARATOR + USER_PROXY_NAME);
	
	if (jobids.size() != 0) {
		edglog(info)<<"Creating sub job directories for job:\n"<<jobid<<endl;
		if (wmputilities::managedir(document_root, userid, jobids)) {
			edglog(fatal)<<"\nUnable to create sub jobs local directories for job:\n\t"
				<<jobid<<"\n"<<strerror(errno)<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"setJobFileSystem(const string &delegation_id, const string "
				"&jobid, vector<string> &jobids)",
				wmputilities::WMS_FATAL, "Unable to create sub jobs local directories");
		} else {
			string dest_uri =
				wmputilities::to_filename(glite::wmsutils::jobid::JobId(jobid));
			string target;
			string link;
			for (unsigned int i = 0; i < jobids.size(); i++) {
				string target = document_root + FILE_SEPARATOR + dest_uri
					+ FILE_SEPARATOR + USER_PROXY_NAME;
				string link = document_root + FILE_SEPARATOR
					+ wmputilities::to_filename(glite::wmsutils::jobid::JobId(jobids[i]))
					+ FILE_SEPARATOR + USER_PROXY_NAME;
				edglog(info)<<"Creating proxy symbolic link in: "<<jobids[i]<<endl;
				if (symlink(target.c_str(), link.c_str())) {
			      	edglog(fatal)<<"\nUnable to create symbolic link to proxy file:\n\t"
			      		<<link<<"\n"<<strerror(errno)<<endl;
			      
			      	throw FileSystemException(__FILE__, __LINE__,
						"setJobFileSystem(const string &delegation_id, const string "
						"&jobid, vector<string> &jobids)", wmputilities::WMS_FATAL, 
						"Unable to create symbolic link to proxy file");
			    }
			}
		}
	}
	
  	GLITE_STACK_CATCH();
}

void
regist(jobRegisterResponse &jobRegister_response, const string &delegation_id,
	const string &jdl, JobAd *jad)
{
	GLITE_STACK_TRY("regist(jobRegisterResponse &jobRegister_response, "
		"const string &delegation_id, const string &jdl, JobAd *jad)");
	
	edglog_fn("   wmpoperations::regist JOB");
	
	// Creating unique identifier
	JobId *jid = new JobId();
	if (LB_PORT == 0) {
		jid->setJobId(LB_ADDRESS);
	} else {
		jid->setJobId(LB_ADDRESS, LB_PORT);
	}
	string stringjid = jid->toString();

	// Getting Input Sandbox Destination URI
	getSandboxDestURIResponse getSandboxDestURI_response;
	getSandboxDestURI(getSandboxDestURI_response, stringjid);
	string dest_uri = getSandboxDestURI_response.path;
	edglog(info)<<"\nDestination uri: "<<dest_uri<<endl;

	// Setting job identifier
	jad->setAttribute(JDL::JOBID, stringjid);
	
	// Adding WMPInputSandboxBaseURI and InputSandboxBaseURI attributes
	if (!jad->hasAttribute(JDL::ISB_BASE_URI)) {
		jad->setAttribute(JDL::ISB_BASE_URI, dest_uri);
	}
	jad->setAttribute(JDL::WMPISB_BASE_URI, dest_uri);

	// Initializing logger
	WMPEventLogger wmplogger;
	wmplogger.init(NS_ADDRESS, NS_PORT, jid);
	
	// Registering the job
	wmplogger.registerJob(jad);
	
	// Creating private job directory with delegated Proxy
	vector<string> jobids;
	setJobFileSystem(delegation_id, stringjid, jobids);
	
	// Registering for Proxy renewal
	if (jad->hasAttribute(JDL::MYPROXY)) {
		/*wmplogger.registerProxyRenewal(
			WMPDelegation::getDelegatedProxyPath(delegation_id),
			(jad->getStringValue(JDL::MYPROXY))[0]);*/
		string document_root = getenv(DOCUMENT_ROOT);
		wmplogger.registerProxyRenewal(document_root + FILE_SEPARATOR
			+ wmputilities::to_filename(*jid) + FILE_SEPARATOR
			+ USER_PROXY_NAME, (jad->getStringValue(JDL::MYPROXY))[0]);
	}
	
	// Logging delegation id & original jdl
	wmplogger.logUserTag(JDL::DELEGATION_ID, delegation_id);
	wmplogger.logUserTag(JDL::JDL_ORIGINAL, jdl);

	// Creating job identifier structure to return to the caller
	JobIdStructType *job_id_struct = new JobIdStructType();
	job_id_struct->id = stringjid;
	delete jid;
	job_id_struct->name = new string("");
	job_id_struct->childrenJob = new vector<JobIdStructType*>;
	jobRegister_response.jobIdStruct = job_id_struct;

	GLITE_STACK_CATCH();
}

void
regist(jobRegisterResponse &jobRegister_response, const string &delegation_id,
	const string &jdl, WMPExpDagAd *dag)
{
	regist(jobRegister_response, delegation_id, jdl, dag);
}

void
regist(jobRegisterResponse &jobRegister_response, const string &delegation_id,
	const string &jdl, WMPExpDagAd *dag, JobAd *jad = NULL)
{
	GLITE_STACK_TRY("regist(jobRegisterResponse &jobRegister_response, "
	"const string &jdl, WMPExpDagAd *dag, JobAd *jad)");

	// Creating unique identifier
	JobId *jid = new JobId();
	if (LB_PORT == 0 ) {
		jid->setJobId(LB_ADDRESS);
	} else  {
		jid->setJobId(LB_ADDRESS, LB_PORT);
	}
	
	// Getting Input Sandbox Destination URI
	getSandboxDestURIResponse getSandboxDestURI_response;
	getSandboxDestURI(getSandboxDestURI_response, jid->toString());
	string dest_uri = getSandboxDestURI_response.path;
	
	// Setting job identifier
	dag->setAttribute(WMPExpDagAd::EDG_JOBID, jid->toString());
	
	// Adding WMProxyDestURI and InputSandboxDestURI attributes
	if (!dag->hasAttribute(JDL::ISB_BASE_URI)) {
		dag->setReserved(JDL::ISB_BASE_URI, dest_uri);
	}
	dag->setReserved(JDL::WMPISB_BASE_URI, dest_uri);

	// Initializing logger
	WMPEventLogger wmplogger;
	wmplogger.init(NS_ADDRESS, NS_PORT, jid);
	
	// Checking for partitionable registration needs
	vector<string> jobids;
	if (jad) { ///TBC
		// Partitionable job registration
		jobListMatchResponse jobListMatch_response;
		jobListMatch(jobListMatch_response, jad->toSubmissionString());
		int res_num = (jobListMatch_response.CEIdAndRankList->file)->size(); // TBD JobSteps Check
		if (jad->hasAttribute(JDL::PREJOB)) {
			res_num++;
		}
		if (jad->hasAttribute(JDL::POSTJOB)) {
			res_num++;
		}
		jobids = wmplogger.registerPartitionable(dag, res_num);
	} else {
		jobids = wmplogger.registerDag(dag);
	}
	
	// Creating private job directory with delegated Proxy
	setJobFileSystem(delegation_id, jid->toString(), jobids);
	
	// Logging delegation id & original jdl
	wmplogger.logUserTag(JDL::DELEGATION_ID, delegation_id);
	wmplogger.logUserTag(JDL::JDL_ORIGINAL, jdl);
	
	// Creating job identifier structure to return to the caller
	JobIdStructType *job_id_struct = new JobIdStructType();
	JobIdStruct job_struct = dag->getJobIdStruct();
	delete jid;
	job_id_struct->id = job_struct.jobid.toString(); // should be equal to: jid->toString()
	job_id_struct->name = job_struct.nodeName;
	job_id_struct->childrenJob = convertJobIdStruct(job_struct.children);
	jobRegister_response.jobIdStruct = job_id_struct;

	GLITE_STACK_CATCH();
}

void
jobStart(jobStartResponse &jobStart_response, const string &job_id)
{
	GLITE_STACK_TRY("jobStart(jobStartResponse &jobStart_response, const "
		"string &job_id)");
	edglog_fn("   wmpoperations::jobStart");
	
	JobId *jid = new JobId(job_id);

	// Checking if the job has already been started  //TBD do it with events
	JobStatus status = getStatus(jid);
	if (status.status != JobStatus::SUBMITTED) {
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart(jobStartResponse &jobStart_response, const string &job_id)",
			wmputilities::WMS_OPERATION_NOT_ALLOWED, "The job has already been started");
	} else {
		// TBD Check the field INSTANCE to see if the register has been done by
		// the same WMProxy manager
		string proxy(getenv(DOCUMENT_ROOT) + FILE_SEPARATOR
			+ wmputilities::to_filename(*jid)
			+ FILE_SEPARATOR + USER_PROXY_NAME);
		
		// Getting jdl
		string jdl = status.getValString(JobStatus::JDL);
		Ad *ad = new Ad(jdl);
		
		// Adding Proxy attributes
		ad->setAttribute(JDL::CERT_SUBJ, wmputilities::getUserDN());
		ad->setAttribute(JDLPrivate::USERPROXY, proxy);
		
		submit(ad->toString(), jid);
		delete jid;
	}

	GLITE_STACK_CATCH();
}

void
submit(const string &jdl, JobId *jid)
{
	WMPEventLogger wmplogger;
	edglog_fn("   wmpoperations::submit");
	wmplogger.init(NS_ADDRESS, NS_PORT, jid);

	wmpmanager::WMPManager manager;
	//task::Pipe<classad::ClassAd*> pipe;
	//task::Task taskManager(manager, pipe, 1); // Single manager
	
	// Vector of parameters to runCommand()
	vector<string> params;
	params.push_back(jdl);
	string document_root = getenv(DOCUMENT_ROOT);
	params.push_back(document_root);
	params.push_back(document_root + FILE_SEPARATOR
		+ wmputilities::to_filename(*jid));
	
	wmp_fault_t wmp_fault = manager.runCommand("JobSubmit", params);

	if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
		wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ABORT,
			wmp_fault.message.c_str(), true, true);
		throw JobOperationException(__FILE__, __LINE__,
			"submit(const string &jdl, JobId *jid)",
			wmp_fault.code, wmp_fault.message);
	} else {
		wmplogger.logEvent(eventlogger::LOG_ACCEPT, "");
	}
}

void
jobSubmit(jobSubmitResponse &jobSubmit_response, const string &jdl,
	const string &delegation_id)
{
	edglog_fn("   wmpoperations::jobSubmit");
	GLITE_STACK_TRY("jobSubmit(jobSubmitResponse &jobSubmit_response, const "
		"string &jdl, const string &delegation_id)");

	// Registering the job for submission
	jobRegisterResponse jobRegister_response;
	jobRegister(jobRegister_response, jdl, delegation_id);

	// Getting job identifier from register response
	string jobid = jobRegister_response.jobIdStruct->id;

	// Starting job submission
	JobId *jid = new JobId(jobid);
	submit(jdl, jid);
	delete jid;

	jobSubmit_response.jobIdStruct = jobRegister_response.jobIdStruct;

	GLITE_STACK_CATCH();
}

void
jobCancel(jobCancelResponse &jobCancel_response, const string &job_id)
{
	GLITE_STACK_TRY("jobCancel(jobCancelResponse &jobCancel_response, const "
		"string &job_id)");
		
	edglog_fn("   wmpoperations::jobCancel");
	edglog(info)<<"\njobCancel called for job: \n\t"<<job_id<<endl;

  	JobId *jid = new JobId(job_id);
  	
  	// Getting job status to check if cancellation is possible
	JobStatus status = getStatus(jid);
	
	if (status.getValBool(JobStatus::CANCELLING)) {
		edglog(severe)<<"\nCancel has already been requested"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobCancel(jobCancelResponse &jobCancel_response, const string "
			"&job_id)", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"Cancel has already been requested");
	}

	WMPEventLogger wmplogger;
	
	// Vector of parameters to pass to runCommand()
	vector<string> params;
	
	// TBC
	wmpmanager::WMPManager manager;
	//task::Pipe<classad::ClassAd*> pipe;
	//task::Task taskManager(manager, pipe, 1); // Single manager
	wmp_fault_t wmp_fault;
	
	// Initializing logger
	wmplogger.init(NS_ADDRESS, NS_PORT, jid);
	switch (status.status) {
		case JobStatus::SUBMITTED: //TBD check this state with call to LBProxy (use events)
			// The register of the job has been done
			//TBC should I check if MyProxyServer was present in jdl??
			wmplogger.unregisterProxyRenewal();
			wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_ABORT,
				"Cancelled by user", true, true);
			break;
		case JobStatus::WAITING:
		case JobStatus::READY:
		case JobStatus::SCHEDULED:
		case JobStatus::RUNNING:
			params.push_back(jid->toString());
			wmp_fault = manager.runCommand("jobCancel", params,
				&jobCancel_response);
			
			if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
				edglog(fatal)<<"\n" + wmp_fault.message<<endl;
				throw JobOperationException(__FILE__, __LINE__,
					"jobCancel(jobCancelResponse &jobCancel_response, const "
					"string &job_id)",
					wmp_fault.code, wmp_fault.message);
			}
			wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_CANCEL,
				"Cancelled by user", true, true);
			break;
		case JobStatus::DONE:
			// If the job is DONE, then cancellation is allowed only if
			// DONE_CODE = Failed (1)
			if (status.getValInt(JobStatus::DONE_CODE) == 1) {
				params.push_back(jid->toString());
				wmp_fault = manager.runCommand("jobCancel", params,
					&jobCancel_response);
				
				if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
					edglog(fatal)<<"\n" + wmp_fault.message<<endl;
					throw JobOperationException(__FILE__, __LINE__,
						"jobCancel(jobCancelResponse &jobCancel_response, const "
						"string &job_id)",
						wmp_fault.code, wmp_fault.message);
				}
				wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_CANCEL,
					"Cancelled by user", true, true);
			}
			break;
		default: // Any other value: CLEARED, ABORTED, CANCELLED, PURGED
			edglog(severe)<<"\nJob status doesn't allow job cancel"<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"jobCancel(jobCancelResponse &jobCancel_response, "
				"const string &job_id)", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"Job status doesn't allow job cancel");
			break;
  	}
  	delete jid;

 	GLITE_STACK_CATCH();
}

void
getMaxInputSandboxSize(getMaxInputSandboxSizeResponse
	&getMaxInputSandboxSize_response)
{
	GLITE_STACK_TRY("getMaxInputSandboxSize(getMaxInputSandboxSizeResponse "
		"&getMaxInputSandboxSize_response)");
	edglog_fn("   wmpoperations::getMaxInputSandboxSize");

	try {
		getMaxInputSandboxSize_response.size =
		// WARNING: Temporal cast TBD
		// WARNING: double temporarely casted into long (soon long will be returned directly
		(long)singleton_default<WmproxyConfiguration>::instance()
			.wmp_config->max_input_sandbox_size();
	} catch (exception &ex) {
		edglog(severe)<<"\n"<<"Unable to get max input sandbox size: "<<ex.what()<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"getMaxInputSandboxSize(getMaxInputSandboxSizeResponse "
			"&getMaxInputSandboxSize_response)",
			wmputilities::WMS_IS_FAILURE, "Unable to get max input sandbox size");
	}

	GLITE_STACK_CATCH();
}

void
getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response,
	const string &jid)
{
	GLITE_STACK_TRY("getSandboxDestURI(getSandboxDestURIResponse "
		"&getSandboxDestURI_response, const string &jid)");
	edglog_fn("   wmpoperations::getSandboxDestURI");
	edglog(info)<<"\nGetting Sandbox dest URI for job:\n\t"<<jid<<endl;
	
	try {
		getSandboxDestURI_response.path = getenv(DOCUMENT_ROOT)
			+ FILE_SEPARATOR + wmputilities::to_filename(JobId(jid));
		// TBD check value???
		/*if (document_root ...) {
			edglog(fatal)<<"\nUnable to get DOCUMENT_ROOT environment variable"<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"setJobFileSystem(const string &delegation_id, const string &jobid, "
				"vector<string> &jobids)",
				WMS_FATAL, "Unable to get DOCUMENT_ROOT environment variable");
		}*/
	} catch (Exception &ex) {
		edglog(severe)<<"\n"<<ex.what()<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"getSandboxDestURI(getSandboxDestURIResponse "
			"&getSandboxDestURI_response, string jid)",
			wmputilities::WMS_IS_FAILURE, ex.what());
	}
	
	edglog(info)<<"\nSandbox path retrieved successfully:\n\t"
		<<getSandboxDestURI_response.path<<endl;
	
	GLITE_STACK_CATCH();
}

void
getQuota(getQuotaResponse &getQuota_response)
{
	GLITE_STACK_TRY("getQuota(getQuotaResponse &getQuota_response)");
	edglog_fn("   wmpoperations::getQuota");
	
	//TBD Choose a file to save into
	//FILE * file = new FILE();
	cerr<<"----- WMPAuthorizer"<<endl;
	authorizer::WMPAuthorizer *auth = new authorizer::WMPAuthorizer(stderr);
	auth->checkUserAuthZ();
	auth->mapUser();
	string uname = auth->getUserName();
	cerr<<"----- User Name: "<<uname<<endl;
	//TBD Use LCAS method
	//string uname = "peppe";
	pair<long, long> quotas;
	if (!wmputilities::getUserQuota(quotas, uname)) {
		edglog(severe)<<"\nUnable to get total quota"<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"getQuota(getQuotaResponse &getQuota_response)",
			wmputilities::WMS_IS_FAILURE, "Unable to get total quota");
	}
	getQuota_response.softLimit = quotas.first;
	getQuota_response.hardLimit = quotas.second;
	
	GLITE_STACK_CATCH();
}

void
getFreeQuota(getFreeQuotaResponse &getFreeQuota_response)
{
	GLITE_STACK_TRY("getFreeQuota(getFreeQuotaResponse &getFreeQuota_response)");
	edglog_fn("   wmpoperations::getFreeQuota");
	
	//TBD Use LCAS method
	string uname = "peppe";
	pair<long, long> quotas;
	if (!wmputilities::getUserFreeQuota(quotas, uname)) {
		edglog(severe)<<"\nUnable to get free quota"<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"getFreeQuota(getFreeQuotaResponse &getFreeQuota_response)",
			wmputilities::WMS_IS_FAILURE, "Unable to get free quota");
	}
	getFreeQuota_response.softLimit = quotas.first;
	getFreeQuota_response.hardLimit = quotas.second;
	
	GLITE_STACK_CATCH();
}

void
jobPurge(jobPurgeResponse &jobPurge_response, const string &jid)
{
	GLITE_STACK_TRY("jobPurge(jobPurgeResponse &jobPurge_response, const "
		"string &jid)");
	edglog_fn("   wmpoperations::jobPurge");
  	
  	JobId *jobid = new JobId(jid);
  	
  	// Getting job status to check if purge is possible
	JobStatus status = getStatus(jobid);
	
	// Purge allowed if the job is in ABORTED state or DONE success
	// DONE_CODE = Failed (0)
	if ((status.status == JobStatus::ABORTED)
		|| ((status.status == JobStatus::DONE)
			&& (status.getValInt(JobStatus::DONE_CODE) == 0))) {
		int level = 0;
		string path = getenv(DOCUMENT_ROOT);
		if (!wmputilities::doPurge(jid, path)) {
			edglog(severe)<<"\nUnable to perform job purge"<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"jobPurge(jobPurgeResponse &jobPurge_response, const string &jid)",
				wmputilities::WMS_IS_FAILURE, "Unable to perform job purge");
		}
		// Initializing logger
		WMPEventLogger wmplogger;
		wmplogger.init(NS_ADDRESS, NS_PORT, jobid);
		wmplogger.logEvent(eventlogger::WMPEventLogger::LOG_CLEAR,
			"Job purged by user");
	} else {
		edglog(severe)<<"\nJob state doesn't allow purge operation"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"jobPurge(jobPurgeResponse &jobPurge_response, const string &jid)",
			wmputilities::WMS_OPERATION_NOT_ALLOWED, "Job state doesn't allow "
			"purge operation");
	}
	
	edglog(info) << "\njob Purged successfully" << endl;
	GLITE_STACK_CATCH();
}

void
getOutputFileList(getOutputFileListResponse &getOutputFileList_response,
	const string &jid)
{
	GLITE_STACK_TRY("getOutputFileList(getOutputFileListResponse "
		"&getOutputFileList_response, const string &jid)");
	edglog_fn("   wmpoperations::getOutputFileList");
	
	JobId *jobid = new JobId(jid);
  	
  	// Getting job status to check if purge is possible
	JobStatus status = getStatus(jobid);

	if ((status.status == JobStatus::ABORTED)
		|| ((status.status == JobStatus::DONE)
			&& (status.getValInt(JobStatus::DONE_CODE) == 0))) {
		// OUTPUT stage area = SandboxDestURIResponse/output
		getSandboxDestURIResponse getSandboxDestURI_response;
		getSandboxDestURI(getSandboxDestURI_response, jid);
		string output_uri = getSandboxDestURI_response.path + FILE_SEPARATOR
			+ "output";
		edglog(debug)<<"\noutput_uri = " << output_uri <<endl;
		edglog(debug)<<"\nnow calling path... " << output_uri <<endl;
		
		// find files inside directory
		const boost::filesystem::path p(output_uri,boost::filesystem::system_specific);
		edglog(debug)<<"\nPath filled" << endl;
		std::vector<std::string> found;
		glite::wms::wmproxy::commands::list_files(p, found);
		edglog(debug)<<"\nlist files called, size is: "<<found.size()<<endl;
		
		// Create and return the list:
		StringAndLongList *list = new StringAndLongList();
		vector<StringAndLongType*> *file = new vector<StringAndLongType*>;
		StringAndLongType *item = NULL;
		for (unsigned int i = 0; i < found.size(); i++) {
			item = new StringAndLongType();
			item->name = string(found[i]);
			item->size = 5 + i;
			edglog(debug)<<"\npush back address = " <<item<<endl;
			file->push_back(item);
		}
		list->file = file;
		getOutputFileList_response.OutputFileAndSizeList = list;
		
		edglog(info)<<"\nSuccessfully retrieved files: "<<found.size()<<endl;
	} else {
		edglog(severe)<<"\nJob state doesn't allow get output file list operation"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"getOutputFileList(getOutputFileListResponse "
			"&getOutputFileList_response, const string &jid)",
			wmputilities::WMS_OPERATION_NOT_ALLOWED, "Job state doesn't allow "
			"get output file list operation");
	}
	
	GLITE_STACK_CATCH();
}

void
jobListMatch(jobListMatchResponse &jobListMatch_response, const string &jdl)
{
	GLITE_STACK_TRY("jobListMatch(jobListMatchResponse &jobListMatch_response, "
		"const string &jdl)");
	edglog_fn("   wmpoperations::jobListMatch");
	int type = getType(jdl);

	/*if (type == TYPE_JOB) {
		vector<string> params;
		wmpmanager::WMPManager manager;
		task::Pipe<classad::ClassAd*> pipe;
		task::Task taskManager(manager, pipe, 1); // Single manager
		params.push_back(jdl);
		wmp_fault_t wmp_fault = manager.runCommand("jobListMatch", params,
			&jobListMatch_response);
		cerr<<"----- wmp_fault: "<<wmp_fault.code<<endl;
		if (wmp_fault.code != wmputilities::WMS_NO_ERROR) {
			throw JobOperationException(__FILE__, __LINE__,
				"jobListMatch(jobListMatchResponse &jobListMatch_response, "
				"const string &jdl)",
				wmp_fault.code, wmp_fault.message);
		}
	} else {
		throw JobOperationException(__FILE__, __LINE__,
			"jobListMatch(jobListMatchResponse &jobListMatch_response, "
			"string jdl)", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"Operation permitted only for normal job");
	}

	cerr<<"----- FUORI CICLO"<<endl;
	for (int i = 0; (jobListMatch_response.CEIdAndRankList)->file->size(); i++) {
		cerr<<"CICLO"<<endl;
		cerr<<(*(*(jobListMatch_response.CEIdAndRankList)->file)[i]).name<<endl;
	}*/
	
	/// To remove. Only to test
	StringAndLongList *list = new StringAndLongList();
	vector<StringAndLongType*> *file = new vector<StringAndLongType*>;
	StringAndLongType *item = new StringAndLongType();
	item->name = string("First");
	item->size = 5;
	file->push_back(item);
	StringAndLongType *item2 = new StringAndLongType();
	item2->name = string("Second");
	item2->size = 50;
	file->push_back(item2);
	list->file = file;
	jobListMatch_response.CEIdAndRankList = list;
	/// END To remove. Only to test
	
	GLITE_STACK_CATCH();
}
	
void
getJobTemplate(getJobTemplateResponse &getJobTemplate_response,
	JobTypeList jobType, const string &executable,
	const string &arguments, const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getJobTemplate(getJobTemplateResponse "
		"&getJobTemplate_response, JobTypeList jobType, const string "
		"&executable, const string &arguments, const string &requirements, "
		"const string &rank)");
	edglog_fn("   wmpoperations::getJobTemplate");
	
	getJobTemplate_response.jdl =
		(AdConverter::createJobTemplate(convertJobTypeListToInt(jobType),
		executable, arguments, requirements, rank))->toString();
		
	edglog(info) << "\nTemplate retrieved successfully" << endl;
	GLITE_STACK_CATCH();
}

void
getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response,
	GraphStructType dependencies, const string &requirements,
	const string &rank)
{
	GLITE_STACK_TRY("getDAGTemplate(getDAGTemplateResponse "
		"&getDAGTemplate_response, GraphStructType dependencies, const string "
		"&requirements, const string &rank)");
	edglog_fn("   wmpoperations::getDAGTemplate");
	
	getDAGTemplate_response.jdl = AdConverter::createDAGTemplate(
		convertGraphStructTypeToNodeStruct(dependencies),
		requirements, rank)->toString();
		
	edglog(info) << "\nTemplate retrieved successfully" << endl;
	GLITE_STACK_CATCH();
}

void
getCollectionTemplate(getCollectionTemplateResponse
	&getCollectionTemplate_response, int jobNumber,
	const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getCollectionTemplate(getCollectionTemplateResponse "
		"&getCollectionTemplate_response, int jobNumber, const string "
		"&requirements, const string &rank)");
	edglog_fn("   wmpoperations::getCollectionTemplate");

	getCollectionTemplate_response.jdl =
		AdConverter::createCollectionTemplate(jobNumber, requirements,
			rank)->toString();
			
	edglog(info) << "\nTemplate retrieved successfully" << endl;
	GLITE_STACK_CATCH();
}

void
getIntParametricJobTemplate(
	getIntParametricJobTemplateResponse &getIntParametricJobTemplate_response,
	StringList *attributes, int param, int parameterStart, int parameterStep,
	const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("");
	edglog_fn("   wmpoperations::getIntParametricJobTemplate");
	
	getIntParametricJobTemplate_response.jdl =
		AdConverter::createIntParametricTemplate(*(attributes->Item), param,
			parameterStart, parameterStep, requirements, rank)->toString();
			
	edglog(info) << "\nTemplate retrieved successfully" << endl;
	
	GLITE_STACK_CATCH();
}

void
getStringParametricJobTemplate(
	getStringParametricJobTemplateResponse &getStringParametricJobTemplate_response,
	StringList *attributes,
	StringList *param, const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("");
	edglog_fn("   wmpoperations::getStringParametricJobTemplate");
	
	getStringParametricJobTemplate_response.jdl =
		AdConverter::createStringParametricTemplate(*(attributes->Item),
		*(param->Item), requirements, rank)->toString();
		
	edglog(info) << "\nTemplate retrieved successfully" << endl;
	
	GLITE_STACK_CATCH();
}

void
getProxyReq(getProxyReqResponse &getProxyReq_response,
	const string &delegation_id)
{
	GLITE_STACK_TRY("getProxyReq(getProxyReqResponse &getProxyReq_response, "
		"const string &delegation_id)");
	edglog_fn("   wmpoperations::getProxyReq");
	
	if (delegation_id == "") {
		edglog(severe)<<"\nProvided delegation id not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"getProxyReq(getProxyReqResponse &getProxyReq_response, "
			"const string &delegation_id)",
			wmputilities::WMS_DELEGATION_ERROR, "Provided delegation id not valid");
	}
	
	getProxyReq_response.request =
		WMPDelegation::getProxyRequest(delegation_id);
	edglog(info) << "\nProxy requested successfully" << endl;
	
	GLITE_STACK_CATCH();
}

void
putProxy(putProxyResponse &putProxyReq_response, const string &delegation_id,
	const string &proxy)
{
	GLITE_STACK_TRY("putProxy(putProxyResponse &putProxyReq_response, "
		"const string &delegation_id, const string &proxy)");
	edglog_fn("   wmpoperations::putProxy");
	
	if (delegation_id == "") {
		edglog(severe)<<"\nProvided delegation id not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"putProxy(putProxyResponse &putProxyReq_response, const string "
			"&delegation_id, const string &proxy)",
			wmputilities::WMS_DELEGATION_ERROR, "Provided delegation id not valid");
	}
	
	WMPDelegation::putProxy(delegation_id, proxy);
	edglog(info)<<"\nProxy put successfully"<<endl;
	
	GLITE_STACK_CATCH();
}


//} // wmproxy
//} // wms
//} // glite
