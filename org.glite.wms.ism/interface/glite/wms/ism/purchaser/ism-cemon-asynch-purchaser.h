// File: ism-cemon-purchaser.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_ISM_CEMON_ASYNCH_PURCHASER_H
#define GLITE_WMS_ISM_PURCHASER_ISM_CEMON_ASYNCH_PURCHASER_H

#include <string>
#include <vector>
#include "glite/wms/ism/purchaser/common.h"
#include "glite/wms/ism/purchaser/ism-purchaser.h"

class CEConsumer;

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

class ism_cemon_asynch_purchaser : public ism_purchaser
{
public:
                
  ism_cemon_asynch_purchaser(
    std::string const& topic,
    int listening_port = 5120,
    exec_mode_t mode = loop,
    size_t interval = 30,
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = skip_predicate_type()
  );

  void do_purchase();

  void operator()();

private:
  int parse_classad_event_messages(boost::shared_ptr<CEConsumer>, gluece_info_container_type&);

private:                
  std::string m_topic;
  int m_listening_port;
  std::vector<std::string> m_multi_attributes;
};

namespace cemon_asynch {
// the types of the class factories
typedef ism_cemon_asynch_purchaser* create_t(
    std::string const& topic,
    int listening_port = 5120,
    exec_mode_t mode = loop,
    size_t interval = 30,
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = skip_predicate_type()
);
typedef void destroy_t(ism_cemon_asynch_purchaser*);
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
