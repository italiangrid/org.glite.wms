#include <unistd.h>

#include <string>
#include <iostream>

#include <classad_distribution.h>

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
  utilities::FileList<classad::ClassAd>            flist( queuefile );
  utilities::FileListDescriptorMutex               listmutex( flist );
  utilities::FileListLock                          locker( listmutex, false );
  utilities::FileList<classad::ClassAd>::iterator  flIt;
  utilities::ForwardExtractor<utilities::FileList<classad::ClassAd> >    extractor( flist );

  try {
    while( true ) {
      while( true ) {
	locker.lock();
	flIt = extractor.get_next();

	if( flIt != extractor->end() ) break;
	else {
	  locker.unlock();
	  sleep( 2 );
	}
      }

      cout << "Read (" << *flIt << ")" << endl;
      locker.unlock();

      sleep( 2 );

      locker.lock(); extractor.erase( flIt ); locker.unlock();
    }
  }
  catch( utilities::FileContainerError &error ) {
    cerr << "Got error: " << error.string_error() << endl;
  }

  return( 0 );
}
