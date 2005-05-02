

#include "jobsubmit.h"
// exceptions
#include "utilities/excman.h"
#include "utilities/utils.h"
#include <sstream> //osstream
#include <iostream>

namespace glite {
namespace wms{
namespace client {
namespace services {

using namespace std ;
using namespace glite::wms::client::utilities ;

const unsigned int DEFAULT_ERR_CODE      =       7772;
/*
*	default constructor
*/
JobSubmit::JobSubmit( ){
	// init of the string attributes
	 configvo = NULL ;
	 logfile = NULL ;
	 chkpt  = NULL;
	 lmrs = NULL ;
	 to = NULL ;
	 ouput = NULL ;
	 input  = NULL;
	 config = NULL ;
	 resource = NULL ;
	// init of the valid attribute (long type)
	valid = 0 ;
	// init of the boolean attributes
	nomsg = false ;
	nogui = false ;
	nolisten = false ;
	noint  = false;
	version  = false;
	help = false ;
	// init of the option object
	opts = new Options(Options::JOBSUBMIT) ;
};

void JobSubmit::readOptions (int argc,char **argv){
	ostringstream err ;
	opts->readOptions(argc, (const char**)argv);
	// input & resource (no together)
	input = opts->getStringAttribute(Options::INPUT);
	resource = opts->getStringAttribute(Options::RESOURCE);
	if (input && resource){
		err << "the following options cannot be specified together:\n" ;
		err << opts->getAttributeUsage(Options::INPUT) << "\n";
		err << opts->getAttributeUsage(Options::RESOURCE) << "\n\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
	// lmrs has to be used with input o resource
	lmrs = opts->getStringAttribute(Options::LMRS);
	if (lmrs && !( resource || input) ){
		err << "LRMS option cannot be specified without a resource:\n";
		err << "use " + opts->getAttributeUsage(Options::LMRS) << " with\n";
		err << opts->getAttributeUsage(Options::RESOURCE) << "\n";
		err << "or\n" + opts->getAttributeUsage(Options::INPUT) << "\n\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
	// "valid" & "to" (no together)
	valid = opts->getStringAttribute(Options::VALID);
	to = opts->getStringAttribute(Options::TO);
	if (valid && to){
		err << "the following options cannot be specified together:\n" ;
		err << opts->getAttributeUsage(Options::VALID) << "\n";
		err << opts->getAttributeUsage(Options::TO) << "\n\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
	if (valid){
		try{
			if (! isAfter(*valid, 2) ){

				throw WmsClientException(__FILE__,__LINE__,
					"readOptions", DEFAULT_ERR_CODE,
					"Invalid Time Value", "time is out of limit" );
			}
		} catch (WmsClientException &exc) {
			err << exc.what() << " (use: " << opts->getAttributeUsage(Options::VALID) << ")\n";
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Wrong Time Value",err.str() );
		}
	}
	if (to){
		try{
			if (! isAfter(*to) ){

				throw WmsClientException(__FILE__,__LINE__,
					"readOptions", DEFAULT_ERR_CODE,
					"Invalid Time Value", "time is out of limit" );
			}
		} catch (WmsClientException &exc) {
			err << exc.what() << " (use: " << opts->getAttributeUsage(Options::TO) <<")\n";
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Wrong Time Value",err.str() );
		}
	}
	// path to the JDL file
	jdlFile = opts->getPath2Jdl( );
	// reads the JDL file
	//jdlString = opts->getJdlString(jdlString);
}

void JobSubmit::submission ( ){

};
}}}} // ending namespaces
