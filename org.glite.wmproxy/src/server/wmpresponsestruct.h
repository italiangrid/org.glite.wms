/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

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


// Response structures

struct getVersionResponse {
  std::string version;
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
  int size;
};

struct getSandboxDestURIResponse {
  std::string path;
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

#endif // GLITE_WMS_WMPROXY_WMPRESPONSESTRUCT_H
