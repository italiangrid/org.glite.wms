// File: ism-purchaser.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_ISM_PURCHASER_H
#define GLITE_WMS_ISM_PURCHASER_ISM_PURCHASER_H

#include <string>
#include <boost/function.hpp>

#include "glite/wms/ism/purchaser/common.h"
 
namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

typedef boost::function<bool(void)> exit_predicate_type;
typedef boost::function<bool(std::string const&)> skip_predicate_type;

class ism_purchaser
{
public:
  ism_purchaser(exec_mode_t mode, 
    size_t interval, 
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = skip_predicate_type()) :
	m_mode(mode), m_interval(interval), m_exit_predicate(exit_predicate), m_skip_predicate(skip_predicate) {}

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

  void exit_predicate(exit_predicate_type const& p) {
    m_exit_predicate = p; 
  }

  void skip_predicate(skip_predicate_type const &p) {
    m_skip_predicate = p;
  }

protected:               
  exec_mode_t m_mode;
  size_t m_interval;
  exit_predicate_type m_exit_predicate;
  skip_predicate_type m_skip_predicate;
};

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
