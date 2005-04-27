#include <utilities/options_utils.h>
#include <iostream>
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"

#include "services/joboutput.h"

using namespace std ;
using namespace glite::wms::wmproxyapi;

/*
*	main
*/
int main (int argc,char **argv){

	ConfigContext *cfs = NULL;
	string *endpoint = NULL ;
	string *del_id ;
	string jdl_string = "";
	try {
		// reads the user options
		Options opts (Options::JOBOUTPUT) ;
		opts.readOptions(argc, (const char**)argv);
/*
		// init of the context
		cfs = new ConfigContext("", *endpoint, GLITE_TRUSTED_CERTS);
		jobid = jobSubmit ( jdl_string , *del_id, cfs);
*/

	} catch (exception &ex) {
		cout << flush << ex.what() << "\n" ;
	}

};
