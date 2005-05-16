#ifndef GLITE_WMS_CLIENT_SERVICES_DELEGATEPROXY_H
#define GLITE_WMS_CLIENT_SERVICES_DELEGATEPROXY_H


// options utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class DelegateProxy {

	public :
		/*
		*	default constructor
		*/
		DelegateProxy ( );
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /*
		*	performs the operations to submit the job(s)
		*/
		void delegation ( ) ;
                /*
		*	print the help usage message on the std output
                *	@param exename the executable name
		*/
                void printUsageMsg (const char* exename );

	private :

		/*
                *	string input arguments
                */
		std::string* logOpt ;
		std::string* dgOpt ;
		std::string* outOpt ;
		std::string* cfgOpt ;
                std::string* voOpt ;
		/*
                *	boolean input arguments
                */
                bool autodgOpt ;
		bool nointOpt ;
                bool dbgOpt ;

		/*
                *	handles the input options
                */
		glite::wms::client::utilities::Options *wmcOpts ;
		/*
                *	utilities object
                */
                glite::wms::client::utilities::Utils::Utils *wmcUtils ;
                /*
                *	configuration contex
                */
                glite::wms::wmproxyapi::ConfigContext *cfgCxt ;
                /*
                *	WMProxy endpoint
                */
               std::string* wmpEndPoint ;

};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

