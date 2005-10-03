

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
std::string Job::readOptions (int argc,char **argv, Options::WMPCommands command){
	// init of option objects object
	wmcOpts = new Options(command) ;
	string opts = wmcOpts->readOptions(argc, (const char**)argv);
	// utilities
	wmcUtils    = new Utils (wmcOpts);
	// LOG file and verbisty level on the stdoutput
	logOpt      =  wmcUtils->getLogFileName( ) ;
	logInfo     = new Log (logOpt,  (LogLevel)wmcOpts->getVerbosityLevel( ));
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
        // returns the info string on the options
        return opts;
}
/** After option parseing, some common check can be performed.
So far: proxy time left*/
void Job::postOptionchecks(unsigned int proxyMinTime){
	// Check Proxy time validity
	if (proxyFile!=NULL){
		unsigned int proxyTimeLeft =getProxyTimeLeft(proxyFile);
		if (proxyTimeLeft<0){
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
void Job::getEndPointVersion(std::string &endpoint, std::string &version, const bool &info) {
	string *opt = NULL;
	int n, index  = 0;
	bool success = false;
	vector<string> urls;
	if (!cfgCxt){ cfgCxt = new ConfigContext("", "", "");}
	// The "endpoint" input parameter is not empty

	// Checks if endpoint already contains the WMProxy URL
	// (it could be set inside or outside this method)
/*	if (endPoint){
		cfgCxt->endpoint = string(*endPoint);
		// if the autodelegation is needed

		if (autodgOpt) {
			// Delegation ID
			dgOpt = wmcUtils->getDelegationId ();
			if ( ! dgOpt  ){
				throw WmsClientException(__FILE__,__LINE__,
						"readOptions",DEFAULT_ERR_CODE,
						"Missing Information",
						"" );
			}
			endpoint = wmcUtils->delegateProxy (cfgCxt, *dgOpt);

			if (info) {
				logInfo->print (WMS_INFO, "Getting the version from the service", cfgCxt->endpoint, true);
			} else {
				logInfo->print (WMS_DEBUG, "Getting the version from the service", cfgCxt->endpoint);
			}
			version = getVersion((ConfigContext *)cfgCxt);

		try {
			if (info) {
				logInfo->print (WMS_INFO, "Getting the version from the service", cfgCxt->endpoint, true);
			} else {
				logInfo->print (WMS_DEBUG, "Getting the version from the service", cfgCxt->endpoint);
			}
			version = getVersion((ConfigContext *)cfgCxt);
			endpoint = string (cfgCxt->endpoint);
		} catch (BaseException &exc) {
			throw WmsClientException(__FILE__,__LINE__,
					"getVersion", ECONNABORTED,
					"Operation failed", errMsg(exc) );
		}

	} else {*/	if (endpoint.size() > 0){
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
					if (info) {
						logInfo->print (WMS_INFO, "Getting the version from the service", cfgCxt->endpoint, true);
					} else {
						logInfo->print (WMS_DEBUG, "Getting the version from the service", cfgCxt->endpoint);
					}
					version = getVersion((ConfigContext *)cfgCxt);
					endpoint = string (cfgCxt->endpoint);
					success = true;
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
						if (urls.empty( )){
							throw WmsClientException(__FILE__,__LINE__,
							"getEndPoint", ECONNABORTED,
							"Operation failed", "Unable to contact any specified enpoints");
						}
					}
				}
				if (success){break;}
			}
		//}
	//}
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
	//srv << "WMProxy:\t" << endpoint << "\n";
	srv << "Version: " << version << "\n";
	cout << srv.str() << "\n";
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
