// File: Helper.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_HELPER_JOBADAPTER_HELPER_H
#define GLITE_WMS_HELPER_JOBADAPTER_HELPER_H

#include <boost/shared_ptr.hpp>
#include "glite/wms/helper/HelperImpl.h"

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {

class Helper: public helper::HelperImpl
{
public:
  std::string id() const;
  std::string output_file_suffix() const;
  classad::ClassAd* resolve(
    classad::ClassAd const* input_ad,
    boost::shared_ptr<std::string> jw_template
  ) const;
};

} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite 

#endif
