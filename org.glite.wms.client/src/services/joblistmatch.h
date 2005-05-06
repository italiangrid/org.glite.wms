#ifndef GLITE_WMS_CLIENT_SERVICES_JOBLISTMATCH_H
#define GLITE_WMS_CLIENT_SERVICES_JOBLISTMATCH_H

// options utilities
#include "utilities/options_utils.h"
// wmproxy API
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"
// Ad's
#include "glite/wms/jdl/Ad.h"
#include "glite/wms/jdl/ExpDagAd.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class JobListMatch {

	public :
        	/*
		*	default constructor
		*/
		JobListMatch( );
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /*
		*	performs the main operations
		*/
                void getListMatching( ) ;
  	private :
        	/*
                *	check the input JDL
                */
        	void checkAd ( ) ;
        	 /*
                *	string input arguments
                */
		std::string*	config ;
		std::string*	vo ;
		std::string*	output ;
 		std::string*	logfile;
        	/*
                *	boolean input arguments
                */
                bool version ;
                bool rank ;
                bool noint;
                bool debug ;
		/*
                *	handles the input options
                */
		glite::wms::client::utilities::Options *opts ;
                /*
                *	configuration contex
                */
                glite::wms::wmproxyapi::ConfigContext *cfgCxt ;
		/*
                *	Ad
                */
                glite::wms::jdl::Ad *ad ;
		/*
                *	Dag
                */
                glite::wms::jdl::ExpDagAd *dag ;
                /*
		*	path to the JDL file
		*/
		std::string *jdlFile ;
		/*
		*	string of the user JDL
		*/
		std::string *jdlString ;
                /*
                *	WMProxy endpoint
                */
                char* wmpEndPoint ;
};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBLISTMATCH_H

