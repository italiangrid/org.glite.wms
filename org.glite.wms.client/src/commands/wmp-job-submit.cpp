
#include <iostream>
// utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"
// exceptions
#include "utilities/excman.h"

#include "services/jobsubmit.h"

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::client::services ;
using namespace glite::wms::wmproxyapi;

/*
*	main
*/
int main (int argc,char **argv){

	ConfigContext *cfs = NULL;
	string *endpoint = NULL ;
	string *del_id ;
	string jdl = "";
	try {
		JobSubmit js ;
		js.readOptions(argc, argv );

	} catch ( WmsClientException&ex) {
		cout << flush << ex.what() << "\n" ;
	}

};
