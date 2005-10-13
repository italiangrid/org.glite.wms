/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$


#include "delegateproxy.h"
// streams
#include "sstream"
#include "iostream"
// time fncts
#include "time.h"
// exceptions
#include "utilities/excman.h"

// wmproxy-api
#include "glite/wms/wmproxyapi/wmproxy_api.h"
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"

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

const string monthStr[]  = {"Jan", "Feb", "March", "Apr", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec"};

/*
*	Default constructor
*/
DelegateProxy::DelegateProxy( ){ };
/*
*	Default destructor
*/
DelegateProxy::~DelegateProxy( ){ };
/*
* Handles the command line arguments
*/
void DelegateProxy::readOptions (int argc,char **argv){
        Job::readOptions  (argc, argv, Options::JOBDELEGATION);
        // checks if the proxy file pathname is set
	if (proxyFile) {
        	logInfo->print (WMS_DEBUG, "Proxy File:", proxyFile);
 	} else {
                throw WmsClientException(__FILE__,__LINE__,
                                "readOptions",DEFAULT_ERR_CODE,
                                "Invalid Credential",
                                "No valid proxy file pathname" );
        }
};
/*
* Performs the main operations
*/
void DelegateProxy::delegation ( ){
	postOptionchecks();
	ostringstream out ;
	string proxy = "" ;
	if (!cfgCxt){ cfgCxt = new ConfigContext("","","");}
        // Delegation ID String
        dgOpt = wmcUtils->getDelegationId ();
	if ( ! dgOpt  ){
		throw WmsClientException(__FILE__,__LINE__,
				"delegation",DEFAULT_ERR_CODE,
				"Missing Information", "no proxy delegation ID" );
	}
        logInfo->print (WMS_DEBUG, "Delegation Identifier string: " , *dgOpt);
        // DELEGATION (return the EnPoint URL where the proxy has been delegated)
        endPoint = new string (wmcUtils->delegateProxy (cfgCxt, *dgOpt) );
	// output message
        // OUTPUT MESSAGE ============================================
	out << "\n" << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " Success") ) << "\n\n";
	out << "Your proxy has been successfully delegated to the WMProxy:\n" ;
	out << *endPoint << "\n\n";
	out << "with the delegation identifier: " << *dgOpt << "\n";
	out << infoToFile( ) ;
	out << "\n" << wmcUtils->getStripe(74, "=") << "\n\n";
	out << getLogFileMsg ( ) << "\n";
        // ==============================================================
        // prints the message on the standard output
	cout << out.str() ;
};


std::string DelegateProxy::infoToFile( ){
	string rm = "";
	if (outOpt){
		char ws = (char)32;
		time_t now = time(NULL);
		struct tm *ns = localtime(&now);
		// date
		ostringstream date;
		date << Utils::twoDigits(ns->tm_mday) << ws << monthStr[ns->tm_mon]  << ws <<  (ns->tm_year+1900) << "," << ws;
		date << Utils::twoDigits(ns->tm_hour) << ":" << Utils::twoDigits(ns->tm_min) << ":" << Utils::twoDigits(ns->tm_sec) << ws;
		string msg = "glite-wms-delegate proxy (" + date.str() + ")\n";
		msg += "=========================================================================\n";
		msg += "WMProxy: " + *endPoint + "\ndelegation ID: " + *dgOpt + "\n";
		if( wmcUtils->toFile(*outOpt, msg, true) < 0 ){
			logInfo->print (WMS_WARNING, "unable to write the delegation operation result " , Utils::getAbsolutePath(*outOpt));
		} else {
			logInfo->print (WMS_DEBUG, "The DelegateProxy result has been saved in the output file ", Utils::getAbsolutePath(*outOpt));
			rm += "\nThe DelegateProxy result  has been saved in the following file:\n";
			rm += Utils::getAbsolutePath(*outOpt) + "\n";
		}
	}
        return rm;
}
}}}} // ending namespaces
