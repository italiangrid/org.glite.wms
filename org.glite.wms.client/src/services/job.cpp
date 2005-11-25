
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
#include "glite/wms/jdl/jdl_attributes.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/extractfiles.h"
#include "glite/wms/jdl/adconverter.h"
#include <boost/lexical_cast.hpp>


using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;
using namespace glite::wms::jdl;

namespace glite {
namespace wms{
namespace client {
namespace services {

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
	wmpVersion = 0;
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
/** After option parsing, some common check can be performed.
So far: proxy time left*/
void Job::postOptionchecks(unsigned int proxyMinTime){

	int proxyTimeLeft =getProxyTimeLeft(getProxyPath());
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

glite::wms::wmproxyapi::ConfigContext* Job::getContext( ){
	string endpoint = "";
	// Check Proxy time validity
	if (cfgCxt==NULL){
		cfgCxt = new ConfigContext(getProxyPath(), getEndPoint(), getCertsPath());
	}
	return cfgCxt;
}

const std::string Job::getEndPoint (){
	string endpoint = "";
	if (endPoint==NULL){ setEndPoint( );}
	endpoint = string(*endPoint);
	return endpoint;
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



const std::string Job::getWmpVersion (std::string &endpoint) {
	string version = "";
	try {
		// Cfg context object
		ConfigContext *cfg = new ConfigContext (getProxyPath(), endpoint, getCertsPath());
		logInfo->print (WMS_DEBUG, "Getting the version from the service",endpoint);
		logInfo->print (WMS_INFO, "Connecting to the service", endpoint);
		// Version string number
		version = getVersion((ConfigContext *)cfg);
	} catch (BaseException &exc){
		throw WmsClientException(__FILE__,__LINE__,
			"getWmpVersion", ECONNABORTED,
			"Operation failed",
			errMsg(exc) );
	}
	return version ;
}

const int Job::getWmpVersion ( ) {
	return wmpVersion ;
}
/**
* Endpoint version
*/
void Job::checkWmpList (std::vector<std::string> &urls, std::string &endpoint, std::string &version, const bool &all) {
	int n, index  = 0;
	if (urls.empty( )){
		throw WmsClientException(__FILE__,__LINE__,
		"getEndPointVersion", ECONNABORTED,
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
			version = getWmpVersion (endpoint);
			if (all) {
				logInfo->print(WMS_INFO, "WMProxy Version: " + version, "" );
			} else {
				logInfo->print(WMS_DEBUG, "WMProxy Version: " + version, "" );
				// Credential Delegation in case autodelegation has been requested
				if (autodgOpt) { delegateUserProxy(endpoint); }
				break;
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
					"Operation failed", "Unable to contact any specified enpoints");
				}
			}
		}

	}
}

void Job::setEndPoint( ) {
	vector<string> urls;
	string *endpoint = NULL;
	string version = "";
	// Reads the credential delegation options
	setDelegationId( );
	// endPoint URL and ConfigurationContext
 	endpoint =  wmcOpts->getStringAttribute (Options::ENDPOINT) ;
        if (endpoint){
		logInfo->print(WMS_DEBUG, "EndPoint URL from user option:", *endpoint);
		// Gets the server version
		version = getWmpVersion (*endpoint);
		logInfo->print(WMS_DEBUG, "WMProxy Version: " + version, "" );
		wmpVersion = atoi (version.substr(0,1).c_str() );
		// Performs CredentialDelegation if auto-delegation has been requested
		if (autodgOpt){ delegateUserProxy(*endpoint);}
        } else {
        	char* ep = getenv("GLITE_WMS_WMPROXY_ENDPOINT");
		if (ep){
  			endpoint = new string (ep);
			logInfo->print(WMS_DEBUG, "EndPoint URL from GLITE_WMS_WMPROXY_ENDPOINT environment variable:", *endpoint );
			// Gets the server version
			version = getWmpVersion (*endpoint);
			logInfo->print(WMS_DEBUG, "WMProxy Version: " + version, "" );
			// Performs CredentialDelegation if auto-delegation has been requested
			if (autodgOpt){ delegateUserProxy(*endpoint);}
                } else {
			// list of endpoints from the configuration file
			logInfo->print(WMS_DEBUG, "Getting Endpoint URL from configuration file", "" );
			urls = wmcUtils->getWmps ( );
			endpoint = new string("");
			checkWmpList(urls, *endpoint, version);
		}
         }
	 // Sets the attribute of this class related to the WMP server
	 wmpVersion = atoi (version.substr(0,1).c_str() );
	 endPoint = new string(*endpoint);
	 cfgCxt = new ConfigContext(getProxyPath(),*endpoint, getCertsPath());
}

void Job::setEndPoint(const std::string& endpoint) {
	endPoint = new string(endpoint);
	cfgCxt = new ConfigContext(getProxyPath(),endpoint, getCertsPath());
	logInfo->print(WMS_DEBUG, "Endpoint URL: " + cfgCxt->endpoint, "");
}


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



/*
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



const std::string Job::delegateProxy( ) {
	string endpoint = "";
	setEndPoint( );
	endpoint = getEndPoint( );
	delegateUserProxy(endpoint);
	return endpoint;
}
/*
* Performs credential delegation
*/
void Job::delegateUserProxy(const std::string &endpoint) {
	string proxy = "";
	string id = getDelegationId( );
	try{
		ConfigContext *cfg = new ConfigContext (getProxyPath(), endpoint, getCertsPath());
		// Proxy Request
		logInfo->print(WMS_DEBUG, "Sending Proxy Request to",  endpoint);
		if  (getWmpVersion() > Options::WMPROXY_OLD_VERSION){
			proxy = grstGetProxyReq(*dgOpt, cfg) ;
			logInfo->print(WMS_DEBUG, "Delegating Credential to the service",  endpoint);
			grstPutProxy(*dgOpt, proxy, cfg);

		} else {
			proxy = getProxyReq(*dgOpt, cfg) ;
			logInfo->print(WMS_DEBUG, "Delegating Credential to the service",  endpoint);
			putProxy(*dgOpt, proxy, cfg);
		}
	} catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
			"delegateProxy", ECONNABORTED,
			"Operation failed",
			"Unable to delegate the credential to the endpoint: " + endpoint + "\n" + errMsg(exc) );
       }
       logInfo->print  (WMS_DEBUG, "The proxy has been successfully delegated with the identifier:",  *dgOpt);
}

/**
* Contacts the endpoint to retrieve the version
*/
void Job::printServerVersion( ) {
	ostringstream srv;
	string *endpoint = NULL;
	string version = "";
	vector<string> urls;
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
	checkWmpList (urls, *endpoint, version, true);
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

void Job::setProxyPath( ){
	char* proxy   = (char*)getProxyFile(cfgCxt);
	if (proxy==NULL) {
		throw WmsClientException(__FILE__,__LINE__,
			"Job::readOptions",DEFAULT_ERR_CODE,
			"Proxy File Not Found", "No path to valid proxy file has been found");
	}
	proxyFile = new string(proxy);
}
void Job::setCertsPath( ){
	// trusted Certs
	char* certs = (char*)getTrustedCert(cfgCxt);
	if (certs==NULL) {
		throw WmsClientException(__FILE__,__LINE__,
			"Job::readOptions",DEFAULT_ERR_CODE,
			"Directory Not Found", "No path to valid trusted certificates directory has been found");
	}
	trustedCerts = new string(certs);
}
const char* Job::getProxyPath( ) {
	if (proxyFile == NULL){ setProxyPath( );}
	return proxyFile->c_str();
}
const char* Job::getCertsPath( ) {
	if (trustedCerts == NULL){ setCertsPath( );}
	return trustedCerts->c_str();
}


}}}} // ending namespaces
