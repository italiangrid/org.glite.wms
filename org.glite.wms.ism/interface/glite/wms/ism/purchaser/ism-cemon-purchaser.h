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
    std::vector<std::string> const& service,
    std::string const& topic,
    int rate = 30,
    exec_mode_t mode = loop,
    size_t interval = 30
  );

  void operator()();

private:                
  std::string m_topic;
  int m_rate;
  std::vector<std::string> m_multi_attributes;
  std::vector<std::string> m_services;
};

namespace cemon {
// the types of the class factories
typedef ism_cemon_purchaser* create_t(std::vector<std::string> const& service,
    std::string const& topic,
    int rate = 30,
    exec_mode_t mode = loop,
    size_t interval = 30
);
typedef void destroy_t(ism_cemon_purchaser*);
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
