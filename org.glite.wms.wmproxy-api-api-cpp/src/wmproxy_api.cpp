//#include "soapWMProxy.h"
#include "soapWMProxyProxy.h"
#include "WMProxy.nsmap"
#include <stdlib.h> // getenv(...)
#include <unistd.h> // getuid()
#include <sys/types.h> // getuid()
#include <boost/lexical_cast.hpp> // int to string conversion
#include <fstream> //strsream
#include <ctype.h>
#include "wmproxy_api.h"
using namespace std;
namespace glite {
namespace wms {
namespace wmproxyapi {
/*****************************************************************
soapErrorMng - common soap failure managemente
******************************************************************/
void soapErrorMng (const WMProxy &wmp){
	soap_print_fault(wmp.soap, stderr);
	soap_destroy(wmp.soap); // delete deserialized class instances (for C++ only)
	soap_end(wmp.soap); // remove deserialized data and clean up
	soap_done(wmp.soap); // detach the gSOAP environment
}
/*****************************************************************
Look for a file existence
******************************************************************/
const char* checkFileExistence(const char* file_in){
	ifstream f_in( file_in);
	if (  !f_in.good() ) return NULL;
	else return file_in;
}
/*****************************************************************
Calculate proxy configuration file
******************************************************************/
const char* getTrustedCert(ConfigContext *cfs){
	if (cfs!=NULL) if (cfs->trusted_cert_dir!="") return checkFileExistence(cfs->trusted_cert_dir.c_str());
	else return NULL ;
}
const char* getProxyFile(ConfigContext *cfs){
	if (cfs!=NULL) if (cfs->proxy_file!="") return checkFileExistence(cfs->proxy_file.c_str());
	else{
		const char * env_proxy = getenv ("X509_USER_PROXY");
		if(env_proxy!=NULL)return checkFileExistence(env_proxy);
		else{
			string result ="/tmp/x509up_u" + boost::lexical_cast<std::string>(getuid()) ;
			return checkFileExistence( result.c_str() );
		}
	}
}
/*****************************************************************
Perform SSL initialisation
Update configuration properties
******************************************************************/
void soapAuthentication(WMProxy &wmp,ConfigContext *cfs){
	// change, if needed service endpoint
	if (cfs!=NULL)if( cfs->endpoint!="")wmp.endpoint=cfs->endpoint.c_str() ;
	// Perform ssl authentication with server
	if (soap_ssl_client_context(wmp.soap,
		SOAP_SSL_NO_AUTHENTICATION,
		getProxyFile(cfs),// keyfile: required only when client must authenticate to server
		NULL, // password to read the key file
		NULL, // optional cacert file to store trusted certificates (needed to verify server)
		getTrustedCert(cfs), 
		// if randfile!=NULL: use a file with random data to seed randomness
		NULL
	))soapErrorMng(wmp);
}
/*****************************************************************
jobidSoap2cpp
Tranform the soap jobid structure into cpp primitive object structure
******************************************************************/
JobIdStruct* jobidSoap2cpp (ns1__JobIdStructType *s_id){
	JobIdStruct *result = new JobIdStruct ;
	result->jobid=s_id->id ;
	result->nodeName=NULL ;
	if (s_id->name!=NULL) 
		result->nodeName= 
			new string(*(s_id->name));
	else result->nodeName=NULL ;
	for (unsigned int i = 0 ; i< s_id->childrenJob->size(); i++){
		result->children.push_back(jobidSoap2cpp(s_id->childrenJob->front()  ) ) ;
		cout << "jobidSoap2cpp calling front method but ..." << endl ;
	}
	return result ;
}
/*****************************************************************
listSoap2cpp
Tranform the soap string&long list structure into cpp primitive object structure
******************************************************************/
vector <pair<string , long> > listSoap2cpp (ns1__StringAndLongList *s_list){
	vector <pair<string , long> > result ;
	std::vector<ns1__StringAndLongType*> s_vect=*(s_list->file);
	for (unsigned int i = 0 ; i< s_vect.size() ; i++){
		result.push_back( pair<string , long> (s_vect[i]->name,(long)(s_vect[i]->size)));
	}
	return result ;
}
/*****************************************************************
ConfigContext Constructor
******************************************************************/
ConfigContext::ConfigContext(std::string p , std::string s, std::string t):proxy_file(p),endpoint(s),trusted_cert_dir(t){};
/*===============================================
		WMPROXY SERVICE METHODS:
===============================================*/
/*****************************************************************
getVersion
******************************************************************/
string getVersion(ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getVersionResponse response;
	if (wmp.ns1__getVersion(response) == SOAP_OK) {
		return response.version ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobRegister
******************************************************************/
JobIdStruct jobRegister (string &jdl, string &delegationId, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobRegisterResponse response;
	if (wmp.ns1__jobRegister(jdl, delegationId, response) == SOAP_OK) {
		return *jobidSoap2cpp ( response._jobIdStruct ) ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobSubmit
******************************************************************/
JobIdStruct jobSubmit(string &jdl, string &delegationId, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobSubmitResponse response;
	if (wmp.ns1__jobSubmit(jdl, delegationId, response) == SOAP_OK) {
		return *jobidSoap2cpp ( response._jobIdStruct ) ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobStart
******************************************************************/
void jobStart(string &jobid, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobStartResponse response;
	if (wmp.ns1__jobStart(jobid, response) == SOAP_OK) {
		// Ok
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobCancel
******************************************************************/
void jobCancel(string &jobid, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobCancelResponse response;
	if (wmp.ns1__jobCancel(jobid, response) == SOAP_OK) {
		// Ok
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getMaxInputSandboxSize
******************************************************************/
int getMaxInputSandboxSize(ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getMaxInputSandboxSizeResponse response;
	if (wmp.ns1__getMaxInputSandboxSize(response) == SOAP_OK) {
		return response.size;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getSandboxDestURI
******************************************************************/
string getSandboxDestURI(string &jobid, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getSandboxDestURIResponse response;
	if (wmp.ns1__getSandboxDestURI(jobid, response) == SOAP_OK) {
		return response._path;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getTotalQuota
******************************************************************/
pair<long, long> getTotalQuota(ConfigContext *cfs){
	WMProxy wmp;
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
pair<long, long>getFreeQuota(ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
        pair<long, long> quota;
	ns1__getFreeQuotaResponse response;
	if (wmp.ns1__getFreeQuota(response) == SOAP_OK) {
		quota = make_pair( (long)response.hardLimit, (long)response.softLimit );
		return quota;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobPurge
******************************************************************/
void jobPurge(string &jobid, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobPurgeResponse response;
	if (wmp.ns1__jobPurge(jobid, response) == SOAP_OK) {
		// Ok
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getOutputFileList
******************************************************************/
vector <pair<string , long> > getOutputFileList (string &jobid, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getOutputFileListResponse response;
	if (wmp.ns1__getOutputFileList(jobid, response) == SOAP_OK) {
		return listSoap2cpp (response._OutputFileAndSizeList);
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
jobListMatch
******************************************************************/
vector <pair<string , long> > jobListMatch (string &jdl, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobListMatchResponse response;
	if (wmp.ns1__jobListMatch(jdl, response) == SOAP_OK) {
		return listSoap2cpp (response._CEIdAndRankList);
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getJobTemplate
******************************************************************/
ns1__JobTypeList *createJobTypeList(int jobType) {
	ns1__JobTypeList *result  = new ns1__JobTypeList;
	return result ;
}
string  getJobTemplate (int jobType, string &executable,string &arguments,string &requirements,string &rank, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getJobTemplateResponse response;
	if (wmp.ns1__getJobTemplate(createJobTypeList(jobType), executable, arguments, requirements, rank, response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getDAGTemplate
******************************************************************/
/*
string getDAGTemplate(NodeStruct dependencies, string &requirements,string &rank, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getDAGTemplateResponse response;
	if (wmp.ns1__getDAGTemplate(response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
}
*/
/*****************************************************************
getCollectionTemplate
******************************************************************/
string getCollectionTemplate(int jobNumber, string &requirements,string &rank, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getCollectionTemplateResponse response;
	if (wmp.ns1__getCollectionTemplate(jobNumber, requirements, rank, response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getIntParametricJobTemplate
******************************************************************/
ns1__StringList *createStringList ( vector<string> &attributes){
	ns1__StringList *result =new ns1__StringList;
	return result ;
}
string getIntParametricJobTemplate (vector<string> attributes , int parameters , int start , int step , string &requirements, string &rank, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getIntParametricJobTemplateResponse response;
	if (wmp.ns1__getIntParametricJobTemplate(createStringList(attributes), parameters, start, step, requirements, rank, response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getStringParametricJobTemplate
******************************************************************/
string getStringParametricJobTemplate (vector<string>attributes, vector<string> parameters, string &requirements,string &rank, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getStringParametricJobTemplateResponse response;
	if (wmp.ns1__getStringParametricJobTemplate(createStringList(attributes), createStringList(parameters), requirements, rank, response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getProxyRequest
******************************************************************
std::string getProxyRequest(std::string &request, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getProxyRequestResponse response;
	if (wmp.ns1__getProxyRequest(request, response) == SOAP_OK) {
		return response.type;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
putProxy
*****************************************************************
void putProxy(std::string &delegationId, std::string &proxy, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__putProxyResponse response;
	if (wmp.ns1__putProxy(delegationId, proxy, response) == SOAP_OK) {
		//OK
	} else soapErrorMng(wmp) ;
}

} // wmproxy-api namespace
} // wms namespace
} // glite namespace
