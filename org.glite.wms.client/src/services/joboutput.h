#ifndef GLITE_WMS_CLIENT_SERVICES_JOBOUTPUT_H
#define GLITE_WMS_CLIENT_SERVICES_JOBOUTPUT_H

#include "utilities/options_utils.h"
#include "glite/wms/wmproxyapi/wmproxy_api.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class JobOutput {

	public :
        	/*
                *	default constructor
                */
		JobOutput ( );
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
               void readOptions ( int argc,char **argv) ;
		/*
                *	performs the main operations
                */
		void getOutput ( ) ;

        private :
        	/*
 		*	prints the results on the std output
 		*/
		void listResult(std::vector <std::pair<std::string , long> > &files, const std::string jobid );
                /*
                * 	writing callback for curl operations
                */
                static int storegprBody(void *buffer, size_t size, size_t nmemb, void *stream);
                /*
                *	file downloading
                *	@param files list of files to be downloaded and their size
                */
                void getFiles (std::vector <std::pair<std::string , long> > &files);
		/*
		*	struct for files
		*/
		struct httpfile { char *filename; FILE* stream; } ;
        	 /*
                *	string input arguments
                */
		std::string* input ;
		std::string* dir ;
		std::string* config ;
                std::string* vo ;
		std::string* logfile ;
        	/*
                *	boolean input arguments
                */
                bool version ;
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

#endif //GLITE_WMS_CLIENT_SERVICES_JOBOUTPUT_H

