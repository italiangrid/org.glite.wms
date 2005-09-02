#ifndef GLITE_WMS_CLIENT_SERVICES_JOB_H
#define GLITE_WMS_CLIENT_SERVICES_JOB_H

// options utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
#include "utilities/logman.h"
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class Job{
	public :
		/**
		* Default constructor
		*/
		Job();
		/*
		* Default destructror
		*/
		virtual ~Job();
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
                *	@return a string with the list of the specified options
		*/
		virtual std::string readOptions (int argc,char **argv,
			glite::wms::client::utilities::Options::WMPCommands);
                /*
		*	prints the error messages for an exception
		*	@param header header text
		* 	@param exc the exception
		* 	@param exename the executable name
		*/
		virtual void excMsg(const std::string &header, glite::wmsutils::exception::Exception &exc, const std::string &exename="");
		/*
		*  Returns a string with the information on the log filename
		* @return the string containg the information
		*/
		std::string Job::getLogFileMsg ( ) ;
	protected:
		/** Common post-options Checks (proxy time left..)*/
		void postOptionchecks(unsigned int proxyMinTime=0);
		/** Input arguments */
		std::string* logOpt ;	// --logfile <file>
		std::string* outOpt ;	// --output <file>
		std::string* cfgOpt ; // --config <file>
		std::string* voOpt ;	// --vo <VO_Name>
		bool nointOpt ;		// --noint
		bool dbgOpt ;		// --debug
		/** handles the input options*/
		glite::wms::client::utilities::Options *wmcOpts ;
		/** utilities object */
		glite::wms::client::utilities::Utils::Utils *wmcUtils ;
		/** log file */
		glite::wms::client::utilities::Log *logInfo ;
		/** endpoint*/
		std::string* endPoint ;
		/** proxy file pathname*/
		char* proxyFile ;
		/** trusted Certs dirname */
		char* trustedCert ;
		/** configuration contex */
		glite::wms::wmproxyapi::ConfigContext *cfgCxt ;
	private :
        	/*
                * Prints the Ui version on the standard output and ends the execution of the programme
                */
                void getVersion ( );

};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOB_H

