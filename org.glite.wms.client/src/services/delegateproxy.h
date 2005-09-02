#ifndef GLITE_WMS_CLIENT_SERVICES_DELEGATEPROXY_H
#define GLITE_WMS_CLIENT_SERVICES_DELEGATEPROXY_H

// inheritance
#include "job.h"
// options utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"
// LOG
#include "utilities/logman.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class DelegateProxy : public Job {

	public :
		/*
		*	Default constructor
		*/
		DelegateProxy ( );
                /*
		*	Default destructor
		*/
		~DelegateProxy( ) ;
		/*
		*	reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /*
		*	performs the operations to submit the job(s)
		*/
		void delegation ( ) ;
	private :
        	/*
                * Saves the result of the DelegateProxy operation result to the outputfile specified by .--output option
                * @return the info message on the pathname of the file; an empty string in case of error
                */
        	std::string DelegateProxy::infoToFile( );

		/*
                *	string input arguments
                */
		std::string* dgOpt ;
		/*
                *	boolean input arguments
                */
                bool autodgOpt ;
};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

