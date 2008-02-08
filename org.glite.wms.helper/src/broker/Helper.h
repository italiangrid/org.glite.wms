// File: Helper.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_HELPER_BROKER_HELPER_H
#define GLITE_WMS_HELPER_BROKER_HELPER_H

#include "glite/wms/helper/HelperImpl.h"
#include <boost/shared_ptr.hpp>

namespace glite {
namespace wms {
namespace helper {
namespace broker {

class NoCompatibleCEs;

class Helper: public helper::HelperImpl
{
public:

  std::string id() const;
  std::string output_file_suffix() const;
  classad::ClassAd* resolve(
    classad::ClassAd const* input_ad,
    boost::shared_ptr<std::string> jw_template = boost::shared_ptr<std::string>()
  ) const;
};

}}}} // glite::wms::helper::broker

#endif
