// File: WMConfiguration.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "WMConfiguration.h"

namespace glite {
namespace wms {
namespace common {
namespace configuration {

WMConfiguration::WMConfiguration(classad::ClassAd const* ad)
  : confbase_c(ad)
{
}

WMConfiguration::~WMConfiguration(void)
{
}

}}}} // glite::wms::common::configuration

