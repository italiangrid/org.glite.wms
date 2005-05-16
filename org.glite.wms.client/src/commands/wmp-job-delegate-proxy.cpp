
#include "iostream"
// utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// exceptions
#include "utilities/excman.h"

#include "services/delegateproxy.h"

using namespace std ;
using namespace glite::wms::client::services ;
using namespace glite::wms::client::utilities ;
using namespace glite::wmsutils::exception;
/*
*	main
*/
int main (int argc,char **argv){
	DelegateProxy job ;
        // reads the user options
	try {
		job.readOptions(argc, argv);
        } catch (WmsClientException &ex) {
                cerr << "\nError : error on input parameters" << endl;
                cerr << flush << ex.what() << endl;
                job.printUsageMsg(argv[0]);
        } catch (Exception &ex) {
                cerr << "\nError : error on input parameters" << endl;
                cerr << flush <<  ex.what() << endl ;
                job.printUsageMsg(argv[0]);
        }
        // performs the main operations
        try {
                job.delegation( );
	} catch (WmsClientException &ex) {
        	cerr << "\nError : unable to delegate the user credential" << endl;
		cerr << flush <<  ex.what() << endl;
	} catch (Exception &ex) {
		cerr << "\nError : unable to delegate the user credential" << endl;
		cerr << flush << ex.what() << endl ;
	}
	return 0;
};
