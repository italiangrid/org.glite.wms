/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/***
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
* 	Authors:	Alessandro Maraschini <alessandro.maraschini@datamat.it>
* 			Marco Sottilaro <marco.sottilaro@datamat.it>
*
*/

// 	$Id$

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
        	/**
                * Default constructor
                */
		JobOutput ( );
                 /**
                * Default destructror
                */
                ~JobOutput();
		/**
		*	Reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
               void readOptions ( int argc,char **argv)   ;
		/**
                *	Performs the main operations
                */
		void getOutput ( ) ;

        private :
		int  retrieveOutput(std::string &result, Status& status, const std::string& dirAbs, bool firstCall, const bool &child=false);
		/**
		* Retrieves the output files
		*/
		bool retrieveFiles (std::string &result, std::string &errors, const std::string& jobid, const std::string& dirAbs, const bool &child = false);
        	/**
 		* Checks the status of the jobs
		* @param jobids the list of identifiers of the jobs of which the status has to be retrieved
 		*/
		void checkStatus( std::vector<std::string> &jobids);
                /**
 		*	Prints the results on the std output
 		*/
		void  listResult(std::vector <std::pair<std::string , long> > &files, const std::string jobid, const bool &child = false );
		/**
		* Creates a list of warnings/errors to be used at the end of the execution
		* @param msg the message to be added
		*/
		void createWarnMsg(const std::string &msg ) ;
                /**
                * File downloading with globus-url-copy
                * @param files list of files to be downloaded and their destination
		* @param errors string with messages of errors occurred during the operations
                */
		void gsiFtpGetFiles (std::vector <std::pair<std::string , std::string> > &paths, std::string &errors) ;
                /**
                * File downloading with htcp
                * @param paths list of files to be downloaded and their destination
		* @param errors string with messages of errors occurred during the operations
                */
		void htcpGetFiles (std::vector <std::pair<std::string , std::string> > &paths, std::string &errors) ;
        	 /**
                *	String input arguments
                */
		std::string m_inOpt ; 	// --input <file>
		std::string m_dirOpt ; 	// --dir <dir_path>
		std::string m_logName;  // string to append to directory
		std::string m_dirCfg ; 	// OutputStorage configuration value
                /**
                *	Boolean input arguments
                */
	bool m_ok_gsiftp ;
        bool m_listOnlyOpt ;
        bool m_json;
	bool m_pprint;
		bool m_nopgOpt ;
		bool m_firstCall ;
		bool m_nosubdir ;
                /**
                * JobId's
                */
                std::vector<std::string> m_jobIds ;
		/**
		* Info on the  files of the child nodes
		*/
		std::string m_childrenFileList ;
		/**
		* Info on the  files of parent nodes
		*/
		std::string m_parentFileList ;
		std::string m_fileList ;
		/**
		* Boolean Flag: TRUE if the job has out-file to be retrieved
		*/
		bool m_hasFiles ;
		/**
		* Boolean Flag: TRUE if the operation has been successfully performed
		*/
		bool m_successRt;
		/**
		* List of warning messages
		*/
		std::string m_warnsList ;
  };
}}}} // ending namespaces

#endif //GLITE_WMS_CLIENT_SERVICES_JOBOUTPUT_H

