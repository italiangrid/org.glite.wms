#include "soapWorkloadManagerProxyProxy.h"
#include "WorkloadManagerProxy.nsmap"
#include "wms_wmproxy_api_cpp.h"
using namespace std;
namespace glite {
namespace wms {
namespace wmproxy-api {
/*****************************************************************
soapErrorMng - common soap failure managemente
******************************************************************/
void soapErrorMng (const WorkloadManagerProxy &wmp){
	cout << "Operation Failed" << endl;
	soap_print_fault(wmp.soap, stderr);
	return "Failure";
}

void soapAuthentication( const WorkloadManagerProxy &wmp,ConfigContext *cfs)
	string proxy_file=NULL;
	if (  cfs!=NULL )
	if (soap_ssl_client_context(wmp.soap,
		SOAP_SSL_NO_AUTHENTICATION,
		proxy_file,// keyfile: required only when client must authenticate to server
		NULL, // password to read the key file
		NULL, // optional cacert file to store trusted certificates (needed to verify server)
		"/etc/grid-security/certificates",// optional capath to direcoty with trusted certificates
		// if randfile!=NULL: use a file with random data to seed randomness
		NULL
	))soapErrorMng(wmp);
}


/*****************************************************************
jobidSoap2cpp
Tranform the soap jobid structure into cpp primitive object structure
******************************************************************/
JobIdStruct* jobidSoap2cpp (  ns1__JobIdStructType *s_id){
	JobIdStruct *result = new JobIdStruct ;
	result->jobid=s_id->id ;
	result->nodeName=NULL ;
	if (s_id->name!=NULL) result->nodeName= new string(s_id->name  );
	else result->nodeName=NULL ;
	for (unsigned int i = 0 ; i< s_id->childrenJob->size(); i++){
		result->children.push_back(  jobidSoap2cpp(   s_id->childrenJob[i]  ) ) ;
	}
	return result ;
}
/*****************************************************************
listSoap2cpp
Tranform the soap string&long list structure into cpp primitive object structure
******************************************************************/
vector <pair<string , long>> listSoap2cpp (ns1__StringAndLongList s_list){
	vector <pair<string , long>> result ;
	std::vector<ns1__StringAndLongType*>s_vect=s_list->file;
	for (unsigned int i = 0 ; i< s_vect.size() ; i++){
		result.push_back( pair<string , long> (s_vect[i]->name,(long)(s_vect[i]->size));
	}
	return result ;
}
/*****************************************************************
Simple ConfigContext Constructor
******************************************************************/
ConfigContext::ConfigContext( std::string p , std::string s):proxy_file(p),service_address(s) {};



/*===============================================
		WMPROXY SERVICE METHODS:
===============================================*/
/*****************************************************************
getVersion
******************************************************************/
string getVersion(ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getVersionResponse response;
	if (wmp.ns1__getVersion(response) == SOAP_OK) {
		return response.version ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobRegister
******************************************************************/
JobIdStruct jobRegister (const string &jdl, const string &delegationId, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobRegisterResponse response;
	if (wmp.ns1__jobRegister(jdl, response) == SOAP_OK) {
		return jobidSoap2cpp ( response._jobIdStruct ) ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobSubmit
******************************************************************/
JobIdStruct jobSubmit(const string &jdl, const string &delegationId, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobSubmitResponse response;
	if (wmp.ns1__jobSubmit(jdl, delegation, response) == SOAP_OK) {
		return jobidSoap2cpp ( response._jobIdStruct ) ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobStart
******************************************************************/
void jobStart(const string &jobid, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobStartResponse response;
	if (wmp.ns1__jobStart(jobid, response) == SOAP_OK) {
		// Ok
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobCancel
******************************************************************/
void jobCancel(const string &jobid, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobCancelResponse response;
	if (wmp.ns1__jobCancel(jobid, response) == SOAP_OK) {
		// Ok
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getMaxInputSandboxSize
******************************************************************/
int getMaxInputSandboxSize(ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getMaxInputSandboxSizeResponse response;
	if (wmp.ns1__getMaxInputSandboxSize(response) == SOAP_OK) {
		return response.size;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getSandboxDestURI
******************************************************************/
string getSandboxDestURI(const string &jobid, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getSandboxDestURIResponse response;
	if (wmp.ns1__getSandboxDestURI(jobid, response) == SOAP_OK) {
		return response._path;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getTotalQuota
******************************************************************/
pair<long, long> getTotalQuota(ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	pair<long, long> quota;
	ns1__getTotalQuotaResponse response;
	if (wmp.ns1__getTotalQuota(response) == SOAP_OK) {
		quota = make_pair( (long)response.hardLimit, (long)response.softLimit );
		return quota;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getFreeQuota
******************************************************************/
pair<long, long>getFreeQuota(ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getFreeQuotaResponse response;
	if (wmp.ns1__getFreeQuota(response) == SOAP_OK) {
		quota = make_pair( (long)response.hardLimit, (long)response.softLimit );
		return quota;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobPurge
******************************************************************/
void jobPurge(const string &jobid, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobPurgeResponse response;
	if (wmp.ns1__jobPurge(jobid, response) == SOAP_OK) {
		// Ok
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getOutputFileList
******************************************************************/
vector <pair<string , long>> getOutputFileList (const string &jobid, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getOutputFileListResponse response;
	if (wmp.ns1__getOutputFileList(jobid, response) == SOAP_OK) {
		return listSoap2cpp ( response._OutputFileAndSizeList  );
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobListMatch
******************************************************************/
vector<string> jobListMatch (const string &jdl, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobListMatchResponse response;
	if (wmp.ns1__jobListMatch(jdl, response) == SOAP_OK) {
		return listSoap2cpp ( response._CEIdAndRankList  );
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getJobTemplate
******************************************************************/
string  getJobTemplate (vector<string > jobType, const string &executable,const string &arguments,const string &requirements,const string &rank, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getJobTemplateResponse response;
	if (wmp.ns1__getJobTemplate(jobType, executable, arguments, requirements, rank, response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getDAGTemplate
******************************************************************/
string getDAGTemplate(NodeStruct dependencies, const string &requirements,const string &rank, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getDAGTemplateResponse response;
	if (wmp.ns1__getDAGTemplate(response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getCollectionTemplate
******************************************************************/
string getCollectionTemplate(int jobNumber, const string &requirements,const string &rank, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getCollectionTemplateResponse response;
	if (wmp.ns1__getCollectionTemplate(jobNumber, requirements, rank, response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getIntParametricJobTemplate
******************************************************************/
string getIntParametricJobTemplate (vector<string> attributes , int parameters , int start , int step , const string &requirements,const string &rank, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getIntParametricJobTemplateResponse response;
	if (wmp.ns1__getIntParametricJobTemplate(attributes, parameters, start, step, requirements, rank, response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getStringParametricJobTemplate
******************************************************************/
string getStringParametricJobTemplate (vector<string>attributes, vector<string> parameters, const string &requirements,const string &rank, ConfigContext cfs){
	WorkloadManagerProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getStringParametricJobTemplateResponse response;
	if (wmp.ns1__getStringParametricJobTemplate(attributes, parameters, requirements, rank, response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
}
} // wmproxy-api namespace
} // wms namespace
} // glite namespace
