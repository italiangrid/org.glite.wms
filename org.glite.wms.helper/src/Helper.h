// File: Helper.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_HELPER_HELPER_H
#define GLITE_WMS_HELPER_HELPER_H

#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif
#ifndef GLITE_WMS_X_BOOST_UTILITY_HPP
#define GLITE_WMS_X_BOOST_UTILITY_HPP
#include <boost/utility.hpp>
#endif
#include <boost/shared_ptr.hpp>

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace helper {

class HelperImpl;

class Helper: boost::noncopyable
{
  HelperImpl* m_impl;

public:
  Helper(std::string const& id);
  ~Helper();

  std::string id() const;

  std::string resolve(std::string const& input_file) const;
  classad::ClassAd* resolve(
    classad::ClassAd const* input_ad,
      boost::shared_ptr<std::string> jw_template = boost::shared_ptr<std::string>()
  ) const;
};
 
}}} // glite::wms::helper

#endif // GLITE_WMS_HELPER_HELPER_H
