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
//namespace wmproxy {

// Possible values for jdl type attribute
enum type {
	TYPE_JOB,
	TYPE_DAG,
	TYPE_COLLECTION,
};

// "private" methods prototypes
glite::lb::JobStatus getStatus(glite::wmsutils::jobid::JobId *jid);

void regist(jobRegisterResponse &jobRegister_response, const string &jdl,
	JobAd *jad);

void regist(jobRegisterResponse &jobRegister_response, const string &jdl,
	WMPExpDagAd *dag);

void regist(jobRegisterResponse &jobRegister_response, const string &jdl,
	WMPExpDagAd *dag, JobAd *jad);

void start(jobStartResponse &jobStart_response, JobId *jid, WMPLogger &wmplogger);


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
 * If type is not specified -> Job  change it in the future?
 */
int
getType(string jdl)
{
	GLITE_STACK_TRY("getType(string jdl)");
	
	int return_value = TYPE_JOB;
	try {
		Ad *in_ad = new Ad(jdl);
		if (in_ad->hasAttribute(JDL::TYPE)) {
			string type = (in_ad->getStringValue(JDL::TYPE))[0];
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
		cerr<<"---->> getType() Exception: "<<exc.what()<<endl;
		throw exc;
	} catch (exception &ex) {
		cerr<<"---->> getType() exception: "<<ex.what()<<endl;
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
jobRegister(jobRegisterResponse &jobRegister_response, const string &jdl,
	const string &delegationId)
{
	GLITE_STACK_TRY("jobRegister(jobRegisterResponse &jobRegister_response, "
		"const string &jdl, string delegationId)");

	try {
		int type = getType(jdl);
		if (type == TYPE_DAG) {
				cerr<<"---->> DAG"<<endl;
				WMPExpDagAd *dag = new WMPExpDagAd(jdl);
				/// DO IT??  dag->check(); // This changes ISB files to absolute paths
				regist(jobRegister_response, jdl, dag);
				delete dag;
		} else if (type == TYPE_JOB) {
				cerr<<"---->> JOB"<<endl;
				JobAd *jad = new JobAd(jdl);
				cerr<<"---->> AFTER JOB AD"<<endl;
				cerr<<"---->> JOBAD: "<<jad->toString()<<endl;
				//jad->check(); // This changes ISB files to absolute paths
				if (jad->hasAttribute(JDL::JOBTYPE)) {
					cerr<<"---->> AFTER jad->hasAttribute(JDL::JOBTYPE)"<<endl;
					string job_type = (jad->getStringValue(JDL::TYPE))[0];
					cerr<<"---->> AFTER getStringValue"<<endl;
					if (job_type == JDL_JOBTYPE_PARAMETRIC) {
						cerr<<"---->> PARAMETRIC"<<endl;
						delete jad;
						WMPExpDagAd *dag =
							new WMPExpDagAd(*(AdConverter::bulk2dag(jdl)));
						regist(jobRegister_response, jdl, dag);
						delete dag;
					} else if (job_type == JDL_JOBTYPE_PARTITIONABLE) {
						cerr<<"---->> PARTITIONABLE"<<endl;
						WMPExpDagAd *dag =
							new WMPExpDagAd(*(AdConverter::part2dag(jdl)));
						regist(jobRegister_response, jdl, dag, jad);
						delete jad;
						delete dag;
					} else { // Default Job Type is Normal
						cerr<<"JOB NORMAL"<<endl;
						regist(jobRegister_response, jdl, jad);
						delete jad;
					}
				} else {
					cerr<<"---->> AFTER jad->hasAttribute(JDL::JOBTYPE)2 job"<<endl;
					regist(jobRegister_response, jdl, jad);
					delete jad;
				}
		} else if (type == TYPE_COLLECTION) {
				cerr<<"---->> COLLECTION"<<endl;
				cerr<<"---- jdl: "<<jdl<<endl;
				cerr<<"AdConverter::collection2dag(jdl): "<<AdConverter::collection2dag(jdl)<<endl;
				WMPExpDagAd *dag = new WMPExpDagAd(*(AdConverter::collection2dag(jdl)));
				/// DO IT??  dag->check(); // This changes ISB files to absolute paths
				cerr<<"jdl after convertion: "<<dag->toString()<<endl;
				regist(jobRegister_response, jdl, dag);
				delete dag;	
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
regist(jobRegisterResponse &jobRegister_response, const string &jdl, JobAd *jad)
{
	GLITE_STACK_TRY("registJob(jobRegisterResponse &jobRegister_response, const string &jdl)");
	
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
	cerr<<"dest_uri: "<<dest_uri<<endl;
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
	wmplogger.init(NS_ADDRESS, NS_PORT, jid);

	jad->setAttribute(JDL::JOBID, jid->toString());
	cerr<<"---->> wmplogger.registerJob(jad)"<<endl;
	wmplogger.registerJob(jad);
	
	// Logging original jdl
	cerr<<"---->> jdl original: "<<jdl<<endl;
	wmplogger.logOriginalJdl(jdl);

	JobIdStructType *job_id_struct = new JobIdStructType();
	job_id_struct->id = jid->toString();
	delete jid;
	job_id_struct->name = new string("");
	job_id_struct->childrenJob = new vector<JobIdStructType*>;

	jobRegister_response.jobIdStruct = job_id_struct;

	GLITE_STACK_CATCH();
}

void
regist(jobRegisterResponse &jobRegister_response, const string &jdl, WMPExpDagAd *dag)
{
	regist(jobRegister_response, jdl, dag, NULL);
}

void
regist(jobRegisterResponse &jobRegister_response, const string &jdl, WMPExpDagAd *dag, JobAd *jad)
{
	GLITE_STACK_TRY("regist(jobRegisterResponse &jobRegister_response, "
	"const string &jdl, WMPExpDagAd *dag, JobAd *jad)");

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
	
	cerr<<"JID: "<< jid->toString();
	WMPLogger wmplogger;
	wmplogger.init(NS_ADDRESS, NS_PORT, jid);
	wmplogger.setDestinationURI(dest_uri);

	dag->setAttribute(WMPExpDagAd::EDG_JOBID, jid->toString());
	if (jad) { // CHECK IT
		// Partitionable job registration
		jobListMatchResponse jobListMatch_response;
		try {
			jobListMatch(jobListMatch_response, jad->toSubmissionString());
		} catch (Exception &exc) {
			throw JobOperationException(__FILE__, __LINE__,
					"regist(jobRegisterResponse &jobRegister_response, const "
					"string &jdl, WMPExpDagAd *dag, bool is_partitionable)",
					exc.getCode(), (string) exc.what());
		} catch (exception &ex) {
			throw ex;
		}
		int res_num = (jobListMatch_response.CEIdAndRankList->file)->size(); // TBD JobSteps Check
		if (jad->hasAttribute(JDL::PREJOB)) {
			res_num++;
		}
		if (jad->hasAttribute(JDL::POSTJOB)) {
			res_num++;
		}
		wmplogger.registerDag(dag, res_num);
	} else {
		wmplogger.registerDag(dag);
	}
	
	// Logging original jdl
	wmplogger.logOriginalJdl(jdl);
	
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
jobStart(jobStartResponse &jobStart_response, const string &jobId)
{
	GLITE_STACK_TRY("jobStart(jobStartResponse &jobStart_response, string jobId)");

	JobId *jid = new JobId(jobId);
	WMPLogger wmplogger;
	//wmplogger.init(SERVER_ADDRESS, SERVER_PORT, jid);
	//wmplogger.init(LB_ADDRESS, LB_PORT, jid);
	wmplogger.init(NS_ADDRESS, NS_PORT, jid);
	
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
	// it should be like a submit:
	// ask logger for jdl and then check if the job hasn't already been started, if so -> error
	// wmp_fault_t wmp_fault = manager.runCommand("JobSubmit", *jdl, job_start_response);
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
		type = getType(jdl);
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		throw ex;
	}

	try {
		/*wmpmanager::WMPManager manager;
		wmp_fault_t wmp_fault = manager.runCommand("JobStart", jid, jobStart_response);*/
		wmplogger.log(WMPLogger::ACCEPTED, jid->toString(), "");
	} catch (Exception &exc) {
		wmplogger.log(WMPLogger::ABORT, "", exc.what());
		throw exc;
	} catch (exception &ex) {
		wmplogger.log(WMPLogger::ABORT, "", ex.what());
		throw ex;
	}

	GLITE_STACK_CATCH();
}

void
jobSubmit(jobSubmitResponse &jobSubmit_response, const string &jdl)
{
	GLITE_STACK_TRY("jobSubmit(jobSubmitResponse &jobSubmit_response, string jdl)");

	// Registering the job for submission
	jobRegisterResponse jobRegister_response;
	try {
		string delegation_id = "prova";
		jobRegister(jobRegister_response, jdl, delegation_id);
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
jobCancel(jobCancelResponse &jobCancel_response, const string &jobId)
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
	GLITE_STACK_TRY("getMaxInputSandboxSize(getMaxInputSandboxSizeResponse "
		"&getMaxInputSandboxSize_response)");

	/*
	org::glite::daemon::WMPManager manager;
	wmp_fault_t wmp_fault = manager.runCommand("getMaxInputSandboxSize",
		getMaxInputSandboxSize_response);
	*/
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		throw JobOperationException(__FILE__, __LINE__,
				"getMaxInputSandboxSize(getMaxInputSandboxSizeResponse "
				"&getMaxInputSandboxSize_response)",
				wmp_fault.code, wmp_fault.message);
	}
	getMaxInputSandboxSize_response.size = 23;

	GLITE_STACK_CATCH();
}

void
getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response, const string &jid)
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
jobPurge(jobPurgeResponse &jobPurge_response, const string &jid)
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
getOutputFileList(getOutputFileListResponse &getOutputFileList_response, const string &jid)
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
jobListMatch(jobListMatchResponse &jobListMatch_response, const string &jdl)
{
	GLITE_STACK_TRY("jobListMatch(jobListMatchResponse &jobListMatch_response, string jdl)");

	int type = TYPE_JOB;
	try {
		type = getType(jdl);
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
getJobTemplate(getJobTemplateResponse &getJobTemplate_response, JobTypeList jobType, const string &executable,
	const string &arguments, const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getJobTemplate(getJobTemplateResponse &getJobTemplate_response, JobTypeList jobType, string executable, string arguments, string requirements, string rank)");

	string vo = "Fake VO"; // get it from proxy, it should be the default one
	getJobTemplate_response.jdl =
		(AdConverter::createJobTemplate(convertJobTypeListToInt(jobType),
		executable, arguments, requirements, rank, vo))->toString();

	GLITE_STACK_CATCH();
}

void
getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response, GraphStructType dependencies,
	const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response, JobIdStructType dependencies, string requirements, string rank)");

	string vo = "Fake VO";
	getDAGTemplate_response.jdl = AdConverter::createDAGTemplate(
		convertGraphStructTypeToNodeStruct(dependencies),
		requirements, rank, vo)->toString();

	GLITE_STACK_CATCH();
}

void
getCollectionTemplate(getCollectionTemplateResponse &getCollectionTemplate_response, int jobNumber,
	const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getCollectionTemplate(getCollectionTemplateResponse &getCollectionTemplate_response, int job_number, string requirements, string rank)");

	string vo = "Fake VO";
	getCollectionTemplate_response.jdl =
		AdConverter::createCollectionTemplate(jobNumber, requirements, rank,
			vo)->toString();

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

/*int
ns1__getIntParametricJobTemplate(struct soap *soap, ns1__StringList *attributes,
	int param, int parameterStart, int parameterStep, string requirements,
	string rank, struct ns1__getIntParametricJobTemplateResponse &response)
{
void
getCollectionTemplate(getCollectionTemplateResponse &getCollectionTemplate_response, int jobNumber,
	const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getCollectionTemplate(getCollectionTemplateResponse &getCollectionTemplate_response, int job_number, string requirements, string rank)");

	string vo = "Fake VO";
	getCollectionTemplate_response.jdl =
		AdConverter::createCollectionTemplate(jobNumber, requirements, rank,
			vo)->toString();

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
}*/

//} // wmproxy
//} // wms
//} // glite
