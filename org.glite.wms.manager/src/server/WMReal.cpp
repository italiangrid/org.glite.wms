// File: WMReal.cpp
// Authors: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
//          Marco Cecchi <Marco.Cecchi@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <algorithm>

#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "glite/lb/producer.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/JCConfiguration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileListLock.h"
#include "glite/wms/common/utilities/scope_guard.h"

#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"

#include "glite/wms/helper/Request.h"

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"

#include "glite/security/proxyrenewal/renewal.h"

#include "CommandAdManipulation.h"
#include "TaskQueue.hpp"
#include "plan.h"
#include "WMFactory.h"
#include "WMReal.h"

namespace utilities = glite::wms::common::utilities;
namespace configuration = glite::wms::common::configuration;
namespace jobid = glite::wmsutils::jobid;
namespace requestad = glite::wms::jdl;
namespace common = glite::wms::manager::common;

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

std::string normalize(std::string id)
{
  std::transform(id.begin(), id.end(), id.begin(), ::tolower);
  return id;
}

common::WMImpl* create_wm()
{
  return new WMReal;
}

std::string wm_id("real");

struct Register
{
  Register()
  {
    common::WMFactory::instance()->register_wm(normalize(wm_id), create_wm);
  }
  ~Register()
  {
    common::WMFactory *factory = common::WMFactory::instance();
    factory->unregister_wm(wm_id);
  }
};

Register r;

void log_match(
  common::ContextPtr const& context,
  std::string const& ce_id
)
{
#ifdef GLITE_WMS_HAVE_LBPROXY
  boost::function<
    int(edg_wll_Context, char const*)
  > log_function(edg_wll_LogMatchProxy);
  const std::string log_function_name("edg_wll_LogMatchProxy");
#else
  boost::function<
    int(edg_wll_Context, char const*)
  > log_function(edg_wll_LogMatch);
  const std::string log_function_name("edg_wll_LogMatch");
#endif

  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(log_function, _1, ce_id.c_str()
    ),
    context
  );
  if (lb_error) {
    Warning(
      common::get_logger_message(log_function_name, lb_error, context, ctx)
    );
  }
}

void log_enqueued_start(
  common::ContextPtr const& context,
  std::string const& input_list,
  std::string const& job_id
)
{
#ifdef GLITE_WMS_HAVE_LBPROXY
  boost::function<
    int(edg_wll_Context, char const*, char const*, char const*)
  > log_function(edg_wll_LogEnQueuedSTARTProxy);
  const std::string log_fun_name("edg_wll_LogEnQueuedSTARTProxy");
#else
  boost::function<
    int(edg_wll_Context, char const*, char const*, char const*)
  > log_function(edg_wll_LogEnQueuedSTART);
  const std::string log_fun_name("edg_wll_LogEnQueuedSTART");
#endif
  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(log_function, _1, input_list.c_str(), job_id.c_str(), ""),
    context
  );
  if (lb_error) {
    Warning(
      common::get_logger_message(log_fun_name, lb_error, context, ctx));
  }
}

void log_enqueued_ok(
  common::ContextPtr const& context,
  std::string const& input_list,
  std::string const& job_id
)
{
#ifdef GLITE_WMS_HAVE_LBPROXY
  boost::function<
    int(edg_wll_Context, char const*, char const*, char const*)
  > log_function(edg_wll_LogEnQueuedOKProxy);
  const std::string log_fun_name("edg_wll_LogEnQueuedOKProxy");
#else
  boost::function<
    int(edg_wll_Context, char const*, char const*, char const*)
  > log_function(edg_wll_LogEnQueuedOK);
  const std::string log_fun_name("edg_wll_LogEnQueuedOK");
#endif
  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(log_function, _1, input_list.c_str(), job_id.c_str(), ""),
    context
  );
  if (lb_error) {
    Warning(
      common::get_logger_message(log_fun_name, lb_error, context, ctx)
    );
  }
}

void log_enqueued_fail(
  common::ContextPtr const& context,
  std::string const& input_list,
  std::string const& job_id,
  std::string const& error_descr
)
{
#ifdef GLITE_WMS_HAVE_LBPROXY
  boost::function<
    int(edg_wll_Context, char const*, char const*, char const*)
  > log_function(edg_wll_LogEnQueuedFAILProxy);
  const std::string log_fun_name("edg_wll_LogEnQueuedFAILProxy");
#else
  boost::function<
    int(edg_wll_Context, char const*, char const*, char const*)
  > log_function(edg_wll_LogEnQueuedFAIL);
  const std::string log_fun_name("edg_wll_LogEnQueuedFAIL");
#endif
  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(log_function,
      _1,
      input_list.c_str(),
      job_id.c_str(),
      error_descr.c_str()
    ),
    context
  );
  if (lb_error) {
    Warning(
      common::get_logger_message(log_fun_name, lb_error, context, ctx)
    );
  }
}

