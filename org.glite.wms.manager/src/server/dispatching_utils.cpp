// File: dispatching_utils.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "dispatching_utils.h"

#include <sstream>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>

#include "../common/CommandAdManipulation.h"
#include "../common/lb_utils.h"

#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"

#include "glite/lb/context.h"
#include "glite/lb/producer.h"
#include "glite/lb/consumer.h"

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"
#include "glite/wmsutils/jobid/manipulation.h"

#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/FLExtractor.h"
#include "glite/wms/common/utilities/scope_guard.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"

#include "purger.h"

#include "glite/security/proxyrenewal/renewal.h"

namespace jdl = glite::wms::jdl;
namespace jobid = glite::wmsutils::jobid;
namespace task = glite::wms::common::task;
namespace utilities = glite::wms::common::utilities;
namespace configuration = glite::wms::common::configuration;
namespace fs = boost::filesystem;
namespace common = glite::wms::manager::common;

namespace {

boost::mutex f_submit_cancel_mutex;

std::string
get_input_name()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();

  if (!config) {
    Fatal("empty or invalid configuration");
  }
  configuration::WMConfiguration const* const wm_config = config->wm();
  if (!wm_config) {
    Fatal("empty WM configuration");
  }

  return wm_config->input();
}

} // {anonymous}

