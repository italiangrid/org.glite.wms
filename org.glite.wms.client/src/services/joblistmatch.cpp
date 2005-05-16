

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
	// init of the string attributes
        config = NULL ;
        delegation = NULL;
        vo = NULL ;
        output  = NULL;
        logfile = NULL;
	// init of the boolean attributes
        version = false ;
        rank = false ;
        noint = false;
        debug  = false;
	// options
        opts = NULL;
	// context
        cfgCxt = NULL ;
	// parameters
        jdlFile = NULL ;
        jdlString = NULL ;
        wmpEndPoint = NULL;
  };

void JobListMatch::readOptions (int argc,char **argv) {
	ostringstream err ;
	// init of option objects object
        opts = new Options(Options::JOBMATCH) ;
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

        // delegation
        delegation = opts->getStringAttribute( Options::DELEGATION ) ;

	output=  opts->getStringAttribute( Options::OUTPUT ) ;
	logfile = opts->getStringAttribute(Options::LOGFILE );

        version =  opts->getBoolAttribute (Options::VERSION);
        rank =  opts->getBoolAttribute (Options::RANK);
	debug =  opts->getBoolAttribute (Options::DBG);
	noint=  opts->getBoolAttribute (Options::NOINT) ;
        // configuration context
        cfgCxt = new ConfigContext("", wmpEndPoint, "");
};

void JobListMatch::checkAd ( ){
	bool isdag = true;
	if ( !ad ){
		ad = new Ad ( );
        }
	if (! jdlFile){
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
			"Missing Information",
                        "uknown JDL file pathame"   );
        }
	ad->fromFile (*jdlFile);
        // check submitTo
	 if ( ad->hasAttribute(JDL::SUBMIT_TO) ){
		throw WmsClientException(__FILE__,__LINE__,
			"checkAd",  DEFAULT_ERR_CODE,
        		"submitTo" ,
                        "Forcing CEId for job-list-match does not make sense");
        }

        if ( ad->hasAttribute(JDL::TYPE , JDL_TYPE_DAG) ) {
		isdag = true;
                dag = new  ExpDagAd ( ad->toString() );
                if ( debug ) {
                        cout << "\nyou have submitted a DAG job " << endl ;
                }
                // expands the DAG loading all JDL files
                dag->expand( );

                jdlString = new string( dag->toString( ) );
	} else {
        	jdlString = new string( ad->toString( ) );
 	}

        if (debug){
		cout << "\n***************************************************************************\n";
                cout << "\tJDL :\n";
                cout << *jdlString ;
		cout << "***************************************************************************\n";
        }
};

void JobListMatch::getListMatching ( ){
	vector <pair<string , long> > list ;
	vector <pair<string , long> > ::iterator it ;
	ostringstream msg ;
        checkAd ( );
	// check the needed parameters
        if (!jdlString){
		throw WmsClientException(__FILE__,__LINE__,
			"getListMatching",  DEFAULT_ERR_CODE,
			"Null Pointer Error", "null pointer to JDL string"   );
        }
        if (!delegation){
		throw WmsClientException(__FILE__,__LINE__,
			"getListMatching",  DEFAULT_ERR_CODE,
			"Null Pointer Error", "null pointer to delegation string"   );
        }
	if (!cfgCxt){
		throw WmsClientException(__FILE__,__LINE__,
			"getListMatching",  DEFAULT_ERR_CODE,
			"Null Pointer Error", "null pointer to ConfigContext object"   );
        }


        list = jobListMatch(*jdlString, *delegation, cfgCxt);

        msg << "\n***************************************************************************\n";
        msg << "                         COMPUTING ELEMENT IDs LIST ";
        msg << "\n The following CE(s) matching your job requirements have been found:\n";
        msg << "\n\t\t*CEId*";
        if (rank){
                msg << "\t\t\t*Rank*\n";
        }
        for ( it = list.begin( ) ; it != list.end( ); it++ ){
                msg << "\t\t" << it->first << "\t\t\t" << it->second << "\n";
        }
        msg << "\n***************************************************************************\n";
	
        cout << msg ;
};

}}}} // ending namespaces
