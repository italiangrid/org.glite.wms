#include "soapWMProxyProxy.h"
#include "WMProxy.nsmap"
#include <stdlib.h> // getenv(...)
#include <unistd.h> // getuid()
#include <sys/types.h> // getuid()
#include <fstream> //strsream
#include <ctype.h>
#include "glite/wms/wmproxyapi/wmproxy_api.h"
#include "gridsite.h" // GRSTx509MakeProxyCert method
#include <sstream> // int to string conversion

using namespace std;
namespace glite {
namespace wms {
namespace wmproxyapi {

BaseException* createWmpException (BaseException *b_ex ,const string &method , const string &descrption ){
	b_ex->methodName = method ;
	b_ex->Description   = description;
	return b_ex ;
}

BaseException* createWmpException(SOAP_ENV__Detail *detail){
	BaseException *b_ex =NULL;
	ns1__BaseFaultType *ex = (ns1__BaseFaultType*)detail->fault;
	if (ex != NULL) {
		switch (detail->__type){
			case SOAP_TYPE_ns1__OperationNotAllowedFaultType:
				b_ex=new OperationNotAllowedException;
				break;
			case SOAP_TYPE_ns1__JobUnknownFaultType:
				b_ex=new JobUnknownException;
				break;
			case SOAP_TYPE_ns1__NoSuitableResourcesFaultType:
				b_ex=new NoSuitableResourcesException;
				break;
			case SOAP_TYPE_ns1__GetQuotaManagementFaultType:
				b_ex=new GetQuotaManagementException;
				break;
			case SOAP_TYPE_ns1__InvalidArgumentFaultType:
				b_ex=new InvalidArgumentException;
				break;
			case SOAP_TYPE_ns1__AuthenticationFaultType:
				b_ex=new AuthenticationException;
				break;
			case SOAP_TYPE_ns1__GenericFaultType:
				b_ex=new GenericException;
				break;
			default:
				b_ex=new BaseException ;
		}
		b_ex->methodName = ex->methodName ;
		b_ex->Timestamp    = ex->Timestamp    ;
		b_ex->ErrorCode    = ex->ErrorCode    ;
		b_ex->Description   = ex->Description  ;
	}
	return b_ex;
}


/*****************************************************************
soapErrorMng - common soap failure management
******************************************************************/
void soapErrorMng (const WMProxy &wmp){
	soap_print_fault(wmp.soap, stderr);
	BaseException *b_ex =createWmpException (wmp.soap->fault->detail);
	soap_destroy(wmp.soap); // delete deserialized class instances (for C++ only)
	soap_end(wmp.soap); // remove deserialized data and clean up
	soap_done(wmp.soap); // detach the gSOAP environment
	throw *b_ex ;
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
			// Append UID to X509 default file
			stringstream uid_string;
			uid_string << getuid() ;
			string result ="/tmp/x509up_u" + uid_string.str();
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
		"", // password to read the key file
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
		cout << "Warning!! jobidSoap2cpp calling front method but ..." << endl ;
		result->children.push_back(jobidSoap2cpp(s_id->childrenJob->front()  ) ) ;
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

Tranform the soap string&long list structure into cpp primitive object structure
******************************************************************/
ns1__GraphStructType* node2soap(NodeStruct *c_node){
	ns1__GraphStructType *s_node = new ns1__GraphStructType();
	s_node->name=c_node->nodeName;
	if(c_node->childrenNodes.size()>0) s_node->childrenJob=new vector<ns1__GraphStructType*>;
	for(unsigned int i=0; i<c_node->childrenNodes.size();i++){
		s_node->childrenJob->push_back(node2soap(c_node->childrenNodes[i]));
	}
	return s_node;
}
/*****************************************************************
ConfigContext Constructor/Destructor
******************************************************************/
ConfigContext::ConfigContext(std::string p , std::string s, std::string t):proxy_file(p),endpoint(s),trusted_cert_dir(t){};
ConfigContext::~ConfigContext() throw(){};
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
JobIdStruct jobRegister (const string &jdl, const string &delegationId, ConfigContext *cfs){
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
JobIdStruct jobSubmit(const string &jdl, const string &delegationId, ConfigContext *cfs){
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
void jobStart(const string &jobid, ConfigContext *cfs){
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
void jobCancel(const string &jobid, ConfigContext *cfs){
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
long getMaxInputSandboxSize(ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getMaxInputSandboxSizeResponse response;
	if (wmp.ns1__getMaxInputSandboxSize(response) == SOAP_OK) {
		return  response.size;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getSandboxDestURI
******************************************************************/
string getSandboxDestURI(const string &jobid, ConfigContext *cfs){
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
void jobPurge(const string &jobid, ConfigContext *cfs){
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
vector <pair<string , long> > getOutputFileList (const string &jobid, ConfigContext *cfs){
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
vector <pair<string , long> > jobListMatch (const string &jdl, ConfigContext *cfs){
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
ns1__JobTypeList *createJobTypeList(int type) {
	ns1__JobTypeList *result  = new ns1__JobTypeList;
	result->jobType= new std::vector<enum ns1__JobType     > ;
	if ( type & JOBTYPE_PARTITIONABLE ){
		result->jobType->push_back( ns1__JobType__PARTITIONABLE );
	} if ( type & JOBTYPE_CHECKPOINTABLE ){
		result->jobType->push_back(ns1__JobType__CHECKPOINTABLE);
	} if ( type & JOBTYPE_PARAMETRIC ) {
		result->jobType->push_back(ns1__JobType__PARAMETRIC);
	} if ( type & JOBTYPE_INTERACTIVE ) {
		result->jobType->push_back(ns1__JobType__INTERACTIVE);
	} if ( type & JOBTYPE_MPICH ) {
		result->jobType->push_back(ns1__JobType__MPI);
	} if ( result->jobType->size() ==0 ){
	result->jobType->push_back(ns1__JobType__NORMAL);
	}
	return result ;
}
string  getJobTemplate (int jobType, const string &executable,const string &arguments,const string &requirements,const string &rank, ConfigContext *cfs){
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

string getDAGTemplate(NodeStruct dependencies, const string &requirements,const string &rank, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getDAGTemplateResponse response;
	if (wmp.ns1__getDAGTemplate( node2soap(&dependencies), requirements, rank, response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
}

/*****************************************************************
getCollectionTemplate
******************************************************************/
string getCollectionTemplate(int jobNumber, const string &requirements,const string &rank, ConfigContext *cfs){
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
	result->Item=new vector<string>;
	for (unsigned int i = 0; i<attributes.size();i++) result->Item->push_back(attributes[i]);
	return result ;
}
string getIntParametricJobTemplate (vector<string> attributes , int parameters , int start , int step , const string &requirements, const string &rank, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getIntParametricJobTemplateResponse response;
	if (wmp.ns1__getIntParametricJobTemplate(createStringList(attributes), parameters, start, step, requirements, rank, response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
	return "";
}
/*****************************************************************
getStringParametricJobTemplate
******************************************************************/
string getStringParametricJobTemplate (vector<string>attributes, vector<string> parameters, const string &requirements,const string &rank, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getStringParametricJobTemplateResponse response;
	if (wmp.ns1__getStringParametricJobTemplate(createStringList(attributes), createStringList(parameters), requirements, rank, response) == SOAP_OK) {
		return response.jdl ;
	} else soapErrorMng(wmp) ;
	return "";
}
/*****************************************************************
getProxyReq
******************************************************************/
string getProxyReq(const string &delegationId, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getProxyReqResponse response;
	if (wmp.ns1__getProxyReq(delegationId, response) == SOAP_OK) {
		return response.request;
	} else soapErrorMng(wmp) ;
	return "";
}
/*****************************************************************
putProxy
*****************************************************************/
void putProxy(const string &delegationId, const string &request, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	char *certtxt;
	if (GRSTx509MakeProxyCert(&certtxt, stderr, request.c_str(), getProxyFile(cfs), getProxyFile(cfs), 60))
		throw *createWmpException (new GenericException , "GRSTx509MakeProxyCert" , "Method failed" ) ;
	ns1__putProxyResponse response;
	if (wmp.ns1__putProxy(delegationId, string(*certtxt), response) == SOAP_OK) {
		//OK
	} else soapErrorMng(wmp) ;
}

} // wmproxy-api namespace
} // wms namespace
} // glite namespace
