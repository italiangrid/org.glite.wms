/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_WMPOPERATIONS_H
#define GLITE_WMS_WMPROXY_WMPOPERATIONS_H

#include "wmpresponsestruct.h"

//namespace glite {
//namespace wms {
//namespace wmproxyname {

// Web service available operations.
// All methods are void, response values are inserted in the corresponding
// response structure.
// For more information about arguments see Web Service Description Language
// file (wsdl).

void getVersion(getVersionResponse &getVersion_response);

void jobRegister(jobRegisterResponse &jobRegister_response,
	const std::string &jdl, const std::string &delegationId);

void jobStart(jobStartResponse &jobStart_response, const std::string &jobId);

void jobSubmit(jobSubmitResponse &jobSubmit_response, const std::string &jdl);

void jobCancel(jobCancelResponse &jobCancel_response, const std::string &jobId);

void getMaxInputSandboxSize(getMaxInputSandboxSizeResponse
	&getMaxInputSandboxSize_response);

void getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response,
	const std::string &jobId);

void getQuota(getQuotaResponse &getQuota_response);

void getFreeQuota(getFreeQuotaResponse &getFreeQuota_response);

void jobPurge(jobPurgeResponse &jobPurge_response, const std::string &jobId);

void getOutputFileList(getOutputFileListResponse &getOutputFileList_response,
	const std::string &jobId);

void jobListMatch(jobListMatchResponse &jobListMatch_response,
	const std::string &jdl);

void getJobTemplate(getJobTemplateResponse &getJobTemplate_response,
	JobTypeList jobType, const std::string &executable, 
	const std::string &arguments, const std::string &requirements,
	const std::string &rank);

void getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response,
	GraphStructType dependencies, const std::string &requirements,
	const std::string &rank);

void getCollectionTemplate(getCollectionTemplateResponse
	&getCollectionTemplate_response, int jobNumber,
	const std::string &requirements,
	const std::string &rank);


//} // wmproxy
//} // wms
//} // glite

#endif // GLITE_WMS_WMPROXY_WMPOPERATIONS_H

