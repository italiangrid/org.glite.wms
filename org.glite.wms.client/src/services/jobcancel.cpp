/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#include "jobcancel.h"
#include "lbapi.h"
#include <string>
// streams
#include<sstream>
// wmproxy-api
#include "glite/wms/wmproxyapi/wmproxy_api.h"
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"
// JDL
#include "glite/wms/jdl/Ad.h"
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
/**
* Default constructor
*/
JobCancel::JobCancel() {
	// init of the string attributes
        inOpt=NULL  ;
};

/**
* Default destructor
*/
JobCancel::~JobCancel( ) {
	if (inOpt){ delete(inOpt);}
};

/**
*	Reads the command-line user arguments and sets all the class attributes
*/
void JobCancel::readOptions (int argc,char **argv){
	unsigned int njobs = 0;
	Job::readOptions  (argc, argv, Options::JOBCANCEL);
        // input file
        inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	// JobId's
        if (inOpt){
		// From input file
		logInfo->print (WMS_DEBUG, "Reading JobId(s) from the input file:", Utils::getAbsolutePath(*inOpt));
		jobIds = wmcUtils->getItemsFromFile(*inOpt);
		jobIds = wmcUtils->checkJobIds (jobIds);
		logInfo->print (WMS_DEBUG, "JobId(s) in the input file:", Utils::getList (jobIds), false);
        } else {
		// from command line
        	jobIds = wmcOpts->getJobIds();
		jobIds = wmcUtils->checkJobIds (jobIds);
        }
	njobs = jobIds.size( ) ;
	if (njobs > 1 && ! wmcOpts->getBoolAttribute(Options::NOINT) ){
		logInfo->print (WMS_DEBUG, "Multiple JobIds found:", "asking for choosing one or more id(s) in the list ", false);
        	jobIds = wmcUtils->askMenu(jobIds,Utils::MENU_JOBID);
		if (jobIds.size() != njobs) {
			logInfo->print (WMS_DEBUG, "Chosen JobId(s):", Utils::getList (jobIds), false);
		}
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
	// checks if the output file already exists
	if (outOpt && ! wmcUtils->askForFileOverwriting(*outOpt) ){
		cout << "bye\n";
		getLogFileMsg ( );
		Utils::ending(ECONNABORTED);
	}
};

/**
* Perfoms the main operations
*/
void JobCancel::cancel ( ){
	postOptionchecks();
	vector<string>::iterator it ;
        string* cancelled = NULL;
	ostringstream out ;
	string err = "";
	string warns = "";
	// flag for the "connecting"-info  message
	bool info = true;
	// checks that the jobids vector is not empty
        if (jobIds.empty()){
		throw WmsClientException(__FILE__,__LINE__,
			"cancel", DEFAULT_ERR_CODE,
			"JobId Error",
                        "No valid JobId for which the cancellation can be requested" );
        } else{
        	ostringstream info;
                for (it = jobIds.begin(); it != jobIds.end(); it++) { info << " - " << *it << "\n"; }
		logInfo->print (WMS_DEBUG,"Request of cancellation for the following job(s):\n" + info.str() , "" );
        }
        // checks that the config-context is not null
        if (!cfgCxt){
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE,
			"Null Pointer Error",
                        "null pointer to ConfigContext object"   );
        }
	// ask users to be sure to want the job cancelling
         if ( wmcUtils->answerYes("\nAre you sure you want to remove specified job(s)", 1, 1) ){
		LbApi lbApi;
                for (it = jobIds.begin() ; it != jobIds.end() ; it++){
			// JobId
			string jobid = *it;
			logInfo->print(WMS_DEBUG, "Checking the status of the job:",  jobid );
			lbApi.setJobId(jobid);
			try{
				// Retrieves the job status
				Status status =   lbApi.getStatus(true) ;
				// Checks if the status code allows the cancellation
				status.checkCodes(Status::OP_CANCEL, warns );
				if (warns.size()>0){
				// JobId
					glite::wmsutils::jobid::JobId jobid= status.getJobId();
					logInfo->print(WMS_WARNING, jobid.toString() + ": " + warns, "trying to cancel the job...", true);
				}
				// EndPoint URL
				cfgCxt->endpoint= status.getEndpoint();
			} catch (WmsClientException &exc){
				// cancellation not allowed due to its current status
				// If the request contains only one jobid, an exception is thrown
				if (jobIds.size( ) == 1){ throw exc ;}
				// if the request is for multiple jobs, a failed-string is built for the final message

				logInfo->print(WMS_WARNING, "Not allowed to cancel the job:\n" + (*it) , exc.what( ) , true);
				// goes on with the following job
				continue ;
			}

  			try{
                        	logInfo->print(WMS_DEBUG, "Request of cancelling for the job: ", *it );
				// If --debug has not been requested, this message has to be printed only once
				if ( info ){
                                	logInfo->print(WMS_INFO, "Connecting to the service", cfgCxt->endpoint);
				}
				if ( ! wmcOpts->getBoolAttribute(Options::DBG)){
					info = false ;
				}
	                        //  performs cancelling
                                jobCancel(*it, cfgCxt );
                                logInfo->print(WMS_DEBUG, "Cancelling result:", "The request has been successfully sent");
                         	 // list of jobs successfully cancelled
                                if (cancelled){
                                        *cancelled += "- " + string (*it) + ("\n");
                                } else {
                                        cancelled = new string("- " + string(*it) + "\n");
                                }

                        } catch (BaseException &exc){
				// If the request contains only one jobid, an exception is thrown
				if (jobIds.size( ) == 1){
					throw WmsClientException(__FILE__,__LINE__,
					"cancel", ECONNABORTED,
					"Operation Failed",
					 errMsg(exc));
				}
				// if the request is for multiple jobs, a failed-string is built for the final message
				err = errMsg(exc) ;
				logInfo->print(WMS_WARNING, "Not allowed to cancel the job:\n" + (*it),  errMsg(exc), true );
                        }
                } // for

		if (cancelled == NULL){
			throw WmsClientException(__FILE__,__LINE__,
				"cancel", ECONNABORTED,
				"Operation Failed",
				"Unable to cancel any job"  );
		}
		// OUTPUT MESSAGE
		out << "\n" << wmcUtils->getStripe(88, "=" , string (wmcOpts->getApplicationName() + " Success") ) << "\n\n";
		// success message
		out << "The cancellation request has been successfully submitted for the following job(s):\n\n";
		out << *cancelled ;
		if (outOpt) {
			out << "\n" << wmcUtils->getStripe(88, "=" ) << "\n";
                	// save the result in the output file
			if ( ! wmcUtils->toFile(*outOpt, out.str( ), true) ){
                        	// print the result of saving on the std output
                        	ostringstream os;
				os << "\n" << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " Success") ) << "\n\n";
                                os << "Cancellation results for the specified job(s) are stored in the file:\n";
                                os << Utils::getAbsolutePath(*outOpt) << "\n";
				os << "\n" << wmcUtils->getStripe(74, "=" ) << "\n\n";
                                cout << os.str( );
			} else{
                        	// couldn't save the results (prints the result message on the stdout)
                        	logInfo->print (WMS_WARNING,
                                	"unable to write the to cancellation results in the output file " ,
                                	Utils::getAbsolutePath(*outOpt));
				out << "\n" << wmcUtils->getStripe(88, "=" ) << "\n\n";
                                cout << out.str( );
   			}
    		} else {
			out << "\n" << wmcUtils->getStripe(88, "=" ) << "\n\n";
			//prints the result message on the stdout
                	cout << out.str( );
                }

	} else {
        	// The user's reply to the question ("are you sure ...?) was "no"
        	cout << "bye\n";
		cout << getLogFileMsg ( ) << "\n";
		Utils::ending(1);
        }
	cout << getLogFileMsg ( ) << "\n";
}


}}}} // ending namespaces
