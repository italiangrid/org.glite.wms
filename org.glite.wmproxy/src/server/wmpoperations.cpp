/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include <fstream>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "wmpoperations.h"

#include "wmproxy.h"
#include "wmplogger.h"
#include "wmpexpdagad.h"

// Delegation
#include "wmpdelegation.h"

// Exceptions
#include "wmpexceptions.h"
#include "wmpexception_codes.h"
#include "glite/wms/jdl/RequestAdExceptions.h"

// RequestAd
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

// Default name of the delegated Proxy that is copied inside private job 
// directory
const std::string USER_PROXY_NAME = "/proxy";

//#include "glite/wms/common/utilities/globus_ftp_utils.h"

using namespace std;
using namespace glite::lb; // JobStatus
using namespace wmproxyname;
using namespace glite::wms::jdl; // DagAd, AdConverter
using namespace glite::wmsutils::jobid; //JobId
using namespace glite::wmsutils::exception; //Exception
using namespace glite::wms::common::configuration; // Configuration

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
			cerr<<"Type: "<<type<<endl;
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
			//case WMS_NORMAL:
				//type |= AdConverter::ADCONV_JOBTYPE_NORMAL;
				//break;
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
	if (type = 0) {
		type = AdConverter::ADCONV_JOBTYPE_NORMAL;
	}
	cerr<<"job type: "<<type<<endl;
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


