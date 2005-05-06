
#include <iostream>
// utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// exceptions
#include "utilities/excman.h"

#include "services/jobsubmit.h"

using namespace std ;
using namespace glite::wms::client::services ;
using namespace glite::wms::client::utilities ;
/*
*	main
*/
int main (int argc,char **argv){
	try {
		JobSubmit job ;
		// reads the user options
		job.readOptions(argc, argv);
                // performs the main operations
                job.submission( );
	} catch (WmsClientException &ex) {
		cerr << flush << ex.what() << "\n" ;
	}
	return 0;
};
