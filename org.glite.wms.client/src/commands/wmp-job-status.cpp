
#include <iostream>
// utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// exceptions
#include "utilities/excman.h"

#include "services/jobstatus.h"

using namespace std ;
using namespace glite::wms::client::services ;
using namespace glite::wms::client::utilities ;

int main (int argc,char **argv){

	vector<string> jobids;
	try {
		JobStatus job ;
                // reads the user options
		job.readOptions (argc, argv);
                // performs the main operations
                job.getStatus( );

	} catch ( WmsClientException &ex) {
		cerr << flush << ex.what() << "\n" ;
	}

	return 0;
};