// WM Web Service available operations
// To get more infomation see WM service wsdl file

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
	const string &delegation_id)
{
	GLITE_STACK_TRY("jobRegister(jobRegisterResponse &jobRegister_response, "
		"const string &jdl, const string &delegation_id)");

	// Checking delegation id
	if (delegation_id == "") {
  		throw ProxyOperationException(__FILE__, __LINE__,
			"jobRegister(jobRegisterResponse &jobRegister_response, "
			"const string &jdl, const string &delegation_id)",
			WMS_DELEGATION_ERROR, "Delegation id not valid");
	}

	try {
		int type = getType(jdl);
		if (type == TYPE_DAG) {
			cerr<<"---->> DAG"<<endl;
			WMPExpDagAd *dag = new WMPExpDagAd(jdl);
			/// DO IT??  dag->check(); // This changes ISB files to absolute paths
			regist(jobRegister_response, delegation_id, jdl, dag);
			delete dag;
		} else if (type == TYPE_JOB) {
			cerr<<"---->> JOB"<<endl;
			JobAd *jad = new JobAd(jdl);
			//jad->check(); // This changes ISB files to absolute paths
			if (jad->hasAttribute(JDL::JOBTYPE)) {
				string job_type = (jad->getStringValue(JDL::TYPE))[0];
				if (job_type == JDL_JOBTYPE_PARAMETRIC) {
					cerr<<"---->> PARAMETRIC"<<endl;
					delete jad;
					WMPExpDagAd *dag =
						new WMPExpDagAd(*(AdConverter::bulk2dag(jdl)));
					regist(jobRegister_response, delegation_id, jdl, dag);
					delete dag;
				} else if (job_type == JDL_JOBTYPE_PARTITIONABLE) {
					cerr<<"---->> PARTITIONABLE"<<endl;
					WMPExpDagAd *dag =
						new WMPExpDagAd(*(AdConverter::part2dag(jdl)));
					regist(jobRegister_response, delegation_id, jdl, dag, jad);
					delete jad;
					delete dag;
				} else { // Default Job Type is Normal
					cerr<<"---->> NORMAL"<<endl;
					regist(jobRegister_response, delegation_id, jdl, jad);
					delete jad;
				}
			} else {
				regist(jobRegister_response, delegation_id, jdl, jad);
				delete jad;
			}
		} else if (type == TYPE_COLLECTION) {
			cerr<<"---->> COLLECTION"<<endl;
			WMPExpDagAd *dag =
				new WMPExpDagAd(*(AdConverter::collection2dag(jdl)));
			/// DO IT??  dag->check(); // This changes ISB files to absolute paths
			cerr<<"---->> DAG jobad: "<<dag->toString()<<endl;
			regist(jobRegister_response, delegation_id, jdl, dag);
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
setJobFileSystem(const string &delegation_id, const string &dest_uri)
{
	GLITE_STACK_TRY("setJobFileSystem(const string &delegation_id, const "
		"string &dest_uri)");
		
	// Getting delegated Proxy file name
	string delegated_proxy =
		WMPDelegation::getDelegatedProxyPath(delegation_id);
	cerr<<"Delegated Proxy file name: "<<delegated_proxy<<endl;
	
	// Creating destination URI path
	mode_t mode(755);
	cerr<<"creating directory "<<dest_uri<<endl;
	if (mkdir(dest_uri.c_str(), mode) != 0) {
		throw JobOperationException(__FILE__, __LINE__,
			"setJobFileSystem(const string &delegation_id, const "
			"string &dest_uri)",
			WMS_IS_FAILURE, "Unable to create job local directory");
	}
	
	// Copying delegated Proxy to destination URI
	/*char buffer[200];
	string target_file = dest_uri + USER_PROXY_NAME;
	sprintf(buffer, "cp %s %s", delegated_proxy.c_str(), target_file.c_str());
	if (system(buffer) == -1) {
		throw JobOperationException(__FILE__, __LINE__,
			"setJobFileSystem(const string &delegation_id, const "
			"string &dest_uri)",
			WMS_IS_FAILURE, "Unable to copy Proxy file");
	}*/
	char ch;
	ifstream source_stream;
  	ofstream target_stream;
  	filebuf *source_buffer;
  	filebuf *target_buffer;

  	source_stream.open(delegated_proxy.c_str());
  	string target_file = dest_uri + USER_PROXY_NAME;
  	target_stream.open(target_file.c_str());

  	source_buffer=source_stream.rdbuf();
  	target_buffer=source_stream.rdbuf();

  	ch = source_buffer->sgetc();
  	while (ch != EOF) {
    	target_buffer->sputc(ch);
    	ch = source_buffer->snextc();
  	}

  	target_stream.close();
  	source_stream.close();
  	
  	GLITE_STACK_CATCH();
}

void
regist(jobRegisterResponse &jobRegister_response, const string &delegation_id,
	const string &jdl, JobAd *jad)
{
	GLITE_STACK_TRY("regist(jobRegisterResponse &jobRegister_response, "
		"const string &delegation_id, const string &jdl, JobAd *jad)");
	
	// Modifing jdl to set InputSandbox with only absolute paths
	// Alex method??? do it with the check method??
	
	// Creating unique identifier
	JobId *jid = new JobId();
	if (LB_PORT == 0) {
		jid->setJobId(LB_ADDRESS);
	} else  {
		jid->setJobId(LB_ADDRESS, LB_PORT);
	}
	
	// Getting Input Sandbox Destination URI
	getSandboxDestURIResponse getSandboxDestURI_response;
	getSandboxDestURI(getSandboxDestURI_response, jid->toString());
	string dest_uri = getSandboxDestURI_response.path;
	cerr<<"dest_uri: "<<dest_uri<<endl;
	
	// Creating private job directory with delegated Proxy
	setJobFileSystem(delegation_id, dest_uri);
	
	// Setting job identifier
	jad->setAttribute(JDL::JOBID, jid->toString());
	
	// Adding WMProxyDestURI and InputSandboxDestURI attributes
	if (!jad->hasAttribute(JDL::ISB_DEST_URI)) {
		jad->setAttribute(JDL::ISB_DEST_URI, dest_uri);
	}
	jad->setAttribute(JDL::WMPROXY_DEST_URI, dest_uri);

	// Initializing logger
	WMPLogger wmplogger;
	wmplogger.init(NS_ADDRESS, NS_PORT, jid);
	
	// Registering the job
	wmplogger.registerJob(jad);
	
	// Registering for Proxy renewal
	if (jad->hasAttribute(JDL::MYPROXY)) {
		wmplogger.registerProxyRenewal(
			WMPDelegation::getDelegatedProxyPath(delegation_id),
			(jad->getStringValue(JDL::MYPROXY))[0]);
	}
	
	// Logging delegation id & original jdl
	//wmplogger.logUserTag(JDL::DELEGATION_ID, delegation_id);
	wmplogger.logUserTag(JDL::JDL_ORIGINAL, jdl);

	// Creating job identifier structure to return to the caller
	JobIdStructType *job_id_struct = new JobIdStructType();
	job_id_struct->id = jid->toString();
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
	regist(jobRegister_response, delegation_id, jdl, dag, NULL);
}

void
regist(jobRegisterResponse &jobRegister_response, const string &delegation_id, 
	const string &jdl, WMPExpDagAd *dag, JobAd *jad)
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
	
	// Getting Input Sandbox Destination URI
	getSandboxDestURIResponse getSandboxDestURI_response;
	getSandboxDestURI(getSandboxDestURI_response, jid->toString());
	string dest_uri = getSandboxDestURI_response.path;
	
	// Creating private job directory with delegated Proxy
	setJobFileSystem(delegation_id, dest_uri);
	
	// Setting job identifier
	dag->setAttribute(WMPExpDagAd::EDG_JOBID, jid->toString());
	
	// Adding WMProxyDestURI and InputSandboxDestURI attributes
	if (!dag->hasAttribute(JDL::ISB_DEST_URI)) {
		dag->setReserved(JDL::ISB_DEST_URI, dest_uri);
	}
	dag->setReserved(JDL::WMPROXY_DEST_URI, dest_uri);
	
	// Initializing logger
	WMPLogger wmplogger;
	wmplogger.init(NS_ADDRESS, NS_PORT, jid);
	
	// Checking for partitionable registration needs
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
		wmplogger.registerPartitionable(dag, res_num);
	} else {
		wmplogger.registerDag(dag);
	}
	
	// Logging delegation id & original jdl
	//wmplogger.logUserTag(JDL::DELEGATION_ID, delegation_id);
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

	JobId *jid = new JobId(job_id);

	// Checking if the job has already been started  //TBD do it with events
	JobStatus status = getStatus(jid);
	if (status.status != JobStatus::SUBMITTED) {
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart(jobStartResponse &jobStart_response, const string &job_id)",
			WMS_OPERATION_NOT_ALLOWED, "The job has already been started");
	}
	
	// Getting jdl
	string jdl = status.getValString(JobStatus::JDL);
	cerr<<"jdl: "<<jdl<<endl;
	
	try {
		submit(jdl, jid);
		delete jid;
	} catch (Exception &exc) {
		throw JobOperationException(__FILE__, __LINE__,
			"jobStart(jobStartResponse &jobStart_response, const string &job_id)",
			exc.getCode(), (string) exc.what());
	}
	
	GLITE_STACK_CATCH();
}

