#ifndef EDG_WORKLOAD_COMMON_LOGGER_LOGBUF_H
#define EDG_WORKLOAD_COMMON_LOGGER_LOGBUF_H

#include <iostream>

#include "common.h"

COMMON_NAMESPACE_BEGIN {

namespace logger {

class Logbuf : public std::streambuf {
public:
  Logbuf( void );
  Logbuf( const char *filename, level_t lev, const char *format = data_c::bd_s_timeFormat );
  Logbuf( std::streambuf *buffer, level_t lev, const char *format = data_c::bd_s_timeFormat );

  virtual ~Logbuf( void );

  Logbuf *open( const char *name, level_t lev, const char *format = data_c::bd_s_timeFormat );
  Logbuf *open( std::streambuf *buffer, level_t lev, const char *format = data_c::bd_s_timeFormat );
  Logbuf *close( void );
  Logbuf *activate_log_rotation( std::streamsize maxsize, const std::string &basename, unsigned int maxfiles );
  Logbuf *deactivate_log_rotation( void );
  int log_rotate( void );

  inline void show_severity( bool show ) { this->lb_data.show_severity( show ); }
  inline void multiline( bool multi, const char *prefix = NULL ) { this->lb_data.multiline( multi, prefix ); }
  inline void next_level( level_t lev ) { this->lb_data.next_level( lev ); }
  inline void buffer_level( level_t lev ) { this->lb_data.buffer_level( lev ); }
  inline void function( const char *func ) { this->lb_data.function( func ); }
  inline void time_format( const char *format ) { this->lb_data.time_format( format ); }
  inline void clear_function( void ) { this->lb_data.clear_function(); }
  inline void reset_container( DataContainerImpl *dc ) { this->lb_data.reset_container( dc ); }
  inline bool bad( void ) { return this->lb_data.bad(); }
  inline DataContainerImpl *container( void ) { return this->lb_data.container(); }

protected:
  virtual int overflow( int ch );
  virtual int sync( void );

private:
  void writeBuffer( std::streamoff n );
  bool checkRotationBuffer( void );
  int internalSync( bool );
  std::streamsize getBufferSize( void );

  Logbuf( const Logbuf &lb ); // Not implemented
  Logbuf &operator=( const Logbuf &lb ); // Not implemented

  static const char  *lb_s_letterLevels;

  bool                lb_remove, lb_rotate;
  unsigned int        lb_maxfiles;
  std::streamsize     lb_current, lb_maxsize;
  std::streambuf     *lb_buffer;
  std::string         lb_basename;
  data_c              lb_data;
};

}; // Namespace logger

} COMMON_NAMESPACE_END;

#endif /* Guard */

// Local Variables:
// mode: c++
// End:
