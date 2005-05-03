
#include <iostream>
// utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"

#include "services/jobstatus.h"

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;


int main (int argc,char **argv){
/*
	ConfigContext *cfs = NULL;
	string *endpoint = NULL ;
	string *del_id ;
*/
	vector<string> jobids;
	//try {
		// reads the user options
		Options opts (Options::JOBSTATUS) ;
		opts.readOptions(argc, (const char**)argv);
		// gets the jobids
		jobids = opts.getJobIds( );
/*
	} catch (exception &ex) {
		cout << flush << ex.what() << "\n" ;
	}
*/
};
