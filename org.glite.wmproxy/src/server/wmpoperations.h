#ifndef  GLITE_WMS_WMPROXY_WMPOPERATIONS_H
#define GLITE_WMS_WMPROXY_WMPOPERATIONS_H

/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include "glite/lb/JobStatus.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wms/jdl/ExpDagAd.h"

#include "wmplogger.h"
#include "wmpgsoapfaultmanipulator.h"
#include "wmpresponsestruct.h"

//namespace glite {
//namespace wms {
//namespace wmproxyname {

void ping(pingResponse &ping_response);

void getVersion(getVersionResponse &getVersion_response);

void jobRegister(jobRegisterResponse &jobRegister_response, string &jdl);

void jobStart(jobStartResponse &jobStart_response, string jobId);

void jobSubmit(jobSubmitResponse &jobSubmit_response, string jdl)  ;

void jobCancel(jobCancelResponse &jobCancel_response, string jobId);

void getMaxInputSandboxSize(getMaxInputSandboxSizeResponse &getMaxInputSandboxSize_response);

void getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response, string jobId);

void getQuota(getQuotaResponse &getQuota_response);

void getFreeQuota(getFreeQuotaResponse &getFreeQuota_response);

void jobPurge(jobPurgeResponse &jobPurge_response, string jobId);

void getOutputFileList(getOutputFileListResponse &getOutputFileList_response, string jobId);

void jobListMatch(jobListMatchResponse &jobListMatch_response, string jdl);

void getJobTemplate(getJobTemplateResponse &getJobTemplate_response, JobTypeList jobType, string executable, string arguments, string requirements, string rank);

void getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response, GraphStructType dependencies, string requirements, string rank);

void getCollectionTemplate(getCollectionTemplateResponse &getCollectionTemplate_response, int jobNumber, string requirements, string rank);


glite::lb::JobStatus getStatus(glite::wmsutils::jobid::JobId *jid);

void regist(glite::wms::jdl::JobAd *jad, glite::wmsutils::jobid::JobId *jid, jobRegisterResponse &jobRegister_response, string dest_uri);

void regist(glite::wms::jdl::ExpDagAd *dag, glite::wmsutils::jobid::JobId *jid, jobRegisterResponse &jobRegister_response, string dest_uri);

void regist(glite::wms::jdl::ExpDagAd *dag, glite::wmsutils::jobid::JobId *jid, WMPLogger &wmplogger);

void regist(glite::wms::jdl::JobAd *jad, glite::wmsutils::jobid::JobId *jid, WMPLogger &wmplogger);

void start(jobStartResponse &jobStart_response, glite::wmsutils::jobid::JobId *jid, WMPLogger &wmplogger);

//} // wmproxy
//} // wms
//} // glite

#endif // GLITE_WMS_WMPROXY_WMPOPERATIONS_H