std::string
get_user_x509_proxy(jobid::JobId const& jobid)
{
  static std::string const null_string;
  char* c_x509_proxy = NULL;

  int err_code = edg_wlpr_GetProxy(jobid.getId(), &c_x509_proxy);

  if (err_code == 0) {
    return null_string;
  }
  else {
    // currently no proxy is registered if renewal is not requested
    // try to get the original user proxy from the input sandbox
    boost::shared_ptr<char> _c_x509_proxy(c_x509_proxy, ::free);
    configuration::NSConfiguration const * const ns_config
      = configuration::Configuration::instance()->ns();
    assert(ns_config);
    std::string x509_proxy(ns_config->sandbox_staging_path());
    x509_proxy += "/"
      + jobid::get_reduced_part(jobid)
      + "/"
      + jobid::to_filename(jobid)
      + "/user.proxy";

    return x509_proxy;
   }
}

std::string
make_cancel_request(jobid::JobId const& job_id,
  std::string const& sequence_code,
  std::string user_x509_proxy)
{
  boost::scoped_ptr<classad::ClassAd> cmd(
    common::cancel_command_create(job_id.toString(), sequence_code, user_x509_proxy)
  );
  
  return utilities::unparse_classad(*cmd);
}

void
add_cancel_request(
  std::string const& list_name,
  jobid::JobId const& id,
  std::string const& sequence_code
)
{
  try {
    utilities::FileList<std::string> fl(list_name);
    utilities::FileListMutex mx(fl);
    utilities::FileListLock lock(mx);

    fl.push_back(make_cancel_request(id,
      sequence_code,
      get_user_x509_proxy(id))
    );
  } catch(utilities::FileContainerError &error) {
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

  glite::wmsutils::jobid::JobId jobid(requestad::get_edg_jobid(*request_ad_p));
  common::ContextPtr context = get_context(jobid);
  const std::string sequence_code(common::get_lb_sequence_code(context));

  // we make a copy because we change the sequence code
  classad::ClassAd request_ad(*request_ad_p);
  // before doing the planning, some helpers may need a
  // more recent sequence code than what appears in
  // the classad originally received
  requestad::set_lb_sequence_code(request_ad, sequence_code);

  std::auto_ptr<classad::ClassAd> planned_ad(Plan(request_ad));
  const std::string ce_id = requestad::get_ce_id(*planned_ad);
  log_match(context, ce_id);

  const boost::regex cream_ce_id(".+/cream-.+");
  std::string input_list_name;
  if(boost::regex_match(ce_id, cream_ce_id))
  {
    configuration::ICEConfiguration const * const ice_conf
      = configuration::Configuration::instance()->ice();

    input_list_name = ice_conf->input();
  }
  else {
    configuration::JCConfiguration const * const jc_conf
      = configuration::Configuration::instance()->jc();

    input_list_name = jc_conf->input();
  }
  std::string const job_id_str(requestad::get_edg_jobid(*planned_ad));

  log_enqueued_start(context, input_list_name, job_id_str);

  const std::string sequence_code_(common::get_lb_sequence_code(context));
  requestad::set_lb_sequence_code(*planned_ad, sequence_code_);

  try {
    utilities::FileList<std::string> fl(input_list_name);
    utilities::FileListMutex mx(fl);
    utilities::FileListLock lock(mx);

    std::auto_ptr<classad::ClassAd> cmd(
      common::submit_command_create(planned_ad.get())
    );
    if (cmd.get() != 0)
    {
      std::string ad_str = utilities::unparse_classad(*cmd);
      if (!ad_str.empty())
      {
        fl.push_back(ad_str);
        log_enqueued_ok(context, input_list_name, job_id_str);
      }
      // else {
      // error: "cannot add submit request"
      // }
    }

  } catch(utilities::FileContainerError const& error) {
    log_enqueued_fail(
      context, 
      input_list_name, 
      job_id_str,
      error.string_error()
    );
  }
}

void
WMReal::cancel(jobid::JobId const& id)
{
  common::ContextPtr context = get_context(id);
  const std::string sequence_code(common::get_lb_sequence_code(context));

  configuration::ICEConfiguration const * const ice_conf
    = configuration::Configuration::instance()->ice();

  add_cancel_request(ice_conf->input(), id, sequence_code);

  configuration::JCConfiguration const * const jc_conf
    = configuration::Configuration::instance()->jc();

  add_cancel_request(jc_conf->input(), id, sequence_code);
}

}}}}
