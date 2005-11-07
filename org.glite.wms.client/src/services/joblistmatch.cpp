/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#include "joblistmatch.h"
// streams
#include <sstream>
#include <iostream>
// exceptions
#include "utilities/excman.h"
// wmp-client utilities
#include "utilities/utils.h"
// wmproxy API
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


JobListMatch::JobListMatch(){
	// init of the boolean attributes
        rankOpt  = false ;
	// parameters
        jdlFile = NULL ;
        jdlString = NULL ;
        //Ad
        jobAd = NULL;
  };
/*
*	Default destructor
*/
JobListMatch::~JobListMatch( ){
	if (jdlFile) { delete(jdlFile);}
        if (jdlString) { delete(jdlString);}
        if (jobAd) { delete(jobAd);}
};
/*
* performs the main operations
*/
void JobListMatch::readOptions (int argc,char **argv) {
	Job::readOptions  (argc, argv, Options::JOBMATCH);
 	// path to the JDL file
  	jdlFile = wmcOpts->getPath2Jdl( );
	// rank
        rankOpt =  wmcOpts->getBoolAttribute (Options::RANK);
        // Delegation ID
        dgOpt = wmcUtils->getDelegationId ();
         if ( ! dgOpt  ){
                throw WmsClientException(__FILE__,__LINE__,
                                "readOptions",DEFAULT_ERR_CODE,
                                "Missing Information",
                                "no proxy delegation ID" );
         }
	// checks if the proxy file pathname is set
        if (proxyFile) {
        	logInfo->print (WMS_DEBUG, "Proxy File", proxyFile);
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
/*
* performs the main operations
*/
void JobListMatch::listMatching ( ){
	postOptionchecks();
	const int tab = 50 ;
        const string ws = " ";
        int spaces = 0 ;
        string ce = "";
        vector <pair<string , long> > list ;
	vector <pair<string , long> > ::iterator it ;
	ostringstream out ;
	ostringstream os;
        // Reads and checks the JDL
        checkAd( );
        // list matching ....
        list = jobMatching( );
	if (list.empty()){
		// if no resource has been found
		out << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " failure") ) << "\n";
		out << "No Computing Element matching your job requirements has been found!";
	} else {


		out  << wmcUtils->getStripe(74, "=") << "\n\n";
		out << "\t\t     COMPUTING ELEMENT IDs LIST ";
		out << "\n The following CE(s) matching your job requirements have been found:\n";
		out << "\n\t*CEId*";
		if (rankOpt) {
			spaces = tab -14 ;
			for ( int i = 0; i < spaces ; i++ ) { out << ws ; }
			out << "*Rank*\n";
		}
		out << "\n";
		for ( it = list.begin( ) ; it != list.end( ); it++ ){
			ce = it->first ;
			out << " - " << it->first ;
			if(rankOpt) {
				spaces = tab - ce.size ( );
				for ( int i = 0; i < spaces ; i++ ) { out << ws ; }
				out << it->second ;
			}
			out << "\n";
		}
	}
        // saves the result
        if (outOpt){
        	string tofile = "\n" + out.str() + "\n" + wmcUtils->getStripe(74, "=") + "\n";
                if ( wmcUtils->toFile(*outOpt, tofile, true) < 0 ){
                        logInfo->print (WMS_WARNING, "Unable to write the list of CeId's to the output file: " , Utils::getAbsolutePath(*outOpt));
                } else{
                        logInfo->print (WMS_DEBUG, "Computing Element(s) matching your job requirements have been stored in the file:",
				Utils::getAbsolutePath(*outOpt), false);
			os << wmcUtils->getStripe(84, "=" , string (wmcOpts->getApplicationName() + " success") ) << "\n";
                        os << "\nComputing Element(s) matching your job requirements have been stored in the file:\n";
                        os << Utils::getAbsolutePath(*outOpt) << "\n";
			os << "\n" << wmcUtils->getStripe(84, "=") << "\n\n";
			cout << os.str();
                }
        } else{
		// if no output file ....
		out << "\n" << wmcUtils->getStripe(74, "=") << "\n\n";
		cout << out.str( );
	}
	// log file info
	cout << getLogFileMsg ( ) << "\n";
};

/***********************************************
* private methods
************************************************/
/*
* checks the JDL
*/
void JobListMatch::checkAd ( ){
	glite::wms::common::configuration::WMCConfiguration* wmcConf =wmcUtils->getConf();
	if ( !jobAd ){
		jobAd = new Ad ( );
        }
	if (! jdlFile){
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
			"JDL File Missing",
                         "uknown JDL file pathame  (Last argument of the command must be a JDL file)"   );
        }
	logInfo->print (WMS_DEBUG, "The JDL files is:", Utils::getAbsolutePath(*jdlFile));
	jobAd->fromFile (*jdlFile);
        // check submitTo
	 if ( jobAd->hasAttribute(JDL::SUBMIT_TO) ){
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
        		"submitTo" ,
                        "Forcing CEId for job-list-match does not make sense");
        }

	if ( jobAd->hasAttribute(JDL::TYPE , JDL_TYPE_COLLECTION) ) {
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
        		"Not supported Type" ,
                        "Collection type is not supported for list match");
        } else if ( jobAd->hasAttribute(JDL::TYPE , JDL_TYPE_DAG) ) {
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
        		"Not supported Type" ,
                        "DAG type is not supported for list match");
	}
	// Simple Ad manipulation
	AdUtils::setDefaultValuesAd(jobAd,wmcConf);
	JobAd *pass= new JobAd(*(jobAd->ad()));
	AdUtils::setDefaultValues(pass,wmcConf);
	// JDL STRING
	jdlString = new string(pass->toSubmissionString());
	logInfo->print(WMS_DEBUG, "JDL" , *jdlString );
};

/*
* Retrieves the list of matching resources
*/
std::vector <std::pair<std::string , long> > JobListMatch::jobMatching( ) {
	vector <pair<string , long> > list ;
	// checks if jdlstring is not null
        if (!jdlString){
                throw WmsClientException(__FILE__,__LINE__,
                        "jobMatching",  DEFAULT_ERR_CODE ,
                        "Null Pointer Error",
                        "Null pointer to JDL string");
        }
	endPoint =  new string(this->getEndPoint());
 	try{
		logInfo->print(WMS_INFO, "Connecting to the service", cfgCxt->endpoint);
  		// ListMatch
    		list = jobListMatch(*jdlString, *dgOpt, cfgCxt);
      } catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
		"jobListMatch", ECONNABORTED,
		"Operation failed",
		"unable to perform the operation:"  + errMsg(exc));
  	}
       return list ;
}

}}}} // ending namespaces

