/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/


#include "wmpoperations.h"

#include "wmproxy.h"
#include "wmplogger.h"

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

//#include "glite/wmsutils/JobId.h" //JobId
#include "glite/wms/jdl/adconverter.h"

// VARIOUS
//#include "glite/wmsutils/jobid/manipulation.h"  // to_filename method

using namespace std ;
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

/**
 * Returns an int value representing the type of the job described by the jdl
 */
int
getJobType(string jdl)
{
	GLITE_STACK_TRY("getJobType(string jdl)");
	Ad *in_ad = new Ad(jdl);
	if (in_ad->hasAttribute(JDL::TYPE)) {
		string type = (in_ad->getStringValue(JDL::TYPE))[0];
		cerr<<"Type: "<<type<<endl;
		if (type == JDL_TYPE_DAG) {
			cerr<<"DAG"<<endl;
			return TYPE_DAG;
		} else if (type ==JDL_TYPE_JOB) {
			cerr<<"JOB"<<endl;
			return TYPE_JOB;
		//} else if (type == JDL_TYPE_COLLECTION) {
		//	return TYPE_COLLECTION;
		}
	} else {
		cerr<<"No Type"<<endl;
		return TYPE_JOB; // If type is not specified -> Job  change it in the future?
	}
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
	for (int i = 0; i < (*(job_type_list.jobType)).size(); i++) {
		switch ((*job_type_list.jobType)[i]) {
			case PARAMETRIC:
				type |= AdConverter::ADCONV_JOBTYPE_PARAMETRIC;
				break;
			case NORMAL:
				type |= AdConverter::ADCONV_JOBTYPE_NORMAL;
				break;
			case INTERACTIVE:
				type |= AdConverter::ADCONV_JOBTYPE_INTERACTIVE;
				break;
			case MPI:
				type |= AdConverter::ADCONV_JOBTYPE_MPICH;
				break;
			case PARTITIONABLE:
				type |= AdConverter::ADCONV_JOBTYPE_PARTITIONABLE;
				break;
			case CHECKPOINTABLE:
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


/**
	1 - createId()
	2 - getDestURI()
	3 - prendere il jdl ed aggiungere:
		- InputSandboxDestURI = - A quello dell'utente se lo ha specificato
							   - A quello ottenuto con la chiamata a getDestURI()
		- WMProxyDestURI = A quello ottenuto con la chiamata a getDestURI()
				Tale URI serve per sapere dove vengono memorizzati file tipo il proxy. Tale URI è
				quindi necessario anche se l'utente ne fornisce uno di un suo server.
	4 - prendere il jdl e modificare l'ISB in modo tale che abbia solo path assoluti.
		se il path è un URI lo si lascia così
		se il path è assoluto o relativo, lo si cambia in un path assoluto formato da:
			WMProxyDestURI + nome file
	5 - settare, per ogni tipo di job,  il jdl originale come user tag: TAG_NAME = jdl_Original, TAG_VALUE = jdl
		logUserTag(jobId, jdl_Original)
	6 - register()

	Se l'utente dopo la register, fa una richiesta getDestURI(), allora deve essere ritornato lo stesso URI ottenuto
	dalla getDestURI durante la register. Tale valore lo si ottiene chiedendo ad LB il jdl del job identificato col
	jobid nella chiamata getDestURI(). Dal JDL si ricava l'informazione prendendo il WMProxyDestURI.

*/

void
jobRegister(jobRegisterResponse &jobRegister_response, string &jdl)
{
	GLITE_STACK_TRY("jobRegister(jobRegisterResponse &jobRegister_response, string jdl)");

	int type = getJobType(jdl);
	if ((type != TYPE_DAG) && (type != TYPE_JOB)) { // Registration is not allowed
		throw JobOperationException(__FILE__, __LINE__,
			"jobRegister(jobRegisterResponse &jobRegister_response, string jdl)",
			WMS_OPERATION_NOT_ALLOWED, "Job Register not allowed");
	} else { // Registration is allowed

		// Creating unique identifier
		JobId *jid = new JobId() ;
		if (LB_PORT == 0 ) {
			jid->setJobId(LB_ADDRESS);
		} else  {
			jid->setJobId(LB_ADDRESS, LB_PORT);
		}

		// Getting Input Sandbox Destination URI to insert as WMProxyDestURI Attribute
		getSandboxDestURIResponse getSandboxDestURI_response;
		try  {
			getSandboxDestURI(getSandboxDestURI_response, jid->toString());
		} catch (Exception &exc) {
			throw exc;
		} catch (exception &ex) {
			throw ex;
		}
		string dest_uri = getSandboxDestURI_response.path;

		if (type == TYPE_DAG) {
			try {
				ExpDagAd *dag = new ExpDagAd(jdl);
				regist(dag, jid, jobRegister_response, dest_uri);
				delete dag;
				delete jid;
			} catch (Exception &exc) {
				throw exc;
			} catch (exception &ex) {
				throw ex;
			}
		} else if (type == TYPE_JOB) {
			try {
				JobAd *jad = new JobAd(jdl);
				regist(jad, jid, jobRegister_response, dest_uri);
				delete jad;
				delete jid;
			//} catch (AdSyntaxException &ase) {
			//	throw ase;
			} catch (Exception &exc) {
				throw exc;
			} catch (exception &ex) {
				cerr<<"--->> Ellipses"<<endl;
				throw JobOperationException(__FILE__, __LINE__,
					"jobRegister(jobRegisterResponse &jobRegister_response, string &jdl)",
					WMS_JDL_PARSING, "Jdl parsing error");
			}
		}
	}

	GLITE_STACK_CATCH();
}

void
regist(JobAd *jad, JobId *jid, jobRegisterResponse &jobRegister_response, string dest_uri)
{
	GLITE_STACK_TRY("regist(JobAd *jad, JobId *jid, jobRegisterResponse &jobRegister_response, string dest_uri)");

	// Adding WMProxyDestURI and InputSandboxDestURI attributes and original jdl as a User Tag
	try {
		JobAd *job_ad = new JobAd();
		if (jad->hasAttribute(JDL::USERTAGS))  {
			//jod_ad = jad->getAd(JDL::USERTAGS); // UNCOMMENT ONCE IMPLEMENTED!!!
			jad->delAttribute(JDL::USERTAGS);
		}
		job_ad->setAttribute(JDL::JDL_ORIGINAL, jad->toString());
		jad->setAttribute(JDL::USERTAGS, job_ad);
		delete job_ad;
		if (!jad->hasAttribute(JDL::ISB_DEST_URI)) {
			jad->setAttribute(JDL::ISB_DEST_URI, dest_uri);
		}
		jad->setAttribute(JDL::WMPROXY_DEST_URI, dest_uri);
	//} catch (AdSyntaxException &ase) {
	//	throw ase;
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		cerr<<"--->> Ellipses"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"regist(JobAd *jad, JobId *jid, jobRegisterResponse &jobRegister_response, string dest_uri)",
			WMS_JDL_PARSING, "Jdl parsing error");
	}

	// Modifing jdl to set InputSandbox with only absolute paths
	// Alex method???

	WMPLogger wmplogger;
	wmplogger.init(LB_ADDRESS, LB_PORT, jid);

	// Perform the Logging RegisterJobSync
	regist(jad, jid, wmplogger);

	GraphStructType *job_id_struct = new GraphStructType();
	job_id_struct->id = jid->toString();
	job_id_struct->name = "name";
	job_id_struct->childrenJobNum = 0;
	job_id_struct->childrenJob = new vector<GraphStructType*>;

	jobRegister_response.jobIdStruct = job_id_struct;

	GLITE_STACK_CATCH();
}

void
regist(ExpDagAd *dag, JobId *jid, jobRegisterResponse &jobRegister_response, string dest_uri)
{
	GLITE_STACK_TRY("regist(ExpDagAd *dag, JobId *jid, jobRegisterResponse &jobRegister_response, string dest_uri)");

	// Adding WMProxyDestURI and InputSandboxDestURI attributes and original jdl as a User Tag
	try {
		/*JobAd *job_ad = new JobAd();
		if (dag->hasAttribute(JDL::USERTAGS))  {
			//jod_ad = dag->getAd(JDL::USERTAGS);
			dag->delAttribute(JDL::USERTAGS);
		}
		job_ad->setAttribute(JDL::JDL_ORIGINAL, dag->toString());
		if (!dag->hasAttribute(JDL::ISB_DEST_URI)) {
			dag->setAttribute(JDL::ISB_DEST_URI, dest_uri);
		}
		dag->setAttribute(JDL::WMPROXY_DEST_URI, dest_uri);
		*/
	} catch (Exception &exc) {
		throw exc;
	} catch (exception &ex) {
		cerr<<"--->> Ellipses"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"regist(ExpDagAd *dag, JobId *jid, jobRegisterResponse &jobRegister_response, string dest_uri)",
			WMS_JDL_PARSING, "Jdl parsing error");
	}

	// Modifing jdl to set InputSandbox with only absolute paths
	// Alex method???

	WMPLogger wmplogger;
	wmplogger.init(LB_ADDRESS, LB_PORT, jid);

	// Perform the Logging RegisterJobSync
	regist(dag, jid, wmplogger);

	GraphStructType *job_id_struct = new GraphStructType();
	job_id_struct->id = jid->toString();
	job_id_struct->name = "name";
	//job_id_struct->childrenJobNum = job_id_struct->childrenJobs.size();
	//job_id_struct->childrenJob =convertJobIdVector(job_id_struct->childrenJobs);

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

// Private method, perform the submission
void
start(jobStartResponse &jobStart_response, JobId *jid, WMPLogger &wmplogger)
{
	GLITE_STACK_TRY("start(jobStartResponse &jobStart_response, JobId *jid, WMPLogger &wmplogger)");

	JobStatus stat = getStatus(jid);
	string jdl = stat.getValString(JobStatus::JDL);
	cerr<<"jdl: "<<jdl<<endl;
	int type = getJobType(jdl);

	//// Put in register
	//// BEGIN
	if (type == TYPE_DAG) {
		ExpDagAd *dag = new ExpDagAd(jdl);
		wmplogger.transfer(WMPLogger::START, dag->toString());
		dag->setAttribute(ExpDagAd::SEQUENCE_CODE, wmplogger.getSequence());
		wmplogger.logUserTags(dag->getSubAttributes(JDL::USERTAGS));
		jdl = dag->toString() ;
		delete dag;
	} else if (type == TYPE_JOB) {
		// it's a Normal Job
		JobAd *jad = new JobAd(jdl);
		wmplogger.transfer ( WMPLogger::START, jad->toSubmissionString () ) ;
		jad->setAttribute(JDL::LB_SEQUENCE_CODE, wmplogger.getSequence());
		// UserTags implementation
		jdl = jad->toSubmissionString();
		if (jad->hasAttribute(JDL::USERTAGS)) {
			wmplogger.logUserTags((classad::ClassAd*) jad->delAttribute(JDL::USERTAGS));
		}
		delete jad;
	}
	//// END
	try {
		/*wmpmanager::WMPManager manager;
		wmp_fault_t wmp_fault = manager.runCommand("JobStart", jid,  jobStart_response);*/
		wmplogger.transfer(WMPLogger::OK, jdl) ;
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
	cerr<<"sand jid: "<<jid<<endl;
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

	int type = getJobType(jdl);
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
	AdConverter *converter = new AdConverter();
	getJobTemplate_response.jdl = (converter->createJobTemplate(convertJobTypeListToInt(jobType), executable, arguments, requirements, rank, vo))->toString();
	delete converter;

	GLITE_STACK_CATCH();
}

void
getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response, GraphStructType dependencies,
	string requirements, string rank)
{
	GLITE_STACK_TRY("getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response, GraphStructType dependencies, string requirements, string rank)");

	string vo = "Fake VO";
	AdConverter *converter = new AdConverter();
	//getDAGTemplate_response.jdl = converter->createDAGTemplate( dependencies, requirements, rank, vo);  // TO CONVERT dependencies
	delete converter;

	GLITE_STACK_CATCH();
}

void
getCollectionTemplate(getCollectionTemplateResponse &getCollectionTemplate_response, int job_number,
	string requirements, string rank)
{
	GLITE_STACK_TRY("getCollectionTemplate(getCollectionTemplateResponse &getCollectionTemplate_response, int job_number, string requirements, string rank)");

	string vo = "Fake VO";
	AdConverter *converter = new AdConverter();
	//getCollectionTemplate_response.jdl = new string(converter->createCollectionTemplate(job_number, requirements, rank, vo)->toString());
	delete converter;

	GLITE_STACK_CATCH();
}


glite::lb::JobStatus
getStatus(JobId *jid)
{
	GLITE_STACK_TRY("getStatus(JobId *jid)");

	glite::lb::Job lbJob(*jid);
	glite::lb::JobStatus status;
	status = lbJob.status(glite::lb::Job::STAT_CLASSADS); // to get also jdl
	//status = lbJob.status(0); // minimal information about the job
	return status ;

	GLITE_STACK_CATCH();
}


void
regist(ExpDagAd *dag, JobId *jid, WMPLogger &wmplogger)
{
	GLITE_STACK_TRY("regist(ExpDagAd *dag, JobId *jid, WMPLogger &wmplogger)");
	cerr<<"regist jid: "<<*jid<<endl;
	dag->setAttribute(ExpDagAd::EDG_JOBID, jid->toString());
	wmplogger.registerDag(dag);

	GLITE_STACK_CATCH();
}

void
regist(JobAd *jad, JobId *jid, WMPLogger &wmplogger)
{
	GLITE_STACK_TRY("regist(JobAd *jad, JobId *jid, WMPLogger &wmplogger)");

	jad->setAttribute(JDL::JOBID, jid->toString());
	if (jad->hasAttribute(JDL::JOBTYPE, JDL_JOBTYPE_PARTITIONABLE)) {  // CHECK IT
		// Partitioning job registration:
		jobListMatchResponse jobListMatch_response;
		try {
			jobListMatch(jobListMatch_response, jad->toSubmissionString());
		} catch (Exception &exc) {
			throw JobOperationException(__FILE__, __LINE__,
					"regist(JobAd *jad, JobId *jid, WMPLogger &wmplogger)",
					exc.getCode(), (string) exc.what());
		}
		int res_number = 0 ; // TBD JobSteps Check
		res_number = (jobListMatch_response.CEIdList->Item)->size();
		if (jad->hasAttribute(JDL::PREJOB))  {
			res_number++;
		}
		if (jad->hasAttribute(JDL::POSTJOB)) {
			res_number++;
		}
		//ExpDagAd *dag = wmplogger.registerJob(jad, res_number);
		wmplogger.registerJob(jad, res_number);
		// Release un-needed memory
		//delete jad;
		//type = TYPE_DAG;
		// throw JobOperationException( __FILE__ , __LINE__ ,"Dag regist" , WMS_JOBOP_ALLOWED , "Submission not performed...it is a partitioning" ) ;
	} else {
		// Normal job registering:
		cerr<<"jobad: "<<jad->toString()<<endl;
		wmplogger.registerJob(jad);
	}

	GLITE_STACK_CATCH();
}

//} // wmproxy
//} // wms
//} // glite
