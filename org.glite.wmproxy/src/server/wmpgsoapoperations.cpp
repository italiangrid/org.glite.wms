/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include <sstream>

// gSOAP
#include "soapH.h"

#include "wmpoperations.h"
#include "wmpresponsestruct.h"

// Exceptions
#include "wmpexceptions.h"
#include "exception_codes.h"
#include "glite/wms/jdl/JobAdExceptions.h"

#include "glite/lb/JobStatus.h"

namespace jobid = glite::wmsutils::jobid;
namespace errorcodes = wmproxyname;

using namespace std;
using namespace errorcodes;
using namespace glite::wms::jdl; // AdSyntaxException
using namespace glite::wmsutils::exception; // Exception

namespace {

/**
 * Converts int to string
 */
string
itos(int i)
{
	stringstream s;
	s << i;
	return s.str();
}

// To remove
vector<string> *
convertStackVector(vector<string> stack)
{
	vector<string> *returnVector = new vector<string>;
	if (&stack) {
		string element;
		for (int i = 0; i < stack.size(); i++) {
			element = &(stack[i][0]);
			returnVector->push_back(element);
		}
	}
	return returnVector;
}

/**
 * Converts a ns1__JobTypeList pointer to JobTypeList pointer
 */
JobTypeList *
convertFromGSOAPJobTypeList(ns1__JobTypeList *job_type_list)
{
	vector<JobType> *type_vector = new vector<JobType>;
	for (int i = 0; i < job_type_list->jobType->size(); i++) {
		switch ((*(job_type_list->jobType))[i]) {
			case PARAMETRIC:
				type_vector->push_back(WMS_PARAMETRIC);
				break;
			case NORMAL:
				type_vector->push_back(WMS_NORMAL);
				break;
			case INTERACTIVE:
				type_vector->push_back(WMS_INTERACTIVE);
				break;
			case MPI:
				type_vector->push_back(WMS_MPI);
				break;
			case PARTITIONABLE:
				type_vector->push_back(WMS_PARTITIONABLE);
				break;
			case CHECKPOINTABLE:
				type_vector->push_back(WMS_CHECKPOINTABLE);
				break;
			default:
				break;
		}
	}
	JobTypeList *list = new JobTypeList();
	list->jobType = type_vector;
	return list;
}

/**
 * Converts a JobIdStructType vector pointer to ns1__JobIdStructType vector pointer
 */
vector<ns1__JobIdStructType*> *
convertToGSOAPGraphStructTypeVector(vector<JobIdStructType*> *graph_struct_type_vector)
{
	if (!graph_struct_type_vector) {
		return new vector<ns1__JobIdStructType*>;
	}
	vector<ns1__JobIdStructType*> *returnVector = new vector<ns1__JobIdStructType*>;
	ns1__JobIdStructType *element = NULL;
	for (int i = 0; i < graph_struct_type_vector->size(); i++) {
		element = new ns1__JobIdStructType;
		element->id = (*graph_struct_type_vector)[i]->id;
		element->name = (*graph_struct_type_vector)[i]->name;
		//element->childrenJobNum = (*graph_struct_type_vector)[i]->childrenJobNum;
		if (!(*graph_struct_type_vector)[i]) { // Vector not NULL
			element->childrenJob = convertToGSOAPGraphStructTypeVector(
				(*graph_struct_type_vector)[i]->childrenJob);
		} else {
			element->childrenJob = new vector<ns1__JobIdStructType*>;
		}
		returnVector->push_back(element);
	}
	return returnVector;
}

/**
 * Converts a ns1__GraphStructType vector pointer to GraphStructType vector pointer
 */
vector<GraphStructType*> *
convertFromGSOAPGraphStructTypeVector(vector<ns1__GraphStructType*> 
	*graph_struct_type_vector)
{
	vector<GraphStructType*> *returnVector = new vector<GraphStructType*>;
	GraphStructType *element = NULL;
	for (int i = 0; i < graph_struct_type_vector->size(); i++) {
		element = new GraphStructType();
		element->name = (*graph_struct_type_vector)[i]->name;
		if (!(*graph_struct_type_vector)[i]) { // Vector not NULL
			element->childrenJob = convertFromGSOAPGraphStructTypeVector(
				(*graph_struct_type_vector)[i]->childrenJob);
		} else {
			element->childrenJob = new vector<GraphStructType*>;
		}
		returnVector->push_back(element);
	}
	return returnVector;
}

/**
 * Converts a ns1__GraphStructType pointer to GraphStructType pointer
 */
GraphStructType*
convertFromGSOAPGraphStructType(ns1__GraphStructType *graph_struct_type)
{
	GraphStructType *element = new GraphStructType();
	element->name = graph_struct_type->name;
	if (!graph_struct_type) { // Element not NULL
		element->childrenJob = convertFromGSOAPGraphStructTypeVector(
			graph_struct_type->childrenJob);
	} else {
		element->childrenJob = new vector<GraphStructType*>;
	}
	return element;
}

int
getServiceFaultType(int code)
{
	switch (code) {
		case errorcodes::WMS_AUTHENTICATION_ERROR: // AuthenticationFault
			return SOAP_TYPE_ns1__AuthenticationFaultType;
			break;

		case errorcodes::WMS_NOT_AUTHORIZED_USER: // AuthorizationFault
			return SOAP_TYPE_ns1__AuthorizationFaultType;
			break;

		case errorcodes::WMS_JDL_PARSING: // InvalidArgumentFault
			return SOAP_TYPE_ns1__InvalidArgumentFaultType;
			break;

		case errorcodes::WMS_NOT_ENOUGH_QUOTA: // GetQuotaManagementFault
		case errorcodes::WMS_NOT_ENOUGH_SPACE: // ?
			return SOAP_TYPE_ns1__GetQuotaManagementFaultType;
			break;

		case errorcodes::WMS_NO_SUITABLE_RESOURCE: // NoSuitableResourcesFault
		case errorcodes::WMS_MATCHMAKING: // ?
			return SOAP_TYPE_ns1__NoSuitableResourcesFaultType;
			break;

		case errorcodes::WMS_JOB_NOT_FOUND: // JobUnknownFault
			return SOAP_TYPE_ns1__JobUnknownFaultType;
			break;

		case errorcodes::WMS_JOB_NOT_DONE: // OperationNotAllowedFault
		case errorcodes::WMS_OPERATION_NOT_ALLOWED: // Generated inside wmproxy code
			return SOAP_TYPE_ns1__OperationNotAllowedFaultType;
			break;

		case errorcodes::WMS_FATAL:
		case errorcodes::WMS_SANDBOX_IO:
		case errorcodes::WMS_WRONG_COMMAND:
		case errorcodes::WMS_JOB_SIZE:
		case errorcodes::WMS_IS_FAILURE:
		case errorcodes::WMS_MULTI_ATTRIBUTE_FAILURE:
			return SOAP_TYPE_ns1__GenericFaultType;
			break;

		default:
			return SOAP_TYPE_ns1__GenericFaultType;
			break;

		// WMS_CONNECTION_ERROR -> soap server
	}
}

void *
initializeStackPointer(int code)
{
	void *sp = NULL;
	switch(getServiceFaultType(code)) {
		case SOAP_TYPE_ns1__AuthenticationFaultType:
			sp = new ns1__AuthenticationFaultType;
			break;
		case SOAP_TYPE_ns1__AuthorizationFaultType:
			sp = new ns1__AuthorizationFaultType;
			break;
		case SOAP_TYPE_ns1__InvalidArgumentFaultType:
			sp = new ns1__InvalidArgumentFaultType;
			break;
		case SOAP_TYPE_ns1__GetQuotaManagementFaultType:
			sp = new ns1__GetQuotaManagementFaultType;
			break;
		case SOAP_TYPE_ns1__NoSuitableResourcesFaultType:
			sp = new ns1__NoSuitableResourcesFaultType;
			break;
		case SOAP_TYPE_ns1__GenericFaultType:
			sp = new ns1__GenericFaultType;
			break;
		case SOAP_TYPE_ns1__JobUnknownFaultType:
			sp = new ns1__JobUnknownFaultType;
			break;
		case SOAP_TYPE_ns1__OperationNotAllowedFaultType:
			sp = new ns1__OperationNotAllowedFaultType;
			break;
	}
	return sp;
}

void
setFaultDetails(struct soap *soap, int type, void *sp)
{
	if (soap->version == 2) {
		soap->fault->SOAP_ENV__Detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->SOAP_ENV__Detail->__type = type; // stack type
		soap->fault->SOAP_ENV__Detail->value = sp; // point to stack
		soap->fault->SOAP_ENV__Detail->__any = NULL; // no other XML data
	} else {
		soap->fault->detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->detail->__type = type; // stack type
		soap->fault->detail->value = sp; // point to stack
		soap->fault->detail->__any = NULL; // no other XML data
	}
}

void
setSOAPFault(struct soap *soap, int code, string method_name, time_t time_stamp,
	string error_code, string description, vector<string> stack)
{
	// Generating a fault
	ns1__BaseFaultType *sp = (ns1__BaseFaultType*)initializeStackPointer(code);

	// Filling fault fields
	sp->methodName = method_name;
	sp->Timestamp = time_stamp;
	sp->ErrorCode = new string(error_code);
	sp->Description = new string(description);
	sp->FaultCause = convertStackVector(stack);

	// Sending fault
	soap_receiver_fault(soap, "Stack dump", NULL);
	setFaultDetails(soap, getServiceFaultType(code), sp);
}

void
setSOAPFault(struct soap *soap, int code, string method_name, time_t time_stamp,
	string error_code, string description)
{
	setSOAPFault(soap, code, method_name, time_stamp, error_code, description,
		*(new vector<string>));
}

} // namespace


// WM Web Service available operations

int
ns1__getVersion(struct soap *soap, struct ns1__getVersionResponse &response)
{
	GLITE_STACK_TRY("ns1__getVersion(struct soap *soap, struct ns1__getVersionResponse &response)");
	cerr<<"getVersion operation called"<<endl;

	int return_value = SOAP_OK;

	getVersionResponse getVersion_response;

	try {
		getVersion(getVersion_response);
		response.version = getVersion_response.version;
	} catch (Exception &exc) {
		setSOAPFault(soap, exc.getCode(), "getVersion", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
		setSOAPFault(soap, WMS_IS_FAILURE, "getVersion", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	}

	return return_value;
	GLITE_STACK_CATCH();

}

int
ns1__jobRegister(struct soap *soap, string jdl, string delegation_id, struct ns1__jobRegisterResponse &response)
{
	GLITE_STACK_TRY("ns1__jobRegister(struct soap *soap, string jdl, struct ns1__jobRegisterResponse &response)");
	cerr<<"jobRegister operation called"<<endl;

	int return_value = SOAP_OK;

	cerr<<"jdl: "<<jdl<<endl;
	jobRegisterResponse jobRegister_response;

	try {
		jobRegister(jobRegister_response, jdl);
		ns1__JobIdStructType *job_id_struct = new ns1__JobIdStructType();
		job_id_struct->id = jobRegister_response.jobIdStruct->id;
		job_id_struct->name = jobRegister_response.jobIdStruct->name;
		//job_id_struct->childrenJobNum = jobRegister_response.jobIdStruct->childrenJobNum;
		if (!(jobRegister_response.jobIdStruct->childrenJob)) {
			job_id_struct->childrenJob = convertToGSOAPGraphStructTypeVector(jobRegister_response.jobIdStruct->childrenJob);
		} else {
			job_id_struct->childrenJob = new vector<ns1__JobIdStructType*>;
		}
		//job_id_struct->childrenJob = new vector<ns1__JobIdStructType*>;
		response._jobIdStruct = job_id_struct;
	} catch (Exception &exc) {
		// Generating a fault
    		/* FIRST METHOD
		ns1__BaseFaultType *sp = (ns1__BaseFaultType*)initializeStackPointer(exc.getCode());

		// Filling fault fields
		sp->methodName = new string("jobRegister");
		sp->Timestamp = time(NULL);
		sp->ErrorCode = new string(itos(exc.getCode()));
		sp->Description = new string((string) exc.what());
		//sp->FaultCause = convertStackVector(exc.getStackTrace());

		// Sending fault
		soap_receiver_fault(soap, "Stack dump", NULL);
		setFaultDetails(soap, getServiceFaultType(exc.getCode()), sp);
		*/

		/* SECOND METHOD
		SOAPFault *fault = new SOAPFault(soap, exc.getCode());
		fault->setMethodName(new string("jobRegister"));
		fault->setTimestamp(time(NULL));
		fault->setErrorCode(new string(itos(exc.getCode())));
		fault->setDescription(new string((string) exc.what()));
		//soap_receiver_fault(soap, "Stack dump", NULL);
		*/
		cerr<<"exc.what(): "<<exc.what()<<endl;

		// THIRD METHOD
		setSOAPFault(soap, exc.getCode(), "jobRegister", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
		setSOAPFault(soap, WMS_IS_FAILURE, "jobRegister", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	}

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobStart(struct soap *soap, string jobId, struct ns1__jobStartResponse &response)
{
	GLITE_STACK_TRY("ns1__jobStart(struct soap *soap, string jobId, struct ns1__jobStartResponse &response)");
	cerr<<"jobStart operation called"<<endl;

	int return_value = SOAP_OK;

	jobStartResponse jobStart_response;
	try {
		jobStart(jobStart_response, jobId);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "jobStart", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "jobStart", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobSubmit(struct soap *soap, string jdl, struct ns1__jobSubmitResponse &response)
{
	GLITE_STACK_TRY("ns1__jobSubmit(struct soap *soap, string jdl, struct ns1__jobSubmitResponse &response)");
	cerr<<"jobSubmit operation called"<<endl;
	
	int return_value = SOAP_OK;
	
	jobSubmitResponse jobSubmit_response;
	try {
		jobSubmit(jobSubmit_response, jdl);
		response._jobIdStruct->id = jobSubmit_response.jobIdStruct->id;
		response._jobIdStruct->name = jobSubmit_response.jobIdStruct->name;
		//response._jobIdStruct->childrenJobNum = jobSubmit_response.jobIdStruct->childrenJobNum;
		if (!(jobSubmit_response.jobIdStruct->childrenJob)) {
			response._jobIdStruct->childrenJob = convertToGSOAPGraphStructTypeVector(jobSubmit_response.jobIdStruct->childrenJob);
		} else {
			response._jobIdStruct->childrenJob = new vector<ns1__JobIdStructType*>;
		}
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "jobSubmit", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "jobSubmit", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobCancel(struct soap *soap, string jobId, struct ns1__jobCancelResponse &response)
{
	GLITE_STACK_TRY("ns1__jobCancel(struct soap *soap, string jobId, struct ns1__jobCancelResponse &response)");
	cerr<<"jobCancel operation called"<<endl;

	int return_value = SOAP_OK;

	jobCancelResponse jobCancel_response;
	try {
		jobCancel(jobCancel_response, jobId);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "jobCancel", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "jobCancel", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();

}

int
ns1__getMaxInputSandboxSize(struct soap *soap, struct ns1__getMaxInputSandboxSizeResponse &response)
{
	GLITE_STACK_TRY("ns1__getMaxInputSandboxSize(struct soap *soap, struct ns1__getMaxInputSandboxSizeResponse &response)");
	cerr<<"getMaxInputSandboxSize operation called"<<endl;

	int return_value = SOAP_OK;

	getMaxInputSandboxSizeResponse getMaxInputSandboxSize_response;
	try {
		getMaxInputSandboxSize(getMaxInputSandboxSize_response);
		response.size = getMaxInputSandboxSize_response.size;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getMaxInputSandboxSize", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getMaxInputSandboxSize", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getSandboxDestURI(struct soap *soap, string jobId, struct ns1__getSandboxDestURIResponse &response)
{
	GLITE_STACK_TRY("ns1__getSandboxDestURI(struct soap *soap, string jobId, struct ns1__getSandboxDestURIResponse &response)");
	cerr<<"getSandboxDestURI operation called"<<endl;
	
	int return_value = SOAP_OK;
	
	getSandboxDestURIResponse getSandboxDestURI_response;
	try {
		getSandboxDestURI(getSandboxDestURI_response, jobId);
		response._path = getSandboxDestURI_response.path;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getSandboxDestURI", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getSandboxDestURI", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getTotalQuota(struct soap *soap, struct ns1__getTotalQuotaResponse &response)
{
	GLITE_STACK_TRY("ns1__getQuota(struct soap *soap, struct ns1__getQuotaResponse &response)");
	cerr<<"getQuota operation called"<<endl;

	int return_value = SOAP_OK;

	getQuotaResponse getQuota_response;
	try {
		getQuota(getQuota_response);
		response.softLimit = getQuota_response.softLimit;
		response.hardLimit = getQuota_response.hardLimit;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getQuota", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getQuota", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getFreeQuota(struct soap *soap, struct ns1__getFreeQuotaResponse &response)
{
	GLITE_STACK_TRY("ns1__getFreeQuota(struct soap *soap, struct ns1__getFreeQuotaResponse &response)");
	cerr<<"getFreeQuota operation called"<<endl;
	
	int return_value = SOAP_OK;
	
	getFreeQuotaResponse getFreeQuota_response;
	try {
		getFreeQuota(getFreeQuota_response);
		response.softLimit = getFreeQuota_response.softLimit;
		response.hardLimit = getFreeQuota_response.hardLimit;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getFreeQuota", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getFreeQuota", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}

int 
ns1__jobPurge(struct soap *soap, string jobId, struct ns1__jobPurgeResponse &response)
{
	GLITE_STACK_TRY("ns1__jobPurge(struct soap *soap, string *jobId, struct ns1__jobPurgeResponse &response)");
	cerr<<"jobPurge operation called"<<endl;
	
	int return_value = SOAP_OK;
	
	jobPurgeResponse jobPurge_response;
	try {
		jobPurge(jobPurge_response, jobId);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "jobPurge", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "jobPurge", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH()
}

int 
ns1__getOutputFileList(struct soap *soap, string jobId, struct ns1__getOutputFileListResponse &response)
{
	GLITE_STACK_TRY("ns1__getOutputFileList(struct soap *soap, string *jobId, struct ns1__getOutputFileListResponse &response)");
	cerr<<"getOutputFileList operation called"<<endl;

	int return_value = SOAP_OK;

	getOutputFileListResponse getOutputFileList_response;

	try {
		getOutputFileList(getOutputFileList_response, jobId);
		for (int i = 0; i < (*(getOutputFileList_response.OutputFileAndSizeList->file)).size(); i++) {
			(*(response._OutputFileAndSizeList->file))[i]->name = 
				(*(getOutputFileList_response.OutputFileAndSizeList->file))[i]->name;
			(*(response._OutputFileAndSizeList->file))[i]->size = 
				(*(getOutputFileList_response.OutputFileAndSizeList->file))[i]->size;
		}
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getOutputFileList", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getOutputFileList", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobListMatch(struct soap *soap, string jdl, struct ns1__jobListMatchResponse &response)
{
	GLITE_STACK_TRY("ns1__jobListMatch(struct soap *soap, string *jdl, struct ns1__jobListMatchResponse &response)");
	cerr<<"jobListMatch operation called"<<endl;

	int return_value = SOAP_OK;

	jobListMatchResponse jobListMatch_response;
	try {
		jobListMatch(jobListMatch_response, jdl);
		//response._CEIdAndRankList->name = jobListMatch_response.CEIdAndRankList->name;
		//response._CEIdAndRankList->size = jobListMatch_response.CEIdAndRankList->size;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "jobListMatch", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "jobListMatch", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getJobTemplate(struct soap *soap, ns1__JobTypeList *job_type_list, string executable, string arguments, string requirements, string rank,
		struct ns1__getJobTemplateResponse &response)
{
	GLITE_STACK_TRY("ns1__getJobTemplate(struct soap *soap, ns1__JobTypeList *jobType, string *executable, string *argumements, string *requirements, string *rank, struct ns1__getJobTemplateResponse &response)");
	cerr<<"getJobTemplate operation called"<<endl;

	int return_value = SOAP_OK;

	getJobTemplateResponse getJobTemplate_response;
	try {
		getJobTemplate(getJobTemplate_response, *convertFromGSOAPJobTypeList(job_type_list), executable, arguments, requirements, rank);
		response.jdl = getJobTemplate_response.jdl;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getJobTemplate", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getJobTemplate", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getDAGTemplate(struct soap *soap, ns1__GraphStructType *dependencies, string requirements, string rank,
		struct ns1__getDAGTemplateResponse &response)
{
	GLITE_STACK_TRY("ns1__getDAGTemplate(struct soap *soap, ns1__JobIdStructType *dependencies, string *requirements, string *rank, struct ns1__getDAGTemplateResponse &response)");
	cerr<<"getDAGTemplate operation called"<<endl;

	int return_value = SOAP_OK;
	
	getDAGTemplateResponse getDAGTemplate_response;
	try {
		getDAGTemplate(getDAGTemplate_response, *convertFromGSOAPGraphStructType(dependencies), requirements, rank);
		response.jdl = getDAGTemplate_response.jdl;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getDAGTemplate", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getDAGTemplate", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH()
}

int
ns1__getCollectionTemplate(struct soap *soap, int jobNumber, string requirements, string rank,
		struct ns1__getCollectionTemplateResponse &response)
{
	GLITE_STACK_TRY("ns1__getCollectionTemplate(struct soap *soap, int jobNumber, string *requirements, string *rank, struct ns1__getCollectionTemplateResponse &response)");
	cerr<<"getCollectionTemplate operation called"<<endl;

	int return_value = SOAP_OK;

	getCollectionTemplateResponse getCollectionTemplate_response;
	try  {
		getCollectionTemplate(getCollectionTemplate_response, jobNumber, requirements, rank);
		response.jdl = getCollectionTemplate_response.jdl;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getCollectionTemplate", time(NULL), itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getCollectionTemplate", time(NULL), itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getIntParametricJobTemplate(struct soap *soap, ns1__StringList *attributes,
	int param, int parameterStart, int parameterStep, string requirements,
	string rank, struct ns1__getIntParametricJobTemplateResponse &response)
{
	GLITE_STACK_TRY("ns1__getIntParametricJobTemplate(struct soap *soap, "
	"ns1__StringList *attributes, int param, int parameterStart, int parameterStep, "
	"string requirements, string rank, "
	"struct ns1__getIntParametricJobTemplateResponse &response)");
	cerr<<"getIntParametricJobTemplate operation called"<<endl;

	int return_value = SOAP_OK;

	getIntParametricJobTemplateResponse getIntParametricJobTemplate_response;
	try  {
		//getIntParametricJobTemplate(getIntParametricJobTemplate_response,
		//	attributes, param, parameterStart, parameterStep, requirements, rank);
		response.jdl = getIntParametricJobTemplate_response.jdl;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getCollectionTemplate", time(NULL),
	 		itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getCollectionTemplate", time(NULL),
	 		itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getStringParametricJobTemplate(struct soap *soap, ns1__StringList *attributes,
	ns1__StringList *param, string requirements, string rank,
	struct ns1__getStringParametricJobTemplateResponse &response)
{
	GLITE_STACK_TRY("ns1__getStringParametricJobTemplate(struct soap *soap, "
	"ns1__StringList *attributes, ns1__StringList *param, string requirements, "
	"string rank, struct ns1__getStringParametricJobTemplateResponse &response)");
	cerr<<"getStringParametricJobTemplate operation called"<<endl;

	int return_value = SOAP_OK;

	getStringParametricJobTemplateResponse getStringParametricJobTemplate_response;
	try  {
		//getStringParametricJobTemplate(getStringParametricJobTemplate_response,
		//	attributes, param, requirements, rank);
		response.jdl = getStringParametricJobTemplate_response.jdl;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getCollectionTemplate", time(NULL),
	 		itos(exc.getCode()), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getCollectionTemplate", time(NULL),
	 		itos(WMS_IS_FAILURE), (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	return return_value;
	GLITE_STACK_CATCH();
}





