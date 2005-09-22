// File: ism-cemon-purchaser.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_ISM_CEMON_PURCHASER_H
#define GLITE_WMS_ISM_PURCHASER_ISM_CEMON_PURCHASER_H

#include <string>
#include <vector>
#include "glite/wms/ism/purchaser/common.h"
#include "glite/wms/ism/purchaser/ism-purchaser.h"

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

class ism_cemon_purchaser : public ism_purchaser
{
public:
                
  ism_cemon_purchaser(
    std::string const& certfile,
    std::string const& certpath,
    std::vector<std::string> const& service,
    std::string const& topic,
    int rate = 30,
    exec_mode_t mode = loop,
    size_t interval = 30,
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = skip_predicate_type()
  );

  void do_purchase();
  void operator()();

private:
  std::string m_certfile;
  std::string m_certpath;              
  std::string m_topic;
  int m_rate;
  std::vector<std::string> m_multi_attributes;
  std::vector<std::string> m_services;
};

class ism_cemon_purchaser_entry_update
{
public:
  ism_cemon_purchaser_entry_update() {}
  bool operator()(int a,boost::shared_ptr<classad::ClassAd>& ad);
};

namespace cemon {
// the types of the class factories
typedef ism_cemon_purchaser* create_t(std::string const& certfile,
    std::string const& certpath,
    std::vector<std::string> const& service,
    std::string const& topic,
    int rate = 30,
    exec_mode_t mode = loop,
    size_t interval = 30,
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = skip_predicate_type()
);
typedef void destroy_t(ism_cemon_purchaser*);

// type of the entry update function factory
typedef boost::function<bool(int&, ad_ptr)> create_entry_update_fn_t();
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
