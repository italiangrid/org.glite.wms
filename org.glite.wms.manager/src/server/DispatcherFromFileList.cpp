// File: DispatcherFromFileList.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "DispatcherFromFileList.h"
#include <algorithm>
#include <cctype>
#include <boost/thread/xtime.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>
#include "DispatcherFactory.h"
#include "signal_handling.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wmsutils/exception/Exception.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/scope_guard.h"
#include "TaskQueue.hpp"
#include "Request.hpp"
#include "glite/security/proxyrenewal/renewal.h"
#include "purger.h"

#include "glite/lb/producer.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include "CommandAdManipulation.h"
#include "lb_utils.h"

namespace task = glite::wms::common::task;
namespace configuration = glite::wms::common::configuration;
namespace utilities = glite::wms::common::utilities;
namespace jobid = glite::wmsutils::jobid;
namespace requestad = glite::wms::jdl;
namespace common = glite::wms::manager::common;

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

class CannotRetrieveJDL
{
};

}

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

std::string dispatcher_id("FileList");

DispatcherImpl* create_dispatcher()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  if (!config || config->get_module() != configuration::ModuleType::workload_manager) {
    Fatal("empty or invalid configuration");
  }
  configuration::WMConfiguration const* const wm_config = config->wm();
  if (!wm_config) {
    Fatal("empty WM configuration");
  }
  std::string file(wm_config->input());

  // the extractor is shared between the dispatcher and the cleanup functions
  // (see CleanUp below); the latter can live beyond the scope of the dispatcher
  // build the extractor outside the dispatcher to make this sharing more
  // apparent (i.e. the extractor does not belong to the dispatcher) even if it
  // could be built within the dispatcher ctor

  typedef DispatcherFromFileList::extractor_type extractor_type;
  boost::shared_ptr<extractor_type> extractor(new extractor_type(file));
  if (!extractor) {
    Fatal("cannot build FileList extractor");
  }

  Info("reading from " << file);

  return new DispatcherFromFileList(extractor);
}

std::string normalize(std::string const& id)
{
  std::string result(id);
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

struct Register
{
  Register()
  {
    DispatcherFactory::instance()->register_dispatcher(
      normalize(dispatcher_id),
      create_dispatcher
    );
  }
  ~Register()
  {
    DispatcherFactory::instance()->unregister_dispatcher(normalize(dispatcher_id));
  }
};

Register r;

class CleanUp
{
  typedef DispatcherFromFileList::extractor_type extractor_type;
  typedef extractor_type::iterator extractor_iterator;

  boost::shared_ptr<extractor_type> m_extractor;
  extractor_iterator m_it;

public:
  CleanUp(boost::shared_ptr<extractor_type> e, extractor_iterator i)
    : m_extractor(e), m_it(i)
  {
  }

  void operator()()
  {
    m_extractor->erase(m_it);
  }
};

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

void log_dequeued(common::ContextPtr const& context)
{
  std::string input_name(get_input_name());
  char const* const local_jobid = ""; // not needed because no real local id

  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(edg_wll_LogDeQueued, _1, input_name.c_str(), local_jobid),
    context
  );
  if (lb_error) {
    Warning(common::get_logger_message("edg_wll_LogDeQueued", lb_error, context, ctx));
  }
}

void
log_cancel_req(common::ContextPtr const& context)
{
  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(edg_wll_LogCancelREQ, _1, "CANCELLATION REQUEST"),
    context
  );
  if (lb_error) {
    Warning(common::get_logger_message("edg_wll_LogCancelREQ", lb_error, context, ctx));
  }
}

void log_pending(RequestPtr const& req)
{
  common::ContextPtr context = req->lb_context();

  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(edg_wll_LogPending, _1, req->message().c_str()),
    context
  );
  if (lb_error) {
    Warning(common::get_logger_message("edg_wll_LogPending", lb_error, context, ctx));
  }
}

