// File: scope_guard.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// (adapted from a version taken from the boost mailing list)
// Copyright (c) 2003 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef EDG_WORKLOAD_COMMON_UTILITIES_SCOPE_GUARD_H
#define EDG_WORKLOAD_COMMON_UTILITIES_SCOPE_GUARD_H

#include <boost/function.hpp>
#include <boost/utility.hpp>

namespace edg {
namespace workload {
namespace common {
namespace utilities {

class scope_guard: private boost::noncopyable
{
  boost::function<void()> m_f;
  bool m_dismissed;

public:
  explicit scope_guard(boost::function<void()> const& v )
    : m_f(v), m_dismissed(false) 
  {
  }

  ~scope_guard() // throw()
  {
    if (!m_dismissed) {
      try {
        m_f();
      } catch (...) {
      }
    }
  }

  void dismiss() // throw() 
  {
    m_dismissed = true; 
  }

};

}}}} // edg::workload::common::utilities

#endif

// Local Variables:
// mode: c++
// End:
