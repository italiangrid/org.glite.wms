// File: RequestHandler.cpp
// Author: Francesco Giacomini
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "RequestHandler.h"
#include <string>
#include <ctime>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/bind.hpp>
#include <classad_distribution.h>
#include "WMReal.h"
#include "signal_handling.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wmsutils/exception/Exception.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/helper/exceptions.h"
#include "glite/lb/producer.h"
#include "Request.hpp"
#include "TaskQueue.hpp"
#include "listmatch.h"
#include "glite/lb/producer.h"

namespace fs = boost::filesystem;
namespace jobid = glite::wmsutils::jobid;
namespace task = glite::wms::common::task;
namespace exception = glite::wmsutils::exception;
namespace requestad = glite::wms::jdl;
namespace configuration = glite::wms::common::configuration;
namespace utilities = glite::wms::common::utilities;

namespace glite {
namespace wms {
namespace manager {
namespace server {

struct RequestHandler::Impl
{
  WMReal m_wm;
};

RequestHandler::RequestHandler()
  : m_impl(new Impl)
{
}

namespace {

class HitMaxRetryCount
{
  int m_n;
public:
  HitMaxRetryCount(int n): m_n(n) {}
  int count() const { return m_n; }
};

class HitJobRetryCount
{
  int m_n;
public:
  HitJobRetryCount(int n): m_n(n) {}
  int count() const { return m_n; }
};

class HitMaxShallowCount
{
  int m_n;
public:
  HitMaxShallowCount(int n): m_n(n) {}
  int count() const { return m_n; }
};

class HitJobShallowCount
{
  int m_n;
public:
  HitJobShallowCount(int n): m_n(n) {}
  int count() const { return m_n; }
};

class CannotRetrieveJDL
{
};

void flush_lb_events(ContextPtr const& context, jobid::JobId const& id)
{
  struct timeval* timeout = 0;
  int lb_error = edg_wll_LogFlush(context.get(), timeout);
  if (lb_error) {
    Warning(
      "edg_wll_LogFlush failed for " << id
      << " (" << get_lb_message(context) << ")"
    );
  }
}

int get_max_shallow_count()
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

  return wm_config->max_shallow_retry_count();
}

int get_max_retry_count()
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

  return wm_config->max_retry_count();
}

bool is_deep_resubmission(edg_wll_Event const& event)
{
  return event.type == EDG_WLL_EVENT_RESUBMISSION
    && event.resubmission.result == EDG_WLL_RESUBMISSION_WILLRESUB;
}

bool is_shallow_resubmission(edg_wll_Event const& event)
{
  return event.type == EDG_WLL_EVENT_RESUBMISSION
    && event.resubmission.result == EDG_WLL_RESUBMISSION_SHALLOW;
}

LB_Events::const_iterator
find_last_deep_resubmission(LB_Events const& events)
{
  LB_Events::const_reverse_iterator it(
    std::find_if(events.rbegin(), events.rend(), is_deep_resubmission)
  );
  if (it != events.rend()) {
    return (++it).base();
  } else {
    return events.end();
  }
}

// returns (retry_count, shallow_count)
boost::tuple<int, int> retrieve_lb_info(RequestPtr req)
{
  // this can currently happen only in case of a resubmission
  // for a resubmission we need a few information stored in the LB:
  // - previous deep and shallow resubmissions
  // - previous matches
  // - original jdl

  if (!req->marked_resubmitted()) {
    return boost::make_tuple(0, 0);
  }

  ContextPtr const& context = req->lb_context();
  jobid::JobId const& jobid = req->id();

#ifndef GLITE_WMS_HAVE_LBPROXY
  // flush the lb events since we'll query the lb server
  flush_lb_events(context, jobid);
#endif

  LB_Events events(get_interesting_events(context, jobid));
  if (events.empty()) {
    Warning("Cannot retrieve interesting events for " << jobid);
  }

  int current_deep_count(
    std::count_if(events.begin(), events.end(), is_deep_resubmission)
  );
  assert(current_deep_count >= 0);

  LB_Events::const_iterator last_deep_resubmission(
    find_last_deep_resubmission(events)
  );

  int current_shallow_count(
    std::count_if(
      last_deep_resubmission,
      events.end(),
      is_shallow_resubmission
    )
  );
  assert(current_shallow_count >= 0);

  typedef std::vector<std::pair<std::string,int> > matches_type;
  matches_type const previous_matches = get_previous_matches(events);

  // in principle this warning should happen only if the req is a resubmit
  if (previous_matches.empty()) {
    Warning("cannot retrieve previous matches for " << jobid);
  }

  std::vector<std::string> previous_matches_simple;
  for (matches_type::const_iterator it = previous_matches.begin();
       it != previous_matches.end(); ++it) {
    previous_matches_simple.push_back(it->first);
  }

  // retrieve original jdl
  std::string const job_ad_str(get_original_jdl(context, jobid));
  if (job_ad_str.empty()) {
    throw CannotRetrieveJDL();
  }
  std::auto_ptr<classad::ClassAd> job_ad(utilities::parse_classad(job_ad_str));

  requestad::set_edg_previous_matches(*job_ad, previous_matches_simple);
  requestad::set_edg_previous_matches_ex(*job_ad, previous_matches);

  req->jdl(job_ad);

  return boost::make_tuple(current_deep_count, current_shallow_count);
}

bool is_request_expired(RequestPtr req)
{
  return req->expiry_time() < std::time(0);
}

fs::path sandbox_dir()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  assert(config);

