#ifndef  GLITE_WMS_WMPROXYAPICPP_H
#define GLITE_WMS_WMPROXYAPICPP_H

#include <iostream>
#include <string>
#include <vector>

namespace glite {
namespace wms {
namespace wmproxy-api {

struct NodeStruct {
	std::string* name ;
	std::vector< NodeStruct* > childrenNodes ;
};
struct JobIdStruct{
	std::string jobid ;
	std::string* nodeName ;
	std::vector< JobIdStruct* > children ;
};
class ConfigContext{
	public:
		ConfigContext( std::string p , std::string s);
	private:
		std::string proxy_file;
		std::string service_address;
};

std::string getVersion(ConfigContext cfs=NULL);
JobIdStruct jobRegister (const std::string &jdl, const std::string &delegationId, ConfigContext cfs=NULL);
JobIdStruct jobSubmit(const std::string &jdl, const std::string &delegationId, ConfigContext cfs=NULL);

void jobStart(const std::string &jobid, ConfigContext cfs=NULL);
void jobCancel(const std::string &jobid, ConfigContext cfs=NULL);
int getMaxInputSandboxSize(ConfigContext cfs=NULL);
std::string  getSandboxDestURI(const std::string &jobid, ConfigContext cfs=NULL);
std::pair<long, long> getTotalQuota(ConfigContext cfs=NULL);
std::pair<long, long>getFreeQuota(ConfigContext cfs=NULL);
void jobPurge(const std::string &jobid, ConfigContext cfs=NULL);
std::vector <std::pair<std::string , long>> getOutputFileList (const std::string &jobid, ConfigContext cfs=NULL);
std::vector<std::string> jobListMatch (const std::string &jdl, ConfigContext cfs=NULL);

std::string getJobTemplate (std::vector<std::string > jobType, const std::string &executable,const std::string &arguments,const std::string &requirements,const std::string &rank, ConfigContext cfs=NULL);
std::string getDAGTemplate(NodeStruct dependencies, const std::string &requirements,const std::string &rank, ConfigContext cfs=NULL);
std::string getCollectionTemplate(int jobNumber, const std::string &requirements,const std::string &rank, ConfigContext cfs=NULL);
std::string getIntParametricJobTemplate (std::vector<std::string> attributes , int parameters , int start , int step , const std::string &requirements,const std::string &rank, ConfigContext cfs=NULL);
std::string getStringParametricJobTemplate (std::vector<std::string>attributes, std::vector<std::string> parameters, const std::string &requirements,const std::string &rank, ConfigContext cfs=NULL);

} // wmproxy namespace
} // wms namespace
} // glite namespace

#endif
//EOF