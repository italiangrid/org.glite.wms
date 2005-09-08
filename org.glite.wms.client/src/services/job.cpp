

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
	// init of the boolean attributes
	nointOpt   = false;
	dbgOpt     = false ;
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
	// endPoint URL and ConfigurationContext
 	endPoint =  wmcOpts->getStringAttribute (Options::ENDPOINT) ;
        if (endPoint){
		cfgCxt = new ConfigContext("",*endPoint,"");
        } else {
        	char* ep = getenv("GLITE_WMS_WMPROXY_ENDPOINT");
		if (ep){
        		cfgCxt = new ConfigContext("",string(ep),"");
			endPoint = new string(ep);
                } else{
        		cfgCxt = new ConfigContext("","","");
                        endPoint = NULL;
                }
         }
	// user proxy
	proxyFile   = (char*)getProxyFile(cfgCxt);
	// trusted Certs
	trustedCert = (char*)getTrustedCert(cfgCxt);
        // returns the info string on the options
        return opts;
}
/** After option parseing, some common check can be performed.
So far: proxy time left*/
void Job::postOptionchecks(unsigned int proxyMinTime){
	// Check Proxy time validity
	if (proxyFile!=NULL){
		int proxyTimeLeft =getProxyTimeLeft(proxyFile);
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

/*
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

}}}} // ending namespaces
