// File: WMReal.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "WMReal.h"
#include <algorithm>
#include <cctype>
#include "WMFactory.h"
#include "lb_utils.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"

#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/scope_guard.h"
#include "glite/wms/helper/Request.h"
#include "JobController.h"
#include "glite/lb/producer.h"
#include "TaskQueue.hpp"

#include "plan.h"

namespace utilities = glite::wms::common::utilities;
namespace jobid = glite::wmsutils::jobid;
namespace requestad = glite::wms::jdl;
namespace common = glite::wms::manager::common;
namespace controller = glite::wms::jobsubmission::controller;

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

std::string wm_id("Real");

std::string normalize(std::string id)
{
  std::transform(id.begin(), id.end(), id.begin(), ::tolower);
  return id;
}

common::WMImpl* create_wm()
{
  return new WMReal;
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

void Deliver(
  classad::ClassAd const& ad,
  common::ContextPtr const& context
)
{
  edg_wll_Context c_context = context.get();
  controller::JobController(&c_context).submit(&ad);
}

void Cancel(glite::wmsutils::jobid::JobId const& id)
{
  edg_wll_Context c_context = get_context(id).get();
  controller::JobController(&c_context).cancel(id);
}

void log_match(
  glite::wmsutils::jobid::JobId const& job_id,
  common::ContextPtr const& context,
  char const* ce_id
)
{
  int lb_error = edg_wll_LogMatch(context.get(), ce_id); 
  if (lb_error != 0) {
    Warning("edg_wll_LogMatch failed for " << job_id
            << " (" << common::get_lb_message(context) << ")");
  }
}

} // {anonymous}

void
WMReal::submit(classad::ClassAd const* request_ad_p)
{
  if (!request_ad_p) {
    Error("request ad is null");
    return;
  }

  // make a copy because we change the sequence code, see below
  classad::ClassAd request_ad(*request_ad_p);

  glite::wmsutils::jobid::JobId jobid(requestad::get_edg_jobid(request_ad));

  common::ContextPtr context = get_context(jobid);

  // update the sequence code before doing the planning
  // some helpers may need a more recent sequence code than what appears in
  // the classad originally received

  std::string sequence_code(common::get_lb_sequence_code(context));
  requestad::remove_lb_sequence_code(request_ad);
  requestad::set_lb_sequence_code(request_ad, sequence_code);

  boost::scoped_ptr<classad::ClassAd> planned_ad(Plan(request_ad));
  std::string ce_id = requestad::get_ce_id(*planned_ad);
  log_match(jobid, context, ce_id.c_str());

  Deliver(*planned_ad, context);
}

void
WMReal::cancel(glite::wmsutils::jobid::JobId const& request_id)
{
  Cancel(request_id);
}

}}}} // glite::wms::manager::server
