#ifndef EDG_WORKLOAD_COMMON_UTILITIES_BOOST_FS_ADD_H
#define EDG_WORKLOAD_COMMON_UTILITIES_BOOST_FS_ADD_H

#include <string>
#include <iostream>

/**
 *  This file adds some functions to the boost::filesystem
 *  namespace, so it does not follow the EDG naming directives.
 */

namespace boost { namespace filesystem {

class path;

void create_parents( const path &dpath );
std::streampos file_size( const path &file );
std::string normalize_path( const std::string &fpath );

}};

#endif /* EDG_WORKLOAD_COMMON_UTILITIES_BOOST_FS_ADD_H */

// Local Variables:
// mode: c++
// End:
