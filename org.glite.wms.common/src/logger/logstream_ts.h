#ifndef GLITE_WMS_COMMON_LOGGER_LOGSTREAM_TS_H
#define GLITE_WMS_COMMON_LOGGER_LOGSTREAM_TS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRINGSTREAM
#include <sstream>
#else
#include <strstream>
#endif

#include "logstream.h"
#include "container_ts.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

namespace glite { namespace wms { namespace common { namespace logger { namespace threadsafe {
  class logstream;
}}}}};

#define __log_t   glite::wms::common::logger::threadsafe::logstream
namespace std {
  typedef __log_t  &(*lts_manip_t)(__log_t &);
}
#undef __log_t

COMMON_NAMESPACE_BEGIN {

namespace logger { namespace threadsafe {

class logstream {
private:
#ifdef HAVE_STRINGSTREAM
  typedef    std::ostringstream       OutStream;
#else
  class OutStream : public std::ostrstream {
  public:
    OutStream( void );
    ~OutStream( void );

    std::string str( void );
  };
#endif

public:
  logstream( void );
  logstream( const char *name, level_t lev = medium, const char *format = data_c::bd_s_timeFormat );
  logstream( const std::string &name, level_t lev = medium, const char *format = data_c::bd_s_timeFormat );
  logstream( std::ostream &ostr, level_t lev = medium, const char *format = data_c::bd_s_timeFormat );
  ~logstream( void );

  // Setters
  logstream &show_severity( bool show );
  logstream &next_level( level_t lev );
  logstream &current_level( level_t lev );
  logstream &function( const char *func );
  logstream &time_format( const char *format );
  logstream &clear_function( void );

  void open( const char *name, level_t lev = medium, const char *format = data_c::bd_s_timeFormat );
  void open( std::ostream &ostr, level_t lev = medium, const char *format = data_c::bd_s_timeFormat );
  void close( void );
  void unsafe_attach( logger::logstream &ls );
  void activate_log_rotation( std::streamsize maxsize, const std::string &basename, unsigned int maxfiles );
  void deactivate_log_rotation( void );
  void log_rotate( void );
  logstream &endl( void );
  logstream &ends( void );
  logstream &flush( void );

  template <class Type>
  logstream &operator<<( const Type &t );

  inline void open( const std::string &name, level_t lev = medium, const char *format = data_c::bd_s_timeFormat )
  { this->open( name.c_str(), lev, format ); }
  inline void reset( std::ostream &ostr, level_t lev = medium, const char *format = data_c::bd_s_timeFormat )
  { this->open( ostr, lev, format ); }
  inline bool multi( void ) { return this->tl_multi; }
  inline Logbuf *logbuf( void ) { return (Logbuf *)(this->tl_stream.rdbuf()); }
  inline logstream &operator<<( std::lts_manip_t func ) { return (*func)( *this ); }

protected:
  logstream( const logstream &ls ); // Not implemented
  logstream &operator=( const logstream &ls ); // Not implemented

  inline void checkBuffer( void )
  { if( this->tl_buffer.get() == NULL ) this->tl_buffer.reset( new OutStream() ); }

  bool                                    tl_multi;
  DataContainerImpl                      *tl_container;
  logger::logstream                       tl_stream;
  boost::mutex                            tl_mutex;
  boost::thread_specific_ptr<OutStream>   tl_buffer;
};

template <class Type>
logstream &logstream::operator<<( const Type &t )
{
  this->checkBuffer();

  *this->tl_buffer << t;

  return *this;
}

}}; // closing namespaces

} COMMON_NAMESPACE_END;

#define __log_t   glite::wms::common::logger::threadsafe::logstream
namespace std {
  typedef __log_t  &(*lts_manip_t)(__log_t &);

  inline __log_t &endl( __log_t &ll ) { return ll.endl(); }
  inline __log_t &ends( __log_t &ll ) { return ll.ends(); }
  inline __log_t &flush( __log_t &ll ) { return ll.flush(); }
}
#undef __log_t

#endif /* GLITE_WMS_COMMON_LOGGER_LOGSTREAM_TS_H */

// Local Variables:
// mode: c++
// End:
