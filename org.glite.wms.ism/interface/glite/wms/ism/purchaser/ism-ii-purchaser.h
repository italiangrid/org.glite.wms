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
    exec_mode_t mode = loop,
    size_t interval = 30
  );

  void operator()();

private:                
  std::string m_hostname;
  int m_port;
  std::string m_dn;
  int m_timeout;
  exec_mode_t m_mode;
  size_t m_interval;
};

namespace ii {
// the types of the class factories
typedef ism_ii_purchaser* create_t(std::string const& hostname,
    int port,
    std::string const& distinguished_name,
    int timeout = 30,
    exec_mode_t mode = loop,
    size_t interval = 30
  );

typedef void destroy_t(ism_ii_purchaser*);
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
