
#include <iostream>
// utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"

#include "services/joboutput.h"

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;

/*
*	main
*/
int main (int argc,char **argv){
	try {
		// reads the user options
		Options opts (Options::JOBOUTPUT) ;
		opts.readOptions(argc, (const char**)argv);

	} catch (exception &ex) {
		cout << flush << ex.what() << "\n" ;
	}

};
