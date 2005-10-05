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


#ifndef GLITE_WMS_CLIENT_SERVICES_JOBLOGINFO_H
#define GLITE_WMS_CLIENT_SERVICES_JOBLOGINFO_H
// inheritance
#include "job.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class JobLogInfo : public Job {


	public :
        	/*
		*	Default constructor
		*/
		JobLogInfo ( );
        	/*
		*	Default destructor
		*/
                ~JobLogInfo( );
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /*
		*	performs the main operations
		*/
                void getLoggingInfo( ) ;

	private :
        	/*
                * Prints the result info on the standard output
                */
        	void printInfo( );
		/*
                * String Options
                */
		std::string* inOpt;
                /*
                * Integer options
                */
                unsigned int *vbOpt ;
                /*
                * List of jobid's
                */
                std::vector<std::string> jobIds ;

};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBLOGINFO_H
