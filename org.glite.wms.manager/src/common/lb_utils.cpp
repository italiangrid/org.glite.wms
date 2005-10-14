// File: lb_utils.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "lb_utils.h"
#include <sys/types.h>
#include <unistd.h>
#include <map>
#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/lb/producer.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/security/proxyrenewal/renewal.h"

namespace jobid = glite::wmsutils::jobid;

namespace configuration = glite::wms::common::configuration;

namespace glite {
namespace wms {
namespace manager {
namespace common {

ContextPtr
create_context(
  jobid::JobId const& id,
  std::string const& x509_proxy,
  std::string const& sequence_code
)
{
  edg_wll_Context context;
  int errcode = edg_wll_InitContext(&context);
  if (errcode) {
    return ContextPtr();
  }

  errcode |= edg_wll_SetParam(context,
                              EDG_WLL_PARAM_SOURCE,
                              EDG_WLL_SOURCE_WORKLOAD_MANAGER);
  errcode |= edg_wll_SetParam(context, EDG_WLL_PARAM_INSTANCE, "WM");

  errcode |= edg_wll_SetParam(context,
                              EDG_WLL_PARAM_X509_PROXY,
                              x509_proxy.c_str());

  const int flag = EDG_WLL_SEQ_NORMAL;
  // the following fails if the sequence code is not formally correct
  errcode |= edg_wll_SetLoggingJob(context, id, sequence_code.c_str(), flag);

  if (errcode) {
    throw CannotCreateLBContext(errcode);
  }

  return ContextPtr(context, edg_wll_FreeContext);
}

namespace {

std::string
get_proxy_subject(std::string const& x509_proxy)
{
  static std::string const null_string;

  std::FILE* fd = std::fopen(x509_proxy.c_str(), "r");
  if (!fd) return null_string;
  boost::shared_ptr<std::FILE> fd_(fd, std::fclose);

  ::X509* const cert = ::PEM_read_X509(fd, 0, 0, 0);
  if (!cert) return null_string;
  boost::shared_ptr< ::X509> cert_(cert, ::X509_free);

  char* const s = ::X509_NAME_oneline(::X509_get_subject_name(cert), 0, 0);
  if (!s) return null_string;
  boost::shared_ptr<char> s_(s, ::free);

  return std::string(s);
}

}

ContextPtr
create_context_proxy(
  jobid::JobId const& id,
  std::string const& x509_proxy,
  std::string const& sequence_code
)
{
  edg_wll_Context context;
  int errcode = edg_wll_InitContext(&context);
  if (errcode) {
    return ContextPtr();
  }

  errcode |= edg_wll_SetParam(
    context,
    EDG_WLL_PARAM_SOURCE,
    EDG_WLL_SOURCE_WORKLOAD_MANAGER
  );
  errcode |= edg_wll_SetParam(
    context,
    EDG_WLL_PARAM_INSTANCE,
    boost::lexical_cast<std::string>(::getpid()).c_str()
  );

  errcode |= edg_wll_SetParam(context,
                              EDG_WLL_PARAM_X509_PROXY,
                              x509_proxy.c_str());

  std::string const user_dn = get_proxy_subject(x509_proxy);
  int const flag = EDG_WLL_SEQ_NORMAL;
  errcode |= edg_wll_SetLoggingJobProxy(
    context,
    id,
    sequence_code.empty() ? 0 : sequence_code.c_str(),
    user_dn.c_str(),
    flag
  );

  if (errcode) {
    throw CannotCreateLBContext(errcode);
  }

  return ContextPtr(context, edg_wll_FreeContext);
}

LB_Events
get_interesting_events(edg_wll_Context context, jobid::JobId const& id)
{
  edg_wll_QueryRec jobid[2];
  jobid[0].attr    = EDG_WLL_QUERY_ATTR_JOBID;
  jobid[0].op      = EDG_WLL_QUERY_OP_EQUAL;
  jobid[0].value.j = id.getId();
  jobid[1].attr    = EDG_WLL_QUERY_ATTR_UNDEF;

  edg_wll_QueryRec const* job_conditions[2];
  job_conditions[0] = jobid;
  job_conditions[1] = 0;

  edg_wll_QueryRec match_or_resubmit[3];
  match_or_resubmit[0].attr    = EDG_WLL_QUERY_ATTR_EVENT_TYPE;
  match_or_resubmit[0].op      = EDG_WLL_QUERY_OP_EQUAL;
  match_or_resubmit[0].value.i = EDG_WLL_EVENT_MATCH;
  match_or_resubmit[1].attr    = EDG_WLL_QUERY_ATTR_EVENT_TYPE;
  match_or_resubmit[1].op      = EDG_WLL_QUERY_OP_EQUAL;
  match_or_resubmit[1].value.i = EDG_WLL_EVENT_RESUBMISSION;
  match_or_resubmit[2].attr    = EDG_WLL_QUERY_ATTR_UNDEF;

  edg_wll_QueryRec from_wm[2];
  from_wm[0].attr    = EDG_WLL_QUERY_ATTR_SOURCE;
  from_wm[0].op      = EDG_WLL_QUERY_OP_EQUAL;
  from_wm[0].value.i = EDG_WLL_SOURCE_WORKLOAD_MANAGER;
  from_wm[1].attr    = EDG_WLL_QUERY_ATTR_UNDEF;

  edg_wll_QueryRec const* event_conditions[3];
  event_conditions[0] = match_or_resubmit;
  event_conditions[1] = from_wm;
  event_conditions[2] = 0;

  edg_wll_Event* events = 0;
  edg_wll_QueryEventsExtProxy(
    context,
    job_conditions,
    event_conditions,
    &events
  );

  return LB_Events(events);
}

std::vector<std::pair<std::string,int> >
get_previous_matches(edg_wll_Context context, jobid::JobId const& id)
{
  std::vector<std::pair<std::string,int> > result;

  edg_wll_QueryRec job_conditions[2];
  job_conditions[0].attr    = EDG_WLL_QUERY_ATTR_JOBID;
  job_conditions[0].op      = EDG_WLL_QUERY_OP_EQUAL;
  job_conditions[0].value.j = id.getId();
  job_conditions[1].attr    = EDG_WLL_QUERY_ATTR_UNDEF;

  edg_wll_QueryRec event_conditions[2];
  event_conditions[0].attr    = EDG_WLL_QUERY_ATTR_EVENT_TYPE;
  event_conditions[0].op      = EDG_WLL_QUERY_OP_EQUAL;
  event_conditions[0].value.i = EDG_WLL_EVENT_MATCH;
  event_conditions[1].attr    = EDG_WLL_QUERY_ATTR_UNDEF;

  edg_wll_Event* events = 0;
  edg_wll_QueryEvents(context, job_conditions, event_conditions, &events);

  if (events) {
    for (int i = 0; events[i].type; ++i) {
      if (events[i].type == EDG_WLL_EVENT_MATCH) {
        std::string ce_id = events[i].match.dest_id;
        int timestamp = events[i].match.timestamp.tv_sec;
        result.push_back(std::make_pair(ce_id, timestamp));
        edg_wll_FreeEvent(&events[i]);
      }
    }
    free(events);
  }

  return result;
}

std::string
get_original_jdl(edg_wll_Context context, jobid::JobId const& id)
{
  std::string result;

  edg_wll_QueryRec job_conditions[2];
  job_conditions[0].attr    = EDG_WLL_QUERY_ATTR_JOBID;
  job_conditions[0].op      = EDG_WLL_QUERY_OP_EQUAL;
  job_conditions[0].value.j = id.getId();
  job_conditions[1].attr    = EDG_WLL_QUERY_ATTR_UNDEF;

  edg_wll_QueryRec event_conditions[3];
  event_conditions[0].attr    = EDG_WLL_QUERY_ATTR_EVENT_TYPE;
  event_conditions[0].op      = EDG_WLL_QUERY_OP_EQUAL;
  event_conditions[0].value.i = EDG_WLL_EVENT_ENQUEUED;
  event_conditions[1].attr    = EDG_WLL_QUERY_ATTR_SOURCE;
  event_conditions[1].op      = EDG_WLL_QUERY_OP_EQUAL;
  event_conditions[1].value.i = EDG_WLL_SOURCE_NETWORK_SERVER;
  event_conditions[2].attr    = EDG_WLL_QUERY_ATTR_UNDEF;

  edg_wll_Event* events = 0;
  //  edg_wll_QueryEvents(context, job_conditions, event_conditions, &events);
  edg_wll_QueryEventsProxy(context, job_conditions, event_conditions, &events);

  if (events) {
    for (int i = 0; events[i].type; ++i) {
      // in principle there is only one, so save the first one and ignore the
      // others
      if (result.empty()
          && events[i].type == EDG_WLL_EVENT_ENQUEUED
          && events[i].enQueued.result == EDG_WLL_ENQUEUED_OK) {
        result = events[i].enQueued.job;
      }
      edg_wll_FreeEvent(&events[i]);
    }
    free(events);
  }

  return result;
}

std::string
get_user_x509_proxy(jobid::JobId const& jobid)
{
  std::string result;

  char* c_x509_proxy = 0;
  int err_code = edg_wlpr_GetProxy(jobid.getId(), &c_x509_proxy);

  if (err_code == 0) {

    result.assign(c_x509_proxy);
    free(c_x509_proxy);

  } else {

    // currently no proxy is registered if renewal is not requested
    // try to get the original user proxy from the input sandbox

    configuration::Configuration const* const config
      = configuration::Configuration::instance();
    assert(config);

    configuration::NSConfiguration const* const ns_config = config->ns();
    assert(ns_config);

    std::string x509_proxy(ns_config->sandbox_staging_path());
    x509_proxy += "/"
      + jobid::get_reduced_part(jobid)
      + "/"
      + jobid::to_filename(jobid)
      + "/user.proxy";

    result = x509_proxy;

  }

  return result;
}

std::string get_host_x509_proxy()
{
  return configuration::Configuration::instance()->common()->host_proxy_file();
}

namespace {

boost::tuple<int,std::string,std::string>
get_error_info(ContextPtr context)
{
  int error;
  std::string error_txt;
  std::string description_txt;

  char* c_error_txt = 0;
  char* c_description_txt = 0;
  error = edg_wll_Error(context.get(), &c_error_txt, &c_description_txt);

  if (c_error_txt) {
    error_txt = c_error_txt;
  }
  free(c_error_txt);
  if (c_description_txt) {
    description_txt = c_description_txt;
  }
  free(c_description_txt);

  return boost::make_tuple(error, error_txt, description_txt);
}

} // anonymous

std::string
get_lb_message(ContextPtr const& context)
{
  std::string result;

  int error;
  std::string error_txt;
  std::string description_txt;
  boost::tie(error, error_txt, description_txt) = get_error_info(context);

  result += error_txt;
  result += " (";
  result += boost::lexical_cast<std::string>(error);
  result += ") - ";
  result += description_txt;

  return result;
}

std::string
get_lb_sequence_code(ContextPtr const& context)
{
  char* c_sequence_code = edg_wll_GetSequenceCode(context.get());
  std::string sequence_code(c_sequence_code);
  free(c_sequence_code);
  return sequence_code;
}

// 0 success - returned ContextPtr is user context
// 1 failure with user context due to SSL error, success with host proxy - 
//             returned ContextPtr is host context
// 2 failure - return ContextPtr is the last tried context (host context if
//             SSL_ERROR for user context, user context otherwise)
// 3 failure (cannot create the host proxy) - returned ContextPtr is the
//             user context

boost::tuple<int,ContextPtr>
lb_log(boost::function<int(edg_wll_Context)> log_f, ContextPtr user_context)
{
  int result_error = 0;
  ContextPtr result_context = user_context;

  int lb_error = log_f(user_context.get());

  for (int i = 1; i < 3 && lb_error && lb_error != EINVAL; ++i) {

    if (lb_error == EDG_WLL_ERROR_GSS) {

      // try with the host proxy

      // get the sequence code
      std::string host_x509_proxy(get_host_x509_proxy());
      char* c_sequence_code = edg_wll_GetSequenceCode(user_context.get());
      assert(c_sequence_code);
      if (!c_sequence_code) {
        result_error = 3;
        break;
      }
      std::string sequence_code(c_sequence_code);
      free(c_sequence_code);

      // get the jobid
      edg_wlc_JobId c_jobid;
      int e = edg_wll_GetLoggingJob(user_context.get(), &c_jobid);
      assert(e == 0);
      if (e) {
        result_error = 3;
        break;
      }
      jobid::JobId jobid(c_jobid);
      edg_wlc_JobIdFree(c_jobid);

      // create the host context
      ContextPtr host_context = create_context(jobid, host_x509_proxy, sequence_code);
      if (!host_context) {
        result_error = 3;
        break;
      }

      lb_error = log_f(host_context.get());

      for (int k = 1;
           k < 3 && lb_error && lb_error != EINVAL && lb_error != EDG_WLL_ERROR_GSS;
           ++k) {
        boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += 60;
        boost::thread::sleep(xt);

        lb_error = log_f(host_context.get());
      }

      if (lb_error) {
        result_error = 2;
      } else {
        result_error = 1;
      }

      result_context = host_context;

      break;

    } else {

      boost::xtime xt;
      boost::xtime_get(&xt, boost::TIME_UTC);
      xt.sec += 60;
      boost::thread::sleep(xt);

      lb_error = log_f(user_context.get());

    }

  }

  if (lb_error && result_error == 0) { // non-SSL failure with user proxy
    result_error = 2;
  }

  return boost::make_tuple(result_error, result_context);
}

std::string
get_logger_message(
  std::string const& function_name,
  int error,
  ContextPtr user_context,
  ContextPtr last_context
)
{
  std::string result(function_name);
  result += " failed for ";

  edg_wlc_JobId c_jobid;
  int e = edg_wll_GetLoggingJob(user_context.get(), &c_jobid);
  assert(e == 0);
  jobid::JobId jobid(c_jobid);
  edg_wlc_JobIdFree(c_jobid);
  
  result += jobid.toString();

  switch (error) {
  case 0:
    assert(error != 0);
    break;
  case 1:
    // SSL error with user proxy, success with host proxy
    result += "(" + get_lb_message(user_context)
      + ") with the user proxy. Success with host proxy.";
    break;
  case 2:
    if (user_context == last_context) {
      // no-SSL error with user proxy, no retry with host proxy
      result += "(" + get_lb_message(user_context) + ")";
    } else {
      // SSL error with user proxy, failure also with host proxy
      result += "(" + get_lb_message(user_context)
        + ") with the user proxy. Failed with host proxy too ("
        + get_lb_message(last_context) + ")";
    }
    break;
  case 3:
    // SSL error with the user proxy, cannot retry with the host proxy
    result += "(" + get_lb_message(user_context)
      + ") with the user proxy. Cannot retry with the host proxy";
    break;
  }

  return result;
  
}

}}}} // glite::wms::manager::common

