/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpoperations.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#ifndef GLITE_WMS_WMPROXY_WMPOPERATIONS_H
#define GLITE_WMS_WMPROXY_WMPOPERATIONS_H


#include "wmpresponsestruct.h"


// Web service available operations.
// All methods are void, response values are inserted in the corresponding
// response structure.
// For more information about arguments see Web Service Description Language
// file (wsdl).

void getVersion(getVersionResponse &getVersion_response);

void jobRegister(jobRegisterResponse &jobRegister_response,
	const std::string &jdl, const std::string &delegation_id);

void jobStart(jobStartResponse &jobStart_response, const std::string &job_id,
	struct soap *soap);

void jobSubmit(struct ns1__jobSubmitResponse &response, 
	jobSubmitResponse &jobSubmit_response, const std::string &jdl,
	const std::string &delegation_id, struct soap *soap);

void jobCancel(jobCancelResponse &jobCancel_response, const std::string &job_id);

void getMaxInputSandboxSize(getMaxInputSandboxSizeResponse
	&getMaxInputSandboxSize_response);

void getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response,
	const std::string &job_id);

void getSandboxBulkDestURI(getSandboxBulkDestURIResponse
	&getSandboxBulkDestURI_response, const std::string &job_id);
	
void getQuota(getQuotaResponse &getQuota_response);

void getFreeQuota(getFreeQuotaResponse &getFreeQuota_response);

void jobPurge(jobPurgeResponse &jobPurge_response, const std::string &job_id);

void getOutputFileList(getOutputFileListResponse &getOutputFileList_response,
	const std::string &job_id);

void jobListMatch(jobListMatchResponse &jobListMatch_response,
	const std::string &jdl, const std::string &delegation_id);

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

void getIntParametricJobTemplate(getIntParametricJobTemplateResponse
	&getIntParametricJobTemplate_response, StringList *attributes, int param,
	int parameterStart, int parameterStep, const std::string &requirements,
	const std::string &rank);
	
void getStringParametricJobTemplate(getStringParametricJobTemplateResponse
	&getStringParametricJobTemplate_response, StringList *attributes,
	StringList *param, const std::string &requirements, const std::string &rank);
	
void getProxyReq(getProxyReqResponse &getProxyReq_response,
	const std::string &delegation_id);

void putProxy(putProxyResponse &putProxyReq_response, 
	const std::string &delegation_id, const std::string &proxy);

std::vector<std::string> getACLItems(getACLItemsResponse &getACLItems_response,
	const std::string &job_id);
	
void addACLItems(addACLItemsResponse &addACLItems_response,
	const std::string &job_id, StringList *dnlist);
	
void removeACLItem(removeACLItemResponse &removeACLItem_response,
	const std::string &job_id, const std::string &item);

void getDelegatedProxyInfo(getDelegatedProxyInfoResponse 
	&getDelegatedProxyInfo_response, const std::string &job_id,
	bool isjob = false, const std::string &job_id = "");

void enableFilePerusal(enableFilePerusalResponse &enableFilePerusal_response,
	const std::string &job_id, StringList *fileList);
	
std::vector<std::string> getPerusalFiles(getPerusalFilesResponse
	&getPerusalFiles_response, const std::string &job_id, const std::string
	&fileName, bool allChunks);
	

#endif // GLITE_WMS_WMPROXY_WMPOPERATIONS_H

