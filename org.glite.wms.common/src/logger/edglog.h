#ifndef GLITE_WMS_COMMON_LOGGER_EDGLOG_H
#define GLITE_WMS_COMMON_LOGGER_EDGLOG_H

#include "logstream_ts.h"

COMMON_NAMESPACE_BEGIN {

  namespace logger { namespace threadsafe {
    extern logstream  edglog;
  }}; // Namespace threadsafe & logger

} COMMON_NAMESPACE_END;

#define RenameLogStreamNS_ts( ts )  namespace ts = glite::wms::common::logger::threadsafe

#endif /* GLITE_WMS_COMMON_LOGGER_EDGLOG_H */

// Local Variables:
// mode: c++
// End:
