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
	const std::string &jdl, const std::string &delegation_id);

void jobStart(jobStartResponse &jobStart_response, const std::string &job_id);

void jobSubmit(jobSubmitResponse &jobSubmit_response, const std::string &jdl,
	const std::string &delegation_id);

void jobCancel(jobCancelResponse &jobCancel_response, const std::string &job_id);

void getMaxInputSandboxSize(getMaxInputSandboxSizeResponse
	&getMaxInputSandboxSize_response);

void getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response,
	const std::string &job_id);

void getQuota(getQuotaResponse &getQuota_response);

void getFreeQuota(getFreeQuotaResponse &getFreeQuota_response);

void jobPurge(jobPurgeResponse &jobPurge_response, const std::string &job_id);

void getOutputFileList(getOutputFileListResponse &getOutputFileList_response,
	const std::string &job_id);

void jobListMatch(jobListMatchResponse &jobListMatch_response,
	const std::string &jdl);

void getJobTemplate(getJobTemplateResponse &getJobTemplate_response,
	JobTypeList job_type, const std::string &executable, 
	const std::string &arguments, const std::string &requirements,
	const std::string &rank);

void getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response,
	GraphStructType dependencies, const std::string &requirements,
	const std::string &rank);

void getCollectionTemplate(getCollectionTemplateResponse
	&getCollectionTemplate_response, int job_number,
	const std::string &requirements,
	const std::string &rank);

void getProxyReq(getProxyReqResponse &getProxyReq_response,
	const std::string &delegation_id);

void putProxy(putProxyResponse &putProxyReq_response, 
	const std::string &delegation_id, const std::string &proxy);

	
//} // wmproxy
//} // wms
//} // glite

#endif // GLITE_WMS_WMPROXY_WMPOPERATIONS_H