void
submit(const string &jdl, JobId *jid)
{
	WMPLogger wmplogger;
	//wmplogger.init(SERVER_ADDRESS, SERVER_PORT, jid);
	//wmplogger.init(LB_ADDRESS, LB_PORT, jid);
	wmplogger.init(NS_ADDRESS, NS_PORT, jid);
	
	/*wmpmanager::WMPManager manager;
	wmp_fault_t wmp_fault = manager.runCommand("JobSubmit", jdl, jobStart_response);*/

	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		wmplogger.logAbort(wmp_fault.message.c_str());
		throw JobOperationException(__FILE__, __LINE__,
			"submit(const string &jdl, JobId *jid)",
			wmp_fault.code, wmp_fault.message);
	} else {
		wmplogger.logAccepted(jid->toString());
	}
}

void
jobSubmit(jobSubmitResponse &jobSubmit_response, const string &jdl,
	const string &delegation_id)
{
	GLITE_STACK_TRY("jobSubmit(jobSubmitResponse &jobSubmit_response, const "
		"string &jdl, const string &delegation_id)");

	// Registering the job for submission
	jobRegisterResponse jobRegister_response;
	try {
		jobRegister(jobRegister_response, jdl, delegation_id);
	} catch (Exception &exc) {
		throw JobOperationException(__FILE__, __LINE__,
			"jobSubmit(jobSubmitResponse &jobSubmit_response, const string "
			"&jdl, const string &delegation_id)",
			exc.getCode(), (string) exc.what());
	}

	// Getting job identifier from register response
	string jobid = jobRegister_response.jobIdStruct->id;

	// Starting job submission
	JobId *jid = new JobId(jobid);
	try {
		submit(jdl, jid);
		delete jid;
	} catch (Exception &exc) {
		throw JobOperationException(__FILE__, __LINE__,
			"jobSubmit(jobSubmitResponse &jobSubmit_response, const string "
			"&jdl, const string &delegation_id)",
			exc.getCode(), (string) exc.what());
	}

	jobSubmit_response.jobIdStruct = jobRegister_response.jobIdStruct;

	GLITE_STACK_CATCH();
}

void
jobCancel(jobCancelResponse &jobCancel_response, const string &job_id)
{
	GLITE_STACK_TRY("jobCancel(jobCancelResponse &jobCancel_response, const "
		"string &job_id)");

  	JobId *jid = new JobId(job_id);
	JobStatus status = getStatus(jid);
	
	if (status.getValBool(JobStatus::CANCELLING)) {
		throw JobOperationException(__FILE__, __LINE__,
			"jobCancel(jobCancelResponse &jobCancel_response, const string "
			"&job_id)", WMS_OPERATION_NOT_ALLOWED,
			"Cancel has been already requested");
	}
	
	WMPLogger wmplogger;
	
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR; //TBD remove when WMPManager coded
	switch (status.status) {
		case JobStatus::SUBMITTED:
			// The register of the job has been done
			
			// Initializing logger
			wmplogger.init(NS_ADDRESS, NS_PORT, jid);
			wmplogger.unregisterProxyRenewal();
			
			wmplogger.logAbort("Cancelled by user");
			break;
		case JobStatus::WAITING:
		case JobStatus::READY:
		case JobStatus::SCHEDULED:
		case JobStatus::RUNNING:
			/*
			org::glite::daemon::WMPManager manager;
			wmp_fault_t wmp_fault = manager.runCommand("jobCancel",
				jobCancel_response);
			*/
			if (wmp_fault.code != WMS_NO_ERROR) {
				throw JobOperationException(__FILE__, __LINE__,
					"jobCancel(jobCancelResponse &jobCancel_response, const "
					"string &job_id)",
					wmp_fault.code, wmp_fault.message);
			}
			break;
		case JobStatus::DONE:
			// If the job is DONE, then cancellation is allowed only if
			// DONE_CODE = Failed (1)
			if (status.getValInt(JobStatus::DONE_CODE) == 1) {
				/*
				org::glite::daemon::WMPManager manager;
				wmp_fault_t wmp_fault = manager.runCommand("jobCancel",
					jobCancel_response);
				*/
				if (wmp_fault.code != WMS_NO_ERROR) {
					throw JobOperationException(__FILE__, __LINE__,
						"jobCancel(jobCancelResponse &jobCancel_response, const "
						"string &job_id)",
						wmp_fault.code, wmp_fault.message);
				}
			}
			break;
		default: // Any other value: CLEARED, ABORTED, CANCELLED, PURGED
			throw JobOperationException(__FILE__, __LINE__,
				"jobCancel(jobCancelResponse &jobCancel_response, "
				"const string &job_id)", WMS_OPERATION_NOT_ALLOWED,
				"Cancel not allowed: check the status");
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

	try {
		getMaxInputSandboxSize_response.size =
			Configuration::instance()->ns()->max_input_sandbox_size();
	} catch (exception &ex) {
		throw JobOperationException(__FILE__, __LINE__,
			"getMaxInputSandboxSize(getMaxInputSandboxSizeResponse "
			"&getMaxInputSandboxSize_response)",
			WMS_IS_FAILURE, ex.what());
	}

	GLITE_STACK_CATCH();
}

void
getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response,
	const string &jid)
{
	GLITE_STACK_TRY("getSandboxDestURI(getSandboxDestURIResponse "
		"&getSandboxDestURI_response, const string &jid)");
	/*
	org::glite::daemon::WMPManager manager;
	wmp_fault_t wmp_fault = manager.runCommand("getSandboxDestURI", jid,
		getSandboxDestURI_response);
	*/
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		throw JobOperationException(__FILE__, __LINE__,
			"getSandboxDestURI(getSandboxDestURIResponse "
			"&getSandboxDestURI_response, string jid)",
			wmp_fault.code, wmp_fault.message);
	}
	int length = jid.length();
	getSandboxDestURI_response.path = "/home/gridsite/jobdir/"
		+ jid.substr(length - 22, length);

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
	getQuota_response.softLimit = 100;
	getQuota_response.hardLimit = 200;

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
	
	getFreeQuota_response.softLimit = 100;
	getFreeQuota_response.hardLimit = 200;

	GLITE_STACK_CATCH();
}