void flush_lb_events(common::ContextPtr const& context, jobid::JobId const& id)
{
  struct timeval* timeout = 0;
  int lb_error = edg_wll_LogFlush(context.get(), timeout);
  if (lb_error) {
    Warning(
         "edg_wll_LogFlush failed for " << id
      << " (" << common::get_lb_message(context) << ")"
    );
  }  
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

void retrieve_lb_info(RequestPtr const& req)
{
  // this can currently happen only in case of a resubmission, for we need the
  // original jdl and the previous matches
  if (!req->marked_resubmitted()) {
    return;
  }

  common::ContextPtr context = req->lb_context();
  jobid::JobId jobid = req->id();

  // flush the lb events since we'll query the server
  flush_lb_events(context, jobid);

  // retrieve previous matches; continue if failure
  typedef std::vector<std::pair<std::string,int> > matches_type;
  matches_type const previous_matches = common::get_previous_matches(context.get(), jobid);

  // in principle this warning should happen only if the req is a resubmit
  if (previous_matches.empty()) {
    Warning("cannot retrieve previous matches for " << jobid);
  }

  std::vector<std::string> previous_matches_simple;
  for (matches_type::const_iterator it = previous_matches.begin();
       it != previous_matches.end(); ++it) {
    previous_matches_simple.push_back(it->first);
  }

// check the system max retry count; abort if exceeded
  size_t max_retry_count = get_max_retry_count();
  if (max_retry_count <= 0
      || previous_matches.size() > max_retry_count) {
    throw HitMaxRetryCount(max_retry_count);
  }

  // retrieve original jdl
  std::string const job_ad_str = common::get_original_jdl(context.get(), jobid);
  if (job_ad_str.empty()) {
    throw CannotRetrieveJDL();
  }
  std::auto_ptr<classad::ClassAd> job_ad(utilities::parse_classad(job_ad_str));

  // check the job max retry count; abort if exceeded
  bool count_valid = false;
  size_t job_retry_count = requestad::get_retry_count(*job_ad, count_valid);
  if (!count_valid) {
    job_retry_count = 0;
  }
  if (job_retry_count <= 0
      || previous_matches.size() > job_retry_count) {
    throw HitJobRetryCount(job_retry_count);
  }

  // in practice the actual retry limit is
  // max(0, min(job_retry_count, max_retry_count))

  requestad::set_edg_previous_matches(*job_ad, previous_matches_simple);
  requestad::set_edg_previous_matches_ex(*job_ad, previous_matches);

  req->jdl(job_ad.release());
}

bool operator<(boost::xtime const& rhs, boost::xtime const& lhs)
{
  return
       rhs.sec < lhs.sec
    || rhs.sec == lhs.sec && rhs.nsec < lhs.nsec;
}

bool older_than(RequestPtr const& req, boost::xtime const& time)
{
  return req->last_processed() < time;
}

void do_transitions_for_cancel(
  RequestPtr const& req,
  boost::xtime const& current_time,
  task::PipeWriteEnd<RequestPtr>& write_end
)
{
  boost::xtime threshold = current_time;
  threshold.sec -= 300;         // 5 minutes
  Request::State state = req->state();

  if (req->jdl() || older_than(req, threshold)) {
    // new cancel or old one
    // manage immediately, if applicable

    Info("considering cancel of " << req->id());

    switch (state) {
    case Request::WAITING:
    case Request::RECOVERABLE:
      req->clear_jdl();
      req->state(Request::CANCELLED, "cancelled by user");
      req->add_cleanup(
        boost::bind(edg_wlpr_UnregisterProxy, req->id(), "")
      );
      req->add_cleanup(
        boost::bind(purger::purgeStorage, req->id(), std::string())
      );
      break;
    case Request::DELIVERED:
      req->clear_jdl();       // this, together with marked_cancelled(),
      // tells the RequestHandler that it should
      // call wm.cancel()
      write_end.write(req);
      break;
    case Request::READY:
    case Request::PROCESSING:
      // do nothing; these are unstable states, subject to race conditions
      // wait for a stable one; in particular don't clear the jdl because
      // it may be in use (or shortly be) by the RequestHandler
      break;
    case Request::UNRECOVERABLE:
      // don't change state; this will cause the job to be aborted rather than
      // cancelled, so the user knows that there is something wrong with this
      // job
      req->add_cleanup(
        boost::bind(edg_wlpr_UnregisterProxy, req->id(), "")
      );
      req->add_cleanup(
        boost::bind(purger::purgeStorage, req->id(), std::string())
      );
      break;
    case Request::CANCELLED:
      // do nothing; the job has already been cancelled
      break;
    }
  }
}

void do_transitions_for_submit(
  RequestPtr const& req,
  boost::xtime const& current_time,
  task::PipeWriteEnd<RequestPtr>& write_end
)
{
  boost::xtime threshold = current_time;
  threshold.sec -= 300;         // 5 minutes
  Request::State state = req->state();

  switch (state) {

  case Request::WAITING:

    if (older_than(req, threshold)) {
      Info("considering (re)submit of " << req->id());

      try {

        // retrieve from the LB possibly missing information needed to
        // process the request, e.g. the jdl, the previous matches, etc.
        retrieve_lb_info(req);

        req->state(Request::READY);
        write_end.write(req);

      } catch (HitMaxRetryCount& e) {
        Info("hit max retry count (" << e.count() << ") for " << req->id());
        req->state(Request::UNRECOVERABLE);
      } catch (HitJobRetryCount& e) {
        Info("hit job retry count (" << e.count() << ") for " << req->id());
        req->state(Request::UNRECOVERABLE);
      } catch (CannotRetrieveJDL& e) {
        Info("cannot retrieve jdl for " << req->id() << "; keep retrying");
        req->state(Request::RECOVERABLE);
      }        
    }
    break;

  case Request::RECOVERABLE:

    log_pending(req);
    Info("postponing " << req->id());
    req->state(Request::WAITING);
    break;

  case Request::DELIVERED:
    // keep this informational message here because in ~Request it may be
    // "overriden" by the CANCELLED state, i.e. a DELIVERED request may
    // become later a CANCELLED one and be destroyed as such

    Info(req->id() << " delivered");
    break;

  case Request::UNRECOVERABLE:
    Info(req->id() << " failed");
    break;

  case Request::READY:
  case Request::PROCESSING:
    // do nothing; the job is in the hands of the request handler, wait for
    // the job to enter a stable state
    break;

  case Request::CANCELLED:
    Info(req->id() << " cancelled");
    break;

  }
}

void do_transitions(
  TaskQueue& tq,
  task::PipeWriteEnd<RequestPtr>& write_end
)
{
  boost::xtime current_time;
  boost::xtime_get(&current_time, boost::TIME_UTC);

  for (TaskQueue::iterator it = tq.begin(); it != tq.end(); ++it) {
    RequestPtr req(it->second);

    if (req->marked_cancelled()) {
      do_transitions_for_cancel(req, current_time, write_end);
    } else {
      do_transitions_for_submit(req, current_time, write_end);
    }
  }
}

bool is_done(std::pair<std::string, RequestPtr> const& id_req)
{
  RequestPtr const& req = id_req.second;
  return
    req->state() == Request::DELIVERED && !req->marked_cancelled()
    || req->state() == Request::UNRECOVERABLE
    || req->state() == Request::CANCELLED;
}

void remove_done(TaskQueue& tq)
{
  TaskQueue::iterator it = std::find_if(tq.begin(), tq.end(), is_done);
  while (it != tq.end()) {
    tq.erase(it);
    it = std::find_if(tq.begin(), tq.end(), is_done);
  }
}

class is_in_state
{
  RequestPtr m_req;
  int m_states;
public:
  is_in_state(RequestPtr const& req, int states)
    : m_req(req), m_states(states)
  {
  }
  bool operator()() const
  {
    return m_req->state() & m_states;
  }
};

jobid::JobId
aux_get_id(classad::ClassAd const& command_ad, std::string const& command)
{
  if (command == "jobsubmit") {
    return jobid::JobId(
      requestad::get_edg_jobid(
        *common::submit_command_get_ad(command_ad)
      )
    );
  } else if (command == "jobresubmit") {
    return jobid::JobId(common::resubmit_command_get_id(command_ad));
  } else if (command == "jobcancel") {
    return jobid::JobId(common::cancel_command_get_id(command_ad));
  }

  // the following is just to avoid a warning about "control reaches end of
  // non-void function", but there is no possibility to arrive here
  return jobid::JobId();
}

class RequestChecker
{
  std::string m_command;
  jobid::JobId m_id;

public:
  RequestChecker(classad::ClassAd const& command_ad)
  {
    if (!common::command_is_valid(command_ad)) {
      throw InvalidRequest(utilities::unparse_classad(command_ad));
    }
    m_command = common::command_get_command(command_ad);
    m_id = aux_get_id(command_ad, m_command);
  }

  std::string command() const { return m_command; }
  jobid::JobId id() const { return m_id; }
};

void
get_new_requests(
  boost::shared_ptr<utilities::FLExtractor<std::string> >& extractor,
  TaskQueue& tq
)
{
  typedef utilities::FLExtractor<std::string> extractor_type;
  typedef std::vector<extractor_type::iterator> new_requests_type;
  new_requests_type new_requests(extractor->get_all_available());

  for (new_requests_type::iterator it = new_requests.begin();
       it != new_requests.end(); ++it) {

    extractor_type::iterator request_it = *it;
    std::string command_ad_str = *request_it;

    try {

      boost::function<void()> cleanup(CleanUp(extractor, request_it));
      // if the request is not valid, cleanup automatically
      utilities::scope_guard cleanup_guard(cleanup);

      boost::scoped_ptr<classad::ClassAd> command_ad(
        utilities::parse_classad(command_ad_str)
      );

      RequestChecker request_checker(*command_ad);
      std::string command = request_checker.command();
      jobid::JobId id = request_checker.id();

      TaskQueue::iterator it = tq.find(id.toString());

      if (it == tq.end()) {

        cleanup_guard.dismiss();
        RequestPtr request(new Request(*command_ad, cleanup));
        tq.insert(std::make_pair(id.toString(), request));

        if (command == "jobsubmit" || command == "jobresubmit") {
          log_dequeued(request->lb_context());
        } else {
          log_cancel_req(request->lb_context());
        }

      } else {

        RequestPtr request = it->second;

        if (command == "jobsubmit") {
          // this really shouldn't happen, just ignore this new request
          Info("ignoring submit " << id);

        } else if (command == "jobresubmit") {
          // during normal running a resubmit can be seen at this point only
          // if the status of this job is either DELIVERED, PROCESSING (this
          // case is possible because the submit request can be passed to the
          // JC, fail and be resubmitted by the LM before the RequestHandler
          // thread could mark it as DELIVERED) or CANCELLED (this case is
          // possible because the cancel request can be processed - and passed
          // to the JC, moving the request to the CANCELLED state - while the
          // job failed and was resubmitted by the LM)

          // but during start up, w/o the appropriate checks on the status of
          // a job, still to be done) also an initial state (i.e. WAITING) is
          // possible because all the requests are read in one shot from the
          // input

          Debug("jobresubmit " << id << " when in state " << request->state());
          log_dequeued(request->lb_context());
          request->mark_resubmitted();
          cleanup_guard.dismiss();
          request->add_cleanup(cleanup);

        } else if (command == "jobcancel") {
          log_cancel_req(request->lb_context());
          request->mark_cancelled();
          cleanup_guard.dismiss();
          request->add_cleanup(cleanup);
        }

      }

    } catch (utilities::ClassAdError& e) {
      Info(e.what() << " for " << command_ad_str);
    } catch (InvalidRequest& e) {
      Info("Invalid request: " << command_ad_str);
    }
  }
}


} // {anonymous} 

DispatcherFromFileList::DispatcherFromFileList(boost::shared_ptr<extractor_type> extractor)
  : m_extractor(extractor)
{
}

void
DispatcherFromFileList::run(task::PipeWriteEnd<RequestPtr>& write_end)
try {

  Info("Dispatcher: starting");

  TaskQueue& tq(the_task_queue());
  while (!received_quit_signal()) {

    do_transitions(tq, write_end);
    remove_done(tq);
    get_new_requests(m_extractor, tq);

    // wait one second
    boost::xtime xt;
    boost::xtime_get(&xt, boost::TIME_UTC);
    xt.sec += 1;
    boost::thread::sleep(xt);

  }

  Info("Dispatcher: exiting");

} catch (task::SigPipe&) {
  Info("Dispatcher: no RequestHandler listening. Exiting...");
} catch (std::exception const& e) {
  Error("Dispatcher: " << e.what() << ". Exiting...");
} catch (...) {
  Error("Dispatcher: caught unknown exception. Exiting...");
}

}}}} // glite::wms::manager::server

