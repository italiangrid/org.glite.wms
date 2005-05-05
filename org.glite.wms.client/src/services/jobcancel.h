#ifndef GLITE_WMS_CLIENT_SERVICES_JOBCANCEL_H
#define GLITE_WMS_CLIENTSERVICES_JOBCANCEL_H

#include "utilities/options_utils.h"

#include "glite/wms/wmproxyapi/wmproxy_api.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class JobCancel {

	public :
		/*
                *	default constructor
                */
                JobCancel( );
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
                void readOptions (int argc,char **argv) ;
		/*
                *	performs the cancelling of the job(s)
                */
		void cancel ( ) ;

	private:
        	/*
                *	string input arguments
                */
		std::string* input ;
                std::string* output ;
                std::string* config;
                std::string* vo ;
                std::string* logfile ;
		/*
                *	boolean input arguments
                */
		bool version ;
                bool help ;
                bool noint ;
                bool debug ;
                /*
                * JobId's
                */
                std::vector<std::string> jobIds ;
		/*
                *	handles the input options
                */
		glite::wms::client::utilities::Options *opts ;
                /*
                *	configuration contex
                */
                glite::wms::wmproxyapi::ConfigContext *cfgCxt ;
                /*
                *	WMProxy endpoint
                */
                char* wmpEndPoint ;
		/*
                * 	proxy file pathname
                */
                char* proxyFile ;

		/*
                * 	trusted Certs dirname
                */
                char* trustedCert ;
};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBCANCEL_H

