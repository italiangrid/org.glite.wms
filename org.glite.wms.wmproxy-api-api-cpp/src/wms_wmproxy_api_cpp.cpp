#include "soapWorkloadManagerProxyProxy.h"
#include "WorkloadManagerProxy.nsmap"
#include "api.h"
using namespace std;
/*********************************************************
soapErrorMng - common soap failure managemente
**********************************************************/
void soapErrorMng (const WorkloadManagerProxy &wmp ){
	cout << "Operation Failed" << endl;
	soap_print_fault(wmp.soap, stderr);
	cout << "----------------" << endl;
	return "Failure";
}
/*********************************************************
getVersion
**********************************************************/
string getVersion(){
	WorkloadManagerProxy wmp;
	ns1__getVersionResponse response;
	if (wmp.ns1__getVersion(response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
jobRegister
**********************************************************/
jobidStruct jobRegister (const string &jdl, const string &delegationId){
	WorkloadManagerProxy wmp;
	ns1__jobRegisterResponse response;
	if (wmp.ns1__jobRegister(jdl, response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
jobSubmit
**********************************************************/
jobidStruct jobSubmit(const string &jdl, const string &delegationId){
	WorkloadManagerProxy wmp;
	ns1__jobSubmitResponse response;
	if (wmp.ns1__jobSubmit(jdl, delegation, response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
jobStart
**********************************************************/
void jobStart(const string &jobid){
	WorkloadManagerProxy wmp;
	ns1__jobStartResponse response;
	if (wmp.ns1__jobStart(jobid, response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
jobCancel
**********************************************************/
void jobCancel(const string &jobid){
	WorkloadManagerProxy wmp;
	ns1__jobCancelResponse response;
	if (wmp.ns1__jobCancel(jobid, response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
getMaxInputSandboxSize
**********************************************************/
int getMaxInputSandboxSize(){
	WorkloadManagerProxy wmp;
	ns1__getMaxInputSandboxSizeResponse response;
	if (wmp.ns1__getMaxInputSandboxSize(response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
getSandboxDestURI
**********************************************************/
string getSandboxDestURI(const string &jobid){
	WorkloadManagerProxy wmp;
	ns1__getSandboxDestURIResponse response;
	if (wmp.ns1__getSandboxDestURI(jobid, response) == SOAP_OK) {
		return response._path;
	} else soapErrorMng( wmp) ;
}
/*********************************************************
getTotalQuota
**********************************************************/
pair<long, long> getTotalQuota(){
	WorkloadManagerProxy wmp;
	pair<long, long> quota;
	ns1__getTotalQuotaResponse response;
	if (wmp.ns1__getTotalQuota(response) == SOAP_OK) {
		quota = make_pair( (long)response.hardLimit, (long)response.softLimit );
		return quota;
	} else soapErrorMng( wmp) ;
}
/*********************************************************
getFreeQuota
**********************************************************/
pair<long, long>getFreeQuota(){
	WorkloadManagerProxy wmp;
	ns1__getFreeQuotaResponse response;
	if (wmp.ns1__getFreeQuota(response) == SOAP_OK) {
		quota = make_pair( (long)response.hardLimit, (long)response.softLimit );
		return quota;
	} else soapErrorMng( wmp) ;
}
/*********************************************************
jobPurge
**********************************************************/
void jobPurge(const string &jobid){
	WorkloadManagerProxy wmp;
	ns1__jobPurgeResponse response;
	if (wmp.ns1__jobPurge(jobid, response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
getOutputFileList
**********************************************************/
vector <pair<string , long>> getOutputFileList (const string &jobid){
	WorkloadManagerProxy wmp;
	ns1__getOutputFileListResponse response;
	if (wmp.ns1__getOutputFileList(jobid, response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
jobListMatch
**********************************************************/
vector<string> jobListMatch (const string &jdl){

	WorkloadManagerProxy wmp;
	ns1__jobListMatchResponse response;
	if (wmp.ns1__jobListMatch(jdl, response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
getJobTemplate
**********************************************************/
string  getJobTemplate (vector<string > jobType, const string &executable,const string &arguments,const string &requirements,const string &rank){
	WorkloadManagerProxy wmp;
	ns1__getJobTemplateResponse response;
	if (wmp.ns1__getJobTemplate(jobType, executable, arguments, requirements, rank, response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
getDAGTemplate
**********************************************************/
string getDAGTemplate(dependencyStruct dependencies, const string &requirements,const string &rank){
	WorkloadManagerProxy wmp;
	ns1__getDAGTemplateResponse response;
	if (wmp.ns1__getDAGTemplate(response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;

}
/*********************************************************
getCollectionTemplate
**********************************************************/
string getCollectionTemplate(int jobNumber, const string &requirements,const string &rank){
	WorkloadManagerProxy wmp;
	ns1__getCollectionTemplateResponse response;
	if (wmp.ns1__getCollectionTemplate(jobNumber, requirements, rank, response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
getIntParametricJobTemplate
**********************************************************/
string getIntParametricJobTemplate (vector<string> attributes , int parameters , int start , int step , const string &requirements,const string &rank){
	WorkloadManagerProxy wmp;
	ns1__getIntParametricJobTemplateResponse response;
	if (wmp.ns1__getIntParametricJobTemplate(attributes, parameters, start, step, requirements, rank, response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}
/*********************************************************
getStringParametricJobTemplate
**********************************************************/
string getStringParametricJobTemplate (vector<string>attributes, vector<string> parameters, const string &requirements,const string &rank){
	WorkloadManagerProxy wmp;
	ns1__getStringParametricJobTemplateResponse response;
	if (wmp.ns1__getStringParametricJobTemplate(attributes, parameters, requirements, rank, response) == SOAP_OK) {

	} else soapErrorMng( wmp) ;
}












