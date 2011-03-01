/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

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
#include "glite/jdl/jdl_attributes.h"
#include "glite/jdl/JDLAttributes.h"
#include "glite/jdl/extractfiles.h"
#include "glite/jdl/adconverter.h"

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;
using namespace glite::jdl;

namespace glite {
namespace wms{
namespace client {
namespace services {


std::string join( const std::vector<std::string>& array, const std::string& sep)
{
  vector<string>::const_iterator sequence = array.begin( );
  vector<string>::const_iterator end_sequence = array.end( );
  if(sequence == end_sequence) return "";

  string joinstring( "" );
  if (sequence != end_sequence) {
    joinstring += *sequence;
    ++sequence;
  }

  for( ; sequence != end_sequence; ++sequence )
    joinstring += sep + *sequence;
  
  return joinstring;
}

std::string join( const std::vector<long int>& array, const std::string& sep)
{
  vector<long int>::const_iterator sequence = array.begin( );
  vector<long int>::const_iterator end_sequence = array.end( );
  if(sequence == end_sequence) return "";

  string joinstring( "" );
  if (sequence != end_sequence) {
    ostringstream tmp( "" );
    tmp << *sequence;
    joinstring += tmp.str( );//*sequence;
    ++sequence;
  }

  for( ; sequence != end_sequence; ++sequence ) {
  
    ostringstream tmp( "" );
    tmp << *sequence;
  
    joinstring += sep + tmp.str( );//*sequence;
  }
  
  return joinstring;
}

JobListMatch::JobListMatch(){
	// init of the boolean attributes
        rankOpt  = false ;
	// parameters
        m_jdlFile = "";
        m_jdlString = "";
        //Ad
        jobAd = NULL;
	m_json = false;
	pprint = false;
  };
/*
*	Default destructor
*/
JobListMatch::~JobListMatch( ){
        if (jobAd) { delete(jobAd);}
};
/*
* performs the main operations
*/
void JobListMatch::readOptions (int argc,char **argv) {
	Job::readOptions  (argc, argv, Options::JOBMATCH);
 	// path to the JDL file
  	m_jdlFile = wmcOpts->getPath2Jdl( );
	//EndPoint
	retrieveEndPointURL( );
	// rank
        rankOpt =  wmcOpts->getBoolAttribute (Options::RANK);
	// checks if the output file already exists
	
	m_json = wmcOpts->getBoolAttribute (Options::JSON);
	pprint = wmcOpts->getBoolAttribute (Options::PRETTYPRINT);
	
	if (!m_outOpt.empty() && !wmcUtils->askForFileOverwriting(m_outOpt) ){
		cout << "bye\n";
		getLogFileMsg ( );
		Utils::ending(ECONNABORTED);
	}

	if (!m_outOpt.empty() && m_json) {
	  ostringstream info ;
	  info << "The following options cannot be specified together:\n" ;
	  info << wmcOpts->getAttributeUsage(Options::OUTPUT) << "\n";
	  info << wmcOpts->getAttributeUsage(Options::JSON) << "\n";
	  
	  throw WmsClientException(__FILE__,__LINE__,
				   "readOptions",DEFAULT_ERR_CODE,
				   "Input Option Error", info.str());
	}
};
/*
* performs the main operations
*/
void JobListMatch::listMatching ( ){
	const int tab = 60 ;
        const string ws = " ";
        int spaces = 0 ;
        string ce = "";
	ostringstream out ;
	ostringstream os;
        // Reads and checks the JDL
        checkAd( );
        // list matching ....
	listmatchPerformStep(STEP_LISTMATCH);

        //list = jobMatching( );
	if (m_listResult_v.empty()){
		// if no resource has been found
		out << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " failure") ) << "\n";
		out << "No Computing Element matching your job requirements has been found!";
	} else {


		
		if(!m_json ) {
		  out  << wmcUtils->getStripe(74, "=") << "\n\n";
		  out << "\t\t     COMPUTING ELEMENT IDs LIST ";
		  out << "\n The following CE(s) matching your job requirements have been found:\n";
		  out << "\n\t*CEId*";
		}
		if(m_json) {
		  string quote;
		  if(pprint) {
		    quote = "";
		    
		  } else {
		    quote = "\"";
		  }
		  out << "\n";
		  out << "{" << ( pprint ? "\n" : " " );//  {\n ";
		  out << ( pprint ? "\t" : " " ) << quote << "result" << quote << ": " << quote << "success" << quote << ( pprint ? "\n" : ", " );//\"\n";
		  vector <pair<string , long> > ::iterator it = m_listResult_v.begin( );
		  vector <pair<string , long> > ::iterator const end = m_listResult_v.end( );
		  vector<string> ce_pieces;
		  vector<long int> rank_pieces;
		  for (  ; it != end;  it++ ){
			string ce = it->first ;
			long int rank = it->second;
			
			ce_pieces.push_back( ce );
			rank_pieces.push_back( rank );
			
//			out << "\t\"CE\": { ceid=\"" << ce << "\"";
//			if(rankOpt)
//			  out << ", Rank=\"" << it->second << "\" }";
//			else
//			  out << "}\n\n";
//			
//			out << "\n";
		  }
		  
		  string CEs = join( ce_pieces, quote + ", " + quote );
		  string RANKs = join( rank_pieces, ", " );
		  
		  out << ( pprint ? "\t" : " " ) << quote << "ceid" << quote << ": " << (pprint ? "" : "[" ) << quote << CEs << quote << (pprint ? "" : "]" ) << ( pprint ? "\n" : " " );
		  if(rankOpt)
		    out << ( pprint ? "\t" : " " ) << quote << "rank" << quote << ": " << (pprint ? "" : "[" ) << RANKs << (pprint ? "" : "]" ) << ( pprint ? "\n" : " " );
		  
		  out <<"  }\n";
		} else {
		  if (rankOpt) {
			spaces = tab - 9  ;
			for ( int i = 0; i < spaces ; i++ ) { out << ws ; }
			out << "*Rank*\n";
		  }
		  out << "\n";
		  vector <pair<string , long> > ::iterator it = m_listResult_v.begin( );
		  vector <pair<string , long> > ::iterator const end = m_listResult_v.end( );
		  for (  ; it != end;  it++ ){
			ce = it->first ;
			out << " - " << it->first ;
			if(rankOpt) {
				spaces = tab - ce.size ( );
				for ( int i = 0; i < spaces+5 ; i++ ) { out << ws ; }
				out << it->second ;
			}
			out << "\n";
		  }
		}
	}
        // saves the result
        if (!m_outOpt.empty()){
        	string tofile = "\n" + out.str() + "\n" + wmcUtils->getStripe(74, "=") + "\n";
                if ( wmcUtils->saveToFile(m_outOpt, tofile) < 0 ){
                        logInfo->print (WMS_WARNING, "Unable to write the list of CeId's to the output file: " , Utils::getAbsolutePath(m_outOpt));
                } else{
                        logInfo->print (WMS_DEBUG, "Computing Element(s) matching your job requirements have been stored in the file:",
				Utils::getAbsolutePath(m_outOpt), false);
			os << wmcUtils->getStripe(84, "=" , string (wmcOpts->getApplicationName() + " success") ) << "\n";
                        os << "\nComputing Element(s) matching your job requirements have been stored in the file:\n";
                        os << Utils::getAbsolutePath(m_outOpt) << "\n";
			os << "\n" << wmcUtils->getStripe(84, "=") << "\n\n";
			cout << os.str();
                }
        } else{
		// if no output file ....
		if(!m_json) {
		  out << "\n" << wmcUtils->getStripe(74, "=") << "\n\n";
		}
		
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
	glite::jdl::Ad *wmcConf =wmcUtils->getConf();
	if ( !jobAd ){
		jobAd = new Ad ( );
        }
	if (m_jdlFile.empty()){
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
			"JDL File Missing",
                         "uknown JDL file pathame  (Last argument of the command must be a JDL file)"   );
        }
	logInfo->print (WMS_DEBUG, "The JDL file is:", Utils::getAbsolutePath(m_jdlFile));
	jobAd->fromFile (m_jdlFile);
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
	if (!jobAd->hasAttribute (JDL::VIRTUAL_ORGANISATION)){
		jobAd->setAttribute(JDL::VIRTUAL_ORGANISATION, wmcUtils->getVirtualOrganisation());
	}
	AdUtils::setDefaultValuesAd(jobAd,wmcConf);
	JobAd *pass= new JobAd(*(jobAd->ad()));
	AdUtils::setDefaultValues(pass,wmcConf);
	// JDL STRING
	m_jdlString = pass->toSubmissionString();
	logInfo->print(WMS_DEBUG, "JDL" , m_jdlString );
};

/*
* Retrieves the list of matching resources
*/
void JobListMatch::jobMatching( ) {
	// checks if jdlstring is not null
        if (m_jdlString.empty()){
                throw WmsClientException(__FILE__,__LINE__,
                        "jobMatching",  DEFAULT_ERR_CODE ,
                        "Null Pointer Error",
                        "Null pointer to JDL string");
        }
	logInfo->print(WMS_DEBUG, "Sending the request to the service", getEndPoint( ));
 	try{
  		// ListMatch
		logInfo->service(WMP_LISTMATCH_SERVICE);
			
		// Set the SOAP timeout
		setSoapTimeout(SOAP_JOB_LIST_MATCH_TIMEOUT);
			
    		m_listResult_v = jobListMatch(m_jdlString, getDelegationId( ), getContext());
		logInfo->result(WMP_LISTMATCH_SERVICE, "The MatchMaking operations have been successfully performed");
      } catch (BaseException &exc) {
		throw WmsClientException(__FILE__,__LINE__,
		"jobListMatch", ECONNABORTED,
		"Operation failed",
		"Unable to perform the operation: "  + errMsg(exc));
  	}
}

/** Perform a certain operation and, if any problem arises, try and recover all the previous steps */
void JobListMatch::listmatchPerformStep(listmatchRecoveryStep step){
	switch (step){
		case STEP_LISTMATCH:
			try{
                            jobMatching();
                            }
			catch (WmsClientException &exc) {
				logInfo->print(WMS_WARNING, string(exc.what()), "");
				listmatchRecoverStep(step);
			}
			break;
		default:
			throw WmsClientException(__FILE__,__LINE__,
				"submitPerformStep", ECONNABORTED,
				"Fatal Recovery",
				"Unable to recover from specified step");
	}
}
void JobListMatch::listmatchRecoverStep(listmatchRecoveryStep step){
	doneUrls.push_back(m_endPoint);
	this->m_endPoint = "";
	jobPerformStep(STEP_GET_ENDPOINT);
        jobPerformStep(STEP_DELEGATE_PROXY);
	// PERFORM STEP_LISTMATCH
	listmatchPerformStep(STEP_LISTMATCH);
	if (step==STEP_LISTMATCH){return;}
	// no return reached: Unknown STEP
	throw WmsClientException(__FILE__,__LINE__,
		"listmatchRecoverStep", ECONNABORTED,
		"Fatal Recovery",
		"Unable to recover from specified step");
}


}}}} // ending namespaces

