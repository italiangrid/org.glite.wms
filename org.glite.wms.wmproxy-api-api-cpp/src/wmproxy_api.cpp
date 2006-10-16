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
soap Timeout Support
******************************************************************/
int soap_send_timeout=0;	// No timeout set
int soap_receive_timeout=0;	// No timeout set

void setSoapSendTimeout   (int send_timeout)   {soap_send_timeout=send_timeout;      }
void setSoapReceiveTimeout(int receive_timeout){soap_receive_timeout=receive_timeout;}
void soapSetTimeouts(struct soap *soap ){
	if (soap){
		soap->send_timeout=soap_send_timeout;
		soap->recv_timeout=soap_receive_timeout;

	}
}



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

BaseException* createWmpException (BaseException *b_ex ,const std::string &method ,  const std::string &description ){
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
						case SOAP_TYPE_ns1__ServerOverloadedFaultType:
							b_ex=new ServerOverloadedException;
							break;
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
						case SOAP_TYPE__ns4__DelegationException:
							soap_print_fault(soap, stderr);
							exit(-1);
						default:
							b_ex=new BaseException ;
					}
					// method name
					b_ex->methodName = ex->methodName ;
					// Time stamp
					b_ex->Timestamp = ex->Timestamp ;
					// Error Code
					if (ex->ErrorCode){
						//b_ex->ErrorCode    =  new string(*(ex->ErrorCode)) ;
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
		soapDestroy(soap);
	}  // if (soap)

	return b_ex;
}

/**
* Creates a BaseException object for the WMProxy standard methods using GridSite
*/
BaseException* grstCreateWmpException(struct soap *soap){
	SOAP_ENV__Fault  *fault = NULL;
	SOAP_ENV__Detail *detail = NULL;

	_ns4__DelegationException *ex2 = NULL;
	BaseException *b_ex =NULL;
	char *faultstring =  NULL;
	char *faultcode = NULL;
	string message = "";
	if (soap){
		if (!*soap_faultcode(soap)){soap_set_fault(soap);}
		// soap fault string
		faultstring =  (char*)*soap_faultstring(soap);
		// soap fault code
		faultcode = (char*)*soap_faultcode(soap);
		// pointer to the fault description
		fault = soap->fault ;
		if (fault){
        		detail = soap->fault->detail ;
			if (detail) {
				 if (detail->__type == SOAP_TYPE__ns4__DelegationException) {
					ex2 = (_ns4__DelegationException*)detail->fault;
				} else { ex2 = NULL; }
				// if type is ns4__DelegationExceptionType
				if (ex2) {
					b_ex = new GrstDelegationException ;
					if (ex2->msg) {
						message = *(ex2->msg) ;
					} else if (faultstring) {
						message = string(faultstring);
					} else {
						message = "unknown reason";
					}
				} else {
					// other types
					b_ex = new BaseException;
					if (faultstring) {
						message = string(faultstring);
					} else {
						message = "unknown reason";
					}
				}
				// fault message
				b_ex->Description = new string(message);
				// method name (unfortunately unknown)
				b_ex->methodName = "" ;
				// timestamp
				b_ex->Timestamp = getTime ();
			} // detail
		} //fault
		soapDestroy(soap);
	} //soap
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

 	if (b_ex){
		throw *b_ex ;
	} else{
		throw *createWmpException (new GenericException ,
                        	"Soap Error" ,
                                "Unknown Soap fault" ) ;
	}
}

