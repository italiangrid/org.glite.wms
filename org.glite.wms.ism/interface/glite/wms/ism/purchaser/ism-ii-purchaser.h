// File: ism-ii-purchaser.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_ISM_II_PURCHASER_H
#define GLITE_WMS_ISM_PURCHASER_ISM_II_PURCHASER_H

#include <string>
 
namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

class ism_ii_purchaser
{
public:
  enum exec_mode_t {
    once,
    loop
  }; 
                
  ism_ii_purchaser(
    std::string const& hostname,
    int port,
    std::string const& distinguished_name,
    int timeout = 30,
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
  std::string m_hostname;
  int m_port;
  std::string m_dn;
  int m_timeout;
  exec_mode_t m_mode;
  size_t m_interval;
};
                
} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
