
#include "delegateproxy.h"
// streams
#include <sstream>
#include <iostream>
// exceptions
#include "utilities/excman.h"
// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/options_utils.h"


// wmproxy-api
#include "glite/wms/wmproxyapi/wmproxy_api.h"
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"



using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;

namespace glite {
namespace wms{
namespace client {
namespace services {


/*
*	default constructor
*/
DelegateProxy::DelegateProxy( ){
	// init of the string attributes
         dgOpt = NULL;
	 logOpt = NULL ;
	 outOpt = NULL ;
	 cfgOpt = NULL ;
         voOpt = NULL ;
	// init of the boolean attributes
        autodgOpt = false;
        dbgOpt = false ;
	// parameters
        wmpEndPoint = NULL ;
	// utilities objects
	wmcOpts = NULL ;
	wmcUtils = NULL ;
};

void DelegateProxy::readOptions (int argc,char **argv){
	ostringstream info ;
        vector<string> wrongids;
        vector<string> wmps;
	// init of option objects object
        wmcOpts = new Options(Options::JOBDELEGATION) ;
	wmcOpts->readOptions(argc, (const char**)argv);
        // utilities
        wmcUtils = new Utils (wmcOpts);
        // config & vo(no together)
        cfgOpt = wmcOpts->getStringAttribute( Options::CONFIG ) ;
        voOpt = wmcOpts->getStringAttribute( Options::VO ) ;
	if (voOpt && cfgOpt){
		info << "the following options cannot be specified together:\n" ;
		info << wmcOpts->getAttributeUsage(Options::VO) << "\n";
		info << wmcOpts->getAttributeUsage(Options::CONFIG) << "\n\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", info.str());
	}

        dbgOpt = wmcOpts->getBoolAttribute (Options::DBG);
        logOpt = wmcOpts->getStringAttribute(Options::LOGFILE );
	outOpt =  wmcOpts->getStringAttribute( Options::OUTPUT ) ;	// get
        wmps = wmcUtils->getWmps( ) ;
        while ( ! wmps.empty( ) ) {
 		wmpEndPoint = (string*)wmcUtils->getWmpURL(wmps);
                if ( wmpEndPoint  ){
        		cfgCxt = new ConfigContext("", *wmpEndPoint, "");
			try {
                        	if ( dbgOpt || logOpt ){
					info << "trying to contact EndPoint : " << *wmpEndPoint << "\n";
                                        if (dbgOpt)  cout << info;
                                }
   				getVersion(cfgCxt);
                                // if no exception is thrown (available wmproxy; exit from the loop)
                                break;
  			} catch (BaseException &bex) {
				wmpEndPoint = NULL ;
                        }
    		}
	}
	if ( wmps.empty( ) && ! wmpEndPoint ){
                throw WmsClientException(__FILE__,__LINE__,
                        "getWmpURL", DEFAULT_ERR_CODE,
                        "Missing infomration", "no WMProxy URL specified" );
        }

        // Delegation ID
        dgOpt = wmcOpts->getStringAttribute(Options::DELEGATION);
        autodgOpt = wmcOpts->getBoolAttribute(Options::AUTODG);
	if ( ! dgOpt && ! autodgOpt ){
		info << "a mandatory attribute is missing:\n" ;
		info << wmcOpts->getAttributeUsage(Options::DELEGATION) << "\n";
                info << "\tor\n";
                info << wmcOpts->getAttributeUsage(Options::AUTODG) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Missing Information", info.str());
	} else if ( dgOpt && autodgOpt ){
		info << "the following options cannot be specified together:\n" ;
		info << wmcOpts->getAttributeUsage(Options::DELEGATION) << "\n";
                info << wmcOpts->getAttributeUsage(Options::AUTODG) << "\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", info.str());
	} else if (autodgOpt){
        	// Automatic Delegation
        	dgOpt = wmcUtils->getUniqueString( );
                if (!dgOpt ){
                	throw WmsClientException(__FILE__,__LINE__,
				"readOptions", DEFAULT_ERR_CODE,
				"Missing Information", "error during the automatic generation of the delegation string"  );
       		}
        }

};

void DelegateProxy::printUsageMsg (const char* exename ){
	 if (wmcOpts){
		wmcOpts->printUsage (exename);
  	}
}
void DelegateProxy::delegation ( ){
	ostringstream msg ;
	string proxy = "" ;
        char* endpoint = NULL;
	if ( !dgOpt){
		throw WmsClientException(__FILE__,__LINE__,
			"delegation",  DEFAULT_ERR_CODE,
			"Null Pointer Error", "null pointer to DelegationID String"   );
        }
	if (!cfgCxt){
		throw WmsClientException(__FILE__,__LINE__,
			"delegation",  DEFAULT_ERR_CODE,
			"Null Pointer Error", "null pointer to ConfigContext object"   );
        }
        // gets Proxy
        proxy = getProxyReq(*dgOpt, cfgCxt) ;
        // sends the proxy to the endpoint service
        putProxy(*dgOpt, proxy, cfgCxt);
	// output message
	msg << "*************************************************************\n";
	msg << "DELEGATION OPERATION :\n\n\n";
	msg << "your proxy has been successfully stored" ;
	endpoint = (char*)getEndPoint(cfgCxt) ;
        if (endpoint){
		msg << "in the endpoint:\n\n" << endpoint;
         }
	msg << "\t" << endpoint << "\n\n";
	msg << "with the delegation identifier :\t" << *dgOpt << "\n\n";
	msg << "*************************************************************\n";
        // prints the message on the standard output
	cout << msg.str() ;

        if (outOpt){
		msg << "\nproxy :\n\n" << proxy << "\n";
                Utils::toFile (*outOpt, msg.str());
        }

};
}}}} // ending namespaces
