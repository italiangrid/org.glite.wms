// File: ism-purchaser.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_ISM_PURCHASER_H
#define GLITE_WMS_ISM_PURCHASER_ISM_PURCHASER_H

#include <string>
 
namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

class ism_purchaser
{
public:
  ism_purchaser(exec_mode_t mode, size_t interval) :
	m_mode(mode), m_interval(interval) {}
  virtual ~ism_purchaser() {}
  virtual void do_purchase() = 0;
  virtual void operator()() = 0;

  exec_mode_t exec_mode() const
  {
    return m_mode;
  }
  size_t sleep_interval() const
  {
    return m_interval;
  }

protected:               
  exec_mode_t m_mode;
  size_t m_interval;
};

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
