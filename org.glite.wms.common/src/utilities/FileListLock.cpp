/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org\partners for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileListLock.h"
#include "streamdescriptor.h"

namespace glite {
namespace wms {
namespace common {
namespace utilities {

FileListDescriptorMutex::FileListDescriptorMutex( _file_sequence_t &fs ) : fldm_locked( false ), 
  fldm_descriptor( streamdescriptor(fs.fs_container.get_stream()) )
{}

FileListDescriptorMutex::~FileListDescriptorMutex( void ) {}

FileListMutex::FileListMutex( _file_sequence_t &fs ) : FileListDescriptorMutex( fs ), flm_mutex() {}

FileListMutex::~FileListMutex( void ) {}

FileListLock::FileListLock( FileListDescriptorMutex &fldm, bool lock ) : fl_locked( &fldm.fldm_locked ), fl_mutexlock( NULL ),
									 fl_filelock( fldm.fldm_descriptor, lock )
{
  *this->fl_locked = lock;
}

FileListLock::FileListLock( FileListMutex &flm, bool lock ) : fl_locked( &flm.fldm_locked ), 
fl_mutexlock( new boost::mutex::scoped_lock( flm.flm_mutex, lock ) ),
fl_filelock( flm.fldm_descriptor, lock )
{
  //this->fl_mutexlock = new boost::mutex::scoped_lock( flm.flm_mutex, lock );
  *this->fl_locked = lock;
}

FileListLock::~FileListLock( void )
{
  this->unlock();
  if( this->fl_mutexlock ) delete this->fl_mutexlock;
}

int FileListLock::lock( void )
{
  int    res = 0;

//  if( !*this->fl_locked ) {
//    res = this->fl_filelock.lock();
//    if( (res == 0) && this->fl_mutexlock ) this->fl_mutexlock->lock();
//
//    if( res == 0 ) *this->fl_locked = true;
//  }

  if ( this->fl_mutexlock ) this->fl_mutexlock->lock();

  res = this->fl_filelock.lock();

  if( res == 0 ) {
    *this->fl_locked = true;
  } else {
    *this->fl_locked = false;
    if ( this->fl_mutexlock ) this->fl_mutexlock->unlock();
  }

  return( res );
}

int FileListLock::unlock( void )
{
  int    res = -1;

  if( *this->fl_locked ) {
//    if( this->fl_mutexlock ) this->fl_mutexlock->unlock();

    res = this->fl_filelock.unlock();

    if( res == 0 ) *this->fl_locked = false;
  }

  return( res );
}

} // utilities namespace
} // common namespace
} // wms namespace
} // glite namespace