  configuration::NSConfiguration const* const ns_config = config->ns();
  assert(ns_config);

  std::string path_str = ns_config->sandbox_staging_path();

  return fs::path(path_str, fs::native);
}

std::string get_token_file()
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

  return wm_config->token_file();
}

fs::path reallyrunning_token(RequestConstPtr req)
{
  jobid::JobId const& id = req->id();
  fs::path result(sandbox_dir());
  result /= fs::path(jobid::get_reduced_part(id), fs::native);
  result /= fs::path(jobid::to_filename(id), fs::native);
  result /= fs::path(get_token_file(), fs::native);

  return result;
}
void process_match(RequestPtr req)
{
  std::string filename;
  int number_of_results;
  bool include_brokerinfo;
  boost::tie(filename, number_of_results, include_brokerinfo)
    = req->match_parameters();

  Debug(
    "considering match " << req->id()
    << ' ' << filename
    << ' ' << number_of_results
    << ' ' << include_brokerinfo
  );

  if (!match(*req->jdl(), filename, number_of_results, include_brokerinfo)) {
    Info("Failed match for " << req->id());
  }

  req->state(Request::DELIVERED);
}

// in practice the actual retry limit for the deep count is
// max(0, min(job_retry_count, max_retry_count))
// and similarly for the shallow count
// max(0, min(job_shallow_count, max_shallow_count))

void check_shallow_count(RequestConstPtr req, int count)
{
  // check against the job max shallow count
  bool valid = false;
  int job_shallow_count(
    requestad::get_shallow_retry_count(*req->jdl(), valid)
  );
  if (!valid || job_shallow_count < 0) {
    job_shallow_count = 0;
  }
  if (count >= job_shallow_count) {
    throw HitJobShallowCount(job_shallow_count);
  }

  // check against the system max shallow count
  int max_shallow_count = get_max_shallow_count();
  if (max_shallow_count < 0) {
    max_shallow_count = 0;
  }
  if (count >= max_shallow_count) {
    throw HitMaxShallowCount(max_shallow_count);
  }
}

void check_retry_count(RequestConstPtr req, int count)
{
  // check against the job max retry count
  bool valid = false;
  int job_retry_count(requestad::get_retry_count(*req->jdl(), valid));
  if (!valid || job_retry_count < 0) {
    job_retry_count = 0;
  }
  if (count >= job_retry_count) {
    throw HitJobRetryCount(job_retry_count);
  }

  // check against the system max retry count
  int max_retry_count = get_max_retry_count();
  if (count >= max_retry_count) {
    throw HitMaxRetryCount(max_retry_count);
  }
}

int get_job_shallow_count(RequestConstPtr req)
{
  bool valid = false;
  int result = requestad::get_shallow_retry_count(*req->jdl(), valid);
  if (!valid) {
    result = 0;
  }
  return result;
}

bool shallow_resubmission_is_disabled(RequestConstPtr req)
{
  return get_job_shallow_count(req) == -1;
}

void create_token(fs::path const& p)
{
  fs::ofstream _(p);
}

void process_submit(RequestPtr req, WMReal& wm)
{
  Debug("considering (re)submit of " << req->id());

#warning race condition to be addressed
  // the race condition is about a resubmit for an already cancelled
  // job, probably we can check this in retrieve_lb_info(), retrieving
  // also cancel events and ignoring this request if there is one

  // retrieve from the LB possibly missing information needed to
  // process the request, e.g. the jdl, the previous matches, etc.

  try {

    int retry_count;
    int shallow_count;
    boost::tie(retry_count, shallow_count) = retrieve_lb_info(req);

    if (is_request_expired(req)) {

      req->state(Request::UNRECOVERABLE, "request expired");

    } else {

      if (shallow_resubmission_is_disabled(req)) {

        if (req->marked_resubmitted()) {
          check_retry_count(req, retry_count);
          log_resubmission_deep(req->lb_context());
        }

      } else {

        fs::path const token_file(reallyrunning_token(req));

        if (req->marked_resubmitted()) {
          if (fs::exists(token_file)) {
            check_shallow_count(req, shallow_count);
            log_resubmission_shallow(
              req->lb_context(),
              token_file.native_file_string()
            );
          } else {
            check_retry_count(req, retry_count);
            log_resubmission_deep(
              req->lb_context(),
              token_file.native_file_string()
            );
          }
        }

        create_token(token_file);

      }

      wm.submit(req->jdl());
      req->state(Request::DELIVERED);
    }

  } catch (HitMaxRetryCount& e) {
    std::ostringstream os;
    os << "hit max retry count (" << e.count() << ')';
    req->state(Request::UNRECOVERABLE, os.str());
  } catch (HitJobRetryCount& e) {
    std::ostringstream os;
    os << "hit job retry count (" << e.count() << ')';
    req->state(Request::UNRECOVERABLE, os.str());
  } catch (HitMaxShallowCount& e) {
    std::ostringstream os;
    os << "hit max shallow retry count (" << e.count() << ')';
    req->state(Request::UNRECOVERABLE, os.str());
  } catch (HitJobShallowCount& e) {
    std::ostringstream os;
    os << "hit job shallow retry count (" << e.count() << ')';
    req->state(Request::UNRECOVERABLE, os.str());
  } catch (CannotRetrieveJDL& e) {
    req->state(Request::RECOVERABLE, "cannot retrieve jdl");
  } catch (helper::HelperError const& e) {
    req->state(Request::RECOVERABLE, e.what());
  }
}

void process_cancel(RequestPtr req, WMReal& wm)
{
  Debug("considering cancel of " << req->id());

  wm.cancel(req->id());
  req->state(Request::CANCELLED);
}

}

void
RequestHandler::operator()()
try {

  Info("RequestHandler: starting");

  WMReal& wm = m_impl->m_wm;

  while (!received_quit_signal()) {

    RequestPtr req(read_end().read());
    req->last_processed(std::time(0));

    try {

      if (req->jdl()) {

        // submit or match
        if (req->marked_cancelled()) {
          // shortcut submission and cancellation
          // see transition management in the dispatcher
          req->state(Request::RECOVERABLE);
        } else {
          req->state(Request::PROCESSING);
          if (req->marked_match()) {
            process_match(req);
          } else {
            process_submit(req, wm);
          }
        }

      } else if (req->marked_resubmitted()) {

        // resubmit
        if (req->marked_cancelled()) {
          // shortcut resubmission and cancellation
          // see the transition management in the dispatcher
          req->state(Request::RECOVERABLE);
        } else {
          req->state(Request::PROCESSING);
          process_submit(req, wm);
        }

      } else if (req->marked_cancelled()) {

        // normal cancel
        process_cancel(req, wm);

      } else {

        // what's this?
        req->state(Request::UNRECOVERABLE, "invalid request");

      }

    } catch (std::invalid_argument const& e) {
      req->state(Request::UNRECOVERABLE, e.what());
    } catch (requestad::ManipulationException const& e) {
      req->state(Request::UNRECOVERABLE, e.what());
    } catch (exception::Exception const& e) {
      req->state(Request::UNRECOVERABLE, e.what());
    }

  }

  Info("RequestHandler: exiting");

} catch (task::Eof&) {
  Info("RequestHandler: End of input. Exiting...");
} catch (std::exception const& e) {
  Error("RequestHandler: " << e.what() << ". Exiting...");
} catch (...) {
  Error("RequestHandler: caught unknown exception. Exiting...");
}

}}}} // glite::wms::manager::server
