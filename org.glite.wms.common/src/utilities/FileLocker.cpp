#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "FileLocker.h"

using namespace std;

COMMON_NAMESPACE_BEGIN {

namespace utilities {

CannotOpenLockFile::CannotOpenLockFile( int error ) : string( strerror(error) ) {}

CannotOpenLockFile::~CannotOpenLockFile( void ) {}

FileMutex::FileMutex( const string &filename ) : fm_locked( false ), fm_descriptor( -1 ), fm_mutex()
{
  string   lockfile( filename );

  lockfile.append( ".lock" );

  this->fm_descriptor = ::open( lockfile.c_str(), (O_RDONLY | O_CREAT) );
  if( this->fm_descriptor == -1 ) throw CannotOpenLockFile( errno );
}

FileMutex::~FileMutex( void )
{
  if( this->fm_descriptor != -1 ) ::close( this->fm_descriptor );
}

FileLocker::FileLocker( FileMutex &fm, bool lock ) : fl_locked( &fm.fm_locked ), fl_filelock( fm.fm_descriptor, false ),
						     fl_mutexlock( fm.fm_mutex, false )
{
  if( lock ) {
    if( *this->fl_locked ) throw FileMutexLocked();

    this->lock();
  }
}

FileLocker::~FileLocker( void )
{
  if( *this->fl_locked ) this->unlock();
}

int FileLocker::lock( void )
{
  int     answer = 0;

  if( *this->fl_locked ) throw FileMutexLocked();
  else {
    answer = this->fl_filelock.lock();
    if( !answer ) {
      this->fl_mutexlock.lock();

      *this->fl_locked = true;
    }
  }

  return( answer );
}

int FileLocker::unlock( void )
{
  int    answer = 0;

  if( *this->fl_locked ) {
    this->fl_mutexlock.unlock();

    answer = this->fl_filelock.unlock();
    if( !answer ) *this->fl_locked = false;
  }

  return( answer );
}

}; // Namespace utilities

} COMMON_NAMESPACE_END;

