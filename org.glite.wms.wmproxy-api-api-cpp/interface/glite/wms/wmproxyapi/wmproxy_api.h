#ifndef  GLITE_WMS_WMPROXYAPICPP_H
#define GLITE_WMS_WMPROXYAPICPP_H

#include <iostream>
#include <string>
#include <vector>

namespace glite {
namespace wms {
namespace wmproxyapi {



enum jobtype {
	/*** Normal Jobs */
	JOBTYPE_NORMAL=1,
	/*** Parametric Jobs */
	JOBTYPE_PARAMETRIC =2,
	/***  Interactive Jobs */
	JOBTYPE_INTERACTIVE=4,
	/*** Mpi Jobs */
	JOBTYPE_MPICH=8,
	/*** PArtitionable Jobs */
	JOBTYPE_PARTITIONABLE=16,
	/*** Checkpointable Jobs */
	JOBTYPE_CHECKPOINTABLE =32
};

struct NodeStruct {
	std::string* name ;
	std::vector< NodeStruct* > childrenNodes ;
};
struct JobIdStruct{
	std::string jobid ;
	std::string* nodeName ;
	std::vector< JobIdStruct* > children ;
};
struct ConfigContext{
	ConfigContext( std::string p , std::string s, std::string t);
	std::string proxy_file;
	std::string endpoint;
	std::string trusted_cert_dir;
};

std::string getVersion(ConfigContext *cfs=NULL);
JobIdStruct jobRegister (std::string &jdl, std::string &delegationId, ConfigContext *cfs=NULL);
JobIdStruct jobSubmit(std::string &jdl, std::string &delegationId, ConfigContext *cfs=NULL);

void jobStart(std::string &jobid, ConfigContext *cfs=NULL);
void jobCancel(std::string &jobid, ConfigContext *cfs=NULL);
int getMaxInputSandboxSize(ConfigContext *cfs=NULL);
std::string  getSandboxDestURI(std::string &jobid, ConfigContext *cfs=NULL);
std::pair<long, long> getTotalQuota(ConfigContext *cfs=NULL);
std::pair<long, long>getFreeQuota(ConfigContext *cfs=NULL);
void jobPurge(std::string &jobid, ConfigContext *cfs=NULL);
std::vector <std::pair<std::string , long> > getOutputFileList (std::string &jobid, ConfigContext *cfs=NULL);
std::vector <std::pair<std::string , long> > jobListMatch (std::string &jdl, ConfigContext *cfs=NULL);

std::string getJobTemplate (int jobType, std::string &executable,std::string &arguments,std::string &requirements,std::string &rank, ConfigContext *cfs=NULL);
std::string getDAGTemplate(NodeStruct dependencies, std::string &requirements,std::string &rank, ConfigContext *cfs=NULL);
std::string getCollectionTemplate(int jobNumber, std::string &requirements,std::string &rank, ConfigContext *cfs=NULL);
std::string getIntParametricJobTemplate (std::vector<std::string> attributes , int parameters , int start , int step , std::string &requirements,std::string &rank, ConfigContext *cfs=NULL);
std::string getStringParametricJobTemplate (std::vector<std::string>attributes, std::vector<std::string> parameters, const std::string &requirements,const std::string &rank, ConfigContext *cfs=NULL);
std::string getProxyRequest(const std::string &request, ConfigContext *cfs=NULL);
void putProxy(const std::string &delegationId, const std::string &proxy, ConfigContext *cfs=NULL);

} // wmproxy namespace
} // wms namespace
} // glite namespace

#endif
//EOF
