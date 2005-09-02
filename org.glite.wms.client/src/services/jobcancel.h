#ifndef GLITE_WMS_CLIENT_SERVICES_JOBCANCEL_H
#define GLITE_WMS_CLIENTSERVICES_JOBCANCEL_H

// inheritance
#include "job.h"
// utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// WMProxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"
// LOG
#include "utilities/logman.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class JobCancel : public Job {

	public :
		/*
                *	Default constructor
                */
                JobCancel( );
                /*
                *	Default destructor
                */
                ~JobCancel( );
		/*
		*	Reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
                void readOptions (int argc,char **argv) ;
		/*
                *	Performs the cancelling of the job(s)
                */
		void cancel ( ) ;
	private:
        	/*
                *	string input arguments
                */
		std::string* inOpt ;
                /*
                * JobId's
                */
                std::vector<std::string> jobIds ;

};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBCANCEL_H

