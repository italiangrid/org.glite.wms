/*
 * wmpexception_codes.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */

#ifndef GLITE_WMS_WMPROXY_EXCEPTION_CODES_H
#define GLITE_WMS_WMPROXY_EXCEPTION_CODES_H

#include "glite/wmsutils/exception/exception_codes.h"

//namespace glite {
//namespace wms {
namespace wmproxyname {

enum {
      WMS_FATAL = glite::wmsutils::exception::WMS_NS_ERROR_BASE +1,
      WMS_CONNECTION_ERROR,
      WMS_AUTHENTICATION_ERROR,
      WMS_JOB_NOT_FOUND,
      WMS_JOB_NOT_DONE,
      WMS_EMPTY_JOB_LIST,
      WMS_NO_SUITABLE_RESOURCE,
      WMS_NOT_AUTHORIZED_USER,
      WMS_SANDBOX_IO,
      WMS_NOT_ENOUGH_SPACE,
      WMS_JDL_PARSING,
      WMS_MATCHMAKING,
      WMS_WRONG_COMMAND,
      WMS_NOT_ENOUGH_QUOTA,
      WMS_JOB_SIZE,
      WMS_MKDIR,
      WMS_PROXY_RENEWAL_FAILURE,
      WMS_IS_FAILURE,
      WMS_MULTI_ATTRIBUTE_FAILURE,
      WMS_OPERATION_NOT_ALLOWED, // wmproxy internal
      WMS_INVALID_ARGUMENT,
      WMS_PROXY_ERROR,
      WMS_DELEGATION_ERROR,
      WMS_LOGGING_ERROR,
      WMS_NO_ERROR
};

//} // namespace wmproxy
//} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_EXCEPTION_CODES_H

