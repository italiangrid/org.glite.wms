// File: DispatcherFromFileList.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "DispatcherFromFileList.h"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <cstdio>
#include <boost/thread/xtime.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>
#include <openssl/pem.h>
#include <openssl/x509.h>
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
#include "CommandAdManipulation.h"
#include "lb_utils.h"
#include "filelist_utils.h"
#include "filelist_recovery.h"

namespace task = glite::wms::common::task;
namespace configuration = glite::wms::common::configuration;
namespace utilities = glite::wms::common::utilities;
namespace jobid = glite::wmsutils::jobid;
namespace requestad = glite::wms::jdl;
namespace common = glite::wms::manager::common;

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

bool
is_recovery_enabled()
{
  configuration::Configuration const& config(
    *configuration::Configuration::instance()
  );

  return config.wm()->enable_recovery();
}

void log_dequeued(common::ContextPtr const& context, std::string const& input)
{
  char const* const local_jobid = ""; // not needed because no real local id

  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(edg_wll_LogDeQueuedProxy, _1, input.c_str(), local_jobid),
    context
  );
  if (lb_error) {
    Warning(
      common::get_logger_message(
        "edg_wll_LogDeQueuedProxy",
        lb_error,
        context,
        ctx
      )
    );
  }
}

void
log_cancel_req(common::ContextPtr const& context)
{
  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(edg_wll_LogCancelREQProxy, _1, "CANCELLATION REQUEST"),
    context
  );
  if (lb_error) {
    Warning(
      common::get_logger_message(
        "edg_wll_LogCancelREQProxy",
        lb_error,
        context,
        ctx
      )
    );
  }
}

void log_pending(RequestPtr const& req)
{
  common::ContextPtr context = req->lb_context();

  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(edg_wll_LogPendingProxy, _1, req->message().c_str()),
    context
  );
  if (lb_error) {
    Warning(
      common::get_logger_message(
        "edg_wll_LogPendingProxy",
        lb_error,
        context,
        ctx
      )
    );
  }
}

bool older_than(RequestPtr const& req, std::time_t threshold)
{
  return req->last_processed() < threshold;
}

