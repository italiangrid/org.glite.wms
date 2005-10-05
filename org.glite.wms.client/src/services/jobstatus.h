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


#ifndef GLITE_WMS_CLIENT_SERVICES_JOBSTATUS_H
#define GLITE_WMS_CLIENT_SERVICES_JOBSTATUS_H
// inheritance
#include "job.h"

// inheritance
#include "job.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class JobStatus: public Job  {

	public :
        	/*
		*	Default constructor
		*/
		JobStatus ( );
        	/*
		*	Default destructor
		*/
                ~JobStatus( );
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /*
		*	performs the main operations
		*/
                void getStatus( ) ;

	private :
        	/*
                * Prints the result info on the standard output
                */
        	void JobStatus::printInfo( );
        	/*
                * String Options
                */
		std::string* inOpt;
		std::string* fromOpt ;
		std::string* toOpt ;
		std::string* statusOpt;
                std::string* exdOpt ;
		/*
                * Boolean options
                */
		bool allOpt ;
                /*
                * Integer options
                */
                unsigned int *vbOpt ;
		/*
                * Options: List of Usertag's
                */
                std::vector<std::string> userTags ;
                /*
                * List of jobid's
                */
                std::vector<std::string> jobIds ;

};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBSTATUS_H
