// File: lb_utils.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "lb_utils.h"

#include <map>
#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"

#include "glite/lb/producer.h"
#include "glite/lb/consumer.h"

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

namespace {

class ActiveRequests
{
  class Impl;
  Impl* m_impl;

  ActiveRequests();

public:
  ~ActiveRequests();
  static ActiveRequests* instance();
  // return true iff the id WAS NOT there
  bool insert(wmsutils::jobid::JobId const& id, boost::shared_ptr<lb_context_adapter> ctx);
  // return true iff the id WAS there
  bool remove(wmsutils::jobid::JobId const& id);
  boost::shared_ptr<lb_context_adapter> find(wmsutils::jobid::JobId const& id);
};

boost::mutex f_mutex;
ActiveRequests* f_instance;

class ActiveRequests::Impl 
{
  typedef std::map<std::string, boost::shared_ptr<lb_context_adapter> > Requests;
  Requests m_requests;

public:
  bool insert(jobid::JobId const& id, boost::shared_ptr<lb_context_adapter> ctx)
  {
    boost::mutex::scoped_lock l(f_mutex);
    return m_requests.insert(std::make_pair(id.toString(), ctx)).second;
  }
  bool remove(jobid::JobId const& id)
  {
    boost::mutex::scoped_lock l(f_mutex);
    return m_requests.erase(id.toString()) > 0;
  }
  boost::shared_ptr<lb_context_adapter> find(jobid::JobId const& id)
  {
    boost::mutex::scoped_lock l(f_mutex);
    Requests::iterator it = m_requests.find(id.toString());
    return (it != m_requests.end()) ? it->second : boost::shared_ptr<lb_context_adapter>();
  }
};

ActiveRequests::ActiveRequests()
  : m_impl(new Impl)
{
}

ActiveRequests::~ActiveRequests()
{
  delete m_impl;
}

ActiveRequests*
ActiveRequests::instance()
{
  if (f_instance == 0) {
    boost::mutex::scoped_lock l(f_mutex);
    if (f_instance == 0) {
      f_instance = new ActiveRequests;
    }
  }

  return f_instance;
}

bool
ActiveRequests::insert(jobid::JobId const& id, boost::shared_ptr<lb_context_adapter> ctx)
{
  return m_impl->insert(id, ctx);
}

bool
ActiveRequests::remove(jobid::JobId const& id)
{
  return m_impl->remove(id);
}

boost::shared_ptr<lb_context_adapter>
ActiveRequests::find(jobid::JobId const& id)
{
  return m_impl->find(id);
}

} // {anonymous}

ContextPtr
create_context(
  jobid::JobId const& id,
  std::string const& x509_proxy,
  std::string const& sequence_code
)
{
  ContextPtr result;            // default ctor initializes to zero

  // any better test?
  if (x509_proxy.empty()) {
    return result;
  }

  edg_wll_Context context;
  int errcode = edg_wll_InitContext(&context);

  ContextPtr tmp(new lb_context_adapter(context));

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

  if (!errcode) {
    result = tmp;
  }

  return result;
}

bool register_context(jobid::JobId const& id, ContextPtr context)
{
  return ActiveRequests::instance()->insert(id, context);
}

bool unregister_context(jobid::JobId const& id)
{
  return ActiveRequests::instance()->remove(id);
}

ContextPtr get_context(jobid::JobId const& id)
{
  return ActiveRequests::instance()->find(id);
}

std::vector<std::string>
get_previous_matches(edg_wll_Context context, wmsutils::jobid::JobId const& id)
{
  std::vector<std::string> result;

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
        result.push_back(events[i].match.dest_id);
        edg_wll_FreeEvent(&events[i]);
      }
    }
    free(events);
  }

  return result;
}

std::vector<std::pair<std::string,int> >
get_previous_matches_ex(edg_wll_Context context, wmsutils::jobid::JobId const& id)
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
get_original_jdl(edg_wll_Context context, wmsutils::jobid::JobId const& id)
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
  edg_wll_QueryEvents(context, job_conditions, event_conditions, &events);

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
get_error_info(edg_wll_Context context)
{
  int error;
  std::string error_txt;
  std::string description_txt;

  char* c_error_txt = 0;
  char* c_description_txt = 0;
  error = edg_wll_Error(context, &c_error_txt, &c_description_txt);

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
get_lb_message(edg_wll_Context context)
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
get_lb_message(ContextPtr context_ptr)
{
  return get_lb_message(*context_ptr);
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

  int lb_error = log_f(*user_context);

  for (int i = 1; i < 3 && lb_error && lb_error != EINVAL; ++i) {

    if (lb_error == EDG_WLL_ERROR_GSS) {

      // try with the host proxy

      // get the sequence code
      std::string host_x509_proxy(get_host_x509_proxy());
      char* c_sequence_code = edg_wll_GetSequenceCode(*user_context);
      assert(c_sequence_code);
      if (!c_sequence_code) {
        result_error = 3;
        break;
      }
      std::string sequence_code(c_sequence_code);
      free(c_sequence_code);

      // get the jobid
      edg_wlc_JobId c_jobid;
      int e = edg_wll_GetLoggingJob(*user_context, &c_jobid);
      assert(e == 0);
      if (e) {
        result_error = 3;
        break;
      }
      wmsutils::jobid::JobId jobid(c_jobid);
      edg_wlc_JobIdFree(c_jobid);

      // create the host context
      ContextPtr host_context = create_context(jobid, host_x509_proxy, sequence_code);
      if (!host_context) {
        result_error = 3;
        break;
      }

      lb_error = log_f(*host_context);

      for (int k = 1;
           k < 3 && lb_error && lb_error != EINVAL && lb_error != EDG_WLL_ERROR_GSS;
           ++k) {
        boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += 60;
        boost::thread::sleep(xt);

        lb_error = log_f(*host_context);
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

      lb_error = log_f(*user_context);

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
  int e = edg_wll_GetLoggingJob(*user_context, &c_jobid);
  assert(e == 0);
  wmsutils::jobid::JobId jobid(c_jobid);
  edg_wlc_JobIdFree(c_jobid);
  
  result += jobid.toString();

  switch (error) {
  case 0:
    assert(error != 0);
    break;
  case 1:
    // SSL error with user proxy, success with host proxy
    result += "(" + get_lb_message(*user_context)
      + ") with the user proxy. Success with host proxy.";
    break;
  case 2:
    if (user_context == last_context) {
      // no-SSL error with user proxy, no retry with host proxy
      result += "(" + get_lb_message(*user_context) + ")";
    } else {
      // SSL error with user proxy, failure also with host proxy
      result += "(" + get_lb_message(*user_context)
        + ") with the user proxy. Failed with host proxy too ("
        + get_lb_message(*last_context) + ")";
    }
    break;
  case 3:
    // SSL error with the user proxy, cannot retry with the host proxy
    result += "(" + get_lb_message(*user_context)
      + ") with the user proxy. Cannot retry with the host proxy";
    break;
  }

  return result;
  
}

} // common
} // manager
} // wms
} // glite

