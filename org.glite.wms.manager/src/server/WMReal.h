// File: WMReal.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef EDG_WORKLOAD_PLANNING_MANAGER_WMREAL_H
#define EDG_WORKLOAD_PLANNING_MANAGER_WMREAL_H

#ifndef EDG_WORKL0AD_PLANNING_MANAGER_WMIMPL_H
#include "edg/workload/planning/manager/WMImpl.h"
#endif

namespace edg {
namespace workload {
namespace planning {
namespace manager {

template<typename PlanningPolicy, typename DeliveryPolicy, typename CancellingPolicy>
class WMReal:
    public WMImpl,
    public PlanningPolicy,
    public DeliveryPolicy,
    public CancellingPolicy
{

public:
  WMReal();
  ~WMReal();

  void submit(classad::ClassAd const* request_ad);
  void resubmit(common::jobid::JobId const& request_id);
  void cancel(common::jobid::JobId const& request_id);
};

}}}} // edg::workload::planning::manager

#include "edg/workload/planning/manager/WMReal.tcc"

#endif // EDG_WORKLOAD_PLANNING_MANAGER_WMREAL_H
