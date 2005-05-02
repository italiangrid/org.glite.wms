#ifndef GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H
#define GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

#include "utilities/options_utils.h"

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
		*	performs the operations to submit the job(s)
		*/
		void submission ( ) ;
		/*
		*	reads the command-line user arguments
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;


	private :

		std::string* configvo;
		std::string* logfile ;
		std::string* chkpt ;
		std::string* lmrs ;
		std::string* to ;
		std::string* ouput ;
		std::string* input ;
		std::string* config ;
		std::string* resource ;
		std::string* valid ;

		bool nomsg ;
		bool nogui ;
		bool nolisten ;
		bool noint ;
		bool version ;
		bool help ;

		glite::wms::client::utilities::Options *opts ;
		/*
		*	path to the JDL file
		*/
		std::string jdlFile ;
		/*
		*	string of the user JDL
		*/
		std::string jdlString ;
		/*
		*
		*/

};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

