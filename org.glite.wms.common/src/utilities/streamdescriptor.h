#ifndef EDG_WORKLOAD_COMMON_UTILITIES_STREAMDESCRIPTOR_H
#define EDG_WORKLOAD_COMMON_UTILITIES_STREAMDESCRIPTOR_H

#include <fstream>

#include "../common_namespace.h"

#if (__GNUC__>=3)
#include <ext/stdio_filebuf.h>
#endif

COMMON_NAMESPACE_BEGIN {

namespace utilities {

#if (__GNUC__>=3)

inline int streamdescriptor( const std::fstream &fs )
{
  return static_cast<__gnu_cxx::stdio_filebuf<char> *>( fs.rdbuf() )->fd();
}

inline int streamdescriptor( const std::ifstream &ifs )
{
  return static_cast<__gnu_cxx::stdio_filebuf<char> *>( ifs.rdbuf() )->fd();
}

inline int streamdescriptor( const std::ofstream &ofs )
{
  return static_cast<__gnu_cxx::stdio_filebuf<char> *>( ofs.rdbuf() )->fd();
}

inline int bufferdescriptor( const std::filebuf &fb )
{
  return static_cast<__gnu_cxx::stdio_filebuf<char> *>( const_cast<std::filebuf *>(&fb) )->fd();
}

inline void create_file( const char *name )
{
  std::fstream    fs( name, std::ios::in );

  if( !fs.good() ) std::ofstream( name, std::ios::out );

  return;
}

#else

inline int streamdescriptor( const std::fstream &fs )
{
  return fs.rdbuf()->fd();
}

inline int streamdescriptor( const std::ifstream &ifs )
{
  return ifs.rdbuf()->fd();
}

inline int streamdescriptor( const std::ofstream &ofs )
{
  return ofs.rdbuf()->fd();
}

inline int bufferdescriptor( const std::filebuf &fb )
{
  return fb.fd();
}

inline void create_file( const char *name ) { return; }

#endif

}; // Namespace utilities

} COMMON_NAMESPACE_END;

#endif /* EDG_WORKLOAD_COMMON_UTILITIES_STREAMDESCRIPTOR_H */

// Local Variables:
// mode: c++
// End:
