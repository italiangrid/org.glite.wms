#ifndef GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H
#define GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

// options utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"

// Ad's
#include "glite/wms/jdl/Ad.h"
#include "glite/wms/jdl/ExpDagAd.h"
#include "glite/wms/jdl/collectionad.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class JobSubmit {

	public :
		/*
		*	default constructor
		*/
		JobSubmit ( );
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /*
		*	performs the operations to submit the job(s)
		*/
		void submission ( ) ;
                /*
		*	print the help usage message on the std output
                *	@param exename the executable name
		*/
                void printUsageMsg (const char* exename );

	private
                :/*
                 *	contacts the endpoint configurated in the context
                 *	in order to retrieve the http(s) destionationURI of the job
                 *	identified by the jobid
                 *	@param jobid the identifier of the job
                 *	@return a pointer to the string with the http(s) destinationURI (or NULL in case of error)
                */
                std::string* getInputSBDestURI(const std::string &jobid) ;

                /*
                *	checks if the user free quota (on the server endpoint) could support (in size) the transferring of a set of files
                *	@param files the set of files to be transferred
                *	@param limit this parameter returns the value of the user free quota
                *	@return true if the user freequota is enough or no free quota has been set on the server (false, otherwise)
                */
                bool checkFreeQuota ( std::vector<std::pair<std::string,std::string> > files, long &limit ) ;
		/*
                *	callback used by curl functions
                */
              	static size_t JobSubmit::readCallback(void *ptr, size_t size, size_t nmemb, void *stream);
     		/*
		* 	transfers a set of local files to one(more) remote machine(s)
 		*	@param paths list of files to be transferred (each pair is <source,destination>)
                *	@throw WmsClientException if any error occurs during the operations
                *		(the local file doesn't exists, defective credential, errors on remote machine)
		*/
		std::string transferFiles (std::vector<std::pair<std::string,std::string> > paths);
		/*
                *	handles the result of the register for a normal job
                */
                void normalJob( );
                /*
                *	handles the result of the register for a DAG job
                */
                void dagJob ( );
		/*
                *	string input arguments
                */
		std::string* logOpt ;
		std::string* chkptOpt ;
		std::string* dgOpt ;
		std::string* lmrsOpt ;
		std::string* toOpt ;
		std::string* outOpt ;
		std::string* inOpt ;
		std::string* cfgOpt ;
		std::string* resourceOpt ;
		std::string* validOpt ;
                std::string* voOpt ;
		/*
                *	boolean input arguments
                */
                bool autodgOpt ;
		bool nomsgOpt ;
		bool noguiOpt ;
		bool nolistenOpt ;
		bool nointOpt ;
		bool versionOpt ;
                bool dbgOpt ;

                /*
                * JobId's
                */
                glite::wms::wmproxyapi::JobIdApi jobIds ;
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
                *	Ad-objects
		*/
                glite::wms::jdl::Ad *ad ;
		glite::wms::jdl::ExpDagAd *dag  ;
        	glite::wms::jdl::CollectionAd *collect ;


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
               std::string* wmpEndPoint ;
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
#endif //GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

