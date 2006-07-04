
/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#include "job.h"
// streams
#include <sstream>
#include <iostream>
// CURL
#include <curl/curl.h>
// fstat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
// exceptions
#include "utilities/excman.h"
// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/options_utils.h"
// wmproxy-api
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"
// Ad attributes and JDL methods
#include "glite/jdl/jdl_attributes.h"
#include "glite/jdl/JDLAttributes.h"
#include "glite/jdl/extractfiles.h"
#include "glite/jdl/adconverter.h"
#include <boost/lexical_cast.hpp>


using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::jdl;

namespace api = glite::wms::wmproxyapi;
namespace apiutils = glite::wms::wmproxyapiutils;

namespace glite {
namespace wms{
namespace client {
namespace services {


/* Static method contains
* Determine whether an object is already
* contained in the passed vector.
* Eventually fills the vector with the element
* i.e. calling contains twice will return at least one true
*/

bool contains(const std::string& element, std::vector<std::string> &vect){
	vector<string>::iterator it = vect.begin() ;
	vector<string>::iterator const end1 = vect.end();
	for ( ; it != end1; it++){
		if (element==*it){
			return true;
		}
	}
	vect.push_back(element);
	return false;
}

/*
*	Default constructor
*/
Job::Job(){
	// init of the string attributes
	logOpt = NULL ;
	outOpt = NULL ;
	dgOpt = NULL ;
	// init of the boolean attributes
	nointOpt   = false;
	dbgOpt     = false ;
	autodgOpt  = false;
	// parameters
	endPoint = NULL ;
	proxyFile   = NULL ;
	trustedCerts =NULL;
	// utilities objects
	wmcOpts  = NULL ;
	wmcUtils = NULL ;
	logInfo  = NULL;
        cfgCxt = NULL;
        endPoint = NULL ;
	// WMProxy version
	wmpVersion.major = 0;
	wmpVersion.minor = 0;
	wmpVersion.subminor = 0;
	// Service Discovery
	sdContacted=false;
}
/*
*	Default destructor
*/
Job::~Job(){
	// "free memory" for the string attribute
        if (logOpt) { delete(logOpt); }
    	if (outOpt){ delete(outOpt); }
	if (dgOpt){ delete(dgOpt); }
        // "free memory" for the parameters
        if (endPoint) { delete(endPoint);}
 	// "free memory" for the utilities objects
        if (logInfo) { delete(logInfo);}
        if (cfgCxt) { delete(cfgCxt);}
	// "free memory" for certificate objects
        if (proxyFile) { delete(proxyFile);}
        if (trustedCerts) { delete(trustedCerts);}
	if (wmcOpts) { delete(wmcOpts);}
        if (wmcUtils) { delete(wmcUtils);}

}

/*
* Handles the command line options common to all services
*/
void Job::readOptions (int argc,char **argv, Options::WMPCommands command){
	// init of option objects object
	wmcOpts = new Options(command) ;
	wmcOpts->readOptions(argc, (const char**)argv);
	// --help (ends the execution)
	if (wmcOpts->getBoolAttribute(Options::HELP)){
  		wmcOpts->printUsage ((wmcOpts->getApplicationName( )).c_str());
	}
	logInfo = new Log (NULL,  (LogLevel)wmcOpts->getVerbosityLevel( ));
	// utilities
	wmcUtils    = new Utils (wmcOpts);
	// LOG file and verbisty level on the stdoutput
	logOpt      =  wmcUtils->getLogFileName( ) ;
	if (logOpt){ logInfo->createLogFile(*logOpt);}
	// input & resource (no together)
	outOpt      = wmcOpts->getStringAttribute( Options::OUTPUT ) ;
	nointOpt    = wmcOpts->getBoolAttribute (Options::NOINT) ;
	// sets the path location of the ProxyFile and TrustedCertificates attributes
	setProxyPath ( );
	setCertsPath ( );
	// --version (ends the execution)
	if (wmcOpts->getBoolAttribute(Options::VERSION)){
		// prints out the message with the client version
		printClientVersion( );
		// retrieves the server version and print it out
		printServerVersion();
		Utils::ending(0);
	}

}
/**
* After option parsing, some common check can be performed.
* So far: proxy time left
*/
void Job::postOptionchecks(unsigned int proxyMinTime){

	int proxyTimeLeft = apiutils::getProxyTimeLeft(getProxyPath());
	if (proxyTimeLeft<=0){
		throw WmsClientException(__FILE__,__LINE__,
			"postOptionchecks",DEFAULT_ERR_CODE,
			"Proxy validity Error", "Certificate is expired");
	} else if (proxyTimeLeft<proxyMinTime){
		throw WmsClientException(__FILE__,__LINE__,
			"postOptionchecks",DEFAULT_ERR_CODE,
			"Proxy validity Error", "Certificate will expire in less than"
			+ boost::lexical_cast<std::string>(proxyMinTime) + "minutes");
	}

}
/**
* Returns the WMProxy Context containing the following information:
*	- the endpoint URL ;
* 	- the user proxy pathname
*	- the pathanme to the CAs dir
*/
glite::wms::wmproxyapi::ConfigContext* Job::getContext( ){
	string endpoint = "";
	// Check Proxy time validity
	if (cfgCxt==NULL){
		cfgCxt = new api::ConfigContext(getProxyPath(), getEndPoint(), getCertsPath());
	}
	return cfgCxt;
}
/**
* Returns the endpoint URL
*/
const std::string Job::getEndPoint (){
	if (endPoint==NULL){
		// Initialize endPoint value
		retrieveEndPointURL( );
	}
	return string(*endPoint);
}
/**
* Sets the UserProxy pathname
*/
void Job::setProxyPath( ){
	char* proxy   = (char*)apiutils::getProxyFile(cfgCxt);
	if (proxy==NULL) {
		throw WmsClientException(__FILE__,__LINE__,
			"Job::readOptions",DEFAULT_ERR_CODE,
			"Proxy File Not Found", "No path to valid proxy file has been found");
	}
	proxyFile = new string(proxy);
}
/**
* Sets the  pathname to CAs directory
*/
void Job::setCertsPath( ){
	// trusted Certs
	char* certs = (char*)apiutils::getTrustedCert(cfgCxt);
	if (certs==NULL) {
		throw WmsClientException(__FILE__,__LINE__,
			"Job::readOptions",DEFAULT_ERR_CODE,
			"Directory Not Found", "No path to valid trusted certificates directory has been found");
	}
	trustedCerts = new string(certs);
}
/**
* Returns the UserProxy pathname
*/
const char* Job::getProxyPath( ) {
	if (proxyFile == NULL){ setProxyPath( );}
	return proxyFile->c_str();
}
/**
* Returns the  pathname to CAs directory
*/
const char* Job::getCertsPath( ) {
	if (trustedCerts == NULL){ setCertsPath( );}
	return trustedCerts->c_str();
}
/*
* handles the exception
*/
void Job::excMsg(const std::string& header, glite::wmsutils::exception::Exception &exc, const std::string &exename){
        if (logInfo){
		logInfo->print(WMS_ERROR, header, exc,true);
                string* logfile= logInfo->getPathName ( );
                if (logfile){
                	cerr << "\nPossible Errors and Debug messages have been printed in the following file:\n";
                	cerr << *logfile << "\n\n";
                }
        } else{
        	errMsg(WMS_ERROR, header, exc, true);
        }
        if (exc.getCode() == EINVAL && exename.size()>0){
        	wmcOpts->printUsage (exename.c_str());
         }
}

/**
* Gets the information on the log file
*/
std::string Job::getLogFileMsg ( ) {
	string msg = "";
	string *log = wmcUtils->getLogFileName( ) ;
	if (log){
                msg += string ("\t\t*** Log file created ***\n");
		msg += string("Possible Errors and Debug messages have been printed in the following file:\n");
                msg += *log + string("\n");
	}
	return msg ;
}

/**
* Sets the delegationId string
*/
void Job::setDelegationId ( ){
	// Reads the user options
	string *id = wmcOpts->getStringAttribute(Options::DELEGATION);
	bool autodg = wmcOpts->getBoolAttribute(Options::AUTODG);
	if (id && autodg){
		ostringstream err;
		err << "the following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::DELEGATION) << "\n";
		err << wmcOpts->getAttributeUsage(Options::AUTODG) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"getDelegationId",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}  else if (id) {
		// delegation-id string by user option
		logInfo->print  (WMS_DEBUG, "Delegation ID:", *id);
		dgOpt = new string (*id);
		autodgOpt = false;
	} else if (autodg){
		// automatic generation of the delegationId string
		id = Utils::getUniqueString();
		if (id==NULL){
			throw WmsClientException(__FILE__,__LINE__,
				"getDelegationId",DEFAULT_ERR_CODE,
				"Unexpected Severe Error",
				"Unknown problem occurred during the auto-generation of the delegationId string");
		}
		logInfo->print  (WMS_DEBUG, "Auto-Generation of the Delegation Identifier:", *id);
		dgOpt = new string (*id);
		autodgOpt = true;
	} else {
		ostringstream err ;
		err << "a mandatory attribute is missing:\n" ;
		err << wmcOpts->getAttributeUsage(Options::DELEGATION) ;
		err << "\nto use a proxy previously delegated or\n";
		err << wmcOpts->getAttributeUsage(Options::AUTODG) ;
		err << "\nto perform automatic delegation";
		throw WmsClientException(__FILE__,__LINE__,
				"getDelegationId", DEFAULT_ERR_CODE ,
				"Missing Information", err.str());
	}
}
/**
* Returns the delegationId string
*/
const std::string Job::getDelegationId  ( ) {
	string *id = NULL ;
	if (dgOpt) {
		id = new string(*dgOpt);
	} else {
		setDelegationId( );
		id = new string(*dgOpt);
	}
        return (*id);
}

/**
* Retrieves the WMProxy version numbers
*/
const std::string Job::getWmpVersion (const std::string &endpoint) {
	string version = "";
	try {
		// Cfg context object
		api::ConfigContext *cfg = new api::ConfigContext (getProxyPath(), endpoint, getCertsPath());
		logInfo->print (WMS_INFO, "Connecting to the service", endpoint);
		// Version string number
		logInfo->service(WMP_VERSION_SERVICE);
		version = api::getVersion(cfg);
		logInfo->result(WMP_VERSION_SERVICE, "Version numbers successfully retrieved");
	} catch (api::BaseException &exc){
		throw WmsClientException(__FILE__,__LINE__,
			"getWmpVersion", ECONNABORTED,
			"Operation failed",
			errMsg(exc) );
	}
	return version ;
}
/**
* Converts the string containing the WMProxy version numbers
*/
void Job::setVersionNumbers(const string& version) {
	unsigned int p = 0;
	ostringstream info;
	string v = version;
	 // Major version number
	p = version.find(".");
	if (p != string::npos) {
		wmpVersion.major = atoi (v.substr(0,p).c_str() );
		 // Minor version number
		if (p < version.size( )) {
			v =v.substr((p+1),(version.size()-p));
			p = v.find(".");
			if (p != string::npos){
				wmpVersion.minor = atoi (v.substr(0,p).c_str() );
			} else {
				logInfo->print(WMS_WARNING, "error on extracting minor version number", 
					"setting the number to 0",false);
				wmpVersion.minor = 0;
			}
		} else {
			wmpVersion.minor = 0;
		}
	} else {
		wmpVersion.major = 1;
		wmpVersion.minor = 0;
		logInfo->print(WMS_WARNING, "malformed version neumbers",
                                    "setting the version to 1.0.0",false);

	}
	info << "WMProxy: major version[" << wmpVersion.major << "] - minor version[" << wmpVersion.minor << "]";
	logInfo->print(WMS_DEBUG, info.str(), "",false );

}

/**
* Checks if the getProtocolsTransfer service is
* available on the WMProxy server
* (according to the version)
*/
bool Job::checkVersionForTransferProtocols( ){
	bool check = false;
	// Version = MAJOR.MINOR.SUBMINOR
	// CHECK if    major > WMPROXY_GETPROTOCOLS_VERSION
	if ( wmpVersion.major > Options::WMPROXY_GETPROTOCOLS_VERSION ) {
		check =true;
	} else
	// CHECK if    	major = WMPROXY_GETPROTOCOLS_VERSION &&
	//			minor > WMPROXY_GETPROTOCOLS_MINOR_VERSION
	if ( wmpVersion.major == Options::WMPROXY_GETPROTOCOLS_VERSION ) {
		if (wmpVersion.minor > Options::WMPROXY_GETPROTOCOLS_MINOR_VERSION) {
			check = true;
		} else
		// CHECK if    	major = WMPROXY_GETPROTOCOLS_VERSION &&
		//			minor = WMPROXY_GETPROTOCOLS_MINOR_VERSION
		//			subminor >= WMPROXY_GETPROTOCOLS_SUBMINOR_VERSION
		if (wmpVersion.minor == Options::WMPROXY_GETPROTOCOLS_MINOR_VERSION &&
		wmpVersion.subminor >= Options::WMPROXY_GETPROTOCOLS_SUBMINOR_VERSION){
			check = true;
		} else {
			check = false;
		}
	} else {
		check = false;
	}
	return check;
}
/**
* Contacts the endpoint to retrieve the version
*/
void Job::printServerVersion( ) {
	ostringstream srv;
	string *endpoint = NULL;
	string version = "";
 	endpoint =  wmcOpts->getStringAttribute (Options::ENDPOINT) ;
        if (endpoint) {
		logInfo->print(WMS_DEBUG, "EndPoint URL from --" +  wmcOpts->getAttributeUsage(Options::ENDPOINT) +" option:", *endpoint);
		// Gets the server version
		urls.push_back (*endpoint);
        } else {
        	char* ep = getenv("GLITE_WMS_WMPROXY_ENDPOINT");
		if (ep){
  			endpoint = new string (ep);
			//urls.push_back(*endpoint);
			logInfo->print(WMS_DEBUG, "EndPoint URL from GLITE_WMS_WMPROXY_ENDPOINT environment variable:", *endpoint );
			urls.push_back (*endpoint);
                } else {
			// list of endpoints from the configuration file
			logInfo->print(WMS_DEBUG, "Getting Endpoint URL from configuration file", "" );
			urls = wmcUtils->getWmps ( );
			endpoint = new string("");
		}
        }
	lookForWmpEndpoints (*endpoint, version, true);
}
/**
* Prints the UI version
*/
void Job::printClientVersion( ){
	ostringstream clt ;
	ostringstream srv ;
	string endpoint = "";
	string version = "";
	cout << "\n" << Options::getVersionMessage( ) << "\n";
}
/**
* Performs credential delegation
*/
void Job::delegateUserProxy(const std::string &endpoint) {
	string proxy = "";
	string id = getDelegationId( );
	try{
		api::ConfigContext *cfg = new api::ConfigContext (getProxyPath(), endpoint, getCertsPath());
		// Proxy Request
		logInfo->print(WMS_DEBUG, "Sending Proxy Request to",  endpoint);
		// GetProxy
		logInfo->service(WMP_NS2_GETPROXY_SERVICE);
		proxy = api::grstGetProxyReq(id, cfg) ;
		logInfo->result(WMP_NS2_GETPROXY_SERVICE, "The proxy has been successfully retrieved");
		// PutProxy
		logInfo->service(WMP_NS2_PUTPROXY_SERVICE);
		api::grstPutProxy(*dgOpt, proxy, cfg);
		logInfo->result(WMP_NS2_PUTPROXY_SERVICE, string("The proxy has been successfully delegated with the identifier: " + *dgOpt) );
	} catch (api::BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
			"delegateProxy", ECONNABORTED,
			"Operation failed",
			"Unable to delegate the credential to the endpoint: " + endpoint + "\n" + errMsg(exc) );
       }
       logInfo->print  (WMS_DEBUG, "The proxy has been successfully delegated with the identifier:",  *dgOpt);
}
/*
* Retrieves the endpoint URL and performs
* the user  credential delegation
*/

const std::string Job::delegateProxy( ) {
	string endpoint = "";
	retrieveEndPointURL(true);
	endpoint = getEndPoint( );
	delegateUserProxy(endpoint);
	return endpoint;
}
/**
* Sets the endpoint URL where the operation will be performed. The URL is established checking
* the following objects in this order:
*	> the --enpoint user option;
*	> the GLITE_WMS_WMPROXY_ENDPOINT environment variable
*	> the list of URLs specified in the configuration file (In this last case the choice is randomically performed)
*/
void Job::retrieveEndPointURL (const bool &delegation) {
	string *endpoint = NULL;
	string version = "";
	// Reads the credential delegation options:
	if (delegation) {
		setDelegationId( );
	} else {
		dgOpt = wmcOpts->getStringAttribute (Options::DELEGATION) ;
		autodgOpt = false;
	}
	// endPoint URL and ConfigurationContext
 	endpoint =  wmcOpts->getStringAttribute (Options::ENDPOINT);
	char* ep = getenv("GLITE_WMS_WMPROXY_ENDPOINT");
        if (endpoint){
		// --endpoint Specified
		logInfo->print(WMS_DEBUG, "EndPoint URL from user option:", *endpoint);
		// Gets the server version
		doneUrls.push_back(*endpoint);
		version = getWmpVersion (*endpoint);
		logInfo->print(WMS_DEBUG, "WMProxy Version: " + version, "" );
		// Performs CredentialDelegation if auto-delegation has been requested
		if (autodgOpt){delegateUserProxy(*endpoint);}
        } else  if (ep){
		// GLITE_WMS_WMPROXY_ENDPOINT Variable Used
		logInfo->print(WMS_DEBUG, "EndPoint URL from GLITE_WMS_WMPROXY_ENDPOINT environment variable:", *endpoint );
		endpoint = new string (ep);
		// Gets the server version
		doneUrls.push_back(*endpoint);
		version = getWmpVersion (*endpoint);
		logInfo->print(WMS_DEBUG, "WMProxy Version: " + version, "" );
		// Performs CredentialDelegation if auto-delegation has been requested
		if (autodgOpt){ delegateUserProxy(*endpoint);}
	} else {
		// list of endpoints from the configuration file
		logInfo->print(WMS_DEBUG, "Getting Endpoint URL from configuration file", "" );
		urls = wmcUtils->getWmps ( );
		endpoint = new string("");
		lookForWmpEndpoints(*endpoint, version);
        }
	 // sets the attributes related to the version info
	 setVersionNumbers(version);
	 // Sets the attribute of this class related to the WMP server
	 endPoint = new string(*endpoint);
	 cfgCxt = new api::ConfigContext(getProxyPath(),*endpoint, getCertsPath());
}

void Job::setEndPoint(const std::string& endpoint, const bool delegation) {
	string version = "";
	endPoint = new string(endpoint);
	cfgCxt = new api::ConfigContext(getProxyPath(),endpoint, getCertsPath());
	logInfo->print(WMS_DEBUG, "Endpoint URL: " + cfgCxt->endpoint, "");
		// Gets the server version
		version = getWmpVersion (endpoint);
		logInfo->print(WMS_DEBUG, "WMProxy Version: " + version, "" );
		 // sets the attributes related to the version info
		setVersionNumbers(version);
	if (delegation){
		// checks the input options related to the credential delegation
		setDelegationId ( );
		// autodelegation if it is needed
		if (autodgOpt){ delegateUserProxy(endpoint);}
	}
}
/**
* Endpoint version
*/

void Job::lookForWmpEndpoints(std::string &endpoint, std::string &version, const bool &all){
	try{
		// Look for endpoint inside Configuration:
		checkWmpList(endpoint, version, all);
	} catch (WmsClientException &exc){
		//Loof for endpoint inside Service Discovery:
		if (sdContacted){
			// SD already contacted: throw caught exception
			throw exc;
		}else{
			// Try and contact SD
			checkWmpSDList(endpoint, version, all);
		}
	}
}


void Job::checkWmpSDList (std::string &endpoint, std::string &version, const bool &all){
	// Ask user whether to continue:
	logInfo->print(WMS_WARNING, "Unable to find any available endpoint where to connect");
	if (wmcUtils->answerYes ("Do you wish to query Service Discovery for more endpoints?", true, true)){
		// set boolean contacted value to true
		sdContacted =true;
		urls=wmcUtils->lookForWmps(*(wmcUtils->getVirtualOrganisation()));
		checkWmpList (endpoint, version, all);
	}else{
		throw WmsClientException(__FILE__,__LINE__,
		"checkWmpSDList", ECONNABORTED,
		"Operation failed",
		"Unable to find any endpoint where to connect");
	}
}
void Job::checkWmpList (std::string &endpoint, std::string &version, const bool &all) {
	int n, index  = 0;
	if (urls.empty( )){
			throw WmsClientException(__FILE__,__LINE__,
			"checkWmpList", ECONNABORTED,
			"Operation failed",
			"Unable to find any endpoint where to connect");
	}
	while ( ! urls.empty( ) ){
		n = urls.size( );
		if (n > 1){
			// randomic extraction of one URL from the list
			index = wmcUtils->getRandom(n);
		} else {
			index = 0;
		}
		endpoint = string (urls[index]);
		// Removes the extracted URL from the list
		urls.erase ( (urls.begin( ) + index) );
		// Retrieves the version of the server
		try {
			// Skip endpoints that have already been tested
			if (!contains(endpoint,doneUrls)){
				version = getWmpVersion (endpoint);
				if (all) {
					// info message printed
					logInfo->print(WMS_INFO, "WMProxy Version: "  + version, "" );
				} else {
					// Debug message printed
					logInfo->print(WMS_DEBUG, "WMProxy Version: " + version, "" );
					// Credential Delegation in case autodelegation has been requested
					if (autodgOpt) { delegateUserProxy(endpoint); }
					break;
				}
			}
		} catch (WmsClientException &exc){
			if (n==1) {
				// in case of any error on the only specified endpoint
				throw WmsClientException(__FILE__,__LINE__,
					"checkWmpList", ECONNABORTED,
					"Operation failed",
 					"Unable to connect to the service: " + endpoint + "\n" + exc.what( ));
			} else {
				logInfo->print  (WMS_INFO, "Connection failed:", exc.what());
				if (urls.empty( ) && all == false){
					throw WmsClientException(__FILE__,__LINE__,
					"checkWmpList", ECONNABORTED,
					"Operation failed", "Unable to contact any specified enpoint");
				}
			}
		}
	}
}


void Job::checkFileTransferProtocol(  ) {
	vector<string> protocols;
	ostringstream info;
	ostringstream msg;
	int size = 0;
	if (checkVersionForTransferProtocols()) {
		// ==== > getTransferProtocol service available
		logInfo->service(WMP_GETPROTOCOLS_SERVICE);
		// List of WMProxy available protocols
		try {
			protocols = api::getTransferProtocols(cfgCxt );
			size = protocols.size();
			msg << "Available protocols: ";
			for (int i = 0; i < size; i++){
				if (i > 0 && i < size){msg << ", ";}
				msg << protocols[i] ;
			}
			if (size>0) {
				logInfo->result(WMP_GETPROTOCOLS_SERVICE, msg.str() );
			} else {
				logInfo->result(WMP_GETPROTOCOLS_SERVICE,
					"unable to check the protocol (empty list received by the server)");
			}
		} catch (api::BaseException &exc){
			if (fileProto == NULL){
				fileProto= new string (Options::TRANSFER_FILES_DEF_PROTO );
				logInfo->print(WMS_DEBUG, "Server BaseException caught:" , errMsg(exc));
				logInfo->print(WMS_DEBUG, "the user has not specified any File Transfer Protocol; default is:",
				*fileProto);
			}
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", "unable to get list of protocols from the server: " + errMsg(exc) );

		}
		// --proto: checks that the chosen protocol is supported
		if (fileProto) {
			if ( size > 0 ) {
				// checks whether the WMProxy supports the protocol
				// that the user has specified (--proto)
				if (Utils::hasElement(protocols, *fileProto) == false){
					info << "--proto " << *fileProto;
					info << ": the specified FileTransferProtocol is not supported by the server.\n";
					info << msg.str();
					throw WmsClientException(__FILE__,__LINE__,
						"Job::checkFileTransferProtocol",DEFAULT_ERR_CODE,
						"Input Option Error", info.str());
				} else {
					logInfo->print(WMS_DEBUG, "--proto " + *fileProto + ":",
					"the server supports this protocol");
				}
			} else {
				// An empty list has been received:
				// unable to check whether the server supports the specified protocol (--proto)
				logInfo->print(WMS_DEBUG, "--proto - File Transfer Protocol:", *fileProto);
			}
		} else {
			// --proto input option has not been used
			if ( size > 0 ) {
				if (Utils::hasElement(protocols, Options::TRANSFER_FILES_DEF_PROTO)) {
					fileProto= new string (Options::TRANSFER_FILES_DEF_PROTO );
					logInfo->print(WMS_DEBUG, "FileTransferProtocol not specified;", "using the default protocol: "
						+ *fileProto);
				} else if (Utils::hasElement(protocols, Options::TRANSFER_FILES_CURL_PROTO)) {
					fileProto= new string (Options::TRANSFER_FILES_CURL_PROTO );
					logInfo->print(WMS_DEBUG,
					"FileTransferProtocol has not been specified and the server does not support the default protocol ("
						+Options::TRANSFER_FILES_DEF_PROTO+")",
						"using: " + *fileProto);
				} else {
					info << "The server does not support File Transfer Protocol available for this client.\n";
					info << "Server available protocols: " << msg.str( );
					throw WmsClientException(__FILE__,__LINE__,
						"readOptions",DEFAULT_ERR_CODE,
						"Input Option Error", info.str());
				}
			} else {
				// An empty list has been received:
				// unable to check whether the protocols available for this client are supported by the server
				fileProto= new string (Options::TRANSFER_FILES_DEF_PROTO );
				logInfo->print(WMS_DEBUG, "The user has not specified any File Transfer Protocol; default is:",
				*fileProto);
				logInfo->result(WMP_GETPROTOCOLS_SERVICE,
					"could not check the protocol (received list of protocols is empty)");
			}
		}
	} else {
		throw WmsClientException(__FILE__,__LINE__,
			"Job::checkFileTransferProtocol",DEFAULT_ERR_CODE,
				"Input Option Error",
				"No information on the available WMProxy-FileTransferProtocol(s)" );
	}
}

void Job::printWarnings(const std::string& title, const std::vector<std::string> &warnings){
	assert (logInfo!=NULL);
	string msg = title;
	vector<string>::const_iterator it1 = warnings.begin() ;
	vector<string>::const_iterator const end1 = warnings.end();
	for ( ; it1 != end1; it1++){
		msg+= "\n   "+*it1;
	}
	logInfo->print(WMS_WARNING,msg);
}


}}}} // ending namespaces