void grstSoapErrorMng (const DelegationSoapBinding &grst){
	char **fault = NULL ;
	char **details = NULL ;
        string msg = "";
        // retrieve information on the exception
        BaseException *b_ex =grstCreateWmpException (grst.soap);
	soapDestroy(grst.soap);
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
	// initialise timeout settings
	soapSetTimeouts(wmp.soap);
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
void grstSoapAuthentication(DelegationSoapBinding &grst,ConfigContext *cfs){
	grst.endpoint = (cfs->endpoint).c_str();
	const char *proxy = getProxyFile(cfs) ;
	const char *trusted = getTrustedCert(cfs) ;
	if ( proxy ){
		if (trusted){
			if (soap_ssl_client_context(grst.soap,
				SOAP_SSL_NO_AUTHENTICATION,
				proxy, /* keyfile: required only when client must authenticate to server */
				"", /* password to read the key file */
				NULL, /* optional cacert file to store trusted certificates (needed to verify server) */
				trusted, /* optional capath to direcoty with trusted certificates */
				NULL /* if randfile!=NULL: use a file with random data to seed randomness */
			))grstSoapErrorMng(grst);
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
	if (s_id->path) {
                result->jobPath = new string(*(s_id->path));
        } else {
                result->jobPath=NULL ;
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
std::vector <std::pair<std::string , long> > fileSoap2cpp (ns1__StringAndLongList *s_list){
	vector <pair<string , long> > result ;
	if (s_list){
		vector<ns1__StringAndLongType*> s_vect = (*s_list).file;
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
proxyInfoSoap2cpp
Tranform the soap ProxyInfoStructType into the correspondent cpp structure
******************************************************************/
ProxyInfoStructType* proxyInfoSoap2cpp (ns1__ProxyInfoStructType* info) {
	ProxyInfoStructType *result = NULL;
	if (info){
		result = new ProxyInfoStructType;
		// Provy Info
		result->subject	= info->Subject ;
		result->issuer = 	info->Issuer ;
		result->identity	= info-> Identity ;
		result->type = info->Type ;
		result->strength	= info->Strength ;
		result->startTime = info->StartTime ;
		result->endTime	= info->EndTime ;
		// VO's info
		vector<ns1__VOProxyInfoStructType*>::iterator it1 = (info->VOsInfo).begin();
		const vector<ns1__VOProxyInfoStructType*>::iterator end1 = (info->VOsInfo).end();
		for ( ; it1 != end1; it1++){
			VOProxyInfoStructType *vo_result = new VOProxyInfoStructType;
			if ((*it1)){
				ns1__VOProxyInfoStructType *vo_info = (*it1);
				vo_result->user = vo_info->User ;
				vo_result->userCA = vo_info->UserCA ;
				vo_result->server =  vo_info->Server ;
				vo_result->voName  = vo_info->VOName ;
				vo_result->URI = vo_info->URI ;
				vo_result->startTime =  vo_info->StartTime ;
				vo_result->endTime  = vo_info->EndTime ;
				// attribute
				vector<string>::iterator it2 = (vo_info->Attribute).begin() ;
				const vector<string>::iterator end2 = (vo_info->Attribute).end() ;
				for ( ; it2 != end2; it2++){
					(vo_result->attribute).push_back((*it2)) ;
				}
			}
			(result->vosInfo).push_back(vo_result );
		}
	}
	return result ;
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
jobRegisterJSDL
******************************************************************/
JobIdApi jobRegisterJSDL (const string &jsdl, const string &delegationId, ConfigContext *cfs){
	WMProxy wmp;
	JobIdApi jobid ;
	ns2__JobDefinition_USCOREType * _jsdl;
	soapAuthentication (wmp, cfs);
	ns1__jobRegisterJSDLResponse response;
	if (wmp.ns1__jobRegisterJSDL(_jsdl, delegationId, response) == SOAP_OK) {
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
void jobStart(const std::string &jobid, ConfigContext *cfs){
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
void jobCancel(const std::string &jobid, ConfigContext *cfs){
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
getTransferProtocols
******************************************************************/
std::vector<std::string> getTransferProtocols(glite::wms::wmproxyapi::ConfigContext *cfs){
	WMProxy wmp;
	ns1__StringList *ns1_string_list;
	vector<string> vect;
	soapAuthentication (wmp, cfs);
	ns1__getTransferProtocolsResponse response;
	if (wmp.ns1__getTransferProtocols(response) == SOAP_OK) {
		ns1_string_list = response.items;
		if (ns1_string_list) {
			unsigned int size = (*ns1_string_list).Item.size();
			for (unsigned int i = 0; i < size; i++){
				vect.push_back((*ns1_string_list).Item[i]);
			}
		}
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return vect;
}

/*****************************************************************
getSandboxDestURI
******************************************************************/
std::vector<std::string> getSandboxDestURI(const std::string &jobid, ConfigContext *cfs, const std::string &protocol ){
	WMProxy wmp;
	ns1__StringList *ns1_string_list;
	vector<string> vect;
	soapAuthentication (wmp, cfs);
	ns1__getSandboxDestURIResponse response;
	if (wmp.ns1__getSandboxDestURI(jobid, protocol, response) == SOAP_OK) {
		ns1_string_list = response._path;
		if (ns1_string_list) {
			unsigned int size = (*ns1_string_list).Item.size();
			for (unsigned int i = 0; i < size; i++) {
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
 std::vector< std::pair<std::string ,std::vector<std::string > > > getSandboxBulkDestURI(std::string jobid, ConfigContext *cfs, const std::string &protocol ){
	WMProxy wmp;
	vector< pair<string ,vector<string > > > vect;
	vector<string> uris;
	soapAuthentication (wmp, cfs);
	ns1__getSandboxBulkDestURIResponse response;
	if ( wmp.ns1__getSandboxBulkDestURI(jobid, protocol, response) == SOAP_OK) {
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
void jobPurge(const std::string &jobid, ConfigContext *cfs){
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

std::vector <std::pair<std::string , long> > getOutputFileList (const std::string &jobid, ConfigContext *cfs, const std::string &protocol ){
	WMProxy wmp;
	vector <pair<string , long> > vect ;
	soapAuthentication (wmp, cfs);
	ns1__getOutputFileListResponse response;
	if (wmp.ns1__getOutputFileList(jobid, protocol, response) == SOAP_OK) {
		vect = fileSoap2cpp (response._OutputFileAndSizeList);
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return vect ;
}

/*****************************************************************
jobListMatch
******************************************************************/
std::vector <std::pair<std::string , long> > jobListMatch (const std::string &jdl, const std::string &delegationId,ConfigContext *cfs){
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
std::vector<std::string> getPerusalFiles (const std::string &jobid, const std::string &file, const bool &allchunks, ConfigContext *cfs, const std::string &protocol ){
	WMProxy wmp;
	ns1__StringList *ns1_string_list;
	vector<string> vect;
	soapAuthentication (wmp, cfs);
	ns1__getPerusalFilesResponse response;
	if (wmp.ns1__getPerusalFiles(jobid, file, allchunks, protocol, response) == SOAP_OK) {
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

std::string  getJobTemplate (int jobType, const std::string &executable,const std::string &arguments,const std::string &requirements,const std::string &rank, ConfigContext *cfs){
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

std::string getDAGTemplate(NodeStruct dependencies, const std::string &requirements,const std::string &rank, ConfigContext *cfs){
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
std::string getCollectionTemplate(int jobNumber, const std::string &requirements,const std::string &rank, ConfigContext *cfs){
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

std::string getIntParametricJobTemplate (std::vector<std::string> attributes , int parameters , int start , int step , const std::string &requirements, const std::string &rank, ConfigContext *cfs){
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
std::string getStringParametricJobTemplate (std::vector<std::string>attributes, std::vector<std::string> parameters, const std::string &requirements,const std::string &rank, ConfigContext *cfs){
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
std::string getProxyReq(const std::string &delegationId, ConfigContext *cfs){
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


std::string grstGetProxyReq(const std::string &delegationId, ConfigContext *cfs){
	DelegationSoapBinding grst;
	string proxy = "";
	grstSoapAuthentication(grst, cfs);
	ns4__getProxyReqResponse response;
	if (grst.ns4__getProxyReq(delegationId, response) == SOAP_OK) {
		proxy = response._getProxyReqReturn;
		soapDestroy(grst.soap) ;
	} else grstSoapErrorMng(grst) ;
	return proxy;
}

ProxyReqStruct getNewProxyReq(ConfigContext *cfs){
	DelegationSoapBinding grst;
	ProxyReqStruct request;
	struct ns4__getNewProxyReqResponse response;
	grstSoapAuthentication(grst, cfs);
	if (grst.ns4__getNewProxyReq(response) == SOAP_OK) {
		if (response.ns4__NewProxyReq){
			// Proxy
			if (response.ns4__NewProxyReq->proxyRequest) {
				request.proxy = *(response.ns4__NewProxyReq->proxyRequest);
 			} else {
				request.proxy =  "";
			}
			// Delegation-Id
			if (response.ns4__NewProxyReq->delegationID) {
				request.delegationId = *(response.ns4__NewProxyReq->delegationID);
			} else {
				request.delegationId =  "";
			}

		}
		soapDestroy(grst.soap) ;
	} else grstSoapErrorMng(grst) ;
	return request;
}

/*****************************************************************
getDelegationVersion
******************************************************************/
std::string getDelegationVersion(ConfigContext *cfs){
	DelegationSoapBinding grst;
	string request;
	struct ns4__getVersionResponse response;
	grstSoapAuthentication(grst, cfs);
	if (grst.ns4__getVersion(response) == SOAP_OK) {
		request = response.getVersionReturn;
		soapDestroy(grst.soap) ;
	} else grstSoapErrorMng(grst) ;
	return request;
}
/*****************************************************************
getDelegationInterfaceVersion
******************************************************************/
std::string getDelegationInterfaceVersion(ConfigContext *cfs){
	DelegationSoapBinding grst;
	string request;
	struct ns4__getInterfaceVersionResponse response;
	grstSoapAuthentication(grst, cfs);
	if (grst.ns4__getInterfaceVersion(response) == SOAP_OK) {
		request = response.getInterfaceVersionReturn;
		soapDestroy(grst.soap) ;
	} else grstSoapErrorMng(grst) ;
	return request;
}


int getProxyTerminationTime (const std::string &delegationId, ConfigContext *cfs){
	DelegationSoapBinding grst;
	ns4__getTerminationTimeResponse response;
	time_t tt = 0;
	grstSoapAuthentication(grst, cfs);
	if (grst.ns4__getTerminationTime(delegationId, response) == SOAP_OK) {
		tt = response._getTerminationTimeReturn;
		soapDestroy(grst.soap) ;
	}  else grstSoapErrorMng(grst) ;
	return tt;
}

void proxyDestroy (const std::string &delegationId, ConfigContext *cfs){
	DelegationSoapBinding grst;
	ns4__destroyResponse response;
	grstSoapAuthentication(grst, cfs);
	if (grst.ns4__destroy(delegationId, response) == SOAP_OK) {
		soapDestroy(grst.soap) ;
	}  else grstSoapErrorMng(grst) ;
}

std::string renewProxyReq (const std::string &delegationId, ConfigContext *cfs){
	DelegationSoapBinding grst;
	ns4__renewProxyReqResponse response;
	string id = "";
	grstSoapAuthentication(grst, cfs);
	if (grst.ns4__renewProxyReq(delegationId, response) == SOAP_OK) {
		id = response._renewProxyReqReturn;
		soapDestroy(grst.soap) ;
	}  else grstSoapErrorMng(grst) ;
	return id;
}
/*****************************************************************
putProxy
*****************************************************************/
void putProxy(const std::string &delegationId, const std::string &request, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	time_t timeleft ;
	char *certtxt=NULL;
	// gets the path to the user proxy file
	const char *proxy = getProxyFile(cfs);
	//soapDestroy(wmp.soap) ;
	// proxy time  left
	if (!proxy){
		throw *createWmpException (new GenericException , "getProxyFile" , "unable to get a valid proxy" ) ;
	}
	timeleft = getCertTimeLeft(proxy);
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

void grstPutProxy(const std::string &delegationId, const std::string &request, ConfigContext *cfs){
	DelegationSoapBinding grst;
	grstSoapAuthentication (grst, cfs);
	time_t timeleft ;
	char *certtxt;
	// gets the path to the user proxy file
	const char *proxy = getProxyFile(cfs);
	//soapDestroy(wmp.soap) ;
	// proxy time  left
	if (!proxy){
		throw *createWmpException (new GenericException , "getProxyFile" , "unable to get a valid proxy" ) ;
	}
	timeleft = getCertTimeLeft(proxy);
	// makes certificate
	if (GRSTx509MakeProxyCert(&certtxt, stderr, (char*)request.c_str(), (char*) proxy, (char*)proxy,
		timeleft) ){
		throw *createWmpException (new GenericException , "GRSTx509MakeProxyCert" , "Method failed" ) ;
	}
	grstSoapAuthentication (grst, cfs);
	ns4__putProxyResponse response;
	if (grst.ns4__putProxy(delegationId, certtxt, response) == SOAP_OK) {
		soapDestroy(grst.soap) ;
	} else grstSoapErrorMng(grst) ;
}

ProxyInfoStructType* getDelegatedProxyInfo(const std::string &delegationId, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ProxyInfoStructType *result = NULL;
	ns1__getDelegatedProxyInfoResponse response;
	if (wmp.ns1__getDelegatedProxyInfo(delegationId, response) == SOAP_OK) {
		result = proxyInfoSoap2cpp(response._items);
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return (result);
}

ProxyInfoStructType*getJobProxyInfo(const std::string &jobId, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	ProxyInfoStructType *result = NULL;
	ns1__getJobProxyInfoResponse response;
	if (wmp.ns1__getJobProxyInfo(jobId, response) == SOAP_OK) {
		result = proxyInfoSoap2cpp(response._items);
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return (result);
}
std::string getJDL(const std::string &jobid, const JdlType &type, ConfigContext *cfs){
	WMProxy wmp;
	soapAuthentication (wmp, cfs);
	string jdl = "";
	ns1__getJDLResponse response;
	if (wmp.ns1__getJDL(jobid, (ns1__JdlType)type, response) == SOAP_OK) {
		soapDestroy(wmp.soap) ;
	} else soapErrorMng(wmp) ;
	return (response._jdl);
}


} // wmproxy-api namespace
} // wms namespace
} // glite namespace