namespace glite {
namespace wms {
namespace manager {
namespace server {

boost::mutex& submit_cancel_mutex()
{
  return f_submit_cancel_mutex;
}

PPResult preprocess_submit(ClassAdPtr command_ad)
{
  PPResult result = NOTHING_TO_DO;

  if (!common::submit_command_is_valid(*command_ad)) {
    return INVALID_REQUEST;
  }

  classad::ClassAd const* job_ad(common::submit_command_get_ad(*command_ad));
  std::string const jobid_str(jdl::get_edg_jobid(*job_ad));

  jobid::JobId jobid;
  try {
    jobid::JobId const tmp_jobid(jobid_str);
    jobid = tmp_jobid;
  } catch (jobid::JobIdException const& e) {
    Error("Invalid job id " << jobid_str);
    return INVALID_REQUEST;
  }

  Debug("processing job submit for " << jobid);

  utilities::scope_guard storage_guard(
    boost::bind(purger::purgeStorage, jobid, std::string(""))
  );

  utilities::scope_guard proxy_guard(
    boost::bind(edg_wlpr_UnregisterProxy, jobid, static_cast<char const*>(0))
  );

  std::string x509_proxy = jdl::get_x509_user_proxy(*job_ad);
  std::string sequence_code = jdl::get_lb_sequence_code(*job_ad);

  common::ContextPtr context_ptr = common::create_context(jobid, x509_proxy, sequence_code);

  if (!context_ptr) {
    Error("cannot create the LB context for " << jobid);
    return INVALID_REQUEST;
  }

  if (!register_context(jobid, context_ptr)) {
    Error("cannot register context for " << jobid
          << ". A request with the same id is already being processed.");
    return INVALID_REQUEST;
  }

  result = FORWARD_TO_WM;

  utilities::scope_guard context_guard(boost::bind(common::unregister_context, jobid));

  std::string input_name(get_input_name());
  char const* const local_jobid = ""; // not needed because no real local id

  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = lb_log(
    boost::bind(edg_wll_LogDeQueued, _1, input_name.c_str(), local_jobid),
    context_ptr
  );

  if (lb_error != 0) {
    Warning(get_logger_message("edg_wll_LogDeQueued", lb_error, context_ptr, ctx));
  }

  context_guard.dismiss();
  proxy_guard.dismiss();
  storage_guard.dismiss();

  return result;
}

PPResult preprocess_resubmit(ClassAdPtr command_ad)
{
  PPResult result = NOTHING_TO_DO;

  if (!common::resubmit_command_is_valid(*command_ad)) {
    return INVALID_REQUEST;
  }

  std::string jobid_str(common::resubmit_command_get_id(*command_ad));

  jobid::JobId jobid;
  try {
    jobid::JobId const tmp_jobid(jobid_str);
    jobid = tmp_jobid;
  } catch (jobid::JobIdException const& e) {
    Error("Invalid job id " << jobid_str);
    return INVALID_REQUEST;
  }

  Debug("processing job resubmit for " << jobid);

  utilities::scope_guard storage_guard(
    boost::bind(purger::purgeStorage, jobid, std::string(""))
  );

  utilities::scope_guard proxy_guard(
    boost::bind(edg_wlpr_UnregisterProxy, jobid, static_cast<char const*>(0))
  );

  std::string x509_proxy(common::get_user_x509_proxy(jobid));
  std::string sequence_code(common::resubmit_command_get_lb_sequence_code(*command_ad));

  common::ContextPtr context_ptr = common::create_context(jobid, x509_proxy, sequence_code);

  if (!context_ptr) {
    Error("cannot create the LB context for " << jobid);
    return INVALID_REQUEST;
  }

  if (!register_context(jobid, context_ptr)) {
    Error("cannot register context for " << jobid
          << ". A request with the same id is already being processed.");
    return INVALID_REQUEST;
  }

  result = FORWARD_TO_WM;

  utilities::scope_guard context_guard(boost::bind(common::unregister_context, jobid));

  std::string input_name(get_input_name());
  char const* const local_jobid = ""; // not needed because no real local id

  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = lb_log(
    boost::bind(edg_wll_LogDeQueued, _1, input_name.c_str(), local_jobid),
    context_ptr
  );

  context_guard.dismiss();
  proxy_guard.dismiss();
  storage_guard.dismiss();

  return result;
}

PPResult preprocess_cancel(ClassAdPtr command_ad)
{
  PPResult result = NOTHING_TO_DO;

  if (!common::cancel_command_is_valid(*command_ad)) {
    return INVALID_REQUEST;
  }

  std::string jobid_str((common::cancel_command_get_id(*command_ad)));

  jobid::JobId jobid;
  try {
    jobid::JobId const tmp_jobid(jobid_str);
    jobid = tmp_jobid;
  } catch (jobid::JobIdException const& e) {
    Error("Invalid job id " << jobid_str);
    return INVALID_REQUEST;
  }

  Debug("processing job cancel " << jobid);

  utilities::scope_guard storage_guard(
    boost::bind(purger::purgeStorage, jobid, std::string(""))
  );

  utilities::scope_guard proxy_guard(
    boost::bind(edg_wlpr_UnregisterProxy, jobid, static_cast<char const*>(0))
  );

  bool cancel_here = false;
  {
    // to avoid a race condition between the context unregistration and the
    // delivery of the submit request in WM::submit() (see
    // e.g. JCDeliveryPolicy::Deliver())
    boost::mutex::scoped_lock l(submit_cancel_mutex());
    if (common::unregister_context(jobid)) {
      cancel_here = true;
    }
  }

  // need a new context for logging the cancel events
  std::string sequence_code = common::cancel_command_get_lb_sequence_code(*command_ad);
  std::string x509_proxy = common::get_user_x509_proxy(jobid);

  common::ContextPtr context_ptr = common::create_context(jobid, x509_proxy, sequence_code);

  if (!context_ptr) {
    Error("cannot create the LB context for " << jobid);
    return INVALID_REQUEST;
  }

  edg_wll_Context context = *context_ptr;

  if (cancel_here) {

    Debug("cancelling job " << jobid);
    int err; 
    common::ContextPtr ctx;
    boost::tie(err, ctx) = lb_log(
      boost::bind(edg_wll_LogCancelDONE, _1, "SUCCESSFULLY_CANCELLED"),
      context_ptr
    );
    if (err) {
      Warning(get_logger_message("edg_wll_LogCancelDONE", err, context_ptr, ctx));
    }

    // the cleanup is done automatically by the scope guards

  } else {

    int lb_error = edg_wll_LogCancelREQ(context, "CANCELLATION REQUEST"); 
    if (lb_error != 0) {
      Warning("edg_wll_LogCancelREQ failed for " << jobid
              << " (" << common::get_lb_message(context) << ")");
    }

    if (!register_context(jobid, context_ptr)) {
      Error("cannot register context for " << jobid
            << ". Ops, this should never really happen.");
      return INVALID_REQUEST;
    }

    result = FORWARD_TO_WM;

    proxy_guard.dismiss();
    storage_guard.dismiss();
  }

  return result;
}

PPResult preprocess_quit(ClassAdPtr command_ad)
{
  if (common::quit_command_is_valid(*command_ad)) {
    return QUIT;
  } else {
    return INVALID_REQUEST;
  }
}

PPResult preprocess(ClassAdPtr command_ad)
{
  PPResult result = NOTHING_TO_DO;

  std::string command(common::command_get_command(*command_ad));

  std::transform(command.begin(), command.end(), command.begin(), ::tolower);

  if (command == "jobsubmit") {

    result = preprocess_submit(command_ad);

  } else if (command == "jobresubmit") {

    result = preprocess_resubmit(command_ad);

  } else if (command == "jobcancel") {

    result = preprocess_cancel(command_ad);

  } else if (command == "quit") {

    result = preprocess_quit(command_ad);

  }

  return result;
}

class InvalidRequest::Impl
{
public:
  std::string m_what;
  std::string m_str;
};

InvalidRequest::InvalidRequest(std::string const& str)
try {
  m_impl.reset(new Impl);
  m_impl->m_str = str;
} catch (...) {
  m_impl.reset();
}

InvalidRequest::~InvalidRequest() throw()
{
}

std::string
InvalidRequest::str() const
{
  return m_impl ? m_impl->m_str : std::string();
}

char const*
InvalidRequest::what() const throw()
{
  if (m_impl) {
    std::string& w = m_impl->m_what;
    if (w.empty()) {
      w = "InvalidRequest: " + str();
    }
    return w.c_str();
  } else {
    return "InvalidRequest";
  }
}


// return false iff we got the quit command
// throws InvalidRequest if the request does not have the correct format
bool process(const std::string& ad_str,
             PostProcessFunction postprocess,
             task::PipeWriteEnd<pipe_value_type>& write_end)
{
  bool result = true;

  ClassAdPtr command_ad;
  try {
    command_ad.reset(utilities::parse_classad(ad_str));
  } catch (utilities::ClassAdError const& e) {
    Error(e.what());
    throw InvalidRequest(ad_str);
  }

  switch (preprocess(command_ad)) {

  case INVALID_REQUEST:
    postprocess();
    throw InvalidRequest(ad_str);

  case NOTHING_TO_DO:
    postprocess();
    break;

  case FORWARD_TO_WM:
    write_end.write(pipe_value_type(postprocess, command_ad));
    break;

  case QUIT:
    result = false;
    postprocess();
    break;

  }

//   } catch (utilities::ClassAdError& e) {
//     Error("classad_utils exception (" << e.what() << ")");
//     throw InvalidRequest();
//   }

  return result;
}

} // server
} // manager
} // wms
} // glite