void do_transitions_for_cancel(
  RequestPtr const& req,
  std::time_t current_time,
  pipe_type::write_end_type& write_end
)
{
  std::time_t threshold = current_time - 300; // 5 minutes
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

bool is_proxy_expired(jobid::JobId const& id)
{
  std::string proxy_file = common::get_user_x509_proxy(id);

  std::FILE* rfd = std::fopen(proxy_file.c_str(), "r");
  if (!rfd) return true;
  boost::shared_ptr<std::FILE> fd(rfd, std::fclose);

  ::X509* rcert = ::PEM_read_X509(rfd, 0, 0, 0);
  if (!rcert) return true;
  boost::shared_ptr<X509> cert(rcert, ::X509_free);

  return X509_cmp_current_time(X509_get_notAfter(rcert)) <= 0;
}

void do_transitions_for_submit(
  RequestPtr const& req,
  std::time_t current_time,
  pipe_type::write_end_type& write_end
)
{
  std::time_t threshold = current_time - 300; // 5 minutes
  Request::State state = req->state();

  switch (state) {

  case Request::WAITING:

    if (older_than(req, threshold)) {
      Info("considering (re)submit of " << req->id());

      if (is_proxy_expired(req->id())) {
        req->state(Request::UNRECOVERABLE, "proxy expired");
        break;
      }

      req->state(Request::READY);
      write_end.write(req);
    }
    break;

  case Request::RECOVERABLE:

    log_pending(req);
    Info("postponing " << req->id() << " (" << req->message() << ')');
    req->state(Request::WAITING);
    break;

  case Request::DELIVERED:
    // keep this informational message here because in ~Request it may be
    // "overriden" by the CANCELLED state, i.e. a DELIVERED request may
    // become later a CANCELLED one and be destroyed as such

    Info(req->id() << " delivered");
    break;

  case Request::UNRECOVERABLE:
    Info(req->id() << " failed (" << req->message() << ')');
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

void do_transitions_for_match(
  RequestPtr const& req,
  pipe_type::write_end_type& write_end
)
{
  switch (req->state()) {

  case Request::WAITING:
    Info("considering match of " << req->id());
    req->state(Request::READY);
    write_end.write(req);
    break;

  case Request::DELIVERED:
    Info(req->id() << " delivered");
    break;

  case Request::READY:
  case Request::PROCESSING:
    // do nothing; the job is in the hands of the request handler, wait for
    // the job to enter a stable state
    break;

  case Request::RECOVERABLE:
  case Request::UNRECOVERABLE:
  case Request::CANCELLED:
    // not applicable
    assert(!"a match request cannot be in a"
           "RECOVERABLE, UNRECOVERABLE or CANCELLED state");
    break;
  }
}

void do_transitions(
  TaskQueue& tq,
  pipe_type::write_end_type& write_end
)
{
  std::time_t current_time = std::time(0);

  for (TaskQueue::iterator it = tq.begin(); it != tq.end(); ++it) {
    RequestPtr req(it->second);

    if (req->marked_cancelled()) {
      do_transitions_for_cancel(req, current_time, write_end);
    } else if (req->marked_match()) {
      do_transitions_for_match(req, write_end);
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
  TaskQueue::iterator cur = tq.begin();
  TaskQueue::iterator const last = tq.end();
  while ((cur = std::find_if(cur, last, is_done)) != last) {
    TaskQueue::iterator tmp = cur++;
    tq.erase(tmp);
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

void
get_new_requests(
  ExtractorPtr extractor,
  TaskQueue& tq,
  std::string const& input
)
{
  requests_type new_requests(extractor->get_all_available());

  requests_type::iterator it = new_requests.begin();
  requests_type::iterator const end = new_requests.end();
  for ( ; it != end; ++it) {

    extractor_type::iterator request_it = *it;
    boost::function<void()> cleanup(FLCleanUp(extractor, request_it));
    // if the request is not valid, cleanup automatically
    utilities::scope_guard cleanup_guard(cleanup);

    std::string const command_ad_str = *request_it;

    try {

      classad::ClassAd command_ad;
      classad::ClassAdParser parser;
      if (!parser.ParseClassAd(command_ad_str, command_ad)) {
        Info("Invalid request " << command_ad_str);
        continue;
      }

      std::string command;
      jobid::JobId id;
      boost::tie(command, id) = check_request(command_ad);

      TaskQueue::iterator it = tq.find(id.toString());

      if (it == tq.end()) {

        RequestPtr new_request(new Request(command_ad, command, id, cleanup));
        cleanup_guard.dismiss();
        common::ContextPtr context = new_request->lb_context();
        common::JobStatusPtr status(common::job_status(id, context));

        if (((command == "jobsubmit" || command == "jobresubmit")
             && common::is_waiting(status))
            || (command == "jobcancel" && !common::is_cancelled(status))
           ) {

          Info("new " << command << " for " << id);

          tq.insert(std::make_pair(id.toString(), new_request));

          if (command == "jobsubmit" || command == "jobresubmit") {
            log_dequeued(context, input);
          } else if (command == "jobcancel") {
            log_cancel_req(context);
          }

        } else if (command == "match" && !status) {

          tq.insert(std::make_pair(id.toString(), new_request));
          
        } else {

          Info("ignoring " << command << " for " << id);

        }

      } else {

        Request& existing_request(*it->second);

        if (command == "jobsubmit") {

          if (existing_request.marked_match()) {
            // abort the new submit
            std::string message("already existing match request");
            Info(message << ' ' << id);
            // create a fake request to perform the proper actions
            Request r(command_ad, command, id, cleanup);
            cleanup_guard.dismiss();
            r.state(Request::UNRECOVERABLE, message);
          } else {
            // ignore
            Info("ignoring submit " << id);
          }

        } else if (command == "jobresubmit") {

          if (existing_request.marked_match()) {
            // ignore the resubmit
            Info("ignoring resubmit, already existing match request " << id);
          } else {
            // during normal running a resubmit can be seen at this point only
            // if the status of this job is either DELIVERED, PROCESSING (this
            // case is possible because the submit request can be passed to
            // the JC, fail and be resubmitted by the LM before the
            // RequestHandler thread could mark it as DELIVERED) or CANCELLED
            // (this case is possible because the cancel request can be
            // processed - and passed to the JC, moving the request to the
            // CANCELLED state - while the job failed and was resubmitted by
            // the LM)

            // but during start up, w/o the appropriate checks on the status
            // of a job, still to be done) also an initial state
            // (i.e. WAITING) is possible because all the requests are read in
            // one shot from the input

            Debug(
              "jobresubmit " << id << " when in state " << existing_request.state()
            );
            log_dequeued(existing_request.lb_context(), input);
            existing_request.mark_resubmitted();
            existing_request.add_cleanup(cleanup);
            cleanup_guard.dismiss();
          }

        } else if (command == "jobcancel") {

          if (existing_request.marked_match()) {
            // ignore the cancel
            Info("ignoring cancel, already existing match request " << id);
          } else {
            log_cancel_req(existing_request.lb_context());
            existing_request.mark_cancelled();
            existing_request.add_cleanup(cleanup);
            cleanup_guard.dismiss();
          }

        } else if (command == "match") {

          // ignore the match, independently if the existing request with the
          // same id is a (re)submit, cancel or match
          Info("ignoring match " << id);

        }

      }

    } catch (utilities::ClassAdError& e) {
      Info(e.what() << " for " << command_ad_str);
    } catch (common::CannotCreateLBContext& e) {
      Info("Cannot create LB context (error code = " << e.error_code() << ')');
    } catch (InvalidRequest& e) {
      Info("Invalid request: " << command_ad_str);
    }
  }
}

} // {anonymous}

struct DispatcherFromFileList::Impl
{
  std::string m_input;
};

DispatcherFromFileList::DispatcherFromFileList(
  std::string const& input
)
  : m_impl(new Impl)
{
  m_impl->m_input = input;
}

void DispatcherFromFileList::operator()()
try {

  Info("Dispatcher: starting");

  std::string const input_file(m_impl->m_input);
  ExtractorPtr extractor(new extractor_type(input_file));

  if (extractor) {
    Info("Dispatcher: reading from " << input_file);
  } else {
    Error("Dispatcher: cannot read from " << input_file << ". Exiting...");
    return;
  }

  TaskQueue& tq(the_task_queue());

  if (is_recovery_enabled()) {
    Info("Dispatcher: doing recovery");
    recovery(extractor, tq);
  } else {
    Info("Dispatcher: recovery disabled");
  }

  while (!received_quit_signal()) {

    do_transitions(tq, write_end());
    remove_done(tq);
    get_new_requests(extractor, tq, input_file);

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
