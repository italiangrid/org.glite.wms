// File: WMReal.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_WMREAL_H
#define GLITE_WMS_MANAGER_SERVER_WMREAL_H

#ifndef GLITE_WMS_MANAGER_SERVER_WMIMPL_H
#include "../common/WMImpl.h"
#endif

namespace jobid = glite::wmsutils::jobid;
namespace common = glite::wms::manager::common;

namespace glite {
namespace wms {
namespace manager {
namespace server {

template<typename PlanningPolicy, typename DeliveryPolicy, typename CancellingPolicy>
class WMReal:
    public common::WMImpl,
    public PlanningPolicy,
    public DeliveryPolicy,
    public CancellingPolicy
{

public:
  WMReal();
  ~WMReal();

  void submit(classad::ClassAd const* request_ad);
  void resubmit(jobid::JobId const& request_id);
  void cancel(jobid::JobId const& request_id);
};

} // server
} // manager
} // wms
} // glite

#include "WMReal.tcc"

#endif // GLITE_WMS_MANAGER_SERVER_WMREAL_H
