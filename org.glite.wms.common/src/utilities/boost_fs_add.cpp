#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <memory>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

using namespace std;

namespace boost { namespace filesystem {

string normalize_path( const string &fpath )
{
  string                   modified;
  string::const_iterator   last, next;
  string::reverse_iterator check;

  last = fpath.begin();
  do {
    next = find( last, fpath.end(), '/' );

    if( next != fpath.end() ) {
      modified.append( last, next + 1 );

      for( last = next; *last == '/'; ++last );
    }
    else modified.append( last, fpath.end() );
  } while( next != fpath.end() );

  check = modified.rbegin();
  if( *check == '/' ) modified.assign( modified.begin(), modified.end() - 1 );

  return modified;
}

void create_parents( const path &dpath )
{
  string     err( "create_parent(): " );
  path       branch( dpath.branch() );

  if( dpath.is_null() ) {
    err.append( "cannot create an empty path." );

    throw filesystem_error( err );
  }
  else if( !exists(dpath) ) {
    if( branch.is_null() ) create_directory( dpath );
    else if( !exists(branch) ) {
      create_parents( branch );
      create_directory( dpath );
    }
    else if( is_directory(branch) ) create_directory( dpath );
    else {
      err.append( branch.file_path() ); err.append( " is not a directory." );

      throw filesystem_error( err );
    }
  }
  else if( !is_directory(dpath) ) {
    err.append( dpath.file_path() ); err.append( " is not a directory." );

    throw filesystem_error( err );
  }

  return;
}

streampos file_size( const path &file )
{
  bool               exist = true;
  streampos          size = 0;
  string             err( "file_size(): " );
  auto_ptr<fstream>  fs;

  if( (exist = exists(file)) && !is_directory(file) ) {
    fs.reset( new fstream(file.file_path().c_str(), ios::in) );

    fs->seekg( 0, ios::end );
    size = fs->tellg();
  }
  else {
    err.append( file.file_path() );

    if( exist ) err.append( " is a directory." );
    else err.append( " does not exist." );

    throw filesystem_error( err );
  }

  return size;
}

}};
