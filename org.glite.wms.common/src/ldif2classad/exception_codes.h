#ifndef  EDG_WORKLOAD_COMMON_LDIF2CLASSAD_EXCEPTION_CODES_H
#define EDG_WORKLOAD_COMMON_LDIF2CLASSAD_EXCEPTION_CODES_H
/*
 * exception_codes.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */

#include "../utilities/exception_codes.h"

namespace edg {
namespace workload {
namespace common {
namespace ldif2classad {

enum {
      LDAP_FATAL_ERROR = edg::workload::common::utilities::WL_LDAP_ERROR_BASE +1 ,
      LDAP_CONNECTION_ERROR,
      LDAP_QUERY_ERROR,
      LDAP_UNDEFINED_VALUE_ERROR
};

} // namespace ldif2classad
} // namespace common
} // namespace workload
} // namespace edg
#endif

// EOF
