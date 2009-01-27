// File: ldap-dn-utils.h
// Author: Salvatore Monforte
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_LDAP_DN_UTILS_H
#define GLITE_WMS_ISM_PURCHASER_LDAP_FN_UTILS_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include "glite/wms/ism/purchaser/common.h"

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

void tokenize_ldap_dn(std::string const&, std::vector<std::string>&);
bool is_gluecluster_info_dn(std::vector<std::string> const&);
bool is_gluece_info_dn(std::vector<std::string> const&);
bool is_gluesubcluster_info_dn(std::vector<std::string> const&);
bool is_gluecesebind_info_dn(std::vector<std::string> const&);
bool is_gluelocation_info_dn(std::vector<std::string> const&);
bool is_gluevoview_info_dn(std::vector<std::string> const&);
bool is_gluese_info_dn(std::vector<std::string> const&);
bool is_gluesa_info_dn(std::vector<std::string> const&);
bool is_gluese_access_protocol_info_dn(std::vector<std::string> const&);
bool is_gluese_control_protocol_info_dn(std::vector<std::string> const&);

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
