#include <utilities/options_utils.h>
#include <iostream>

// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"

#include "services/jobattach.h"

using namespace std ;

int main (int argc,char **argv){
	try {
		Options opts (Options::JOBATTACH) ;
		opts.readOptions(argc, (const char**)argv);

	} catch (exception &ex) {
		cout << flush << ex.what() << "\n" ;
	}

};
