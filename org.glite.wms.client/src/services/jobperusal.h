/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
* 	Authors:	Alessandro Maraschini <alessandro.maraschini@datamat.it>
* 			Marco Sottilaro <marco.sottilaro@datamat.it>
*
*	$Id: DAGAd.cpp,v 1.11 2005/07/04 14:57:17 amarasch Exp
*/

#ifndef GLITE_WMS_CLIENT_SERVICES_JOBPERUSAL_H
#define GLITE_WMS_CLIENT_SERVICES_JOBPERUSAL_H

// inheritance
#include "job.h"
#include "lbapi.h"
// utilities
#include "utilities/options_utils.h"
// WMProxy API's
#include "glite/wms/wmproxyapi/wmproxy_api.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

/**
* Type of  Operations
*/
enum perusalOperations {
	PERUSAL_NONE,
	PERUSAL_GET,
	PERUSAL_SET,
	PERUSAL_UNSET
};

class JobPerusal  : public Job {

	public :
        	/*
                *	Default constructor
                */
		JobPerusal ( );
                 /*
                * Default destructror
                */
                ~JobPerusal();
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
               void readOptions ( int argc,char **argv)   ;

		void jobPerusal ( ) ;

        private :
		void getPerusal (std::vector <std::string> &paths) ;

		void setPerusal ( ) ;
		void JobPerusal::unsetPerusal ( );

		void checkStatus( );
		void gsiFtpGetFiles (const std::vector <std::string> &uris, std::vector<std::string> &paths, std::string &errors) ;

		void printResult(const perusalOperations &operation, std::vector<std::string> &paths);

		/*
		* Retrieves the output files
		*/
		//bool retrieveFiles (std::ostringstream &msg, const std::string& jobid, const std::string& dirAbs, const bool &child = false);
        	/*
 		* checks the status of the jobs
		* @param jobids the list of identifiers of the jobs of which the status has to be retrieved
 		*/
		//void checkStatus( std::vector<std::string> &jobids);


        	 /*
                *	String input arguments
                */
		std::string* inOpt ; 		// --input <file>
		std::string* outOpt ; 	// --ouput <path>
		std::string* dirOpt ; 		// --dir <path>
		 /*
                *	Boolean input arguments
                */
		bool getOpt ;
		bool setOpt ;
		bool unsetOpt ;
		bool allOpt ;
                bool nodisplayOpt ;
		bool nointOpt;
		/**
		* JobID
		*/
		std::string jobId ;
                /**
                * List of the files
                */
                std::vector<std::string> peekFiles ;

  };
}}}} // ending namespaces

#endif //GLITE_WMS_CLIENT_SERVICES_JOBPERUSAL_H

