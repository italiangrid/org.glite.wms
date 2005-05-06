
#ifndef GLITE_WMS_CLIENT_SERVICES_JOBSTATUS_H
#define GLITE_WMS_CLIENT_SERVICES_JOBSTATUS_H

namespace glite {
namespace wms{
namespace client {
namespace services {

class JobStatus {

	public :
        	/*
		*	default constructor
		*/
		JobStatus ( );
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
};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBSTATUS_H
