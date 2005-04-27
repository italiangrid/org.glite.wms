#include <utilities/options_utils.h>
#include <iostream>

using namespace std ;

int main (int argc,char **argv){
	try {
		Options opts (Options::JOBSTATUS) ;
		opts.readOptions(argc, (const char**)argv);
		string* config = opts.getStringAttribute(Options::CONFIG) ;
		string* vo = opts.getStringAttribute(Options::VO) ;
		bool help = opts.getBoolAttribute (Options::HELP);
		if (config ){
			cout << "config=" << *config << endl ;
		}
		if (vo){
			cout << "vo=" << *vo << endl ;
		}

		cout << "HELP=" << help << endl ;
	} catch (exception &ex) {
		cout << flush << ex.what() << "\n" ;
	}

};
