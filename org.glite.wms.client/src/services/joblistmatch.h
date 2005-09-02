#ifndef GLITE_WMS_CLIENT_SERVICES_JOBLISTMATCH_H
#define GLITE_WMS_CLIENT_SERVICES_JOBLISTMATCH_H

// inheritance
#include "job.h"
// options utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
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

class JobListMatch : public Job {

	public :
        	/*
		*	Default constructor
		*/
		JobListMatch( );
                /*
		*	Default destructor
		*/
		~JobListMatch( );
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /*
		*	performs the main operations
		*/
                void listMatching( ) ;

  	private :
                /*
                * Retrieves the list of matching resources
                */
                std::vector <std::pair<std::string , long> > JobListMatch::jobMatching( ) ;
        	/*
                *	check the input JDL
                */
        	void checkAd ( ) ;
        	 /*
                *	string input arguments
                */
		std::string* dgOpt;
        	/*
                *	boolean input arguments
                */
                bool rankOpt ;
		/*
                *	Ad
                */
                glite::wms::jdl::Ad *jobAd ;
                /*
		*	path to the JDL file
		*/
		std::string *jdlFile ;
		/*
		*	string of the user JDL
		*/
		std::string *jdlString ;
};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBLISTMATCH_H

