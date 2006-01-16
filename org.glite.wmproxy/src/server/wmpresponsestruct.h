/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpresponsestruct.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#ifndef GLITE_WMS_WMPROXY_WMPRESPONSESTRUCT_H
#define GLITE_WMS_WMPROXY_WMPRESPONSESTRUCT_H

#include <string>
#include <vector>


// Possible values for "type" jdl attribute
enum ObjectType {
	WMS_JOB,
	WMS_DAG,
	WMS_COLLECTION,
};

// Possible values for "jobType" jdl attribute (and combination of some)
enum JobType {
	WMS_PARAMETRIC,
	WMS_NORMAL,
	WMS_INTERACTIVE,
	WMS_MPI,
	WMS_PARTITIONABLE,
	WMS_CHECKPOINTABLE,
};

enum JdlType {
	WMS_JDL_ORIGINAL,
	WMS_JDL_REGISTERED,
};

// Base fault type to hold all kinds of faults
struct BaseFaultType
{
    std::string methodName;
    time_t timestamp;
    std::string *errorCode;
    std::string *description;
    std::vector<std::string> *faultCause;
};

// Struct used to communicate with wmp manager
struct wmp_fault_t {
    int code;
    std::string message;
    std::vector<std::string> stack;
};


// Struct used inside response structures (see below)

struct JobTypeList
{
    std::vector<enum JobType> *jobType;
};

struct StringList { 
    std::vector<std::string> *Item;
};

struct StringAndLongType {
    std::string name;
    long size;
};

struct StringAndLongList {
    std::vector<StringAndLongType*> *file;
};

struct JobIdStructType {
    std::string id;
    std::string *name;
    std::vector<JobIdStructType*> *childrenJob;
};

struct GraphStructType {
    std::string *name;
    std::vector<GraphStructType*> *childrenJob;
};

struct DestURIStructType {
	std::string id;
    std::vector<std::string> *destURIs;
};

struct DestURIsStructType {
	std::vector<DestURIStructType*> *Item;
};

struct VOProxyInfoStructType {
	std::string user;
	std::string userCA;
	std::string server;
	std::string serverCA;
	std::string voName;
	std::string uri;
	std::string startTime;
	std::string endTime;
	std::vector<std::string> attribute;
};

struct ProxyInfoStructType {
	std::string subject;
	std::string issuer;
	std::string identity;
	std::string type;
	std::string strength;
	std::string startTime;
	std::string endTime;
	std::vector<VOProxyInfoStructType*> vosInfo;
};

// Response structures

struct getVersionResponse {
  std::string version;
};

struct getJDLResponse {
  std::string jdl;
};

struct jobRegisterResponse {
  JobIdStructType *jobIdStruct;
};

struct jobStartResponse {
};

struct jobSubmitResponse {
  JobIdStructType *jobIdStruct;
};

struct jobCancelResponse {
};

struct getMaxInputSandboxSizeResponse {  
  long size;
};

struct getSandboxDestURIResponse {
  StringList *path;
};

struct getSandboxBulkDestURIResponse {
  DestURIsStructType *destURIsStruct;
};

struct getQuotaResponse {
  long softLimit;
  long hardLimit;
};

struct getFreeQuotaResponse {
  long softLimit;
  long hardLimit;
};

struct jobPurgeResponse {
};

struct getOutputFileListResponse {
  StringAndLongList *OutputFileAndSizeList;
};

struct jobListMatchResponse {
  StringAndLongList *CEIdAndRankList;
};

struct getJobTemplateResponse {
  std::string jdl;
};

struct getDAGTemplateResponse {
  std::string jdl;
};

struct getCollectionTemplateResponse {
  std::string jdl;
};

struct getProxyReqResponse {
  std::string request;
};

struct putProxyResponse {
};

struct getIntParametricJobTemplateResponse {
	std::string jdl;
};
  
struct getStringParametricJobTemplateResponse {
    std::string jdl;
};

struct getACLItemsResponse {
	StringList *items;
};

struct addACLItemsResponse {
};

struct removeACLItemResponse {
};

struct getProxyInfoResponse {
	ProxyInfoStructType *items;
};

struct enableFilePerusalResponse {
};

struct getPerusalFilesResponse {
	StringList *files;
};

#endif // GLITE_WMS_WMPROXY_WMPRESPONSESTRUCT_H
