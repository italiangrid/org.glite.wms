/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://project.eu-egee.org/partners for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// File: ism-ii-gris-purchaser.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2004 EU DataGrid.

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_ISM_II_GRIS_PURCHASER_H
#define GLITE_WMS_ISM_PURCHASER_ISM_II_GRIS_PURCHASER_H

#include <string>
#include "glite/wms/ism/purchaser/common.h"
#include "glite/wms/ism/purchaser/ism-purchaser.h"
 
namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

class ism_ii_gris_purchaser : public ism_purchaser
{
public:
                
  ism_ii_gris_purchaser(
    std::string const& hostname,
    int port,
    std::string const& distinguished_name,
    int timeout = 30,
    exec_mode_t mode = loop,
    size_t interval = 30,
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = skip_predicate_type()	
  );
  
  void operator()();

private:                
  std::string m_hostname;
  int m_port;
  std::string m_dn;
  int m_timeout;
};

class ism_ii_gris_purchaser_entry_update
{
public:
  ism_ii_gris_purchaser_entry_update(std::string const& id, std::string const& hn, int p, std::string const& dn, int timeout) :
    m_id(id), m_ldap_server(hn), m_ldap_dn(dn), m_ldap_port(p), m_ldap_timeout(timeout)  {}
  bool operator()(int a,boost::shared_ptr<classad::ClassAd>& ad);
private:
  std::string m_id;
  std::string m_ldap_server;
  std::string m_ldap_dn;
  int m_ldap_port;
  int m_ldap_timeout;
};

namespace ii_gris {
// the types of the class factories
typedef ism_ii_gris_purchaser* create_t(std::string const& hostname,
    int port,
    std::string const& distinguished_name,
    int timeout = 30,
    exec_mode_t mode = loop,
    size_t interval = 30,
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = skip_predicate_type()
  );

typedef void destroy_t(ism_ii_gris_purchaser*);

// type of the entry update function factory
typedef boost::function<bool(int&, ad_ptr)> create_entry_update_fn_t(std::string const&, 
  std::string const&, int, std::string const&, int);
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
