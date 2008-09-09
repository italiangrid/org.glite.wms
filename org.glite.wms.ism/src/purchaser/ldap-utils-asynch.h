// File: ldap-utils.h
// Author: Salvatore Monforte
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_II_PURCHASER_LDAP_UTILS_H
#define GLITE_WMS_II_PURCHASER_LDAP_UTILS_H

#include <string>
#include "glite/wms/ism/purchaser/common.h"

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

class LDAPException: public std::exception
{
  std::string m_error;
public:
  LDAPException(std::string const& error)
    : m_error(error)
  {
  }
  ~LDAPException() throw()
  {
  }
  virtual char const* what() const throw()
  {
    return m_error.c_str();
  }
};

namespace async {
void
fetch_bdii_ce_info(
  std::string const& hostname,
  size_t port,
  std::string const& basedn,
  time_t timeout,
  std::string const& ldap_ce_filter_ext,
  PurchaserInfoContainer&
);

void
fetch_bdii_se_info(
  std::string const& hostname,
  size_t port,
  std::string const& basedn,
  time_t timeout,
  PurchaserInfoContainer&
);

void 
fetch_bdii_info(
  std::string const& hostname,
  size_t port,
  std::string const& basedn,
  time_t timeout,
  const std::string& ldap_ce_filter_ext,
  PurchaserInfoContainer&,
  PurchaserInfoContainer&
);

}

}}}}

#endif
