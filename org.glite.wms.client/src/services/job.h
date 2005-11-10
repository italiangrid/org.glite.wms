/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
* 	Authors:	Alessandro Maraschini <alessandro.maraschini@datamat.it>
* 			Marco Sottilaro <marco.sottilaro@datamat.it>
*/

// 	$Id$

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
		virtual void readOptions (int argc,char **argv,
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
		virtual std::string Job::getLogFileMsg ( ) ;





virtual void setDelegationId ( );
virtual const std::string getDelegationId ( );
virtual const std::string delegateProxy( );

virtual void setEndPoint ();
virtual void setEndPoint (const std::string& jobid);

virtual const std::string getEndPoint ();
virtual glite::wms::wmproxyapi::ConfigContext* Job::getContext( );

virtual const char* getProxyPath ( );
virtual const char* getCertsPath ( );

virtual const std::string Job::getWmpVersion (std::string &endpoint) ;
virtual const int Job::getWmpVersion ( );

	private:		/**
		* Retrieves the version of one or more WMProxy services.
		* A set of WMProxy to be contacted can be specified as input by the 'urls'  vector.
		* If no urls is specified as input, the following objects are checked to retrieve to service to be contacted:
		* the 'endPoint' private attribute of this class; the specific user environment variable ; the specific list in the configuration file.
		* An exception is throw if either no endpoint is specified or
		* all specified endpoints don't allow performing the requested operation.
		* @param urls the list of the endpoint that can be contacted; at the end of the execution this parameter
		* will contain a list of the endpoints that are not be contacted
		* @param endpoint the url of the contacted endpoint
		* @param version the version number of the contacted endpoint
		* @param all if TRUE, it contacts all endpoints specified
		*/
		void Job::checkWmpList (std::vector<std::string> &urls, std::string &endpoint, std::string &version, const bool &all=false) ;



		/**
		* Performs credential delegation
		* @param id the delegation identifier string to be used for the delegation
		* @return the pointer to the string containing the EndPoint URL
		*/
       		 void delegateUserProxy(const std::string &endpoint);
		virtual void setProxyPath ( );
		virtual void setCertsPath ( );
		 /**
        	* Gets the version message
        	*/
    		void printClientVersion( );
		/**
		* Contacts the endpoint to retrieve the version
		*/
		void printServerVersion( );
	protected:
		/** Common post-options Checks (proxy time left..)*/
		void postOptionchecks(unsigned int proxyMinTime=0);
		/** Input arguments */
		std::string* logOpt ;	// --logfile <file>
		std::string* outOpt ;	// --output <file>
		std::string* cfgOpt ; // --config <file>
		std::string* voOpt ;	// --vo <VO_Name>
		std::string* dgOpt ;	// --delegationid
		bool autodgOpt ;	// --autm-delegation,
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

		/** configuration contex */
		glite::wms::wmproxyapi::ConfigContext *cfgCxt ;
		/*
		* Major Version number of the server
		*/
		int wmpVersion;
		bool needDelegation ;
		private :
			std::string* proxyFile ;
			std::string* trustedCerts ;
			std::string* jobId ;
};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOB_H

