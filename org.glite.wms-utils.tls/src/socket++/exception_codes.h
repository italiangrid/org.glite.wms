#ifndef  EDG_WORKLOAD_COMMON_SOCKETPP_EXCEPTION_CODES_H
#define EDG_WORKLOAD_COMMON_SOCKETPP_EXCEPTION_CODES_H
/*
 * exception_codes.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */

#include "glite/wms/common/utilities/exception_codes.h"

namespace edg {
namespace workload {
namespace common {
namespace socket_pp {

enum {
      SOCKET_FATAL_ERROR = edg::workload::common::utilities::WL_SOCKET_ERROR_BASE +1 ,
      SOCKET_IO_ERROR ,
      SOCKET_AUTHORIZATION_ERROR ,
      SOCKET_AUTHENTICATION_ERROR
};

} // namespace socket_pp
} // namespace common
} // namespace workload
} // namespace edg
#endif

// EOF
