#ifndef GLITE_WMS_TLS_SOCKETPP_EXCEPTION_CODES_H
#define GLITE_WMS_TLS_SOCKETPP_EXCEPTION_CODES_H
/*
 * exception_codes.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */

#include "glite/wms/common/utilities/exception_codes.h"

namespace glite {
namespace wms {
namespace tls {
namespace socket_pp {

enum {
      SOCKET_FATAL_ERROR = glite::wms::common::utilities::WMS_SOCKET_ERROR_BASE +1 ,
      SOCKET_IO_ERROR ,
      SOCKET_AUTHORIZATION_ERROR ,
      SOCKET_AUTHENTICATION_ERROR
};

} // namespace socket_pp
} // namespace tls
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_TLS_SOCKETPP_EXCEPTION_CODES_H

// EOF
