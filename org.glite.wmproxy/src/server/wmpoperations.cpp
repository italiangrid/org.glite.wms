/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include "wmpoperations.h"

#include "wmproxy.h"
#include "wmplogger.h"
#include "wmpexpdagad.h"

// Exceptions
#include "wmpexceptions.h"
#include "exception_codes.h"
#include "glite/wms/jdl/JobAdExceptions.h"

// Request Ad
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/jdl_attributes.h"

// Logging and Bookkeeping
#include "glite/lb/producer.h"
#include "glite/lb/Event.h"
#include "glite/lb/Job.h"
#include "glite/lb/JobStatus.h"

// AdConverter class for jdl convertion and templates
#include "glite/wms/jdl/adconverter.h"

using namespace std;
using namespace glite::wmsutils::exception; //Exception
using namespace glite::wmsutils::jobid; //JobId
using namespace glite::wms::jdl; // DagAd, AdConverter
using namespace glite::lb; // JobStatus

using namespace wmproxyname;


//namespace glite {
//namespace wms {
//namespace wmproxyname {

// Possible values for jdl type attribute
enum type {
	TYPE_JOB,
	TYPE_DAG,
	TYPE_COLLECTION,
};

// "private" methods prototypes
glite::lb::JobStatus getStatus(glite::wmsutils::jobid::JobId *jid);

void registJob(jobRegisterResponse &jobRegister_response, const string &jdl);

void registDag(jobRegisterResponse &jobRegister_response, const string &jdl);

void start(jobStartResponse &jobStart_response, JobId *jid, WMPLogger &wmplogger);


/**
 * Converts JobIdStruct vector into a GraphStructType vector pointer
 */
vector<GraphStructType*> *
convertJobIdStruct(vector<JobIdStruct*> &job_struct)
{
	vector<GraphStructType*> *graph_struct_type = new vector<GraphStructType*>;
	GraphStructType *graph_struct = NULL;
	for (unsigned int i = 0; i < job_struct.size(); i++) {
		graph_struct = new GraphStructType();
		graph_struct->id = job_struct[i]->jobid.toString(); // should be equal to: jid->toString()
		graph_struct->name = *(job_struct[i]->nodeName);
		graph_struct->childrenJobNum = job_struct[i]->children.size();
		graph_struct->childrenJob = convertJobIdStruct(job_struct[i]->children);
		graph_struct_type->push_back(graph_struct);
	}
	return graph_struct_type;
}
	
/**
 * Returns an int value representing the type of the job described by the jdl
 * If type is not specified -> Job  change it in the future?
 */
int
getJobType(string jdl)
{
	GLITE_STACK_TRY("getJobType(string jdl)");
	
	int return_value = TYPE_JOB;
	try {
		Ad *in_ad = new Ad(jdl);
		if (in_ad->hasAttribute(JDL::TYPE, JDL_TYPE_DAG)) {
			return_value = TYPE_DAG;
		}
		/*
		if (in_ad->hasAttribute(JDL::TYPE)) {
			string type = (in_ad->getStringValue(JDL::TYPE))[0];
			if (type == JDL_TYPE_DAG) {
				return_value = TYPE_DAG;
			} else if (type == JDL_TYPE_JOB) {
				return_value = TYPE_JOB;
			//} else if (type == JDL_TYPE_COLLECTION) {
			//	return_value = TYPE_COLLECTION;
			}
		}
		*/
		delete in_ad;
	} catch (Exception &exc) {
		cerr<<"---->> getJobType() Exception: "<<exc.what()<<endl;
		throw exc;
	} catch (exception &ex) {
		cerr<<"---->> getJobType() exception: "<<ex.what()<<endl;
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
	
	int type = 0;
	for (unsigned int i = 0; i < job_type_list.jobType->size(); i++) {
		switch ((*job_type_list.jobType)[i]) {
			case WMS_PARAMETRIC:
				type |= AdConverter::ADCONV_JOBTYPE_PARAMETRIC;
				break;
			case WMS_NORMAL:
				type |= AdConverter::ADCONV_JOBTYPE_NORMAL;
				break;
			case WMS_INTERACTIVE:
				type |= AdConverter::ADCONV_JOBTYPE_INTERACTIVE;
				break;
			case WMS_MPI:
				type |= AdConverter::ADCONV_JOBTYPE_MPICH;
				break;
			case WMS_PARTITIONABLE:
				type |= AdConverter::ADCONV_JOBTYPE_PARTITIONABLE;
				break;
			case WMS_CHECKPOINTABLE:
				type |= AdConverter::ADCONV_JOBTYPE_CHECKPOINTABLE;
				break;
			default:
				break;
		}
	}
	return type;
	
	GLITE_STACK_CATCH();
}

void
ping(pingResponse &ping_response)
{
	GLITE_STACK_TRY("ping(pingResponse &ping_response)");
	/*
	org::glite::daemon::WMPManager manager;
	return manager.runCommand("Ping", ping_response);
	*/
	ping_response.isUp = true;

	GLITE_STACK_CATCH();
}

void
getVersion(getVersionResponse &getVersion_response)
{
	GLITE_STACK_TRY("getVersion(getVersionResponse &getVersion_response)");
	/*
	org::glite::daemon::WMPManager manager;
	return manager.runCommand("getVersion", getVersion_response);
	*/
	getVersion_response.version = "version";

	GLITE_STACK_CATCH();
}

void
jobRegister(jobRegisterResponse &jobRegister_response, const string &jdl)
{
	GLITE_STACK_TRY("jobRegister(jobRegisterResponse &jobRegister_response, string jdl)");

	try {
		switch (getJobType(jdl)) {
			case TYPE_DAG:
				cerr<<"---->> DAG"<<endl;
				registDag(jobRegister_response, jdl);
				break;
			case TYPE_JOB:
				cerr<<"---->> JOB"<<endl;
				registJob(jobRegister_response, jdl);
				break;
			default:
				throw JobOperationException(__FILE__, __LINE__,
					"jobRegister(jobRegisterResponse &jobRegister_response, string jdl)",
					WMS_OPERATION_NOT_ALLOWED, "Job Register not allowed");
		}
	} catch (Exception &exc) {
		cerr<<"---->> jobRegister() Exception: "<<exc.what()<<endl;
		throw exc;
	} catch (exception &ex) {
		cerr<<"---->> jobRegister() exception: "<<ex.what()<<endl;
		throw ex;
	}

	GLITE_STACK_CATCH();
}

void
registJob(jobRegisterResponse &jobRegister_response, const string &jdl)
{
	GLITE_STACK_TRY("registJob(jobRegisterResponse &jobRegister_response, const string &jdl)");

	// Creating job instance
	JobAd *jad = NULL;
	try {
		cerr<<"---->> jdl before parse: "<<jdl<<endl;
		jad = new JobAd(jdl);
		jad->check(); // This changes ISB files to absolute paths
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		throw JobOperationException(__FILE__, __LINE__,
			"registJob(jobRegisterResponse &jobRegister_response, const string &jdl)",
			WMS_JDL_PARSING, "Jdl parsing error");
	}
	
	// Modifing jdl to set InputSandbox with only absolute paths
	// Alex method??? do it with the check method??
	
	// Creating unique identifier
	JobId *jid = new JobId();
	if (LB_PORT == 0 ) {
		jid->setJobId(LB_ADDRESS);
	} else  {
		jid->setJobId(LB_ADDRESS, LB_PORT);
	}
	
	// Getting Input Sandbox Destination URI to insert as WMProxyDestURI
	// attribute
	getSandboxDestURIResponse getSandboxDestURI_response;
	try {
		getSandboxDestURI(getSandboxDestURI_response, jid->toString());
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		throw ex;
	}
	string dest_uri = getSandboxDestURI_response.path;
	
	// Adding WMProxyDestURI and InputSandboxDestURI attributes
	try {
		if (!jad->hasAttribute(JDL::ISB_DEST_URI)) {
			jad->setAttribute(JDL::ISB_DEST_URI, dest_uri);
		}
		jad->setAttribute(JDL::WMPROXY_DEST_URI, dest_uri);
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		throw ex;
	}

	WMPLogger wmplogger;
	wmplogger.init(LB_ADDRESS, LB_PORT, jid);

	jad->setAttribute(JDL::JOBID, jid->toString());
	if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_PARTITIONABLE)) { // CHECK IT
		// Partitioning job registration
		jobListMatchResponse jobListMatch_response;
		try {
			jobListMatch(jobListMatch_response, jad->toSubmissionString());
		} catch (Exception &exc) {
			throw JobOperationException(__FILE__, __LINE__,
					"registJob(jobRegisterResponse &jobRegister_response, const string &jdl)",
					exc.getCode(), (string) exc.what());
		} catch (exception &ex) {
			throw ex;
		}
		int res_number = (jobListMatch_response.CEIdList->Item)->size(); // TBD JobSteps Check
		if (jad->hasAttribute(JDL::PREJOB)) {
			res_number++;
		}
		if (jad->hasAttribute(JDL::POSTJOB)) {
			res_number++;
		}
		wmplogger.registerJob(jad, res_number);
	} else {
		// Normal job registration
		wmplogger.registerJob(jad);
	}
	delete jad;
	
	// Logging original jdl
	cerr<<"---->> jdl original: "<<jdl<<endl;
	wmplogger.logOriginalJdl(jdl);

	GraphStructType *job_id_struct = new GraphStructType();
	job_id_struct->id = jid->toString();
	delete jid;
	job_id_struct->name = "";
	job_id_struct->childrenJobNum = 0;
	job_id_struct->childrenJob = new vector<GraphStructType*>;

	jobRegister_response.jobIdStruct = job_id_struct;

	GLITE_STACK_CATCH();
}

/*void
setChildrenDestinationURI(WMPExpDagAd *dag, string dest_uri)
{
		// Adding WMProxyDestURI and InputSandboxDestURI attributes to children
		JobIdStruct structure = dag->getStructure();
		vector<JobIdStruct*> children = structure.children;
		for (unsigned int i = 0; i < children.size(); i++) {
			try {
				dag->getNodeAttribute(children[i]->nodeName, JDL::ISB_DEST_URI);
			} catch (Exception &exc) {
				// Attribute not present
				dag->setNodeAttribute(children[i]->nodeName, JDL::ISB_DEST_URI,
					dest_uri);
			}
			if (children[i]->children->size() != 0) {
				// RECURSIVE CALL!!!
			}
		}
		
		
		//struct JobIdStruct{
		//	glite::wmsutils::jobid::JobId jobid ;
		//	std::string* nodeName ;
		//	std::vector< JobIdStruct* > children ;
		//};
}*/

void
setDestinationURI(WMPExpDagAd *dag, string dest_uri)
{
	try {
		/*try {
			dag->getAttribute(JDL::ISB_DEST_URI);
		} catch (Exception &exc) {
			// Attribute not present
			dag->removeAttribute(JDL::ISB_DEST_URI);
			dag->setAttribute(JDL::ISB_DEST_URI, dest_uri);
		}
		dag->setAttribute(JDL::WMPROXY_DEST_URI, dest_uri);
		*/
		/*
		JobIdStruct structure = dag->getStructure();
		vector<JobIdStruct*> children = structure.children;
		for (unsigned int i = 0; i < children.size(); i++) {
			try {
				dag->getNodeAttribute(children[i]->nodeName, JDL::ISB_DEST_URI);
			} catch (Exception &exc) {
				// Attribute not present
				dag->removeNodeAttribute(JDL::ISB_DEST_URI);
				dag->setNodeAttribute(children[i]->nodeName, JDL::ISB_DEST_URI,
					dest_uri);
			}
			dag->setNodeAttribute(children[i]->nodeName, JDL::WMP_DEST_URI,
					dest_uri);
		}*/
		//setChildrenDestinationURI(dag, dest_uri);
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		throw ex;
	}
}

	
void
registDag(jobRegisterResponse &jobRegister_response, const string &jdl)
{
	GLITE_STACK_TRY("registDag(jobRegisterResponse &jobRegister_response, const string &jdl)");

	// Creating job instance
	WMPExpDagAd *dag = NULL;
	try {
		cerr<<"---->> jdl before parse: "<<jdl<<endl;
		dag = new WMPExpDagAd(jdl);
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		throw JobOperationException(__FILE__, __LINE__,
			"registDag(jobRegisterResponse &jobRegister_response, const string &jdl)",
			WMS_JDL_PARSING, "Jdl parsing error");
	}
	
	// Modifing jdl to set InputSandbox with only absolute paths
	// Alex method??? do it with the check method??
	
	// Creating unique identifier
	JobId *jid = new JobId();
	if (LB_PORT == 0 ) {
		jid->setJobId(LB_ADDRESS);
	} else  {
		jid->setJobId(LB_ADDRESS, LB_PORT);
	}
	
	// Getting Input Sandbox Destination URI to insert as WMProxyDestURI
	// attribute
	getSandboxDestURIResponse getSandboxDestURI_response;
	try {
		getSandboxDestURI(getSandboxDestURI_response, jid->toString());
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		throw ex;
	}
	string dest_uri = getSandboxDestURI_response.path;
	
	// Adding WMProxyDestURI and InputSandboxDestURI attributes
	try {
		if (!dag->hasAttribute(JDL::ISB_DEST_URI)) {
			dag->setReserved(JDL::ISB_DEST_URI, dest_uri);
		}
		dag->setReserved(JDL::WMPROXY_DEST_URI, dest_uri);
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		throw ex;
	}

	WMPLogger wmplogger;
	wmplogger.init(LB_ADDRESS, LB_PORT, jid);
	delete jid;
	wmplogger.setDestinationURI(dest_uri);

	dag->setAttribute(WMPExpDagAd::EDG_JOBID, jid->toString());
	wmplogger.registerDag(dag);
	delete dag;
	
	// Logging original jdl
	wmplogger.logOriginalJdl(jdl);
	
	GraphStructType *job_id_struct = new GraphStructType();
	JobIdStruct job_struct = dag->getJobIdStruct();
	job_id_struct->id = job_struct.jobid.toString(); // should be equal to: jid->toString()
	job_id_struct->name = *job_struct.nodeName;
	job_id_struct->childrenJobNum = job_struct.children.size();
	job_id_struct->childrenJob = convertJobIdStruct(job_struct.children);

	jobRegister_response.jobIdStruct = job_id_struct;

	GLITE_STACK_CATCH();
}

void
jobStart(jobStartResponse &jobStart_response, string jobId)
{
	GLITE_STACK_TRY("jobStart(jobStartResponse &jobStart_response, string jobId)");

	JobId *jid = new JobId(jobId);
	WMPLogger wmplogger;
	wmplogger.init(SERVER_ADDRESS, SERVER_PORT, jid);

	try {
		start(jobStart_response, jid, wmplogger);
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		throw ex;
	}
	delete jid;

	// Running wmproxy command
	/*wmpmanager::WMPManager manager;
	wmp_fault_t wmp_fault = manager.runCommand("JobStart", *jobId, job_start_response);*/
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		throw JobOperationException(__FILE__, __LINE__,
				"jobStart(jobStartResponse &jobStart_response, string jobId)",
				wmp_fault.code, wmp_fault.message);
	}

	GLITE_STACK_CATCH();
}

void
start(jobStartResponse &jobStart_response, JobId *jid, WMPLogger &wmplogger)
{
	GLITE_STACK_TRY("start(jobStartResponse &jobStart_response, JobId *jid, WMPLogger &wmplogger)");

	JobStatus stat = getStatus(jid);
	string jdl = stat.getValString(JobStatus::JDL);
	cerr<<"jdl: "<<jdl<<endl;
	
	int type = TYPE_JOB;
	try {
		type = getJobType(jdl);
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		throw ex;
	}

	if (type == TYPE_DAG) {
		WMPExpDagAd *dag = new WMPExpDagAd(jdl);
		wmplogger.transfer(WMPLogger::START, dag->toString());
		dag->setAttribute(WMPExpDagAd::SEQUENCE_CODE, wmplogger.getSequence());
		wmplogger.logUserTags(dag->getSubAttributes(JDL::USERTAGS));
		jdl = dag->toString();
		delete dag;
	} else if (type == TYPE_JOB) {
		JobAd *jad = new JobAd(jdl);
		wmplogger.transfer(WMPLogger::START, jad->toSubmissionString());
		jad->setAttribute(JDL::LB_SEQUENCE_CODE, wmplogger.getSequence());
		jdl = jad->toSubmissionString();
		if (jad->hasAttribute(JDL::USERTAGS)) {
			wmplogger.logUserTags((classad::ClassAd*) jad->delAttribute(JDL::USERTAGS));
		}
		delete jad;
	}

	try {
		/*wmpmanager::WMPManager manager;
		wmp_fault_t wmp_fault = manager.runCommand("JobStart", jid, jobStart_response);*/
		wmplogger.transfer(WMPLogger::OK, jdl);
	} catch (Exception &exc) {
		wmplogger.transfer(WMPLogger::FAIL, jdl, exc.what());
		throw exc;
	} catch (exception &ex) {
		wmplogger.transfer(WMPLogger::FAIL, jdl, ex.what());
		throw ex;
	}

	GLITE_STACK_CATCH();
}

