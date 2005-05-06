
#include "jobcancel.h"
#include <string>
// streams
#include<sstream>
// wmproxy-api
#include "glite/wms/wmproxyapi/wmproxy_api.h"
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"
// exceptions
#include "utilities/excman.h"
// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/options_utils.h"

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;


namespace glite {
namespace wms{
namespace client {
namespace services {

JobCancel::JobCancel() {
	// init of the string attributes
        input = NULL  ;
        output = NULL ;
        config= NULL ;
        vo = NULL ;
        logfile= NULL ;
	// init of the boolean attributes
        version = false;
        help = false ;
        noint = false ;
	debug = false ;
        // parameters
        wmpEndPoint = NULL ;
        proxyFile = NULL ;
        trustedCert = NULL ;
	// option object
	opts = NULL ;
};

void JobCancel::readOptions (int argc,char **argv){
	ostringstream err ;
	// init of option objects object
        opts = new Options(Options::JOBCANCEL) ;
        // reads the input arguments
	opts->readOptions(argc, (const char**)argv);
	 // config & vo(no together)
        config= opts->getStringAttribute( Options::CONFIG ) ;
        vo = opts->getStringAttribute( Options::VO ) ;
	if (vo && config){
		err << "the following options cannot be specified together:\n" ;
		err << opts->getAttributeUsage(Options::VO) << "\n";
		err << opts->getAttributeUsage(Options::CONFIG) << "\n\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
        // configuration context
        cfgCxt = new ConfigContext("", wmpEndPoint, "");
	// user proxy
	proxyFile =  (char*)getProxyFile(cfgCxt);
 	// trusted Certs
	trustedCert =  (char*)getTrustedCert(cfgCxt);
};

void JobCancel::cancel ( ){
	vector<string>::iterator it ;
	// checks that the jobids vector is not empty
        if (jobIds.empty()){
		throw WmsClientException(__FILE__,__LINE__,
			"cancel", DEFAULT_ERR_CODE,
			"Missing Information",
                        "unknown jobid(s)" );
        }
        // checks that the config-context is not null
        if (!cfgCxt){
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE,
			"Null Pointer Error", "null pointer to ConfigContext object"   );
        }
	// performs cancelling
        for (it = jobIds.begin() ; it != jobIds.end() ; it++){
		jobCancel(*it, cfgCxt );
        }
};

}}}} // ending namespaces
