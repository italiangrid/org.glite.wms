// File: Helper.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef EDG_WORKLOAD_PLANNING_JOBADAPTER_HELPER_H
#define EDG_WORKLOAD_PLANNING_JOBADAPTER_HELPER_H

#include "edg/workload/planning/helper/HelperImpl.h"

namespace edg {
namespace workload {
namespace planning {
namespace jobadapter {

class Helper: public helper::HelperImpl
{
public:

  std::string id() const;
  std::string output_file_suffix() const;
  classad::ClassAd* resolve(classad::ClassAd const* input_ad) const;
};

}}}} // edg::workload::planning::jobadapter

#endif
