
#include "soapWMProxyProxy.h"
#include "WMProxy.nsmap"
#include <stdlib.h> // getenv(...)

#include "soapDelegationSoapBindingProxy.h"

#include <ctype.h>
#include "glite/wms/wmproxyapi/wmproxy_api.h"
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"
extern "C" {
#include "gridsite.h" // GRSTx509MakeProxyCert method
}
#include <sstream> // int to string conversion


using namespace std;
using namespace glite::wms::wmproxyapiutils;

namespace glite {
namespace wms {
namespace wmproxyapi {



/*****************************************************************
Performs SSL object destruction
******************************************************************/
void soapDestroy(struct soap *soap ){
	if (soap){
		// deletes deserialized class instances
		soap_destroy(soap);
		// removes deserialized data and cleans up
		soap_end(soap);
		// detaches the gSOAP environment
		soap_done(soap);
	}
}

BaseException* createWmpException (BaseException *b_ex ,const string &method ,  const string &description ){
	b_ex=new BaseException;
	b_ex->methodName = method ;
	b_ex->Description   = new string(description);
	return b_ex ;
}

/**
* Creates a BaseException object for the WMProxy standard methods
*/
BaseException* createWmpException(struct soap *soap){
	SOAP_ENV__Fault  *fault = NULL;
	SOAP_ENV__Detail *detail = NULL;
	BaseException *b_ex =NULL;
	if (soap){
		if (!*soap_faultcode(soap)){
			soap_set_fault(soap);
		}
		fault = soap->fault ;
		if (fault){
        		detail = soap->fault->detail ;
			if (detail) {
				ns1__BaseFaultType *ex = (ns1__BaseFaultType*)detail->fault;
				if ( ex ) {
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
						case SOAP_TYPE_ns1__AuthorizationFaultType:
							b_ex=new AuthorizationException;
							break;
						case SOAP_TYPE_ns1__GenericFaultType:
							b_ex=new GenericException;
							break;
						default:
							b_ex=new BaseException ;
					}
					// method name
					b_ex->methodName = ex->methodName ;
					// Time stamp
					b_ex->Timestamp = ex->Timestamp ;
					// Error Code
					if (ex->ErrorCode){
						b_ex->ErrorCode    =  new string(*(ex->ErrorCode)) ;
					} else if (fault->faultcode) {
						b_ex->ErrorCode = new string(fault->faultcode);
					} else {
						b_ex->ErrorCode = NULL;
					}
					// Description
					if ( ex->Description ) {
						b_ex->Description   = new string(*(ex->Description))  ;
					} else if (fault->faultstring){
						b_ex->Description = new string(fault->faultstring);
					} else {
						b_ex->Description = NULL;
					}

				} // if(ex)
			} // if (detail)
		} // if (fault)

		if (b_ex == NULL){
			b_ex=new BaseException ;
			char *faultstring =  (char*)*soap_faultstring(soap);
			char *faultcode = (char*)*soap_faultcode(soap);
			if (faultstring){
				b_ex->Description = new string(faultstring);
			} else{
				b_ex->Description = NULL;
			}
			if (faultcode){
				b_ex->ErrorCode = new string(faultcode);
			}else {
				b_ex->ErrorCode = NULL;
			}
			// Timestamp
			b_ex->Timestamp = getTime();
		}
		// Fault cause
		if (b_ex){
			const char **s = soap_faultdetail(soap);
			if (s && *s ){
				b_ex->FaultCause = new vector<string>;
				(b_ex->FaultCause)->push_back(string(*s));
			} else{
					b_ex->FaultCause = NULL ;
			}
		}
	}  // if (soap)
	return b_ex;
}

/**
* Creates a BaseException object for the WMProxy standard methods using GridSite
*/
BaseException* grstCreateWmpException(struct soap *soap){
	SOAP_ENV__Fault  *fault = NULL;
	SOAP_ENV__Detail *detail = NULL;
	BaseException *b_ex =NULL;
	char *faultstring =  NULL;
	char *faultcode = NULL;
	if (soap){
		if (!*soap_faultcode(soap)){ soap_set_fault(soap);}
		// soap fault string
		faultstring =  (char*)*soap_faultstring(soap);
		// soap fault code
		faultcode = (char*)*soap_faultcode(soap);
		// pointer to the fault description
		fault = soap->fault ;
		if (fault){
        		detail = soap->fault->detail ;
			if (detail) {
				ns2__DelegationException *ex = (ns2__DelegationException*)detail->fault;
				if ( ex ) {
					if (detail->__type ==SOAP_TYPE_ns2__DelegationException ) {
						b_ex=new DelegationException;
					} else {
						b_ex=new BaseException ;
					}
					// method name
					b_ex->methodName = "" ;
					// Timestamp
					b_ex->Timestamp = getTime ();
					// no Error Code
					if (faultcode){ b_ex->ErrorCode = new string(faultcode);
					}else { b_ex->ErrorCode = NULL; }
					// Description
					if ( ex->message ) {
						b_ex->Description   = new string(*(ex->message))  ;
					} else if (fault->faultstring){
						b_ex->Description = new string(fault->faultstring);
					} else {
						b_ex->Description = NULL;
					}
				} // if(ex)
			} // if (detail)
		} // if (fault)
		if (b_ex == NULL){
			b_ex=new BaseException ;

			char *faultstring =  (char*)*soap_faultstring(soap);
			char *faultcode = (char*)*soap_faultcode(soap);
			if (faultstring){
				b_ex->Description = new string(faultstring);
			} else{
				b_ex->Description = NULL;
			}
			// Timestamp
			b_ex->Timestamp = getTime ();
			// Error Code
			if (faultcode){ b_ex->ErrorCode = new string(faultcode);
			}else { b_ex->ErrorCode = NULL;}
		}
		// Fault cause
		if (b_ex){
			const char **s = soap_faultdetail(soap);
			if (s && *s ){
				b_ex->FaultCause = new vector<string>;
				(b_ex->FaultCause)->push_back(string(*s));
			} else{
					b_ex->FaultCause = NULL ;
			}
		}
	}  // if (soap)
	return b_ex;
}
/*****************************************************************
soapErrorMng - common soap failure management
******************************************************************/

void soapErrorMng (const WMProxy &wmp){
	char **fault = NULL ;
	char **details = NULL ;
        string msg = "";
        // retrieve information on the exception
        BaseException *b_ex =createWmpException (wmp.soap);
	soapDestroy(wmp.soap);
 	if (b_ex){
		throw *b_ex ;
	} else{
		throw *createWmpException (new GenericException ,
                        	"Soap Error" ,
                                "Unknown Soap fault" ) ;
	}
}

void grstSoapErrorMng (const DelegationSoapBinding &deleg){
	char **fault = NULL ;
	char **details = NULL ;
        string msg = "";
        // retrieve information on the exception
        BaseException *b_ex =grstCreateWmpException (deleg.soap);
	soapDestroy(deleg.soap);
 	if (b_ex){
		throw *b_ex ;
	} else{
		throw *createWmpException (new GenericException ,
                        	"Soap Error" ,
                                "Unknown Soap fault" ) ;
	}
}
/*****************************************************************
Performs SSL initialisation
Updates configuration properties
******************************************************************/
void soapAuthentication(WMProxy &wmp,ConfigContext *cfs){
	// change, if needed service endpoint
	if (cfs!=NULL)if( cfs->endpoint!="")wmp.endpoint=cfs->endpoint.c_str() ;
	soap_init(wmp.soap);
	const char *proxy = getProxyFile(cfs) ;
	const char *trusted = getTrustedCert(cfs) ;
	if ( proxy ){
		if (trusted){
			// Perform ssl authentication with server
			if (soap_ssl_client_context(wmp.soap,
				SOAP_SSL_NO_AUTHENTICATION,
				proxy,// keyfile: required only when client must authenticate to server
				"", // password to read the key file
				NULL, // optional cacert file to store trusted certificates (needed to verify server)
				trusted,
				// if randfile!=NULL: use a file with random data to seed randomness
				NULL
			))soapErrorMng(wmp);
		} else {
			throw *createWmpException (new ProxyFileException ,
                        	"Trusted Certificates Location  Error" ,
                                "Unable to find a valid directory with CA certificates" ) ;
		}
	} else {
		throw *createWmpException (new ProxyFileException ,
                        	"Proxy File Error" ,
                                "Unable to find a valid proxy file" ) ;
	}
}

/*****************************************************************
Performs SSL initialisation
Updates configuration properties
******************************************************************/
void grstSoapAuthentication(DelegationSoapBinding &deleg,ConfigContext *cfs){
	deleg.endpoint = (cfs->endpoint).c_str();
	const char *proxy = getProxyFile(cfs) ;
	const char *trusted = getTrustedCert(cfs) ;
	if ( proxy ){
		if (trusted){
			if (soap_ssl_client_context(deleg.soap,
				SOAP_SSL_NO_AUTHENTICATION,
				proxy, /* keyfile: required only when client must authenticate to server */
				"", /* password to read the key file */
				NULL, /* optional cacert file to store trusted certificates (needed to verify server) */
				trusted, /* optional capath to direcoty with trusted certificates */
				NULL /* if randfile!=NULL: use a file with random data to seed randomness */
			))grstSoapErrorMng(deleg);
		} else {
			throw *createWmpException (new ProxyFileException ,
                        	"Trusted Certificates Location  Error" ,
                                "Unable to find a valid directory with CA certificates" ) ;
		}
	} else {
		throw *createWmpException (new ProxyFileException ,
                        	"Proxy File Error" ,
                                "Unable to find a valid proxy file" ) ;
	}

}
/*****************************************************************
jobidSoap2cpp
Tranform the soap jobid structure into cpp primitive object structure
******************************************************************/

JobIdApi* jobidSoap2cpp (ns1__JobIdStructType *s_id){
	vector<ns1__JobIdStructType*  > children ;
	JobIdApi *result = new JobIdApi ;
	// jobid
	result->jobid=s_id->id ;
	// node name
	if (s_id->name) {
		result->nodeName= new string(*(s_id->name));
	} else {
		result->nodeName=NULL ;
	}
	// children
	children = s_id->childrenJob;
	for (unsigned int i = 0 ; i< children.size(); i++){
		result->children.push_back(jobidSoap2cpp( children[i] ) );
	}
	return result ;
}
/*****************************************************************
destURISoap2cpp
Tranform the soap destURI structure into cpp primitive object structure
******************************************************************/
std::vector< std::pair<std::string ,std::vector<std::string > > > destURISoap2cpp(ns1__DestURIsStructType* dest_uris){
	vector<string> uris;
	vector<ns1__DestURIStructType*> list ;
	ns1__DestURIStructType item_list;
	vector< pair<string ,vector<string > > > result;
	if (dest_uris) {
		list = dest_uris->Item;
		for (int i = 0; i < list.size(); i++) {
			if (!uris.empty()) { uris.clear ( ); }
			if (list[i]) {
				item_list = *list[i];
				for (int j = 0; j < item_list.Item.size(); j++) {
					uris.push_back( item_list.Item[j] );
				}
				result.push_back (make_pair((*list[i]).id, uris ) );
			}
		}
	}
	return result;
}
/*****************************************************************
fileSoap2cpp
Tranform the soap string&long list structure into cpp primitive object structure
**************************************************************/
vector <pair<string , long> > fileSoap2cpp (ns1__StringAndLongList *s_list){
	vector <pair<string , long> > result ;
	if (s_list){
		std::vector<ns1__StringAndLongType*> s_vect = (*s_list).file;
		for (unsigned int i = 0 ; i < s_vect.size() ; i++){
			if (s_vect[i]){
				result.push_back( pair<string , long> (s_vect[i]->name,(long)(s_vect[i]->size)));
			}
		}
	}
	return result ;
}
/*****************************************************************
soap2cpp
Tranform the soap string list structure into cpp primitive object structure
**************************************************************/
std::vector <std::string> listSoap2cpp (ns1__StringList *s_list){
	vector <string> empty;
	if (s_list){
		return (*s_list).Item;
	}
	return empty ;
}
/*****************************************************************
vect2soap
Tranform the cpp vector object into a soap string-list
**************************************************************/
ns1__StringList *vect2soap(const std::vector<std::string> &attributes){
	ns1__StringList *result =new ns1__StringList;
	// result.Item is std::vector<std::string>
	for (unsigned int i = 0; i<attributes.size();i++){
		(*result).Item.push_back(attributes[i]);
	}
	return result ;
}

/*****************************************************************
node2soap
Tranform the soap string&long list structure into cpp primitive object structure
******************************************************************/
ns1__GraphStructType* node2soap(NodeStruct *c_node){
	ns1__GraphStructType *s_node = new ns1__GraphStructType();
	if (c_node){
		if (c_node->nodeName){
			s_node->name = new string();
			*(s_node->name)  = *(c_node->nodeName);
		} else {
			s_node->name = NULL;
		}
		for(unsigned int i=0; i<c_node->childrenNodes.size();i++){
			s_node->childrenJob.push_back(node2soap(c_node->childrenNodes[i]));
		}
	}
	return s_node;
}
/*****************************************************************
ConfigContext Constructor/Destructor
******************************************************************/
ConfigContext::ConfigContext(std::string p , std::string s, std::string t){
	char* path = NULL;
	// proxy
        if (p.size( )== 0){
        	 path = (char*)getProxyFile(NULL) ;
       		  if ( ! path ){
			throw *createWmpException (new GenericException ,
                        	"ConfigContext" , "unable to find a valid proxy file" ) ;
		}

        } else{
		path= (char*)checkPathExistence(p.c_str());
       		  if ( ! path ){
			throw *createWmpException (new GenericException ,
                        	"ConfigContext" , "invalid proxy: (" + p + ")" ) ;
		}
        }
        proxy_file = string ( path ) ;
	// Endpoint URL
        endpoint = s;
	// trusted certificate
        if (t.size( )== 0){
        	path = (char*)getTrustedCert(NULL);
                if ( ! path ){
			throw *createWmpException (new GenericException ,
                        	"ConfigContext" ,
                                "no valid trusted certficates path (default should be /etc/grid-security/certficates)" ) ;
		}

        } else{
        	path = (char*)checkPathExistence(t.c_str());
                if ( ! path ){
			throw *createWmpException (new GenericException ,
                        	"ConfigContext" ,
                                "no valid trusted certficates path ("+ t + ")" ) ;
		}
        }
        trusted_cert_dir = string (path);
}

ConfigContext::~ConfigContext() throw(){};
/*===============================================
		WMPROXY SERVICE METHODS:
===============================================*/
/*****************************************************************
getVersion
******************************************************************/
string getVersion(ConfigContext *cfs){
	string version = "";
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__getVersionResponse response;
	if (wmp.ns1__getVersion(response) == SOAP_OK) {
		version = response.version;
		soapDestroy(wmp.soap);
	} else soapErrorMng(wmp) ;
	return version ;
}
/*****************************************************************
jobRegister
******************************************************************/
JobIdApi jobRegister (const string &jdl, const string &delegationId, ConfigContext *cfs){
	WMProxy wmp;
	JobIdApi jobid ;
	soapAuthentication (wmp, cfs);
	ns1__jobRegisterResponse response;
	if (wmp.ns1__jobRegister(jdl, delegationId, response) == SOAP_OK) {
		jobid = *jobidSoap2cpp ( response._jobIdStruct  ) ;
		soapDestroy(wmp.soap);
	} else soapErrorMng(wmp) ;
	return jobid;
}

/*****************************************************************
jobSubmit
******************************************************************/
JobIdApi jobSubmit(const string &jdl, const string &delegationId, ConfigContext *cfs){
	WMProxy wmp;
	JobIdApi jobid ;
	soapAuthentication (wmp, cfs);
	ns1__jobSubmitResponse response;
	if (wmp.ns1__jobSubmit(jdl, delegationId, response) == SOAP_OK) {
		jobid = *jobidSoap2cpp ( response._jobIdStruct  ) ;
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return jobid ;
}
/*****************************************************************
jobStart
******************************************************************/
void jobStart(const string &jobid, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobStartResponse response;
	if (wmp.ns1__jobStart(jobid, response) == SOAP_OK) {
		soapDestroy(wmp.soap) ;
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
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getMaxInputSandboxSize
******************************************************************/
long getMaxInputSandboxSize(ConfigContext *cfs){
	WMProxy wmp;
	long size = 0;
	soapAuthentication (wmp, cfs);
	ns1__getMaxInputSandboxSizeResponse response;
	if (wmp.ns1__getMaxInputSandboxSize(response) == SOAP_OK) {
		size =  response.size;
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return size ;
}
/*****************************************************************
getSandboxDestURI
******************************************************************/
vector<string> getSandboxDestURI(const string &jobid, ConfigContext *cfs){
	WMProxy wmp;
	ns1__StringList *ns1_string_list;
	vector<string> vect;
	soapAuthentication (wmp, cfs);
	ns1__getSandboxDestURIResponse response;
	if (wmp.ns1__getSandboxDestURI(jobid, response) == SOAP_OK) {
		ns1_string_list = response._path;
		if (ns1_string_list) {
			for (unsigned int i = 0; i < (*ns1_string_list).Item.size(); i++) {
				vect.push_back((*ns1_string_list).Item[i]);
			}
		}
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return vect;
}

/*****************************************************************
getSandboxBulkDestURI
******************************************************************/
 std::vector< std::pair<std::string ,std::vector<std::string > > > getSandboxBulkDestURI(std::string jobid, ConfigContext *cfs){
	WMProxy wmp;
	vector< pair<string ,vector<string > > > vect;
	vector<string> uris;
	soapAuthentication (wmp, cfs);
	ns1__getSandboxBulkDestURIResponse response;
	if ( wmp.ns1__getSandboxBulkDestURI(jobid, response) == SOAP_OK) {
		vect = destURISoap2cpp(response._DestURIsStructType);
	} else soapErrorMng(wmp) ;
	return vect;
}
/*****************************************************************
getTotalQuota
******************************************************************/
pair<long, long> getTotalQuota(ConfigContext *cfs){
	WMProxy wmp;
	pair<long, long> quota;
	soapAuthentication (wmp, cfs);
	ns1__getTotalQuotaResponse response;
	if (wmp.ns1__getTotalQuota(response) == SOAP_OK) {
		quota = make_pair( (long)response.hardLimit, (long)response.softLimit );
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return quota;
}
/*****************************************************************
getFreeQuota
******************************************************************/
pair<long, long>getFreeQuota(ConfigContext *cfs){
	WMProxy wmp;
	pair<long, long> quota;
	soapAuthentication (wmp, cfs);
	ns1__getFreeQuotaResponse response;
	if (wmp.ns1__getFreeQuota(response) == SOAP_OK) {
		quota = make_pair( (long)response.hardLimit, (long)response.softLimit );
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return quota;
}
/*****************************************************************
jobPurge
******************************************************************/
void jobPurge(const string &jobid, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ns1__jobPurgeResponse response;
	if (wmp.ns1__jobPurge(jobid, response) == SOAP_OK) {
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
}
/*****************************************************************
getOutputFileList
******************************************************************/

vector <pair<string , long> > getOutputFileList (const string &jobid, ConfigContext *cfs){
	WMProxy wmp;
	vector <pair<string , long> > vect ;
	soapAuthentication (wmp, cfs);
	ns1__getOutputFileListResponse response;
	if (wmp.ns1__getOutputFileList(jobid, response) == SOAP_OK) {
		vect = fileSoap2cpp (response._OutputFileAndSizeList);
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return vect ;
}

/*****************************************************************
jobListMatch
******************************************************************/
vector <pair<string , long> > jobListMatch (const string &jdl, const string &delegationId,ConfigContext *cfs){
	WMProxy wmp;
	vector <pair<string , long> > vect ;
	soapAuthentication (wmp, cfs);
	ns1__jobListMatchResponse response;
	if (wmp.ns1__jobListMatch(jdl, delegationId, response) == SOAP_OK) {
		 vect = fileSoap2cpp (response._CEIdAndRankList);
		 soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return vect ;
}

/*****************************************************************
enableFilePerusal
******************************************************************/
void enableFilePerusal (const std::string &jobid, const std::vector<std::string> &files, ConfigContext *cfs){
	WMProxy wmp;
	ns1__StringList *ns1_string_list;
	soapAuthentication (wmp, cfs);
	ns1__enableFilePerusalResponse response;
	ns1_string_list = vect2soap(files);
	if (wmp.ns1__enableFilePerusal(jobid, ns1_string_list, response) == SOAP_OK) {
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
}

/*****************************************************************
getPerusalFiles
******************************************************************/
std::vector<std::string> getPerusalFiles (const std::string &jobid, const std::string &file, const bool &allchunks, ConfigContext *cfs ){
	WMProxy wmp;
	ns1__StringList *ns1_string_list;
	vector<string> vect;
	soapAuthentication (wmp, cfs);
	ns1__getPerusalFilesResponse response;
	if (wmp.ns1__getPerusalFiles(jobid, file, allchunks, response) == SOAP_OK) {
		vect = listSoap2cpp (response._fileList);
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return vect;
}
/*****************************************************************
getJobTemplate
******************************************************************/
ns1__JobTypeList *createJobTypeList(int type) {
	ns1__JobTypeList * result  = new ns1__JobTypeList;
	//result->jobType ;//= new std::vector<enum ns1__JobType     > ;
	if ( type & JOBTYPE_PARTITIONABLE ){
		result->jobType.push_back( ns1__JobType__PARTITIONABLE );
	} if ( type & JOBTYPE_CHECKPOINTABLE ){
		result->jobType.push_back(ns1__JobType__CHECKPOINTABLE);
	} if ( type & JOBTYPE_PARAMETRIC ) {
		result->jobType.push_back(ns1__JobType__PARAMETRIC);
	} if ( type & JOBTYPE_INTERACTIVE ) {
		result->jobType.push_back(ns1__JobType__INTERACTIVE);
	} if ( type & JOBTYPE_MPICH ) {
		result->jobType.push_back(ns1__JobType__MPI);
	} if ( result->jobType.size() ==0 ){
		result->jobType.push_back(ns1__JobType__NORMAL);
	}
	return result ;
}
string  getJobTemplate (int jobType, const string &executable,const string &arguments,const string &requirements,const string &rank, ConfigContext *cfs){
	WMProxy wmp;
	string tpl = "";
	soapAuthentication (wmp, cfs);
	ns1__getJobTemplateResponse response;
	if (wmp.ns1__getJobTemplate(createJobTypeList(jobType), executable, arguments, requirements, rank, response) == SOAP_OK) {
		tpl = response._jdl ;
 		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return tpl ;
}
/*****************************************************************
getDAGTemplate
******************************************************************/

string getDAGTemplate(NodeStruct dependencies, const string &requirements,const string &rank, ConfigContext *cfs){
	WMProxy wmp;
	string tpl = "";
	soapAuthentication (wmp, cfs);
	ns1__getDAGTemplateResponse response;
	if (wmp.ns1__getDAGTemplate( node2soap(&dependencies), requirements, rank, response) == SOAP_OK) {
		tpl = response._jdl ;
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return tpl ;
}

/*****************************************************************
getCollectionTemplate
******************************************************************/
string getCollectionTemplate(int jobNumber, const string &requirements,const string &rank, ConfigContext *cfs){
	WMProxy wmp;
	string tpl = "";
	soapAuthentication (wmp, cfs);
	ns1__getCollectionTemplateResponse response;
	if (wmp.ns1__getCollectionTemplate(jobNumber, requirements, rank, response) == SOAP_OK) {
		tpl = response._jdl ;
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return tpl ;
}
/*****************************************************************
getIntParametricJobTemplate
******************************************************************/

string getIntParametricJobTemplate (vector<string> attributes , int parameters , int start , int step , const string &requirements, const string &rank, ConfigContext *cfs){
	WMProxy wmp;
	string tpl = "";
	soapAuthentication (wmp, cfs);
	ns1__getIntParametricJobTemplateResponse response;
	if (wmp.ns1__getIntParametricJobTemplate(vect2soap(attributes), parameters, start, step, requirements, rank, response) == SOAP_OK) {
		tpl = response._jdl ;
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return tpl ;
}
/*****************************************************************
getStringParametricJobTemplate
******************************************************************/
string getStringParametricJobTemplate (vector<string>attributes, vector<string> parameters, const string &requirements,const string &rank, ConfigContext *cfs){
	WMProxy wmp;
	string tpl = "";
	soapAuthentication (wmp, cfs);
	ns1__getStringParametricJobTemplateResponse response;
	if (wmp.ns1__getStringParametricJobTemplate(vect2soap(attributes), vect2soap(parameters), requirements, rank, response) == SOAP_OK) {
		tpl = response._jdl ;
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return tpl;
}
/*****************************************************************
getProxyReq
******************************************************************/
string getProxyReq(const string &delegationId, ConfigContext *cfs){
	WMProxy wmp;
	string proxy = "";
	soapAuthentication (wmp, cfs);
	ns1__getProxyReqResponse response;
	if (wmp.ns1__getProxyReq(delegationId, response) == SOAP_OK) {
		proxy = response._request;
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return proxy;
}

string grstGetProxyReq(const string &delegationId, ConfigContext *cfs){
	DelegationSoapBinding deleg;
	string proxy = "";
	grstSoapAuthentication(deleg, cfs);
	ns2__getProxyReqResponse response;
	if (deleg.ns2__getProxyReq(delegationId, response) == SOAP_OK) {
		proxy = response._getProxyReqReturn;
		soapDestroy(deleg.soap) ;
	} else grstSoapErrorMng(deleg) ;
	return proxy;
}


/*****************************************************************
putProxy
*****************************************************************/
void putProxy(const string &delegationId, const string &request, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	time_t timeleft ;
	char *certtxt;
	// gets the path to the user proxy file
	const char *proxy = getProxyFile(cfs);
	//soapDestroy(wmp.soap) ;
	// proxy time  left
	if (!proxy){
		throw *createWmpException (new GenericException , "getProxyFile" , "unable to get a valid proxy" ) ;
	}
	timeleft = getProxyTimeLeft(proxy);
	// makes certificate
	if (GRSTx509MakeProxyCert(&certtxt, stderr, (char*)request.c_str(), (char*) proxy, (char*)proxy,
		timeleft) ){
		throw *createWmpException (new GenericException , "GRSTx509MakeProxyCert" , "Method failed" ) ;
	}
	soapAuthentication (wmp, cfs);
	ns1__putProxyResponse response;
	if (wmp.ns1__putProxy(delegationId, certtxt, response) == SOAP_OK) {
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
}

void grstPutProxy(const string &delegationId, const string &request, ConfigContext *cfs){
	DelegationSoapBinding deleg;
	grstSoapAuthentication (deleg, cfs);
	time_t timeleft ;
	char *certtxt;
	// gets the path to the user proxy file
	const char *proxy = getProxyFile(cfs);
	//soapDestroy(wmp.soap) ;
	// proxy time  left
	if (!proxy){
		throw *createWmpException (new GenericException , "getProxyFile" , "unable to get a valid proxy" ) ;
	}
	timeleft = getProxyTimeLeft(proxy);
	// makes certificate
	if (GRSTx509MakeProxyCert(&certtxt, stderr, (char*)request.c_str(), (char*) proxy, (char*)proxy,
		timeleft) ){
		throw *createWmpException (new GenericException , "GRSTx509MakeProxyCert" , "Method failed" ) ;
	}
	grstSoapAuthentication (deleg, cfs);
	ns2__putProxyResponse response;
	if (deleg.ns2__putProxy(delegationId, certtxt, response) == SOAP_OK) {
		soapDestroy(deleg.soap) ;
	} else grstSoapErrorMng(deleg) ;
}
} // wmproxy-api namespace
} // wms namespace
} // glite namespace
