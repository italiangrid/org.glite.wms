#ifndef EDG_WORKLOAD_COMMON_LOGGER_EDGLOG_H
#define EDG_WORKLOAD_COMMON_LOGGER_EDGLOG_H

#include "logstream_ts.h"

COMMON_NAMESPACE_BEGIN {

  namespace logger { namespace threadsafe {
    extern logstream  edglog;
  }}; // Namespace threadsafe & logger

} COMMON_NAMESPACE_END;

#define RenameLogStreamNS_ts( ts )  namespace ts = edg::workload::common::logger::threadsafe

#endif /* EDG_WORKLOAD_COMMON_LOGGER_EDGLOG_H */

// Local Variables:
// mode: c++
// End:
