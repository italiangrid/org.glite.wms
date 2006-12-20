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
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include <sstream>

// Boost
#include <boost/lexical_cast.hpp>

// gSOAP
#include "soapH.h"

// JSDL converter
//#include "jsdl.h"

// Server
#include "wmpoperations.h"
#include "wmpcoreoperations.h"
#include "wmpresponsestruct.h"
#include "wmpstructconverter.h"
#include "wmpgsoapfaultmanipulator.h"

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

int
ns1__getVersion(struct soap *soap, struct ns1__getVersionResponse &response)
{
	GLITE_STACK_TRY("ns1__getVersion(struct soap *soap, struct "
		"ns1__getVersionResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getVersion");
	edglog(info)<<"getVersion operation called"<<endl;

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
	
	edglog(info)<<"getVersion operation completed\n"<<endl;
	
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
	edglog(info)<<"getJDL operation called"<<endl;

	int return_value = SOAP_OK;

	getJDLResponse getJDL_response;
	
	JdlType jdltype;
	switch (type) {
		case ns1__JdlType__ORIGINAL:
			jdltype = WMS_JDL_ORIGINAL;
			break;
		case ns1__JdlType__REGISTERED:
			jdltype = WMS_JDL_REGISTERED;
			break;
		default:
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
	
	edglog(info)<<"getJDL operation completed\n"<<endl;
	
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
	edglog(info)<<"jobRegister operation called"<<endl;

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

	edglog(info)<<"jobRegister operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}


int
ns1__jobRegisterJSDL(struct soap *soap, jsdlns__JobDefinition_USCOREType *jsdl,
	string delegation_id, struct ns1__jobRegisterJSDLResponse &response)
{
	GLITE_STACK_TRY("ns1__jobRegisterJSDL(struct soap *soap, "
	"jsdlns__JobDefinition_USCOREType* jsdl, "
	"string delegation_id, struct ns1__jobRegisterResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__jobRegisterJSDL");
	edglog(info)<<"jobRegisterJSDL operation called"<<endl;

	int return_value = SOAP_OK;

	jobRegisterResponse jobRegister_response;

	try {
		// Converting JSDL to JDL before calling
		edglog(debug)<<"_____ JSDL: "<<endl<<jsdl->__any<<endl;
		
		/*jsdlns__JobDefinition_USCOREType *temp
			= soap_get_jsdlns__JobDefinition_USCOREType(soap,
				(jsdlns__JobDefinition_USCOREType *) jsdl,
				"http://schemas.ggf.org/jsdl/2005/11/jsdl:JobDefinition",
				"http://schemas.ggf.org/jsdl/2005/11/jsdl:JobDefinition_Type");*/
				
/*		jsdlns__JobDescription_USCOREType *temp
			= soap_get_jsdlns__JobDescription_USCOREType(soap,
				(jsdlns__JobDescription_USCOREType *) jsdl, "jsdl:JobDescription",
				"jsdl:JobDescription_Type");*/
			
		/*if (temp) {
			edglog(debug)<<"_____ TEMP item: "<<temp->__item<<endl;
			edglog(debug)<<"_____ TEMP any: "<<temp->__any<<endl;
		} else {
			edglog(debug)<<"_____ ERROR: "<<endl;
		}*/
		//string jdl = jsdl->__item;
		
		string jdl = "["
			"Type=\"job\";"
			"Executable = \"/bin/ls\";"
			"Arguments = \"-las\";"
			"StdOutput = \"ls.out\" ;"
			"StdError = \"ls.err\";"
			"OutputSandbox = {\"ls.out\", \"ls.err\"};"
			"PerusalFileEnable = true;"
			"PerusalTimeInterval = 11;"
			"RetryCount = 0;"
			"VirtualOrganisation = \"EGEE\";"
			"requirements = other.GlueCEStateStatus == \"Production\";"
			"rank = 3;"
			"]";
		
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

	edglog(info)<<"jobRegister operation completed\n"<<endl;
	
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
	edglog(info)<<"jobStart operation called"<<endl;

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

	edglog(info)<<"jobStart operation completed\n"<<endl;
	
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
	edglog(info)<<"jobSubmit operation called"<<endl;
	
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
	 
	edglog(info)<<"jobSubmit operation completed\n"<<endl;

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
	edglog(info)<<"jobCancel operation called"<<endl;

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

	edglog(info)<<"jobCancel operation completed\n"<<endl;
	
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
	edglog(info)<<"getMaxInputSandboxSize operation called"<<endl;

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

	edglog(info)<<"getMaxInputSandboxSize operation completed\n"<<endl;
	
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
	edglog(info)<<"getSandboxDestURI operation called"<<endl;
	
	// Setting default value for protocol. gSOAP set it as "" if not provided
	if (protocol == "") {
		protocol = ALL_PROTOCOLS;	
	}
	
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

	edglog(info)<<"getSandboxDestURI operation completed\n"<<endl;
	
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
	edglog(info)<<"getSandboxBulkDestURI operation called"<<endl;
	
	// Setting default value for protocol. gSOAP set it as "" if not provided
	if (protocol == "") {
		protocol = ALL_PROTOCOLS;	
	}
	
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

	edglog(info)<<"getSandboxBulkDestURI operation completed\n"<<endl;
	
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
	edglog(info)<<"getTotalQuota operation called"<<endl;

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

	edglog(info)<<"getTotalQuota operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getFreeQuota(struct soap *soap, struct ns1__getFreeQuotaResponse &response)
{
	GLITE_STACK_TRY("ns1__getFreeQuota(struct soap *soap, struct "
		"ns1__getFreeQuotaResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getFreeQuota");
	edglog(info)<<"getFreeQuota operation called"<<endl;
	
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

	edglog(info)<<"getFreeQuota operation completed\n"<<endl;
	
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
	edglog(info)<<"jobPurge operation called"<<endl;
	
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

	edglog(info)<<"jobPurge operation completed\n"<<endl;
	
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
	edglog(info)<<"getOutputFileList operation called"<<endl;

	// Setting default value for protocol. gSOAP set it as "" if not provided
	if (protocol == "") {
		protocol = DEFAULT_PROTOCOL;
	}
	
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

	edglog(info)<<"getOutputFileList operation completed\n"<<endl;
	
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
	edglog(info)<<"jobListMatch operation called"<<endl;

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

	edglog(info)<<"jobListMatch operation completed\n"<<endl;
	
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
	edglog(info)<<"getJobTemplate operation called"<<endl;
	
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
	
	edglog(info)<<"getJobTemplate operation completed\n"<<endl;

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
	edglog(info)<<"getDAGTemplate operation called"<<endl;

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
	
	edglog(info)<<"getDAGTemplate operation completed\n"<<endl;

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
	edglog(info)<<"getCollectionTemplate operation called"<<endl;

	int return_value = SOAP_OK;

	getCollectionTemplateResponse getCollectionTemplate_response;
	try {
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

	edglog(info)<<"getCollectionTemplate operation completed\n"<<endl;
	
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
	edglog(info)<<"getIntParametricJobTemplate operation called"<<endl;

	int return_value = SOAP_OK;

	if (!attributes) {
		setSOAPFault(soap, WMS_INVALID_ARGUMENT, "getIntParametricJobTemplate",
			time(NULL), WMS_INVALID_ARGUMENT, "Invalid Argument");
		return_value = SOAP_FAULT;
	} else {
		getIntParametricJobTemplateResponse 
			getIntParametricJobTemplate_response;
		try {
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

	edglog(info)<<"getIntParametricJobTemplate operation completed\n"<<endl;
	
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
	edglog(info)<<"getStringParametricJobTemplate operation called"<<endl;

	int return_value = SOAP_OK;

	if (!attributes || !param) {
		setSOAPFault(soap, WMS_INVALID_ARGUMENT, "getDAGTemplate", time(NULL),
	 		WMS_INVALID_ARGUMENT, "Invalid Argument");
		return_value = SOAP_FAULT;
	} else {
		getStringParametricJobTemplateResponse 
			getStringParametricJobTemplate_response;
		try {
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

	edglog(info)<<"getStringParametricJobTemplate operation completed\n"<<endl;
	
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
	edglog(info)<<"getProxyReq operation called"<<endl;
	
	int return_value = SOAP_OK;

	getProxyReqResponse getProxyReq_response;
	try {
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
	 
	edglog(info)<<"getProxyReq operation completed\n"<<endl;

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
	edglog(info)<<"putProxy operation called"<<endl;
	
	int return_value = SOAP_OK;

	putProxyResponse putProxy_response;
	try {
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
	
	edglog(info)<<"putProxy operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}


int
ns2__getVersion (struct soap *soap,
	struct ns2__getVersionResponse &response){
	GLITE_STACK_TRY("ns2__getVersion(struct soap *soap,"
		"struct ns2__getVersionResponse &response)");
	edglog_fn("wmpgsoapoperations::ns2__getVersion");
	edglog(info)<<"ns2__getVersion operation called"<<endl;
	int return_value = SOAP_OK;
	getVersionResponse getVersion_response;
	try {
		getDelegationVersion(getVersion_response);
		response.getVersionReturn = getVersion_response.version;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "getVersion", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "getVersion", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns2__getInterfaceVersion (struct soap *soap,
	struct ns2__getInterfaceVersionResponse &response){
	GLITE_STACK_TRY("ns2__getInterfaceVersion(struct soap *soap,"
		"struct ns2__getInterfaceVersionResponse &response)");
	edglog_fn("wmpgsoapoperations::ns2__getInterfaceVersion");
	edglog(info)<<"ns2__getInterfaceVersion operation called"<<endl;
	int return_value = SOAP_OK;
	getVersionResponse getVersion_response;
	try {
		getDelegationIntefaceVersion(getVersion_response);
		response.getInterfaceVersionReturn = getVersion_response.version;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "getInterfaceVersion", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "getInterfaceVersion", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns2__getServiceMetadata (struct soap *soap, string key,
	struct ns2__getServiceMetadataResponse &response){
	GLITE_STACK_TRY("ns2__getServiceMetadata(struct soap *soap,"
		"struct ns2__getServiceMetadata &response)");
	edglog_fn("wmpgsoapoperations::ns2__getServiceMetadata");
	edglog(info)<<"ns2__getServiceMetadata operation called"<<endl;
	int return_value = SOAP_OK;
	response._getServiceMetadataReturn = "Sorry, this service has not been implemented yet !";
	return return_value;
	GLITE_STACK_CATCH();
}
int
ns2__getProxyReq(struct soap *soap, string delegation_id,
	struct ns2__getProxyReqResponse &response)
{
	GLITE_STACK_TRY("ns2__getProxyReq(struct soap *soap, string delegation_id, "
		"ns2__getProxyReqResponse &response)");
	edglog_fn("wmpgsoapoperations::ns2__getProxyReq");
	edglog(info)<<"getProxyReq operation called"<<endl;
	
	int return_value = SOAP_OK;

	getProxyReqResponse getProxyReq_response;
	try {
		getProxyReq(getProxyReq_response, delegation_id);
		response._getProxyReqReturn = getProxyReq_response.request;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "getProxyReq", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "getProxyReq", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	 
	edglog(info)<<"getProxyReq operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns2__putProxy(struct soap *soap, string delegation_id, string proxy,
	struct ns2__putProxyResponse &response)
{
	GLITE_STACK_TRY("ns2__putProxy(struct soap *soap, string delegation_id, "
		"string proxy, struct ns2__putProxyResponse &response)");
	edglog_fn("wmpgsoapoperations::ns2__putProxy");
	edglog(info)<<"putProxy operation called"<<endl;
	
	int return_value = SOAP_OK;

	putProxyResponse putProxy_response;
	try {
		putProxy(putProxy_response, delegation_id, proxy);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "putProxy", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "putProxy", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(info)<<"putProxy operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns2__getNewProxyReq(struct soap *soap, struct ns2__getNewProxyReqResponse &response)
{
	GLITE_STACK_TRY("ns2__getNewProxyReq(struct soap *soap, struct "
		"ns2__getNewProxyReqResponse &response)");
	edglog_fn("wmpgsoapoperations::ns2__getNewProxyReq");
	edglog(info)<<"getNewProxyReq operation called"<<endl;

	int return_value = SOAP_OK;
	pair<string, string> retpair;

	try {
		response.ns2__NewProxyReq = new _ns2__NewProxyReq;
		getNewProxyReq(retpair);
		edglog(debug)<<"____ retpair.1: "<<retpair.first<<endl;
		edglog(debug)<<"____ retpair.2: "<<retpair.second<<endl;
		response.ns2__NewProxyReq->proxyRequest = new string(retpair.second);
		response.ns2__NewProxyReq->delegationID = new string(retpair.first);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "getNewProxyReq", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "getNewProxyReq", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	return return_value;
	GLITE_STACK_CATCH();
}

int
ns2__renewProxyReq(struct soap *soap, string delegation_id,
	struct ns2__renewProxyReqResponse &response)
{
	GLITE_STACK_TRY("ns2__renewProxyReq(struct soap *soap, string delegation_id,"
		"struct ns2__renewProxyReqResponse &response)");
	edglog_fn("wmpgsoapoperations::ns2__renewProxyReq");
	edglog(info)<<"renewProxyReq operation called"<<endl;
	
	int return_value = SOAP_OK;

	// N.B. same as getProxyReq
	renewProxyReqResponse renewProxyReq_response;
	try {
		renewProxyReq(renewProxyReq_response, delegation_id);
		response._renewProxyReqReturn = renewProxyReq_response.request;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "renewProxyReq", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "renewProxyReq", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(info)<<"renewProxyReq operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns2__getTerminationTime(struct soap *soap, string delegation_id,
	struct ns2__getTerminationTimeResponse &response)
{
	GLITE_STACK_TRY("ns2__getTerminationTime(struct soap *soap, string delegation_id,"
		"struct ns2__getTerminationTimeResponse &response)");
	edglog_fn("wmpgsoapoperations::ns2__getTerminationTime");
	edglog(info)<<"getTerminationTime operation called"<<endl;
	
	int return_value = SOAP_OK;

	getProxyTerminationTimeResponse getProxyTerminationTime_response;
	try {
		getProxyTerminationTime(getProxyTerminationTime_response, delegation_id);
		response._getTerminationTimeReturn = getProxyTerminationTime_response;
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "getTerminationTime", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "getTerminationTime", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(info)<<"getTerminationTime operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns2__destroy(struct soap *soap, string delegation_id,
	struct ns2__destroyResponse &response)
{
	GLITE_STACK_TRY("ns2__destroy(struct soap *soap, string delegation_id,"
		"struct ns2__destroyResponse &response)");
	edglog_fn("wmpgsoapoperations::ns2__destroy");
	edglog(info)<<"destroy operation called"<<endl;
	
	int return_value = SOAP_OK;

	try {
		destroyProxy(delegation_id);
	} catch (Exception &exc) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "destroy", time(NULL),
	 		exc.getCode(), (string) exc.what(), exc.getStackTrace());
		return_value = SOAP_FAULT;
	} catch (exception &ex) {
	 	setSOAPFault(soap, SOAP_TYPE__ns2__DelegationException, "destroy", time(NULL),
	 		WMS_IS_FAILURE, (string) ex.what());
		return_value = SOAP_FAULT;
	}
	
	edglog(info)<<"destroy operation completed\n"<<endl;

	return return_value;
	GLITE_STACK_CATCH();
}

int
ns1__getACLItems(struct soap *soap, string jobId,
	struct ns1__getACLItemsResponse &response)
{
	GLITE_STACK_TRY("ns1__getACLItems(struct soap *soap, string jobId,"
		"struct ns1__getACLItemsResponse &response)");
	edglog_fn("wmpgsoapoperations::ns1__getACLItems");
	edglog(info)<<"getACLItems operation called"<<endl;
	
	int return_value = SOAP_OK;
	
	getACLItemsResponse getACLItems_response;
	try {
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
	
	edglog(info)<<"getACLItems operation completed\n"<<endl;

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
	edglog(info)<<"addACLItems operation called"<<endl;
	
	int return_value = SOAP_OK;

	addACLItemsResponse addACLItems_response;
	try {
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
	
	edglog(info)<<"addACLItems operation completed\n"<<endl;

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
	edglog(info)<<"removeACLItem operation called"<<endl;
	
	int return_value = SOAP_OK;

	removeACLItemResponse removeACLItem_response;
	try {
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
	
	edglog(info)<<"removeACLItem operation completed\n"<<endl;

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
	edglog(info)<<"getDelegatedProxyInfo operation called"<<endl;
	
	int return_value = SOAP_OK;
	
	getProxyInfoResponse getProxyInfo_response;
	try {
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
	
	edglog(info)<<"getDelegatedProxyInfo operation completed\n"<<endl;

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
	edglog(info)<<"getJobProxyInfo operation called"<<endl;
	
	int return_value = SOAP_OK;
	
	getProxyInfoResponse getProxyInfo_response;
	try {
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
	
	edglog(info)<<"getJobProxyInfo operation completed\n"<<endl;

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
	edglog(info)<<"enableFilePerusal operation called"<<endl;
	
	int return_value = SOAP_OK;
	
	enableFilePerusalResponse enableFilePerusal_response;
	try {
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
	
	edglog(info)<<"enableFilePerusal operation completed\n"<<endl;

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
	edglog(info)<<"getPerusalFiles operation called"<<endl;
	
	// Setting default value for protocol. gSOAP set it as "" if not provided
	if (protocol == "") {
		protocol = DEFAULT_PROTOCOL;	
	}
	
	int return_value = SOAP_OK;
	
	getPerusalFilesResponse getPerusalFiles_response;
	try {
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
	
	edglog(info)<<"getPerusalFiles operation completed\n"<<endl;

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
	edglog(info)<<"getTransferProtocols operation called"<<endl;
	
	int return_value = SOAP_OK;
	
	ns1__StringList *list = new ns1__StringList();
	list->Item = *(new vector<string>);
	
	getTransferProtocolsResponse getTransferProtocols_response;
	try {
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
	
	edglog(info)<<"getTransferProtocols operation completed\n"<<endl;
	
	return return_value;
	GLITE_STACK_CATCH();
}


