/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
* 	Authors:	Alessandro Maraschini <alessandro.maraschini@datamat.it>
* 			Marco Sottilaro <marco.sottilaro@datamat.it>
*
*/

// 	$Id$


#ifndef GLITE_WMS_CLIENT_SERVICES_JOBLISTMATCH_H
#define GLITE_WMS_CLIENT_SERVICES_JOBLISTMATCH_H

// inheritance
#include "job.h"
// options utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// wmproxy API
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"
// Ad's
#include "glite/jdl/Ad.h"
#include "glite/jdl/ExpDagAd.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class JobListMatch : public Job {

	public :
        	/**
		*	Default constructor
		*/
		JobListMatch( );
                /**
		*	Default destructor
		*/
		~JobListMatch( );
		/**
		*	Reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /**
		*	Performs the main operations
		*/
                void listMatching( ) ;

  	private :
                /**
                * Retrieves the list of matching resources
                */
                void JobListMatch::jobMatching( ) ;
        	/**
                *	check the input JDL
                */
        	void checkAd ( ) ;
        	/**
                *	boolean input arguments
                */
                bool rankOpt ;
		/**
                *	Ad
                */
                glite::jdl::Ad *jobAd ;
                /**
		*	Path to the JDL file
		*/
		std::string m_jdlFile ;
		/**
		*	String with the user JDL
		*/
		std::string m_jdlString ;
		/**
		* Failover approach: if a service call fails the client may recover
		* from the reached point contacting another wmproxy endpoint
		*/
		enum listmatchRecoveryStep {
			STEP_LISTMATCH
		};
		/**
		* FailOver Approach: when a problem occurred, recover from a certain step
		* @param step where the listmatching arrived so far
		*/
		void listmatchRecoverStep(listmatchRecoveryStep step);
		/**
		* FailOver Approach: Perform a desired step
		*/
		void listmatchPerformStep(listmatchRecoveryStep step);
		/**
		* List of the matched CEs
		*/
		std::vector <std::pair<std::string , long> > m_listResult_v ;
};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBLISTMATCH_H

