#include <iostream>
#include <string>
#include <vector>
std::string getVersion();
jobidStruct jobRegister (const std::string &jdl, const std::string &delegationId);
jobidStruct jobSubmit(const std::string &jdl, const std::string &delegationId);
void jobStart(const std::string &jobid);
void jobCancel(const std::string &jobid);
int getMaxInputSandboxSize();
std::string  getSandboxDestURI(const std::string &jobid);
std::pair<long, long> getTotalQuota();
std::pair<long, long>getFreeQuota();
void jobPurge(const std::string &jobid);
std::vector <std::pair<std::string , long>> getOutputFileList (const std::string &jobid);
std::vector<std::string> jobListMatch (const std::string &jdl);
std::string  getJobTemplate (std::vector<std::string > jobType, const std::string &executable,const std::string &arguments,const std::string &requirements,const std::string &rank);
std::string getDAGTemplate(dependencyStruct dependencies, const std::string &requirements,const std::string &rank);
std::string getCollectionTemplate(int jobNumber, const std::string &requirements,const std::string &rank);
std::string getIntParametricJobTemplate (std::vector<std::string> attributes , int parameters , int start , int step , const std::string &requirements,const std::string &rank);
std::string getStringParametricJobTemplate (std::vector<std::string>attributes, std::vector<std::string> parameters, const std::string &requirements,const std::string &rank);