/*
Copyright (c) Members of the EGEE Collaboration. 2004. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License");
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
// File: wmpgsoapoperations.cpp
// Author: Giuseppe Avellino <egee@datamat.it>
//

#include <sstream>

// Boost
#include <boost/lexical_cast.hpp>

// gSOAP
#include "soapH.h"
 #include <signal.h>  // sig_atomic

// Server
#include "wmpoperations.h"
#include "wmpcoreoperations.h"
#include "wmpresponsestruct.h"
#include "wmpstructconverter.h"
#include "wmpgsoapfaultmanipulator.h"

#include "wmpsignalhandler.h"

// Logger
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "utilities/logging.h"

// Exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"
#include "glite/jdl/RequestAdExceptions.h"

#include "glite/lb/JobStatus.h"

namespace jobid        = glite::wmsutils::jobid;
namespace logger       = glite::wms::common::logger;
namespace wmputilities = glite::wms::wmproxy::utilities; //Exception codes

using namespace std;
using namespace wmputilities;
using namespace glite::jdl; // AdSyntaxException
using namespace glite::wmsutils::exception; // Exception

const std::string ALL_PROTOCOLS = "all";
const std::string DEFAULT_PROTOCOL = "default";

// WM Web Service available operations
// To get more infomation see WM service wsdl file

void printJSDL(jsdlns__JobDefinition_USCOREType *jsdl)
{
	jsdlns__JobDescription_USCOREType *JobDescription = jsdl->jsdlns__JobDescription;
	jsdlns__JobIdentification_USCOREType *JobIdentification = JobDescription->jsdlns__JobIdentification;;
	jsdlns__Application_USCOREType *Application = JobDescription->jsdlns__Application;

	// Print out JOB IDENTIFICATION
	edglog(info) << "JobIdentification JobName " << JobIdentification->jsdlns__JobName->c_str() << endl;
	
	// Print out APPLICATION
	edglog(info) << "Application ApplicationName " << Application->jsdlns__ApplicationName->c_str() << endl;
	edglog(info) << "Application ApplicationVersion " << Application->jsdlns__ApplicationVersion->c_str() << endl;
	edglog(info) << "Application ApplicationDescription " << Application->jsdlns__Description->c_str() << endl;
}

void initializingSignalHandler(){
	// Initializing signal handler for 'graceful' stop/restart
	extern volatile sig_atomic_t handled_signal_recv;
	handled_signal_recv = 0;
	glite::wms::wmproxy::server::initsignalhandler();
}

int
ns1__getVersion(struct soap *soap, struct ns1__getVersionResponse &response)
{
	GLITE_STACK_TRY("ns1__getVersion(struct soap *soap, struct "
		"ns1__getVersionResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getVersion");
	edglog(debug)<<"getVersion operation called"<<endl;
        initializingSignalHandler();
	int return_value = SOAP_OK;
	getVersionResponse getVersion_response;

	try {
		getVersion(getVersion_response);
		response.version = getVersion_response.version;
	} catch (Exception &exc) {
		setSOAPFault(soap, exc.getCode(), "getVersion", time(NULL),
			exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
		setSOAPFault(soap, WMS_IS_FAILURE, "getVersion", time(NULL),
			WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"getVersion operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getJDL(struct soap *soap, string job_id, ns1__JdlType type,
	struct ns1__getJDLResponse &response)
{
	GLITE_STACK_TRY("ns1__getJDL(struct soap *soap, struct "
		"ns1__getJDLResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getJDL");
	edglog(debug)<<"getJDL operation called"<<endl;

        initializingSignalHandler();
	int return_value = SOAP_OK;
	getJDLResponse getJDL_response;

	// `JdlType jdltype' might be used uninitialized - warning message removed
	JdlType jdltype = WMS_JDL_ORIGINAL;
	switch (type) {
		case ns1__JdlType__ORIGINAL:
			jdltype = WMS_JDL_ORIGINAL;
			break;
		case ns1__JdlType__REGISTERED:
			jdltype = WMS_JDL_REGISTERED;
			break;
		default:
			// UNknown jdlType (unreachable point!)
			setSOAPFault(soap, SOAP_FAULT, "getJDL", time(NULL), SOAP_FAULT, "Unknown jdl type");
			return_value = SOAP_FAULT;


			break;
	}

	try {
		getJDL(job_id, jdltype, getJDL_response);
		response._jdl = getJDL_response.jdl;
	} catch (Exception &exc) {
		setSOAPFault(soap, exc.getCode(), "getJDL", time(NULL),
			exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
		setSOAPFault(soap, WMS_IS_FAILURE, "getJDL", time(NULL),
			WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"getJDL operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobRegister(struct soap *soap, string jdl, string delegation_id,
	struct ns1__jobRegisterResponse &response)
{
	GLITE_STACK_TRY("ns1__jobRegister(struct soap *soap, string jdl, "
		"string delegation_id, struct ns1__jobRegisterResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__jobRegister");
	edglog(debug)<<"jobRegister operation called"<<endl;

        initializingSignalHandler();
	int return_value = SOAP_OK;

	jobRegisterResponse jobRegister_response;

	try {
		jobRegister(jobRegister_response, jdl, delegation_id);
		ns1__JobIdStructType *job_id_struct = new ns1__JobIdStructType();
		job_id_struct->id = jobRegister_response.jobIdStruct->id;
		job_id_struct->name = jobRegister_response.jobIdStruct->name;
		if (job_id_struct->path) {
			job_id_struct->path = jobRegister_response.jobIdStruct->path;
		} else {
			job_id_struct->path = NULL;
		}

		if (jobRegister_response.jobIdStruct->childrenJob) {
			job_id_struct->childrenJob =
				*convertToGSOAPJobIdStructTypeVector(jobRegister_response
				.jobIdStruct->childrenJob);
		} else {
			job_id_struct->childrenJob = *(new vector<ns1__JobIdStructType*>);
		}
		response._jobIdStruct = job_id_struct;
	} catch (Exception &exc) {
		setSOAPFault(soap, exc.getCode(), "jobRegister", time(NULL),
			exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
		setSOAPFault(soap, WMS_IS_FAILURE, "jobRegister", time(NULL),
			WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(debug)<<"jobRegister operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobRegisterJSDL(struct soap *soap, jsdlns__JobDefinition_USCOREType *jsdl,
	string delegation_id, struct ns1__jobRegisterJSDLResponse &response)
{
	GLITE_STACK_TRY("ns1__jobRegisterJSDL(struct soap *soap, jsdlns__JobDefinition_USCOREType *jsdl, "
		"string delegation_id, struct ns1__jobRegisterResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__jobRegisterJSDL");
	edglog(info)<<"jobRegisterJSDL operation called"<<endl;
	printJSDL(jsdl);

	int return_value = SOAP_OK;

	jobRegisterResponse jobRegister_response;
	
	string jdl = "[ \
        requirements = other.GlueCEStateStatus == \"Production\"; \
        rank =  -other.GlueCEStateEstimatedResponseTime; \
        DefaultRank =  -other.GlueCEStateEstimatedResponseTime; \
        AllowZippedISB = false; \
        Type = \"job\"; \
        JobType = \"normal\"; \
        Executable = \"/usr/bin/ls\"; \
        Arguments = \"File1.txt\"; \
        StdOutput = \"std.out\"; \
        StdError = \"std.err\"; \
        InputSandbox = {\"file:///home/civvu/JDL/File1.txt\"}; \
        OutputSandbox = {\"std.out\",\"std.err\"}; \
        DEFAULTrank =  -other.GlueCEStateEstimatedResponseTimeDAG; \
        VirtualOrganisation = \"infngrid\";]";

	try {
		jobRegister(jobRegister_response, jdl, delegation_id);
		ns1__JobIdStructType *job_id_struct = new ns1__JobIdStructType();
		job_id_struct->id = jobRegister_response.jobIdStruct->id;
		job_id_struct->name = jobRegister_response.jobIdStruct->name;
		if (job_id_struct->path) {
			job_id_struct->path = jobRegister_response.jobIdStruct->path;
		} else {
			job_id_struct->path = NULL;
		}

		if (jobRegister_response.jobIdStruct->childrenJob) {
			job_id_struct->childrenJob =
				*convertToGSOAPJobIdStructTypeVector(jobRegister_response
				.jobIdStruct->childrenJob);
		} else {
			job_id_struct->childrenJob = *(new vector<ns1__JobIdStructType*>);
		}
		response._jobIdStruct = job_id_struct;
	} catch (Exception &exc) {
		setSOAPFault(soap, exc.getCode(), "jobRegisterJSDL", time(NULL),
			exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
		setSOAPFault(soap, WMS_IS_FAILURE, "jobRegisterJSDL", time(NULL),
			WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(info)<<"jobRegisterJSDL operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobStart(struct soap *soap, string job_id,
	struct ns1__jobStartResponse &response)
{
	GLITE_STACK_TRY("ns1__jobStart(struct soap *soap, string job_id, struct "
		"ns1__jobStartResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__jobStart");
	edglog(debug)<<"jobStart operation called"<<endl;
	initializingSignalHandler();
	int return_value = SOAP_OK;

	jobStartResponse jobStart_response;
	try {
		jobStart(jobStart_response, job_id, soap);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "jobStart", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "jobStart", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	edglog(debug)<<"jobStart operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobSubmit(struct soap *soap, string jdl, string delegation_id,
	struct ns1__jobSubmitResponse &response)
{
	GLITE_STACK_TRY("ns1__jobSubmit(struct soap *soap, string jdl, string "
		"delegation_id, struct ns1__jobSubmitResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__jobSubmit");
	edglog(debug)<<"jobSubmit operation called"<<endl;
	initializingSignalHandler();
	int return_value = SOAP_OK;
	
	jobSubmitResponse jobSubmit_response;
	try {
		jobSubmit(response, jobSubmit_response, jdl, delegation_id, soap);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "jobSubmit", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
 	} catch (exception &ex) {
 		setSOAPFault(soap, WMS_IS_FAILURE, "jobSubmit", time(NULL),
 			WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	 
	edglog(debug)<<"jobSubmit operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobSubmitJSDL(struct soap *soap, jsdlns__JobDefinition_USCOREType *jsdl,
	string delegation_id, struct ns1__jobSubmitJSDLResponse &response)
{
	GLITE_STACK_TRY("ns1__jobSubmitJSDL(struct soap *soap, jsdlns__JobDefinition_USCOREType *jsdl, "
		"string delegation_id, struct ns1__jobSubmitJSDLResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__jobSubmitJSDL");
	edglog(info)<<"jobSubmitJSDL operation called"<<endl;
	printJSDL(jsdl);

	int return_value = SOAP_OK;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobCancel(struct soap *soap, string job_id,
	struct ns1__jobCancelResponse &response)
{
	GLITE_STACK_TRY("ns1__jobCancel(struct soap *soap, string job_id, struct "
		"ns1__jobCancelResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__jobCancel");
	edglog(debug)<<"jobCancel operation called"<<endl;
	initializingSignalHandler();
	int return_value = SOAP_OK;

	jobCancelResponse jobCancel_response;
	try {
		jobCancel(jobCancel_response, job_id);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "jobCancel", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
		setSOAPFault(soap, WMS_IS_FAILURE, "jobCancel", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(debug)<<"jobCancel operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();

}

int
ns1__getMaxInputSandboxSize(struct soap *soap,
	struct ns1__getMaxInputSandboxSizeResponse &response)
{
	GLITE_STACK_TRY("ns1__getMaxInputSandboxSize(struct soap *soap, struct "
		"ns1__getMaxInputSandboxSizeResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getMaxInputSandboxSize");
	edglog(debug)<<"getMaxInputSandboxSize operation called"<<endl;
	initializingSignalHandler();
	int return_value = SOAP_OK;

	getMaxInputSandboxSizeResponse getMaxInputSandboxSize_response;
	try {
		getMaxInputSandboxSize(getMaxInputSandboxSize_response);
		response.size = getMaxInputSandboxSize_response.size;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getMaxInputSandboxSize",
	 		time(NULL), exc.getCode(), (string) exc.what(),
	 		exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getMaxInputSandboxSize",
	 		time(NULL), WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	edglog(debug)<<"getMaxInputSandboxSize operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getSandboxDestURI(struct soap *soap, string job_id, string protocol,
	struct ns1__getSandboxDestURIResponse &response 
	= *(new ns1__getSandboxDestURIResponse()))
{
	GLITE_STACK_TRY("ns1__getSandboxDestURI(struct soap *soap, string job_id, "
		"struct ns1__getSandboxDestURIResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getSandboxDestURI");
	edglog(debug)<<"getSandboxDestURI operation called"<<endl;
	
	// Setting default value for protocol. gSOAP set it as "" if not provided
	if (protocol == "") {
		protocol = ALL_PROTOCOLS;	
	}
	initializingSignalHandler();
	int return_value = SOAP_OK;
	
	getSandboxDestURIResponse getSandboxDestURI_response;
	
	response._path = new ns1__StringList();
	response._path->Item = *(new vector<string>(0));
	try {
		getSandboxDestURI(getSandboxDestURI_response, job_id, protocol);
		for (unsigned int i = 0; 
				i < getSandboxDestURI_response.path->Item->size(); i++) {
			response._path->Item.push_back(
				(*(getSandboxDestURI_response.path->Item))[i]);
		}
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getSandboxDestURI", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getSandboxDestURI", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(debug)<<"getSandboxDestURI operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getSandboxBulkDestURI(struct soap *soap, string job_id, string protocol,
	struct ns1__getSandboxBulkDestURIResponse &response
	= *(new ns1__getSandboxBulkDestURIResponse()))
{
	GLITE_STACK_TRY("ns1__getSandboxBulkDestURI(struct soap *soap, "
		"string job_ids, ns1__getSandboxBulkDestURIResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getSandboxBulkDestURI");
	edglog(debug)<<"getSandboxBulkDestURI operation called"<<endl;



	// Setting default value for protocol. gSOAP set it as "" if not provided
	if (protocol == "") {
		protocol = ALL_PROTOCOLS;	
	}
	initializingSignalHandler();
	int return_value = SOAP_OK;

	getSandboxBulkDestURIResponse getSandboxBulkDestURI_response;
	
	try {
		getSandboxBulkDestURI(getSandboxBulkDestURI_response, job_id, protocol);
			
		vector<ns1__DestURIStructType*> *uris = 
			new vector<ns1__DestURIStructType*>(0);
		for (unsigned int i = 0; i < 
				getSandboxBulkDestURI_response.destURIsStruct->Item->size(); 
				i++) {
			ns1__DestURIStructType *destURIStruct = new ns1__DestURIStructType();
			destURIStruct->id = 
				(*getSandboxBulkDestURI_response.destURIsStruct->Item)[i]->id;
			destURIStruct->Item = 
				*((*getSandboxBulkDestURI_response.destURIsStruct->Item)[i]->destURIs);
			uris->push_back(destURIStruct);
			
		}
		response._DestURIsStructType = new ns1__DestURIsStructType();
		response._DestURIsStructType->Item = *uris;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getSandboxBulkDestURI", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getSandboxBulkDestURI", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(debug)<<"getSandboxBulkDestURI operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getTotalQuota(struct soap *soap,
	struct ns1__getTotalQuotaResponse &response)
{
	GLITE_STACK_TRY("ns1__getTotalQuota(struct soap *soap, struct "
		"ns1__getQuotaResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getTotalQuota");
	edglog(debug)<<"getTotalQuota operation called"<<endl;
	initializingSignalHandler();
	int return_value = SOAP_OK;

	getQuotaResponse getQuota_response;
	try {
		getQuota(getQuota_response);
		response.softLimit = getQuota_response.softLimit;
		response.hardLimit = getQuota_response.hardLimit;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getTotalQuota", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getTotalQuota", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(debug)<<"getTotalQuota operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getFreeQuota(struct soap *soap, struct ns1__getFreeQuotaResponse &response)
{
	GLITE_STACK_TRY("ns1__getFreeQuota(struct soap *soap, struct "
		"ns1__getFreeQuotaResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getFreeQuota");
	edglog(debug)<<"getFreeQuota operation called"<<endl;
	initializingSignalHandler();
	int return_value = SOAP_OK;
	
	getFreeQuotaResponse getFreeQuota_response;
	try {
		getFreeQuota(getFreeQuota_response);
		response.softLimit = getFreeQuota_response.softLimit;
		response.hardLimit = getFreeQuota_response.hardLimit;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getFreeQuota", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getFreeQuota", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(debug)<<"getFreeQuota operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int 
ns1__jobPurge(struct soap *soap, string job_id,
	struct ns1__jobPurgeResponse &response)
{
	GLITE_STACK_TRY("ns1__jobPurge(struct soap *soap, string *job_id, struct "
		"ns1__jobPurgeResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__jobPurge");
	edglog(debug)<<"jobPurge operation called"<<endl;
	initializingSignalHandler();
	int return_value = SOAP_OK;
	
	jobPurgeResponse jobPurge_response;
	try {
		jobPurge(jobPurge_response, job_id);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "jobPurge", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "jobPurge", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(debug)<<"jobPurge operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH()
}

int 
ns1__getOutputFileList(struct soap *soap, string job_id, string protocol,
	struct ns1__getOutputFileListResponse &response 
	= *(new ns1__getOutputFileListResponse()))
{
	GLITE_STACK_TRY("ns1__getOutputFileList(struct soap *soap, string *job_id, "
		"struct ns1__getOutputFileListResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getOutputFileList");
	edglog(debug)<<"getOutputFileList operation called"<<endl;
	


	// Setting default value for protocol. gSOAP set it as "" if not provided
	if (protocol == "") {
		protocol = DEFAULT_PROTOCOL;
	}
	initializingSignalHandler();
	int return_value = SOAP_OK;

	getOutputFileListResponse getOutputFileList_response;
	
	response._OutputFileAndSizeList = new ns1__StringAndLongList();
	response._OutputFileAndSizeList->file = *(new vector<ns1__StringAndLongType*>(0));
	ns1__StringAndLongType *item = NULL;

	try {
		getOutputFileList(getOutputFileList_response, job_id, protocol);
		
		if (getOutputFileList_response.OutputFileAndSizeList->file) {
			for (unsigned int i = 0; i < getOutputFileList_response
					.OutputFileAndSizeList->file->size(); i++) {
				item = new ns1__StringAndLongType();
				item->name = 
					(*getOutputFileList_response.OutputFileAndSizeList->file)[i]->name;
				item->size = 
					(*getOutputFileList_response.OutputFileAndSizeList->file)[i]->size;
				response._OutputFileAndSizeList->file.push_back(item);
			}
		}
		
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getOutputFileList", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getOutputFileList", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(debug)<<"getOutputFileList operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobListMatch(struct soap *soap, string jdl, string delegation_id,
	struct ns1__jobListMatchResponse &response)
{
	GLITE_STACK_TRY("ns1__jobListMatch(struct soap *soap, string *jdl, "
		"string delegation_id, struct ns1__jobListMatchResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__jobListMatch");
	edglog(debug)<<"jobListMatch operation called"<<endl;

	initializingSignalHandler();
	int return_value = SOAP_OK;

	jobListMatchResponse jobListMatch_response;
	
	response._CEIdAndRankList = new ns1__StringAndLongList();
	response._CEIdAndRankList->file = *(new vector<ns1__StringAndLongType*>);
	ns1__StringAndLongType *item = NULL;
	
	try {
		jobListMatch(jobListMatch_response, jdl, delegation_id);
		for (unsigned int i = 0;
				i < jobListMatch_response.CEIdAndRankList->file->size(); i++) {
			item = new ns1__StringAndLongType();
			item->name = (*jobListMatch_response.CEIdAndRankList->file)[i]->name;
			item->size = (*jobListMatch_response.CEIdAndRankList->file)[i]->size;
			response._CEIdAndRankList->file.push_back(item);
		}
		
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "jobListMatch", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "jobListMatch", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(debug)<<"jobListMatch operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__jobListMatchJSDL(struct soap *soap, jsdlns__JobDefinition_USCOREType *jsdl,
	string delegation_id, struct ns1__jobListMatchJSDLResponse &response)
{
	GLITE_STACK_TRY("ns1__jobListMatchJSDL(struct soap *soap, jsdlns__JobDefinition_USCOREType *jsdl, "
		"string delegation_id, struct ns1__jobListMatchJSDLResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__jobListMatchJSDL");
	edglog(info)<<"ns1__jobListMatchJSDL operation called"<<endl;
	printJSDL(jsdl);

	int return_value = SOAP_OK;

	edglog(info)<<"jobListMatchJSDL operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getJobTemplate(struct soap *soap, ns1__JobTypeList *job_type_list,
	string executable, string arguments, string requirements, string rank,
	struct ns1__getJobTemplateResponse &response)
{
	GLITE_STACK_TRY("ns1__getJobTemplate(struct soap *soap, ns1__JobTypeList "
		"*job_type_list, string executable, string arguments, string "
		"requirements, string rank, struct ns1__getJobTemplateResponse "
		"&response)");
	edglog_fn("wmpgsoapoperations::ns1__getJobTemplate");
	edglog(debug)<<"getJobTemplate operation called"<<endl;
	
	initializingSignalHandler();
	int return_value = SOAP_OK;
	
	if (!job_type_list) {
		setSOAPFault(soap, WMS_INVALID_ARGUMENT, "getJobTemplate", time(NULL),
	 		WMS_INVALID_ARGUMENT, "Invalid Argument");
		return_value = SOAP_FAULT;
	} else {
		getJobTemplateResponse getJobTemplate_response;
		try {
			getJobTemplate(getJobTemplate_response, 
				*convertFromGSOAPJobTypeList(job_type_list), executable,
				arguments, requirements, rank);
			response._jdl = getJobTemplate_response.jdl;
		} catch (Exception &exc) {
		 	setSOAPFault(soap, exc.getCode(), "getJobTemplate", time(NULL),
		 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
			return_value = SOAP_FAULT;
		} catch (exception &ex) {
		 	setSOAPFault(soap, WMS_IS_FAILURE, "getJobTemplate", time(NULL),
		 		WMS_IS_FAILURE, (string) ex.what());
			return_value = SOAP_FAULT;
		}
	}
	
	edglog(debug)<<"getJobTemplate operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getDAGTemplate(struct soap *soap, ns1__GraphStructType *dependencies, 
	string requirements, string rank,
	struct ns1__getDAGTemplateResponse &response)
{
	GLITE_STACK_TRY("ns1__getDAGTemplate(struct soap *soap, "
		"ns1__GraphStructType *dependencies, string requirements, string rank, "
		"struct ns1__getDAGTemplateResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getDAGTemplate");
	edglog(debug)<<"getDAGTemplate operation called"<<endl;

	initializingSignalHandler();
	int return_value = SOAP_OK;
	
	if (!dependencies) {
		setSOAPFault(soap, WMS_INVALID_ARGUMENT, "getDAGTemplate", time(NULL),
	 		WMS_INVALID_ARGUMENT, "Invalid Argument");
		return_value = SOAP_FAULT;
	} else {
		getDAGTemplateResponse getDAGTemplate_response;
		try {
			getDAGTemplate(getDAGTemplate_response,
				*convertFromGSOAPGraphStructType(dependencies), requirements,
				rank);
			response._jdl = getDAGTemplate_response.jdl;
		} catch (Exception &exc) {
		 	setSOAPFault(soap, exc.getCode(), "getDAGTemplate", time(NULL),
		 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
			return_value = SOAP_FAULT;
		} catch (exception &ex) {
		 	setSOAPFault(soap, WMS_IS_FAILURE, "getDAGTemplate", time(NULL),
		 		WMS_IS_FAILURE, (string) ex.what());
			return_value = SOAP_FAULT;
		}
	}
	
	edglog(debug)<<"getDAGTemplate operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH()
}

int
ns1__getCollectionTemplate(struct soap *soap, int job_number,
	string requirements, string rank,
	struct ns1__getCollectionTemplateResponse &response)
{
	GLITE_STACK_TRY("ns1__getCollectionTemplate(struct soap *soap, int "
		"job_number, string requirements, string rank, "
		"struct ns1__getCollectionTemplateResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getCollectionTemplate");
	edglog(debug)<<"getCollectionTemplate operation called"<<endl;

	initializingSignalHandler();
	int return_value = SOAP_OK;

	getCollectionTemplateResponse getCollectionTemplate_response;
	try  {
		getCollectionTemplate(getCollectionTemplate_response, job_number,
			requirements, rank);
		response._jdl = getCollectionTemplate_response.jdl;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getCollectionTemplate", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	 } catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getCollectionTemplate", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	 }

	edglog(debug)<<"getCollectionTemplate operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getIntParametricJobTemplate(struct soap *soap, ns1__StringList *attributes,
	int param, int parameter_start, int parameter_step, string requirements,
	string rank, struct ns1__getIntParametricJobTemplateResponse &response)
{
	GLITE_STACK_TRY("ns1__getIntParametricJobTemplate(struct soap *soap, "
		"ns1__StringList *attributes, int param, int parameter_start, "
		"int parameter_step, string requirements, string rank, "
		"struct ns1__getIntParametricJobTemplateResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getIntParametricJobTemplate");
	edglog(debug)<<"getIntParametricJobTemplate operation called"<<endl;

	initializingSignalHandler();
	int return_value = SOAP_OK;

	if (!attributes) {
		setSOAPFault(soap, WMS_INVALID_ARGUMENT, "getIntParametricJobTemplate",
			time(NULL), WMS_INVALID_ARGUMENT, "Invalid Argument");
		return_value = SOAP_FAULT;
	} else {
		getIntParametricJobTemplateResponse 
			getIntParametricJobTemplate_response;
		try  {
			getIntParametricJobTemplate(getIntParametricJobTemplate_response,
				convertToStringList(attributes), param, parameter_start, parameter_step,
				requirements, rank);
			response._jdl = getIntParametricJobTemplate_response.jdl;
		} catch (Exception &exc) {
		 	setSOAPFault(soap, exc.getCode(), "getIntParametricJobTemplate",
		 		time(NULL), exc.getCode(), (string) exc.what(),
		 		exc.getStackTrace());
			return_value = SOAP_FAULT;
		} catch (exception &ex) {
		 	setSOAPFault(soap, WMS_IS_FAILURE, "getIntParametricJobTemplate",
		 		time(NULL), WMS_IS_FAILURE, (string) ex.what());
			return_value = SOAP_FAULT;
		}
	}

	edglog(debug)<<"getIntParametricJobTemplate operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getStringParametricJobTemplate(struct soap *soap,
	ns1__StringList *attributes, ns1__StringList *param, string requirements, 
	string rank, struct ns1__getStringParametricJobTemplateResponse &response)
{
	GLITE_STACK_TRY("ns1__getStringParametricJobTemplate(struct soap *soap, "
		"ns1__StringList *attributes, ns1__StringList *param, string "
		"requirements, string rank, struct "
		"ns1__getStringParametricJobTemplateResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getStringParametricJobTemplate");
	edglog(debug)<<"getStringParametricJobTemplate operation called"<<endl;

	initializingSignalHandler();
	int return_value = SOAP_OK;

	if (!attributes || !param) {
		setSOAPFault(soap, WMS_INVALID_ARGUMENT, "getDAGTemplate", time(NULL),
	 		WMS_INVALID_ARGUMENT, "Invalid Argument");
		return_value = SOAP_FAULT;
	} else {
		getStringParametricJobTemplateResponse 
			getStringParametricJobTemplate_response;
		try  {
			getStringParametricJobTemplate(getStringParametricJobTemplate_response,
				convertToStringList(attributes), convertToStringList(param),
				requirements, rank);
			response._jdl = getStringParametricJobTemplate_response.jdl;
		} catch (Exception &exc) {
		 	setSOAPFault(soap, exc.getCode(), "getCollectionTemplate", time(NULL),
		 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
			return_value = SOAP_FAULT;
		} catch (exception &ex) {
		 	setSOAPFault(soap, WMS_IS_FAILURE, "getCollectionTemplate", time(NULL),
		 		WMS_IS_FAILURE, (string) ex.what());
			return_value = SOAP_FAULT;
		}
	}

	edglog(debug)<<"getStringParametricJobTemplate operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getProxyReq(struct soap *soap, string delegation_id,
	struct ns1__getProxyReqResponse &response)
{
	GLITE_STACK_TRY("ns1__getProxyReq(struct soap *soap, string delegation_id, "
		"ns1__getProxyReqResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getProxyReq");
	edglog(debug)<<"getProxyReq operation called"<<endl;
	
	initializingSignalHandler();
	int return_value = SOAP_OK;

	getProxyReqResponse getProxyReq_response;
	try  {
		getProxyReq(getProxyReq_response, delegation_id);
		response._request = getProxyReq_response.request;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getProxyReq", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getProxyReq", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	 
	edglog(debug)<<"getProxyReq operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__putProxy(struct soap *soap, string delegation_id, string proxy,
	struct ns1__putProxyResponse &response)
{
	GLITE_STACK_TRY("ns1__putProxy(struct soap *soap, string delegation_id, "
		"string proxy, struct ns1__putProxyResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__putProxy");
	edglog(debug)<<"putProxy operation called"<<endl;
	initializingSignalHandler();
	int return_value = SOAP_OK;
	putProxyResponse putProxy_response;
	try  {
		putProxy(putProxy_response, delegation_id, proxy);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "putProxy", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "putProxy", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	edglog(debug)<<"putProxy operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

//          ##################  NEW DELEGATION (2.0.0) METHODS ##############################
int delegationns__getVersion(struct soap *soap, delegationns__getVersionResponse &response)
{
	GLITE_STACK_TRY("delegationns__getVersion(struct soap *soap,"
		"struct delegationns__getVersionResponse &response)");
	edglog_fn("wmpgsoapoperations::delegationns__getVersion");
	edglog(info)<<"delegationns__getVersion operation called"<<endl;
	int return_value = SOAP_OK;
	getVersionResponse getVersion_response;
	try {
		getDelegationVersion(getVersion_response);
		response.getVersionReturn = getVersion_response.version;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException, "getVersion", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException, "getVersion", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	return return_value;
	GLITE_STACK_CATCH();






}
int
delegationns__getInterfaceVersion (struct soap *soap,
	struct delegationns__getInterfaceVersionResponse &response){
	GLITE_STACK_TRY("delegationns__getInterfaceVersion(struct soap *soap,"
		"struct delegationns__getInterfaceVersionResponse &response)");
	edglog_fn("wmpgsoapoperations::delegationns__getInterfaceVersion");
	edglog(info)<<"delegationns__getInterfaceVersion operation called"<<endl;
	int return_value = SOAP_OK;
	getVersionResponse getVersion_response;
	try {
		getDelegationIntefaceVersion(getVersion_response);
		response.getInterfaceVersionReturn = getVersion_response.version;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException, "getInterfaceVersion", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException, "getInterfaceVersion", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	return return_value;
	GLITE_STACK_CATCH();
}


// METHOD NOT IMPLEMENTED !! TODO
int delegationns__getServiceMetadata(struct soap *soap, std::basic_string<char, std::char_traits<char>, std::allocator<char> >, delegationns__getServiceMetadataResponse&)
{
	int return_value = SOAP_OK;
	// TODO implement the method if needed
	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
			"getServiceMetadata",
			time(NULL),
			SOAP_FAULT,
			"Method not supported");
	return_value = SOAP_FAULT;
	return return_value;
}


int
delegationns__getNewProxyReq(struct soap *soap,
	struct delegationns__getNewProxyReqResponse &response)
{
	GLITE_STACK_TRY("delegationns__getNewProxyReq(struct soap *soap, struct "
		"delegationns__getNewProxyReqResponse &response)");
	edglog_fn("wmpgsoapoperations::delegationns__getNewProxyReq");
	edglog(debug)<<"getNewProxyReq operation called"<<endl;

        initializingSignalHandler();
	int return_value = SOAP_OK;
	pair<string, string> retpair;
	try {
		response.delegationns__NewProxyReq = new _delegationns__NewProxyReq;
		getNewProxyReq(retpair);
		response.delegationns__NewProxyReq->proxyRequest = new string(retpair.second);
		response.delegationns__NewProxyReq->delegationID = new string(retpair.first);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 		"getNewProxyReq", time(NULL), exc.getCode(), (string) exc.what(),
	 		exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 		"getNewProxyReq", time(NULL), WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	return return_value;
	GLITE_STACK_CATCH();
}

int
delegationns__renewProxyReq(struct soap *soap, string delegation_id,
	struct delegationns__renewProxyReqResponse &response)
{
	GLITE_STACK_TRY("delegationns__renewProxyReq(struct soap *soap, string "
		"delegation_id, struct delegationns__renewProxyReqResponse &response)");
	edglog_fn("wmpgsoapoperations::delegationns__renewProxyReq");
	edglog(debug)<<"renewProxyReq operation called"<<endl;

        initializingSignalHandler();
	int return_value = SOAP_OK;

	// N.B. same as getProxyReq
	string renewProxyReq_response;
	try {
		renewProxyReq(renewProxyReq_response, delegation_id);
		response._renewProxyReqReturn = renewProxyReq_response;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 		"renewProxyReq", time(NULL), exc.getCode(), (string) exc.what(),
	 		exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 		"renewProxyReq", time(NULL), WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"renewProxyReq operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
delegationns__getTerminationTime(struct soap *soap, string delegation_id,
	struct delegationns__getTerminationTimeResponse &response)
{
	GLITE_STACK_TRY("delegationns__getTerminationTime(struct soap *soap, "
		"string delegation_id, struct delegationns__getTerminationTimeResponse "
		"&response)");
	edglog_fn("wmpgsoapoperations::delegationns__getTerminationTime");
	edglog(debug)<<"getTerminationTime operation called"<<endl;
	
        initializingSignalHandler();
	int return_value = SOAP_OK;

	time_t getProxyTerminationTime_response;
	try {
		getProxyTerminationTime(getProxyTerminationTime_response, delegation_id);
		response._getTerminationTimeReturn = getProxyTerminationTime_response;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 		"getTerminationTime", time(NULL), exc.getCode(), (string) exc.what(),
	 		exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 	"getTerminationTime", time(NULL), WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"getTerminationTime operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
delegationns__destroy(struct soap *soap, string delegation_id,
	struct delegationns__destroyResponse &response)
{
	GLITE_STACK_TRY("delegationns__destroy(struct soap *soap, string delegation_id,"
		"struct delegationns__destroyResponse &response)");
	edglog_fn("wmpgsoapoperations::delegationns__destroy");
	edglog(debug)<<"destroy operation called"<<endl;
	
        initializingSignalHandler();
	int return_value = SOAP_OK;

	try {
		destroyProxy(delegation_id);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 		"destroy", time(NULL), exc.getCode(), (string) exc.what(),
	 		exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 	"destroy", time(NULL), WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"destroy operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

//          ##################  COMMON METHODS FOR DELEGATION ##############################
int
delegationns__getProxyReq(struct soap *soap, string delegation_id,
	struct delegationns__getProxyReqResponse &response)
{
	GLITE_STACK_TRY("delegationns__getProxyReq(struct soap *soap, string "
		"delegation_id, delegationns__getProxyReqResponse &response)");
	edglog_fn("wmpgsoapoperations::delegationns__getProxyReq");
	edglog(debug)<<"getProxyReq operation called"<<endl;

	initializingSignalHandler();
	int return_value = SOAP_OK;

	getProxyReqResponse getProxyReq_response;
	try  {
		getProxyReq(getProxyReq_response, delegation_id);
		response._getProxyReqReturn = getProxyReq_response.request;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 		"getProxyReq", time(NULL), exc.getCode(), (string) exc.what(),
	 		exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 	"getProxyReq", time(NULL), WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(debug)<<"getProxyReq operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
delegationns__putProxy(struct soap *soap, string delegation_id, string proxy,
	struct delegationns__putProxyResponse &response)
{
	GLITE_STACK_TRY("delegationns__putProxy(struct soap *soap, string delegation_id, "
		"string proxy, struct delegationns__putProxyResponse &response)");
	edglog_fn("wmpgsoapoperations::delegationns__putProxy");
	edglog(debug)<<"putProxy operation called"<<endl;

	initializingSignalHandler();
	int return_value = SOAP_OK;

	putProxyResponse putProxy_response;
	try  {
		putProxy(putProxy_response, delegation_id, proxy);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 	"putProxy", time(NULL), exc.getCode(), (string) exc.what(),
	 	exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__delegationns__DelegationException,
	 	"putProxy", time(NULL), WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}

	edglog(debug)<<"putProxy operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}
//          ##################  END DELEGATION ##############################


int
ns1__getACLItems(struct soap *soap, string jobId,
	struct ns1__getACLItemsResponse &response)
{
	GLITE_STACK_TRY("ns1__getACLItems(struct soap *soap, string jobId,"
		"struct ns1__getACLItemsResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getACLItems");
	edglog(debug)<<"getACLItems operation called"<<endl;
	
	initializingSignalHandler();
	int return_value = SOAP_OK;
	
	getACLItemsResponse getACLItems_response;
	try  {
		getACLItems(getACLItems_response, jobId);
		response._items
			= convertVectorToGSOAPStringList(getACLItems_response);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getACLItems", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getACLItems", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"getACLItems operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__addACLItems(struct soap *soap, string jobId, ns1__StringList* items,
	struct ns1__addACLItemsResponse &response)
{
	GLITE_STACK_TRY("ns1__addACLItems(struct soap *soap, string "
		"jobId, ns1__StringList* proxy, struct "
		"ns1__addACLItemsResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__addACLItems");
	edglog(debug)<<"addACLItems operation called"<<endl;
	
	initializingSignalHandler();
	int return_value = SOAP_OK;

	addACLItemsResponse addACLItems_response;
	try  {
		addACLItems(addACLItems_response, jobId, convertToStringList(items));
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "addACLItems", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "addACLItems", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"addACLItems operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__removeACLItem(struct soap *soap, string jobId, string item,
	struct ns1__removeACLItemResponse &response)
{
	GLITE_STACK_TRY("ns1__removeACLItem(struct soap *soap, string jobId, "
		"string item, struct ns1__removeACLItemResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__removeACLItem");
	edglog(debug)<<"removeACLItem operation called"<<endl;
	
	initializingSignalHandler();
	int return_value = SOAP_OK;

	removeACLItemResponse removeACLItem_response;
	try  {
		removeACLItem(removeACLItem_response, jobId, item);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "removeACLItem", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "removeACLItem", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"removeACLItem operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getDelegatedProxyInfo(struct soap *soap, string delegation_id,
	struct ns1__getDelegatedProxyInfoResponse &response)
{
	GLITE_STACK_TRY("ns1__getDelegatedProxyInfo(struct soap *soap, string "
		"delegation_id, struct ns1__getDelegatedProxyInfoResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getDelegatedProxyInfo");
	edglog(debug)<<"getDelegatedProxyInfo operation called"<<endl;
	
	initializingSignalHandler();
	int return_value = SOAP_OK;
	
	getProxyInfoResponse getProxyInfo_response;
	try  {
		getProxyInfo(getProxyInfo_response, delegation_id);
		response._items =
			convertToGSOAPProxyInfoStructType(getProxyInfo_response.items);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getDelegatedProxyInfo", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getDelegatedProxyInfo", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"getDelegatedProxyInfo operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getJobProxyInfo(struct soap *soap, string job_id,
	struct ns1__getJobProxyInfoResponse &response)
{
	GLITE_STACK_TRY("ns1__getJobProxyInfo(struct soap *soap, string "
		"delegation_id, string job_id, struct ns1__getJobProxyInfoResponse"
		" &response)");
	edglog_fn("wmpgsoapoperations::ns1__getJobProxyInfo");
	edglog(debug)<<"getJobProxyInfo operation called"<<endl;
	
	initializingSignalHandler();
	int return_value = SOAP_OK;
	
	getProxyInfoResponse getProxyInfo_response;
	try  {
		getProxyInfo(getProxyInfo_response, job_id, true);
		response._items =
			convertToGSOAPProxyInfoStructType(getProxyInfo_response.items);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getJobProxyInfo", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getJobProxyInfo", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"getJobProxyInfo operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__enableFilePerusal(struct soap *soap, string jobId, ns1__StringList *filelist,
	struct ns1__enableFilePerusalResponse &response)
{
	GLITE_STACK_TRY("ns1__enableFilePerusal(struct soap *soap, string jobId, "
		"ns1__StringList *filelist, struct ns1__enableFilePerusalResponse "
		"&response)");
	edglog_fn("wmpgsoapoperations::ns1__enableFilePerusal");
	edglog(debug)<<"enableFilePerusal operation called"<<endl;
	
	initializingSignalHandler();
	int return_value = SOAP_OK;
	
	enableFilePerusalResponse enableFilePerusal_response;
	try  {
		enableFilePerusal(enableFilePerusal_response, jobId,
			convertToStringList(filelist));
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "enableFilePerusal", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "enableFilePerusal", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"enableFilePerusal operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getPerusalFiles(struct soap *soap, string jobId, string file,
	bool allChunks = false, string protocol = "",
	struct ns1__getPerusalFilesResponse &response
	= *(new ns1__getPerusalFilesResponse()))
{
	GLITE_STACK_TRY("ns1__getPerusalFiles(struct soap *soap, string jobId, "
		"string file, bool allChunks, struct ns1__getPerusalFilesResponse "
		"&response)");
	edglog_fn("wmpgsoapoperations::ns1__getPerusalFiles");
	edglog(debug)<<"getPerusalFiles operation called"<<endl;
	
	// Setting default value for protocol. gSOAP set it as "" if not provided
	if (protocol == "") {
		protocol = DEFAULT_PROTOCOL;
	}
	initializingSignalHandler();	
	int return_value = SOAP_OK;
	
	getPerusalFilesResponse getPerusalFiles_response;
	try  {
		getPerusalFiles(getPerusalFiles_response, jobId, file, allChunks,
			protocol);
		response._fileList
			= convertVectorToGSOAPStringList(getPerusalFiles_response);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getPerusalFiles", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getPerusalFiles", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"getPerusalFiles operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getTransferProtocols(struct soap *soap, 
	struct ns1__getTransferProtocolsResponse &response)
{
	GLITE_STACK_TRY("ns1__getTransferProtocols(struct soap soap*, "
		"struct ns1__getTransferProtocolsResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getTransferProtocols");
	edglog(debug)<<"getTransferProtocols operation called"<<endl;

	initializingSignalHandler();
	int return_value = SOAP_OK;

	ns1__StringList *list = new ns1__StringList();
	list->Item = *(new vector<string>);

	getTransferProtocolsResponse getTransferProtocols_response;
	try  {
		getTransferProtocols(getTransferProtocols_response);
		for (unsigned int i = 0;
				i < getTransferProtocols_response.protocols->Item->size(); i++) {
			list->Item.push_back((*getTransferProtocols_response.protocols->Item)[i]);
		}
		response.items = list;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "getTransferProtocols", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "getTransferProtocols", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(debug)<<"getTransferProtocols operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}


int ns1__getJobStatus(struct soap *soap, string job_id, struct ns1__getJobStatusResponse &response)
{
	GLITE_STACK_TRY("ns1__getJobStatus(struct soap soap*, "
		"struct ns1__getJobStatusResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getJobStatus");
	edglog(debug)<<"ns1__getJobStatus operation called"<<endl;

	initializingSignalHandler();
	int return_value = SOAP_OK;

	ns1__StringList *list = new ns1__StringList();
	list->Item = *(new vector<string>);

	getJobStatusResponse getJobStatus_response;

	try  {
		// Performing wmpoperations::getJobStatus
		getJobStatusOp(getJobStatus_response, job_id);
		// ns1__getJobStatusResponse STRUCT:  ns1__JobStatusStructType * JobStatusStruct;
		// getJobStatusResponse STRUCT: JobStatusStructType * jobStatus;
		response.JobStatusStruct = convertToGSOAPJobStatusStructType  ( getJobStatus_response.jobStatus );
	} catch (Exception &exc) {
	 	setSOAPFault(soap, exc.getCode(), "ns1__getJobStatus", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, WMS_IS_FAILURE, "ns1__getJobStatus", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	edglog(debug)<<"getJobStatus operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}
