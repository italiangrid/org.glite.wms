// File: Helper.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <fstream>
#include <boost/scoped_ptr.hpp>
#include <classad_distribution.h>
#include "edg/workload/planning/jobadapter/Helper.h"
#include "edg/workload/planning/helper/HelperFactory.h"
#include "edg/workload/planning/jobadapter/JobAdapter.h"

namespace edg {
namespace workload {
namespace planning {
namespace jobadapter {

namespace {

std::string const helper_id("JobAdapterHelper");

helper::HelperImpl* create_helper()
{
  return new Helper;
}

struct Register
{
  Register()
  {
    helper::HelperFactory::instance()->register_helper(helper_id, create_helper);
  }
  ~Register()
  {
    helper::HelperFactory::instance()->unregister_helper(helper_id);
  }
};

Register const r;

std::string const f_output_file_suffix(".jah");

} // {anonymous}

std::string
Helper::id() const
{
  return helper_id;
}

std::string
Helper::output_file_suffix() const
{
  return f_output_file_suffix;
}

classad::ClassAd*
Helper::resolve(classad::ClassAd const* input_ad) const
{
  return JobAdapter(input_ad).resolve();
}

}}}} // edg::workload::planning::jobadapter
