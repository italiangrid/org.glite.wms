#ifndef GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H
#define GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

#include "utilities/options_utils.h"
#include "glite/wms/wmproxyapi/wmproxy_api.h"

// Ad's
#include "glite/wms/jdl/Ad.h"
#include "glite/wms/jdl/ExpDagAd.h"

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
		std::string* logfile ;
		std::string* chkpt ;
		std::string* lmrs ;
		std::string* to ;
		std::string* ouput ;
		std::string* input ;
		std::string* config ;
		std::string* resource ;
		std::string* valid ;
                std::string* vo ;
		/*
                *	boolean input arguments
                */
		bool nomsg ;
		bool nogui ;
		bool nolisten ;
		bool noint ;
		bool version ;
                bool debug ;

                /*
                * JobId's
                */
                glite::wms::wmproxyapi::JobIdApi jobIds ;
		/*
                *	Ad
                */
                glite::wms::jdl::Ad *ad ;
		/*
                *	dagad
                */
                glite::wms::jdl::ExpDagAd *dag ;
		/*
                *	handles the input options
                */
		glite::wms::client::utilities::Options *opts ;
                /*
                *	configuration contex
                */
                glite::wms::wmproxyapi::ConfigContext *cfgCxt ;


		/*
		*	path to the JDL file
		*/
		std::string *jdlFile ;
		/*
		*	string of the user JDL
		*/
		std::string *jdlString ;
                /*
                *	user delegation-ID
                */
                std::string* delegID ;
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
#endif //GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