void
jobPurge(jobPurgeResponse &jobPurge_response, const string &jid)
{
	GLITE_STACK_TRY("jobPurge(jobPurgeResponse &jobPurge_response, const "
		"string &jid)");
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
getOutputFileList(getOutputFileListResponse &getOutputFileList_response,
	const string &jid)
{
	GLITE_STACK_TRY("getOutputFileList(getOutputFileListResponse "
		"&getOutputFileList_response, const string &jid)");
	/*
	org::glite::daemon::WMPManager manager;
	retun manager.runCommand("getOutputFileList", jid,
		getOutputFileList_response);
	*/
	wmp_fault_t wmp_fault;
	wmp_fault.code = WMS_NO_ERROR;
	if (wmp_fault.code != WMS_NO_ERROR) {
		throw JobOperationException(__FILE__, __LINE__,
			"getOutputFileList(getOutputFileListResponse "
			"&getOutputFileList_response, const string &jid)",
			wmp_fault.code, wmp_fault.message);
	}
	
	/// To remove. Only to test
	StringAndLongList *list = new StringAndLongList();
	vector<StringAndLongType*> *file = new vector<StringAndLongType*>;
	StringAndLongType *item = new StringAndLongType();
	item->name = *(new string("First"));
	item->size = 5;
	file->push_back(item);
	StringAndLongType *item2 = new StringAndLongType();
	item2->name = *(new string("Second"));
	item2->size = 50;
	file->push_back(item2);
	list->file = file;
	getOutputFileList_response.OutputFileAndSizeList = list;
	///
	
	GLITE_STACK_CATCH();
}

void
jobListMatch(jobListMatchResponse &jobListMatch_response, const string &jdl)
{
	GLITE_STACK_TRY("jobListMatch(jobListMatchResponse &jobListMatch_response, "
		"const string &jdl)");

	int type = getType(jdl);
	
	if (type == TYPE_JOB) {
		/*
		org::glite::daemon::WMPManager manager;
		wmp_fault_t wmp_fault = manager.runCommand("jobListMatch", jdl,
			jobListMatch_response);
		*/
		wmp_fault_t wmp_fault;
		wmp_fault.code = WMS_NO_ERROR;
		if (wmp_fault.code != WMS_NO_ERROR) {
			throw JobOperationException(__FILE__, __LINE__,
				"jobListMatch(jobListMatchResponse &jobListMatch_response, "
				"const string &jdl)",
				wmp_fault.code, wmp_fault.message);
		}
	} else {
		throw JobOperationException(__FILE__, __LINE__,
			"jobListMatch(jobListMatchResponse &jobListMatch_response, "
			"string jdl)", WMS_OPERATION_NOT_ALLOWED,
			"Operation permitted only for normal job");
	}
	
	/// To remove. Only to test
	StringAndLongList *list = new StringAndLongList();
	vector<StringAndLongType*> *file = new vector<StringAndLongType*>;
	StringAndLongType *item = new StringAndLongType();
	item->name = *(new string("First"));
	item->size = 5;
	file->push_back(item);
	StringAndLongType *item2 = new StringAndLongType();
	item2->name = *(new string("Second"));
	item2->size = 50;
	file->push_back(item2);
	list->file = file;
	jobListMatch_response.CEIdAndRankList = list;
	///

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

	string vo = "Fake VO"; // get it from proxy, it should be the default one
	getJobTemplate_response.jdl =
		(AdConverter::createJobTemplate(convertJobTypeListToInt(jobType),
		executable, arguments, requirements, rank, vo))->toString();

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

	string vo = "Fake VO";
	getDAGTemplate_response.jdl = AdConverter::createDAGTemplate(
		convertGraphStructTypeToNodeStruct(dependencies),
		requirements, rank, vo)->toString();

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

	string vo = "Fake VO";
	getCollectionTemplate_response.jdl =
		AdConverter::createCollectionTemplate(jobNumber, requirements, rank,
			vo)->toString();

	GLITE_STACK_CATCH();
}

void
getIntParametricJobTemplate(
	getIntParametricJobTemplateResponse &getIntParametricJobTemplate_response,
	StringList *attributes, int param, int parameterStart, int parameterStep,
	const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("");

	string vo = "Fake VO";
	int parametrised = 2;
	getIntParametricJobTemplate_response.jdl =
		AdConverter::createIntParametricTemplate(parametrised, param,
			parameterStart, parameterStep, requirements, rank, vo)->toString();

	GLITE_STACK_CATCH();
}

void
getStringParametricJobTemplate(
	getStringParametricJobTemplateResponse &getStringParametricJobTemplate_response,
	StringList *attributes,
	StringList *param, const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("");

	string vo = "Fake VO";
	int parametrised = 2;
	getStringParametricJobTemplate_response.jdl =
		AdConverter::createStringParametricTemplate(parametrised,
		*(param->Item), requirements, rank, vo)->toString();

	GLITE_STACK_CATCH();
}

void
getProxyReq(getProxyReqResponse &getProxyReq_response,
	const string &delegation_id)
{
	GLITE_STACK_TRY("getProxyReq(getProxyReqResponse &getProxyReq_response, "
		"const string &delegation_id)");

	if (delegation_id == "") {
  		throw ProxyOperationException(__FILE__, __LINE__,
			"getProxyReq(getProxyReqResponse &getProxyReq_response, "
			"const string &delegation_id)",
			WMS_DELEGATION_ERROR, "Delegation id not valid");
	}
	
	getProxyReq_response.request =
		WMPDelegation::getProxyRequest(delegation_id);
	
	GLITE_STACK_CATCH();
} 
	
void
putProxy(putProxyResponse &putProxyReq_response, const string &delegation_id,
	const string &proxy)
{ 
	GLITE_STACK_TRY("putProxy(putProxyResponse &putProxyReq_response, "
		"const string &delegation_id, const string &proxy)");
	
	if (delegation_id == "") {
  		throw ProxyOperationException(__FILE__, __LINE__,
			"putProxy(putProxyResponse &putProxyReq_response, const string "
			"&delegation_id, const string &proxy)",
			WMS_DELEGATION_ERROR, "Delegation id not valid");
	}
	
	WMPDelegation::putProxy(delegation_id, proxy);
	
	GLITE_STACK_CATCH();
}


//} // wmproxy
//} // wms
//} // glite
