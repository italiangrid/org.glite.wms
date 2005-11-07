
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
	trustedCert = NULL ;
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
	logInfo = new Log (NULL,  (LogLevel)wmcOpts->getVerbosityLevel( ));
	// utilities
	wmcUtils    = new Utils (wmcOpts);
	// LOG file and verbisty level on the stdoutput
	logOpt      =  wmcUtils->getLogFileName( ) ;
	if (logOpt){ logInfo->createLogFile(*logOpt);}
	// input & resource (no together)
	outOpt      = wmcOpts->getStringAttribute( Options::OUTPUT ) ;
	nointOpt    = wmcOpts->getBoolAttribute (Options::NOINT) ;
	// Delegation
	dgOpt = wmcOpts->getStringAttribute(Options::DELEGATION);
	autodgOpt = wmcOpts->getBoolAttribute(Options::AUTODG);
	// endPoint URL and ConfigurationContext
 	endPoint =  wmcOpts->getStringAttribute (Options::ENDPOINT) ;
        if (endPoint){
		cfgCxt = new ConfigContext("",*endPoint,"");
        } else {
        	char* ep = getenv("GLITE_WMS_WMPROXY_ENDPOINT");
		if (ep){
        		cfgCxt = new ConfigContext("",string(ep),"");
			endPoint = new string(ep);
			logInfo->print(WMS_DEBUG, "GLITE_WMS_WMPROXY_ENDPOINT environment variable:", *endPoint );
                } else{
        		cfgCxt = new ConfigContext("","","");
                        endPoint = NULL;
                }
         }
	// user proxy
	proxyFile   = (char*)getProxyFile(cfgCxt);
	// trusted Certs
	trustedCert = (char*)getTrustedCert(cfgCxt);
	// --version (ends the execution)
	if (wmcOpts->getBoolAttribute(Options::VERSION)){
		// prints out the message with the client version
		printClientVersion( );
		// retrieves the server version and print it out
		printServerVersion();
		Utils::ending(0);
	}
	// --help (ends the execution)
	if (wmcOpts->getBoolAttribute(Options::HELP)){
  		wmcOpts->printUsage ((wmcOpts->getApplicationName( )).c_str());
	}
}
/** After option parseing, some common check can be performed.
So far: proxy time left*/
void Job::postOptionchecks(unsigned int proxyMinTime){
	// Check Proxy time validity
	if (proxyFile!=NULL){
		int proxyTimeLeft =getProxyTimeLeft(proxyFile);
		if (proxyTimeLeft<=0){
			throw WmsClientException(__FILE__,__LINE__,
				"postOptionchecks",DEFAULT_ERR_CODE,
				"Proxy validity Error", "Certificate is expired");
		}else if (proxyTimeLeft<proxyMinTime){
			throw WmsClientException(__FILE__,__LINE__,
				"postOptionchecks",DEFAULT_ERR_CODE,
				"Proxy validity Error", "Certificate will expire less than 20 minutes");
		}
	}
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
* Endpoint version
*/
void Job::getEndPointVersion(std::string &endpoint, std::string &version, const bool &all) {
	string *opt = NULL;
	int n, index  = 0;
	bool success = false;
	bool result = false;
	vector<string> urls;
	if (!cfgCxt){ cfgCxt = new ConfigContext("", "", "");}
			if (endpoint.size() > 0){
				urls.push_back(endpoint);
			} else if (endPoint){
				urls.push_back(*endPoint);
			} else {
				// --endpoint option
				opt = wmcOpts->getStringAttribute(Options::ENDPOINT);
				if (opt) {
					urls.push_back(*opt);
				} else {
					// list of endpoints from the configuration file
					urls = wmcUtils->getWmps ( );
				}
			}
			// initial number of Url's
			n = urls.size( );
			if (n==0){
				throw WmsClientException(__FILE__,__LINE__,
				"getEndPointVersion", ECONNABORTED,
				"Operation failed",
				"Unable to find any endpoint where to connect");
			}
			while ( ! urls.empty( ) ){
				int size = urls.size();
				if (size > 1){
					// randomic extraction of one URL from the list
					index = wmcUtils->getRandom(size);
				} else{
					index = 0;
				}
				// setting of the EndPoint ConfigContext field
				cfgCxt->endpoint = urls[index];
				// Removes the extracted URL from the list
				urls.erase ( (urls.begin( ) + index) );
				try {
					if (all) {
						cout << "\nGetting the version from the service " << cfgCxt->endpoint << "\n\n";
					} else {
						logInfo->print (WMS_DEBUG, "Getting the version from the service", cfgCxt->endpoint);
					}
					version = getVersion((ConfigContext *)cfgCxt);
					endpoint = string (cfgCxt->endpoint);

					if (all ) {
						cout << "Version " << version << "\n\n";
						result = true;
					} else {
						// exits from this loop if not all endpoints
						success = true;
					}
				}catch (BaseException &exc){
					if (n==1) {
						ostringstream err ;
						err << "Unable to connect to the service: " << cfgCxt->endpoint << "\n";
						err << errMsg(exc) ;
						// in case of any error on the only specified endpoint
						throw WmsClientException(__FILE__,__LINE__,
							"getEndPoint", ECONNABORTED,
							"Operation failed", err.str());
					} else {
						logInfo->print  (WMS_INFO, "Connection failed:", errMsg(exc));
						sleep(1);
						if (urls.empty( ) && result == false){
							throw WmsClientException(__FILE__,__LINE__,
							"getEndPoint", ECONNABORTED,
							"Operation failed", "Unable to contact any specified enpoints");
						}
					}
				}
				if (success){break;}
			}
}

std::string Job::getEndPoint( ) {
	string endpoint = "";
	string version = "";
	vector<string> urls;
	if  (wmpVersion == 0) {
		// checks if endpoint already contains the WMProxy URL
		if (endPoint){
			endpoint = string(*endPoint);
		} else if (autodgOpt && dgOpt==NULL) {
			// delegationId
			dgOpt = wmcUtils->getDelegationId( );
			// if the autodelegation is needed
			endpoint = wmcUtils->delegateProxy (cfgCxt, *dgOpt);
		}
		Job::getEndPointVersion(endpoint, version);
		logInfo->print (WMS_DEBUG, "Version:", version);
		wmpVersion = atoi (version.substr(0,1).c_str() );
	} else {
		if (endPoint) {
			endpoint = string(*endPoint);
		} else {
			throw WmsClientException(__FILE__,__LINE__,
				"getEndPoint",  DEFAULT_ERR_CODE ,
				"Null Pointer Error",
				"null pointer to endPoint object"   );
		}
	}
	return endpoint;
}
/**
* Contacts the endpoint to retrieve the version
*/
void Job::printServerVersion( ) {
	ostringstream srv;
	string endpoint = "";
	string version = "";
	if (endPoint){ endpoint = string(*endPoint);}
	getEndPointVersion (endpoint, version, true);
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
}}}} // ending namespaces
