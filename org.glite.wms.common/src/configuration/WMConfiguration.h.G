// File: WMConfiguration.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id: WMConfiguration.h.G,v 1.4 2003/03/13 11:48:53 peluso Exp $

#ifndef EDG_WORKLOAD_COMMON_CONFIGURATION_WMCONFIGURATION_H
#define EDG_WORKLOAD_COMMON_CONFIGURATION_WMCONFIGURATION_H

#ifndef EDG_WORKLOAD_COMMON_CONFIGURATION_CONFBASE_H
#include "edg/workload/common/configuration/confbase.h"
#endif

namespace classad {
class ClassAd;
}

namespace edg {
namespace workload {
namespace common {
namespace configuration {

class WMConfiguration: public confbase_c
{
public:
  WMConfiguration(classad::ClassAd const* ad);
  ~WMConfiguration();

  /* Tmpl: partial include class WMConfiguration */
};

}}}} // edg::workload::common::configuration

#endif

// Local Variables:
// mode: c++
// End:
