// File: WMConfiguration.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "edg/workload/common/configuration/WMConfiguration.h"

namespace edg {
namespace workload {
namespace common {
namespace configuration {

WMConfiguration::WMConfiguration(classad::ClassAd const* ad)
  : confbase_c(ad)
{
}

WMConfiguration::~WMConfiguration(void)
{
}

}}}} // edg::workload::common::configuration

