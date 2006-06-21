// File: HelperImpl.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_HELPER_HELPERIMPL_H
#define GLITE_WMS_HELPER_HELPERIMPL_H

#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif
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

class HelperImpl: boost::noncopyable
{
public:
  HelperImpl();
  virtual ~HelperImpl();

  std::string resolve(std::string const& input_file) const;

  virtual std::string id() const = 0;
  virtual std::string output_file_suffix() const = 0;
  virtual classad::ClassAd* resolve(classad::ClassAd* input_ad) const = 0;
};

} // namespace helper
} // namespace wms
} // namespace glite

#endif
