/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#include "jobloginfo.h"
// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/options_utils.h"
// wmproxy-api
#include "glite/wms/wmproxyapi/wmproxy_api.h"
// streams
#include<sstream>

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;

namespace glite {
namespace wms{
namespace client {
namespace services {



/*
* Default constructor
*/
JobLogInfo::JobLogInfo( ){
	// init of the attributes
	vbOpt = NULL;
	inOpt = NULL;
};
/*
* Default destructor
*/
JobLogInfo::~JobLogInfo( ){
	if (vbOpt ){ delete(vbOpt); }
	if (inOpt ){ delete(inOpt); }
};
/**
*	Reads the command-line user arguments and sets all the class attributes
*/
void JobLogInfo::readOptions (int argc,char **argv){
	Job::readOptions  (argc, argv, Options::JOBLOGINFO);
        // input file
        inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	// JobId's
        if (inOpt){
        	jobIds = wmcUtils->getItemsFromFile(*inOpt);
        } else {
        	jobIds = wmcOpts->getJobIds();
        }
        jobIds = wmcUtils->checkJobIds (jobIds);
	if ( jobIds.size( ) > 1 && ! wmcOpts->getBoolAttribute(Options::NOINT) ){
        	jobIds = wmcUtils->askMenu(jobIds, Utils::MENU_JOBID);
         }
         // checks if the proxy file pathname is set
	if (proxyFile) {
        	logInfo->print (WMS_DEBUG, "Proxy File:", proxyFile);
 	} else {
                throw WmsClientException(__FILE__,__LINE__,
                                "readOptions",DEFAULT_ERR_CODE,
                                "Invalid Credential",
                                "No valid proxy file pathname" );
        }
	vbOpt =(unsigned int*) wmcOpts->getIntAttribute(Options::VERBOSE);
	if (!vbOpt){
		vbOpt = (unsigned int*)malloc(sizeof(int));
		*vbOpt = Options::DEFAULT_VERBOSITY ;
	} else if (*vbOpt < 0 ||  *vbOpt > Options::MAX_VERBOSITY ){
		ostringstream err;
		for (unsigned int i = 0; i != (Options::MAX_VERBOSITY+1); i++){
			err << i ;
			if (i < Options::MAX_VERBOSITY){ err << " | "; }
		}
                throw WmsClientException(__FILE__,__LINE__,
                                "readOptions",DEFAULT_ERR_CODE,
                                "Invalid Argument Value",
                                "Invalid Verbosity Level: possible values " + err.str() );
	}

};
/**
* perfroms the main operations
*/
void JobLogInfo::getLoggingInfo ( ){
	postOptionchecks();
	// checks that the jobids vector is not empty
	if (jobIds.empty()){
		throw WmsClientException(__FILE__,__LINE__,
			"getStatus", DEFAULT_ERR_CODE,
			"JobId Error",
			"No valid JobId for which the status can be retrieved" );
	}
};

void JobLogInfo::printInfo( ){

	ostringstream out;
	out << "\n" << wmcUtils->getStripe(88, "=") << "\n\n";
	out << "LOGGING INFORMATION:\n\n";
	out << "Printing info for the Job : " ;  // << jobid "\n\n"
	int nev = 0; // TBD

	// (DEFAULT_VERBOSITY + 2) TBD !!!
	for (int i = 0; i < nev; i++){
		out << "Event: " << "\n" ;
		if (*vbOpt >= (Options::DEFAULT_VERBOSITY + 1)){
			out << "- arrived                 =    " << "\n";
			out << "- host                   	=    "<< "\n";
			out << "- reason                  =	" << "\n";
		}

		if (*vbOpt >= Options::DEFAULT_VERBOSITY){
			out << "- source                  =    " << "\n";
			out << "- timestamp               =    " << "\n";
		}
		if (*vbOpt > (Options::DEFAULT_VERBOSITY + 1)){
			out << "- user                    =	"<< "\n";
		}
		if (*vbOpt >= Options::DEFAULT_VERBOSITY){
		 	out << "	---\n";
		}

	}
	out << "\n" << wmcUtils->getStripe(88, "=") << "\n\n";
}

}}}} // ending namespaces
