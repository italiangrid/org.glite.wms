// File: ldap-utils.h
// Author: Salvatore Monforte
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_LDAP_UTILS_H
#define GLITE_WMS_ISM_PURCHASER_LDAP_UTILS_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include "glite/wms/ism/purchaser/common.h"

namespace glite {
namespace wms {

namespace common {
namespace ldif2classad {
class LDIFObject;
class LDAPConnection;
}
}

namespace ldif2classad = common::ldif2classad;

namespace ism {
namespace purchaser {

std::string get_cluster_name(ldif2classad::LDIFObject& ldif_CE);
void tokenize_ldap_dn(std::string const& s, std::vector<std::string> &v);
bool is_gluecluster_info_dn(std::vector<std::string> const& dn);
bool is_gluece_info_dn(std::vector<std::string> const& dn);
bool is_gluesubcluster_info_dn(std::vector<std::string> const& dn);
bool is_gluecesebind_info_dn(std::vector<std::string> const& dn);

void
fetch_bdii_info(
  boost::shared_ptr<ldif2classad::LDAPConnection> IIconnection,
  gluece_info_container_type& gluece_info_container
);

void 
fetch_bdii_info(
  std::string const& hostname,
  int port,
  std::string const& dn,
  int timeout,
  gluece_info_container_type& gluece_info_container
);


} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
