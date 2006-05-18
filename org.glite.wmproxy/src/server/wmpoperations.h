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


// Web service available operations (see wmpcoreoperations.h for core operations).
// All methods are void, response values are inserted in the corresponding
// response structure.
// For more information about arguments see Web Service Description Language
// file (wsdl).

void getVersion(getVersionResponse &getVersion_response);

void getJDL(const std::string &job_id, JdlType jdltype,
	getJDLResponse &getJDL_response);

void getMaxInputSandboxSize(getMaxInputSandboxSizeResponse
	&getMaxInputSandboxSize_response);

void getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response,
	const std::string &job_id, const std::string &protocol);

void getSandboxBulkDestURI(getSandboxBulkDestURIResponse
	&getSandboxBulkDestURI_response, const std::string &job_id,
	const std::string &protocol);
	
void getQuota(getQuotaResponse &getQuota_response);

void getFreeQuota(getFreeQuotaResponse &getFreeQuota_response);

void getOutputFileList(getOutputFileListResponse &getOutputFileList_response,
	const std::string &job_id, const std::string &protocol);
	
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
/*
void getNewProxyReq(std::pair<std::string, std::string> &retpair);
*/
void putProxy(putProxyResponse &putProxyReq_response, 
	const std::string &delegation_id, const std::string &proxy);
/*
void destroyProxy(const std::string &delegation_id);

void getProxyTerminationTime(getProxyTerminationTimeResponse 
	&getProxyTerminationTime_response, const std::string &delegation_id);
*/
void getACLItems(getACLItemsResponse &getACLItems_response,
	const std::string &job_id);
	
void addACLItems(addACLItemsResponse &addACLItems_response,
	const std::string &job_id, StringList *dnlist);
	
void removeACLItem(removeACLItemResponse &removeACLItem_response,
	const std::string &job_id, const std::string &item);

void getProxyInfo(getProxyInfoResponse &getProxyInfo_response,
	const std::string &id, bool isjobid = false);

void enableFilePerusal(enableFilePerusalResponse &enableFilePerusal_response,
	const std::string &job_id, StringList *fileList);
	
void getPerusalFiles(getPerusalFilesResponse
	&getPerusalFiles_response, const std::string &job_id, const std::string
	&fileName, bool allChunks, const std::string &protocol);
	
void getTransferProtocols(getTransferProtocolsResponse
	&getTransferProtocols_response);

#endif // GLITE_WMS_WMPROXY_WMPOPERATIONS_H

