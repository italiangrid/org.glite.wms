#include <iostream>

#include "../src/utilities/FileList.h"

USING_COMMON_NAMESPACE;

using namespace std;

int main( int argc, char *argv[] )
{
  int       arg;
  utilities::FileList<string>            flist;
  utilities::FileList<string>::iterator  flIt;

  try {
    for( arg = 1; arg < argc; arg++ ) {
      cout << "Opening file: " << argv[arg] << endl;

      flist.open( argv[arg] );
      cout << "\tRead size: " << flist.size() << " (" << flist.size() << ")" << endl;

      for( flIt = flist.begin(); flIt != flist.end(); ++flIt )
	cout << "\tRead: (" << *flIt << ")" << endl;

      //      cout << "End = " << *flIt << endl;

      flist.close();
      cout << "File " << argv[arg] << " closed..." << endl;
    }
  }
  catch( utilities::FileContainerError &error ) {
    cerr << "Error saw: " << error.string_error() << endl;
    return (1);
  }

  return( 0 );
}
