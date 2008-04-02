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
    std::string const& ldap_ce_filter_ext = std::string(),
    bool ldap_search_async = false,
    exec_mode_t mode = loop,
    size_t interval = 30,
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = false_	
  );
  
  void do_purchase();
  void operator()();

private:                
  std::string m_hostname;
  int m_port;
  std::string m_dn;
  int m_timeout;
  std::string m_ldap_ce_filter_ext;
  bool m_ldap_search_async;
};

class ism_ii_purchaser_entry_update
{
public:
  ism_ii_purchaser_entry_update() {}
  bool operator()(int a, boost::shared_ptr<classad::ClassAd>& ad);
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
    skip_predicate_type skip_predicate = false_
  );

typedef void destroy_t(ism_ii_purchaser*);

// type of the entry update function factory
typedef boost::function<bool(int&, boost::shared_ptr<classad::ClassAd>)> create_entry_update_fn_t();
}

}}}}

#endif
