// File: WMReal.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "edg/workload/planning/manager/WMReal.h"
#include <algorithm>
#include <cctype>
#include "edg/workload/planning/manager/WMFactory.h"
#include "edg/workload/planning/manager/RequestPlanningPolicy.h"
#include "edg/workload/planning/manager/JCDeliveryPolicy.h"
#include "edg/workload/planning/manager/JCCancellingPolicy.h"
#include "edg/workload/planning/manager/dispatching_utils.h"
#include "edg/workload/planning/manager/lb_utils.h"
#include "edg/workload/planning/common/logger_utils.h"
#include "edg/workload/common/configuration/Configuration.h"
#include "edg/workload/common/configuration/WMConfiguration.h"
#include "edg/workload/common/jobid/JobId.h"
#include "edg/workload/common/requestad/JobAdManipulation.h"

namespace utilities = edg::workload::common::utilities;
namespace jobid = edg::workload::common::jobid;

namespace edg {
namespace workload {
namespace planning {
namespace manager {

namespace {

WMFactory::wm_type wm_id("Real");

WMFactory::wm_type normalize(WMFactory::wm_type const& id)
{
  WMFactory::wm_type result(id);
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

struct FakePlanningPolicy
{
  static classad::ClassAd* Plan(classad::ClassAd const& ad)
  {
    Debug("planning ad (" << &ad << ")" << utilities::unparse_classad(ad));
    classad::ClassAd* result = new classad::ClassAd(ad);
    if (result != 0) {
      common::requestad::set_ce_id(*result, "bbq.mi.infn.it:2119/jobmanager-pbs-dque");
    }
    return result;
  }
};

struct FakeDeliveryPolicy
{
  static void Deliver(classad::ClassAd const& ad)
  {
    jobid::JobId id(common::requestad::get_edg_jobid(ad));
    boost::mutex::scoped_lock l(submit_cancel_mutex());
    ContextPtr context_ptr = get_context(id);
    if (unregister_context(id)) {
      Debug("delivering job " << id << " (" << utilities::unparse_classad(ad) << ")");
    }
  }
};

struct FakeCancellingPolicy
{
  static void Cancel(jobid::JobId const& request_id) {
    Debug("cancelling job " << request_id);
  }
};

WMImpl* create_wm()
{
  namespace configuration = edg::workload::common::configuration;

  configuration::Configuration const* const config
    = configuration::Configuration::instance();

  if (!config) {
    Fatal("empty or invalid configuration");
  }
  configuration::WMConfiguration const* const wm_config = config->wm();
  if (!wm_config) {
    Fatal("empty WM configuration");
  }

  if (wm_config->fake()) {      // fake policies
    return new WMReal<PlanningPolicy, FakeDeliveryPolicy, FakeCancellingPolicy>;
  } else {                      // real policies
    return new WMReal<PlanningPolicy, DeliveryPolicy, CancellingPolicy>;
  }
}

struct Register
{
  Register()
  {
    WMFactory::instance()->register_wm(normalize(wm_id), create_wm);
  }
  ~Register()
  {
    WMFactory::instance()->unregister_wm(normalize(wm_id));
  }
};

Register r;

}

}}}} // edg::workload::planning::manager

