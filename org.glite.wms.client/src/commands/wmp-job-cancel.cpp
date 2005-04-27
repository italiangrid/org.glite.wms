#include <utilities/options_utils.h>
#include <iostream>

// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"

#include "services/jobcancel.h"
using namespace std ;
using namespace glite::wms::wmproxyapi;

/*
*	main
*/
int main (int argc,char **argv){

	string jdl_string = "";
	try {
		// reads the user options
		Options opts (Options::JOBCANCEL) ;
		opts.readOptions(argc, (const char**)argv);


	} catch (exception &ex) {
		cout << flush << ex.what() << "\n" ;
	}

};
