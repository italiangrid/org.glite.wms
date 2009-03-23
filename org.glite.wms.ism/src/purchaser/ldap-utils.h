// File: ldap-utils.h
// Author: Salvatore Monforte
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_LDAP_UTILS_H
#define GLITE_WMS_ISM_PURCHASER_LDAP_UTILS_H

#include <string>
#include "glite/wms/ism/purchaser/common.h"

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

void
fetch_bdii_ce_info(
  std::string const& hostname,
  size_t port,
  std::string const& basedn,
  time_t timeout,
  const std::string& ldap_ce_filter_ext,
  glite::wms::ism::purchaser::PurchaserInfoContainer&
);

void
fetch_bdii_se_info(
  std::string const& hostname,
  size_t port,
  std::string const& basedn,
  time_t timeout,
  glite::wms::ism::purchaser::PurchaserInfoContainer&
);

void 
fetch_bdii_info(
  std::string const& hostname,
  size_t port,
  std::string const& basedn,
  time_t timeout,
  std::string const& ldap_ce_filter_ext,
  glite::wms::ism::purchaser::PurchaserInfoContainer&,
  glite::wms::ism::purchaser::PurchaserInfoContainer&
);

}}}}

#endif