void
jobSubmit(jobSubmitResponse &jobSubmit_response, string jdl)
{
	GLITE_STACK_TRY("jobSubmit(jobSubmitResponse &jobSubmit_response, string jdl)");

	// Registering the job for submission
	jobRegisterResponse jobRegister_response;
	try {
		jobRegister(jobRegister_response, jdl);
	} catch (Exception &exc) {
		throw JobOperationException(__FILE__, __LINE__,
				"jobSubmit(jobSubmitResponse &jobSubmit_response, string jdl)",
				exc.getCode(), (string) exc.what());
	} catch (exception &ex) {
		throw ex;
	}

	// Getting job identifier from register response
	string jid = jobRegister_response.jobIdStruct->id;

	// Starting job submission
	jobStartResponse jobStart_response;
	try {
		jobStart(jobStart_response, jid);
	} catch (Exception &exc) {
		throw JobOperationException(__FILE__, __LINE__,
				"jobSubmit(jobSubmitResponse &jobSubmit_response, string jdl)",
				exc.getCode(), (string) exc.what());
	} catch (exception &ex) {
		throw ex;
	}

	jobSubmit_response.jobIdStruct = jobRegister_response.jobIdStruct;

	GLITE_STACK_CATCH();
}

void
jobCancel(jobCancelResponse &jobCancel_response, string jobId)
{
	GLITE_STACK_TRY("jobCancel(jobCancelResponse &jobCancel_response, string jobId)");

  	JobId *jid = new JobId(jobId);
	JobStatus status = getStatus(jid);
	delete jid;
	switch (status.status) {
		case JobStatus::SUBMITTED:
		case JobStatus::WAITING:
		case JobStatus::READY:
		case JobStatus::SCHEDULED:
		case JobStatus::RUNNING:
			break;
		case JobStatus::DONE:
			// If the job is DONE, then cancellation is allowed only if  DONE_CODE = Failed (1)
			if (status.getValInt (JobStatus::DONE_CODE) ==1) {
				break;
			}
		default: // Any other value: CLEARED, ABORTED, CANCELLED, PURGED
			throw JobOperationException(__FILE__, __LINE__,
					"jobCancel(jobCancelResponse &jobCancel_response, string jobId)",
					WMS_OPERATION_NOT_ALLOWED, "Cancel not allowed: check the status");
  	}
	if (status.getValBool(JobStatus::CANCELLING)) {
		throw JobOperationException(__FILE__, __LINE__,
				"jobCancel(jobCancelResponse &jobCancel_response, string jobId)",
				WMS_OPERATION_NOT_ALLOWED, "Cancel has been already requested");
	}
	// cases SUBMITTED, WAITING, READY, SCHEDULED, RUNNING, DONE (DONE_CODE = Failed).
	/*org::glite::daemon::WMPManager manager;
	wmp_fault_t wmp_fault = manager.runCommand("jobCancel", jobCancel_response);
	*/
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		throw JobOperationException(__FILE__, __LINE__,
				"jobCancel(jobCancelResponse &jobCancel_response, string jobId)",
				wmp_fault.code, wmp_fault.message);
	}

 	GLITE_STACK_CATCH();
}

