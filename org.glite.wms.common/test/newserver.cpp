#include <unistd.h>

#include <iostream>

#include "../src/utilities/FileList.h"
#include "../src/utilities/FileListLock.h"
#include "../src/utilities/Extractor.h"

#include "queuecommon.h"

using namespace std;

USING_COMMON_NAMESPACE;

typedef  utilities::FileList<string>     list_type;

int main( void )
{
  int                                       ret = 0;
  string                                    buffer;
  utilities::FileList<string>               mylist( queuefile );
  utilities::FileList<string>::iterator     myIt;
  utilities::ForwardExtractor<list_type>    myextractor( mylist );
  utilities::FileListDescriptorMutex        listmutex( mylist );
  utilities::FileListLock                   locker( listmutex, false );

  while( true ) {
    locker.lock(); myIt = myextractor.get_next();

    if( myIt != mylist.end() ) {
      locker.unlock();
      cout << "Got: \"" << *myIt << "\"" << endl;

      locker.lock();
      myextractor.erase( myIt );
      locker.unlock();
    }
    else {
      cout << "Nothing new..." << endl;
      locker.unlock();
    }

    sleep( 5 );
  }

  return ret;
}
