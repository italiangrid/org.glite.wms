
#include <iostream>
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"
// utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"

#include "services/jobloginfo.h"

using namespace std ;
using namespace glite::wms::client::utilities ;
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
		Options opts (Options::JOBMATCH) ;
		opts.readOptions(argc, (const char**)argv);



	} catch (exception &ex) {
		cout << flush << ex.what() << "\n" ;
	}

};
