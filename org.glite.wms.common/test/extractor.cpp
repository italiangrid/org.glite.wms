#include <unistd.h>

#include <string>
#include <iostream>

#include "../src/utilities/FileList.h"
#include "../src/utilities/FileListLock.h"
#include "../src/utilities/Extractor.h"

#include "queuecommon.h"

using namespace std;

USING_COMMON_NAMESPACE;

int main( void )
{
  size_t      size;
  string      robo;
  utilities::FileList<string>         flist( queuefile );
  utilities::FileListDescriptorMutex  listmutex( flist );
  utilities::FileListLock             locker( listmutex, false );
  utilities::FileList<string>::iterator                 flIt;
  utilities::ForwardExtractor<utilities::FileList<string> >    extractor( flist );

  try {
    while( true ) {
      locker.lock();

      if( extractor->empty() ) cout << "Nothing written..." << endl;
      else {
	do {
	  flIt = extractor.get_next();
	  cout << "Read (" << *flIt << ")" << endl;
	  extractor.erase( flIt );
	} while( !extractor->empty() );
      }

      locker.unlock();
      sleep( 2 );
    }
  }
  catch( utilities::FileContainerError &error ) {
    cerr << "Got error: " << error.string_error() << endl;
  }

  return( 0 );
}