void
getMaxInputSandboxSize(getMaxInputSandboxSizeResponse &getMaxInputSandboxSize_response)
{
	GLITE_STACK_TRY("getMaxInputSandboxSize(getMaxInputSandboxSizeResponse &getMaxInputSandboxSize_response)");

	/*
	org::glite::daemon::WMPManager manager;
	wmp_fault_t wmp_fault = manager.runCommand("getMaxInputSandboxSize", getMaxInputSandboxSize_response);
	*/
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		throw JobOperationException(__FILE__, __LINE__,
				"getMaxInputSandboxSize(getMaxInputSandboxSizeResponse &getMaxInputSandboxSize_response)",
				wmp_fault.code, wmp_fault.message);
	}
	getMaxInputSandboxSize_response.size = 23;

	GLITE_STACK_CATCH();
}

void
getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response, string jid)
{
	GLITE_STACK_TRY("getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response, string jid)");
	/*
	org::glite::daemon::WMPManager manager;
	wmp_fault_t wmp_fault = manager.runCommand("getSandboxDestURI", jid,  getSandboxDestURI_response);
	*/
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		throw JobOperationException(__FILE__, __LINE__,
				"getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response, string jid)",
				wmp_fault.code, wmp_fault.message);
	}
	getSandboxDestURI_response.path = "root";

	GLITE_STACK_CATCH();
}

void
getQuota(getQuotaResponse &getQuota_response)
{
	GLITE_STACK_TRY("getQuota(getQuotaResponse &getQuota_response)");
	/*
	org::glite::daemon::WMPManager manager;
	wmp_fault_t wmp_fault = manager.runCommand("getQuota", getQuota_response);
	*/
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		throw JobOperationException(__FILE__, __LINE__,
				"getQuota(getQuotaResponse &getQuota_response)",
				wmp_fault.code, wmp_fault.message);
	}

	GLITE_STACK_CATCH();
}

void
getFreeQuota(getFreeQuotaResponse &getFreeQuota_response)
{
	GLITE_STACK_TRY("getFreeQuota(getFreeQuotaResponse &getFreeQuota_response)");
	/*
	org::glite::daemon::WMPManager manager;
	return manager.runCommand("getFreeQuota", getFreeQuota_response);
	*/
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		throw JobOperationException(__FILE__, __LINE__,
				"getFreeQuota(getFreeQuotaResponse &getFreeQuota_response)",
				wmp_fault.code, wmp_fault.message);
	}

	GLITE_STACK_CATCH();
}

void
jobPurge(jobPurgeResponse &jobPurge_response, string jid)
{
	GLITE_STACK_TRY("jobPurge(jobPurgeResponse &jobPurge_response, string jid)");
	/*
	org::glite::daemon::WMPManager manager;
	return manager.runCommand("jobPurge",jid, jobPurge_response);
	*/
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		throw JobOperationException(__FILE__, __LINE__,
				"jobPurge(jobPurgeResponse &jobPurge_response, string jid)",
				wmp_fault.code, wmp_fault.message);
	}

	GLITE_STACK_CATCH();
}

void
getOutputFileList(getOutputFileListResponse &getOutputFileList_response, string jid)
{
	GLITE_STACK_TRY("getOutputFileList(getOutputFileListResponse &getOutputFileList_response, string jid)");
	/*
	org::glite::daemon::WMPManager manager;
	return manager.runCommand("getOutputFileList", jid, getOutputFileList_response);
	*/
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		throw JobOperationException(__FILE__, __LINE__,
				"getOutputFileList(getOutputFileListResponse &getOutputFileList_response, string jid)",
				wmp_fault.code, wmp_fault.message);
	}
	GLITE_STACK_CATCH();
}

void
jobListMatch(jobListMatchResponse &jobListMatch_response, string jdl)
{
	GLITE_STACK_TRY("jobListMatch(jobListMatchResponse &jobListMatch_response, string jdl)");

	int type = TYPE_JOB;
	try {
		type = getJobType(jdl);
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		throw ex;
	}
	
	if (type == TYPE_JOB) {
		/*
		org::glite::daemon::WMPManager manager;
		wmp_fault_t wmp_fault = manager.runCommand("jobListMatch", jdl, jobListMatch_response);
		*/
		wmp_fault_t wmp_fault;
		wmp_fault.code = WMS_NO_ERROR;
		if (wmp_fault.code != WMS_NO_ERROR) {
			throw JobOperationException(__FILE__, __LINE__,
					"jobListMatch(jobListMatchResponse &jobListMatch_response, string jdl)",
					wmp_fault.code, wmp_fault.message);
		}
	} else {
		throw JobOperationException(__FILE__, __LINE__,
					"jobListMatch(jobListMatchResponse &jobListMatch_response, string jdl)",
					WMS_OPERATION_NOT_ALLOWED, "Operation permitted only for normal job");
	}

	GLITE_STACK_CATCH();
}

void
getJobTemplate(getJobTemplateResponse &getJobTemplate_response, JobTypeList jobType, string executable,
	string arguments, string requirements, string rank)
{
	GLITE_STACK_TRY("getJobTemplate(getJobTemplateResponse &getJobTemplate_response, JobTypeList jobType, string executable, string arguments, string requirements, string rank)");

	string vo = "Fake VO"; // get it from proxy, it should be the default one
	AdConverter converter;
	getJobTemplate_response.jdl =
		(converter.createJobTemplate(convertJobTypeListToInt(jobType),
		executable, arguments, requirements, rank, vo))->toString();

	GLITE_STACK_CATCH();
}

void
getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response, GraphStructType dependencies,
	string requirements, string rank)
{
	GLITE_STACK_TRY("getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response, GraphStructType dependencies, string requirements, string rank)");

	string vo = "Fake VO";
	AdConverter converter;
	//getDAGTemplate_response.jdl = converter.createDAGTemplate( dependencies, requirements, rank, vo);  // TO CONVERT dependencies

	GLITE_STACK_CATCH();
}

void
getCollectionTemplate(getCollectionTemplateResponse &getCollectionTemplate_response, int job_number,
	string requirements, string rank)
{
	GLITE_STACK_TRY("getCollectionTemplate(getCollectionTemplateResponse &getCollectionTemplate_response, int job_number, string requirements, string rank)");

	string vo = "Fake VO";
	AdConverter converter;
	//getCollectionTemplate_response.jdl = new string(converter.createCollectionTemplate(job_number, requirements, rank, vo)->toString());

	GLITE_STACK_CATCH();
}

glite::lb::JobStatus
getStatus(JobId *jid)
{
	GLITE_STACK_TRY("getStatus(JobId *jid)");

	glite::lb::Job lb_job(*jid);
	return lb_job.status(glite::lb::Job::STAT_CLASSADS); // to get also jdl
	// lb_job.status(0) minimal information about the job

	GLITE_STACK_CATCH();
}


//} // wmproxy
//} // wms
//} // glite
