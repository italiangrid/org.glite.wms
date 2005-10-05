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

#ifndef GLITE_WMS_CLIENT_SERVICES_JOBOUTPUT_H
#define GLITE_WMS_CLIENT_SERVICES_JOBOUTPUT_H

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

class JobOutput  : public Job {

	public :
        	/*
                *	Default constructor
                */
		JobOutput ( );
                 /*
                * Default destructror
                */
                ~JobOutput();
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
               void readOptions ( int argc,char **argv)   ;
		/*
                *	performs the main operations
                */
		void getOutput ( ) ;

        private :
		int  retrieveOutput(std::ostringstream &msg, Status& status, const std::string& dirAbs, const bool &child=false);
		/*
		* Retrieves the output files
		*/
		bool retrieveFiles (std::ostringstream &msg, const std::string& jobid, const std::string& dirAbs, const bool &child = false);
        	/*
 		* checks the status of the jobs
		* @param jobids the list of identifiers of the jobs of which the status has to be retrieved
 		*/
		void checkStatus( std::vector<std::string> &jobids);
                /*
 		*	prints the results on the std output
 		*/
		void  listResult(std::vector <std::pair<std::string , long> > &files, const std::string jobid, const bool &child = false );
		/*
		*	Creates a list of warnings/errors to be used at the end of the execution
		*	@param msg the message to be added
		*/
		void createWarnMsg(const std::string &msg ) ;
#if defined(WITH_GRID_FTP_API) || defined(WITH_GRID_FTP)
		 /*
                *	file downloading by gsiftp
                *	@param files list of files to be downloaded and their destination
                */
                void gsiFtpGetFiles (std::vector <std::pair<std::string , std::string> > &paths) ;
  #else
  		/*
		*	struct for files
		*/
		struct httpfile { char *filename; FILE* stream; } ;
                /*
                * 	writing callback for curl operations
                */
                static int storegprBody(void *buffer, size_t size, size_t nmemb, void *stream);
                /*
                *	file downloading by curl
                *	@param files list of files to be downloaded and their destination
                */
                void JobOutput::curlGetFiles (std::vector <std::pair<std::string , std::string> > &paths) ;
  #endif
        	 /*
                *	string input arguments
                */
		std::string* inOpt ; 	// --input <file>
		std::string* dirOpt ; 	// --dir <dir_path>

		std::string logName;  // string to append to directory
		// OutputStorage configuration value
		std::string dirCfg ; 	
                /*
                *	boolean input arguments
                */
                bool listOnlyOpt ;
                /*
                * JobId's
                */
                std::vector<std::string> jobIds ;

		std:: vector<std::string> outDirs ;

		std::string jobId ;

		std::string childrenFileList ;
		std::string parentFileList ;

		bool isDag ;
		bool hasFiles ;

		bool successRt;
		std::string* warnsList ;

  };
}}}} // ending namespaces

#endif //GLITE_WMS_CLIENT_SERVICES_JOBOUTPUT_H

