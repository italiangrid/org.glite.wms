// File: Request.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_HELPER_REQUEST_H
#define GLITE_WMS_HELPER_REQUEST_H

#ifndef GLITE_WMS_X_BOOST_UTILITY_HPP
#define GLITE_WMS_X_BOOST_UTILITY_HPP
#include <boost/utility.hpp>
#endif

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace helper {

class Request: boost::noncopyable
{
  class Impl;

  Impl* m_impl;

public:
  Request(classad::ClassAd const* ad);
  ~Request();
  void resolve();
  bool is_resolved() const;
  classad::ClassAd const* original_ad() const;
  classad::ClassAd const* current_ad() const;
};

inline
classad::ClassAd const*
resolved_ad(Request const& request)
{
  return request.is_resolved() ? request.current_ad() : 0;
}

} // namespace helper
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_HELPER_REQUEST_H

