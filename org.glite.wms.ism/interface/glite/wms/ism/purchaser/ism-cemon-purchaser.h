// File: ism-cemon-purchaser.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_ISM_CEMON_PURCHASER_H
#define GLITE_WMS_ISM_PURCHASER_ISM_CEMON_PURCHASER_H

#include <string>
#include <vector>

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

class ism_cemon_purchaser
{
public:
  enum exec_mode_t {
    once,
    loop
  }; 
                
  ism_cemon_purchaser(
    std::vector<std::string> const& service,
    std::string const& topic,
    int rate = 30,
    exec_mode_t mode = loop,
    size_t interval = 30
  );

  void operator()();

  exec_mode_t exec_mode() const
  {
    return m_mode;
  }
  size_t sleep_interval() const
  {
    return m_interval;
  }

private:                
  std::string m_topic;
  int m_rate;
  exec_mode_t m_mode;
  size_t m_interval;
  std::vector<std::string> m_multi_attributes;
  std::vector<std::string> m_services;
};
                
} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
