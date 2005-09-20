// File: ism-purchaser.h
// Author: Salvatore Monforte
// Author: Francesco Giacomini
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_ISM_PURCHASER_H
#define GLITE_WMS_ISM_PURCHASER_ISM_PURCHASER_H

#include <string>
#include <boost/shared_ptr.hpp>
#include "common.h"

namespace classad {
class ClassAd;
}
typedef boost::shared_ptr<classad::ClassAd> ClassAdPtr;

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

typedef boost::function<bool(void)> exit_predicate_type;
typedef boost::function<bool(std::string const&)> skip_predicate_type;
typedef boost::function<bool(int&, ClassAdPtr)> update_function_type;

class ism_purchaser
{
public:
  ism_purchaser(
    size_t interval,
    skip_predicate_type skip_predicate = skip_predicate_type()
  )
    : m_mode(once), m_interval(interval),
      m_skip_predicate(skip_predicate)
  {
  }
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

  void loop_mode(exit_predicate_type const& p) {
    m_exit_predicate = p;
  }

  void skip_predicate(skip_predicate_type const &p) {
    m_skip_predicate = p;
  }

  virtual update_function_type update_function() const = 0;

protected:
  exit_predicate_type exit_predicate() const
  {
    return m_exit_predicate;
  }

  skip_predicate_type skip_predicate() const
  {
    return m_skip_predicate;
  }

protected:
  exec_mode_t m_mode;
  exit_predicate_type m_exit_predicate;
  size_t m_interval;
  skip_predicate_type m_skip_predicate;
};

}}}}

#endif
