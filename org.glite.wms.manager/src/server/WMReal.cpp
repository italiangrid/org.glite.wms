// File: WMReal.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "WMReal.h"

#include <algorithm>
#include <cctype>

#include "../common/WMFactory.h"

#include "RequestPlanningPolicy.h"
#include "JCDeliveryPolicy.h"
#include "JCCancellingPolicy.h"
#include "dispatching_utils.h"

#include "../common/lb_utils.h"

#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

#include "glite/wmsutils/jobid/JobId.h"

#include "glite/wms/jdl/JobAdManipulation.h"

namespace utilities = glite::wms::common::utilities;
namespace jobid = glite::wmsutils::jobid;
namespace jdl = glite::wms::jdl;
namespace common = glite::wms::manager::common;

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

common::WMFactory::wm_type wm_id("Real");

common::WMFactory::wm_type normalize(common::WMFactory::wm_type const& id)
{
  common::WMFactory::wm_type result(id);
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
      jdl::set_ce_id(*result, "bbq.mi.infn.it:2119/jobmanager-pbs-dque");
    }
    return result;
  }
};

struct FakeDeliveryPolicy
{
  static void Deliver(classad::ClassAd const& ad)
  {
    jobid::JobId id(jdl::get_edg_jobid(ad));
    boost::mutex::scoped_lock l(submit_cancel_mutex());
    common::ContextPtr context_ptr = common::get_context(id);
    if (common::unregister_context(id)) {
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

common::WMImpl* create_wm()
{
  namespace configuration = glite::wms::common::configuration;

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
    common::WMFactory::instance()->register_wm(normalize(wm_id), create_wm);
  }
  ~Register()
  {
    common::WMFactory::instance()->unregister_wm(normalize(wm_id));
  }
};

Register r;

}

} // server
} // manager
} // wms
} // glite 

