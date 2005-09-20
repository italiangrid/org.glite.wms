// File: ism-ii-purchaser.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_ISM_II_PURCHASER_H
#define GLITE_WMS_ISM_PURCHASER_ISM_II_PURCHASER_H

#include <string>
#include "glite/wms/ism/purchaser/common.h"
#include "glite/wms/ism/purchaser/ism-purchaser.h"
 
namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

class ism_ii_purchaser : public ism_purchaser
{
public:
                
  ism_ii_purchaser(
    std::string const& hostname,
    int port,
    std::string const& distinguished_name,
    int timeout = 30,
    size_t interval = 30,
    skip_predicate_type skip_predicate = skip_predicate_type()	
  );
  
  void do_purchase();

  void operator()();

  update_function_type update_function() const;


private:                
  std::string m_hostname;
  int m_port;
  std::string m_dn;
  int m_timeout;
};

class ism_ii_purchaser_entry_update
{
public:
  ism_ii_purchaser_entry_update(std::string const& id, std::string const& hn, int p, std::string const& dn, int timeout) :
    m_id(id), m_ldap_server(hn), m_ldap_port(p), m_ldap_dn(dn), m_ldap_timeout(timeout)  {}
  bool operator()(int a,boost::shared_ptr<classad::ClassAd>& ad);
private:
  std::string m_id;
  std::string m_ldap_server;
  int m_ldap_port;
  std::string m_ldap_dn;
  int m_ldap_timeout;
};

namespace ii {
// the types of the class factories
typedef ism_ii_purchaser* create_t(std::string const& hostname,
    int port,
    std::string const& distinguished_name,
    int timeout = 30,
    exec_mode_t mode = loop,
    size_t interval = 30,
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = skip_predicate_type()
  );

typedef void destroy_t(ism_ii_purchaser*);
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
