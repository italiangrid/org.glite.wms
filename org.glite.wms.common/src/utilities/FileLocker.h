#ifndef EDG_WORKLOAD_COMMON_UTILITIES_FILELOCKER_H
#define EDG_WORKLOAD_COMMON_UTILITIES_FILELOCKER_H

#include <cstdio>

#include <string>

#include <boost/thread/mutex.hpp>

#include "common_namespace.h"

#include "fstreamlock.h"

COMMON_NAMESPACE_BEGIN {

namespace utilities {

class CannotOpenLockFile : public std::string {
public:
  CannotOpenLockFile( int );

  ~CannotOpenLockFile( void );
};

class FileMutexLocked {};

class FileLocker;

class FileMutex {
  friend class FileLocker;

public:
  FileMutex( const std::string &filename );

  ~FileMutex( void );

private:
  FileMutex( const FileMutex &fm ); // Not implemented
  FileMutex &operator=( const FileMutex &fm ); // Not implemented

  bool           fm_locked;
  int            fm_descriptor;
  boost::mutex   fm_mutex;
};

class FileLocker {
public:
  FileLocker( FileMutex &fm, bool lock = true );

  ~FileLocker();

  int lock( void );
  int unlock( void );

private:
  FileLocker( void ); // Not implemented
  FileLocker( const FileLocker &fl ); // Not implemented
  FileLocker &operator=( const FileLocker &fl ); // Not implemented

  bool                       *fl_locked;
  DescriptorLock              fl_filelock;
  boost::mutex::scoped_lock   fl_mutexlock;
};

}; // Namespace utilities

} COMMON_NAMESPACE_END;

#endif /* EDG_WORKLOAD_COMMON_UTILITIES_FILELOCKER_H */

// Local Variables:
// mode: c++
// End:
