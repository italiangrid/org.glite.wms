// File: CopyHelper.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_HELPER_COPYHELPER_H
#define GLITE_WMS_HELPER_COPYHELPER_H

#ifndef GLITE_WMS_HELPER_HELPERIMPL_H
#include "glite/wms/helper/HelperImpl.h"
#endif

namespace edg {
namespace workload {
namespace planning {
namespace helper {

class CopyHelper: public HelperImpl
{
public:
  CopyHelper();
  ~CopyHelper();

  virtual helper_type type() const;
  virtual filename_type resolve(filename_type const& input_file) const;
  virtual classad::ClassAd* resolve(classad::ClassAd const* input_ad) const;
};

} // namespace helper
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_HELPER_COPYHELPER_H
