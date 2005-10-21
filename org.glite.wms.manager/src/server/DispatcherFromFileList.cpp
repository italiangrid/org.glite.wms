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
#include <boost/regex.hpp>
#include <openssl/pem.h>
#include <openssl/x509.h>
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
#include "CommandAdManipulation.h"
#include "lb_utils.h"

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

std::string dispatcher_id("FileList");

DispatcherImpl* create_dispatcher()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  if (!config
      || config->get_module() != configuration::ModuleType::workload_manager) {
    Fatal("empty or invalid configuration");
  }
  configuration::WMConfiguration const* const wm_config = config->wm();
  if (!wm_config) {
    Fatal("empty WM configuration");
  }
  std::string file(wm_config->input());

  // the extractor is shared between the dispatcher and the cleanup functions
  // (see CleanUp below); the latter can live beyond the scope of the
  // dispatcher build the extractor outside the dispatcher to make this
  // sharing more apparent (i.e. the extractor does not belong to the
  // dispatcher) even if it could be built within the dispatcher ctor

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
    DispatcherFactory::instance()->unregister_dispatcher(
      normalize(dispatcher_id)
    );
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

bool
is_recovery_enabled()
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

  return wm_config->enable_recovery();
}

void log_dequeued(common::ContextPtr const& context)
{
  std::string input_name(get_input_name());
  char const* const local_jobid = ""; // not needed because no real local id

  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(edg_wll_LogDeQueuedProxy, _1, input_name.c_str(), local_jobid),
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
  } else if (command == "match") {
    bool id_exists;
    jobid::JobId match_jobid;

    std::string jobidstr(
      requestad::get_edg_jobid(
        *common::match_command_get_ad(command_ad),
        id_exists
      )
    );

    if (id_exists) {
      match_jobid.fromString(jobidstr);
    } else {
      match_jobid.setJobId("localhost", 6000, "");
    }

    return match_jobid;
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

typedef utilities::FLExtractor<std::string> extractor_type;
typedef std::vector<extractor_type::iterator> requests_type;

typedef std::vector<
  boost::tuple<
    std::string,                // command
    extractor_type::iterator,   // iterator
    ClassAdPtr                  // already parsed command classad
  >
> requests_for_id_type;         // requests for a specific id

typedef boost::tuple<
  std::string,                  // jobid
  requests_for_id_type
> id_requests_type;

typedef std::vector<id_requests_type> id_to_requests_type;

class id_equals
{
  std::string m_id;
public:
  id_equals(std::string const& id)
    : m_id(id)
  {
  }
  bool operator()(id_to_requests_type::value_type const& v) const
  {
    std::string const& id = v.get<0>();
    return m_id == id;
  }
};

void catalog_requests_by_id(
  boost::shared_ptr<extractor_type> const& extractor,
  requests_type const& requests,
  id_to_requests_type& id_to_requests
)
{
  requests_type::const_iterator b = requests.begin();
  requests_type::const_iterator const e = requests.end();
  for ( ; b != e; ++b) {
    extractor_type::iterator const request_it = *b;

    boost::function<void()> cleanup(CleanUp(extractor, request_it));
    // if the request is not valid, cleanup automatically
    utilities::scope_guard cleanup_guard(cleanup);

    try {
      std::string const command_ad_str(*request_it);
      ClassAdPtr command_ad(
        utilities::parse_classad(command_ad_str)
      );

      RequestChecker request_checker(*command_ad);
      std::string command = request_checker.command();
      jobid::JobId id = request_checker.id();

      id_to_requests_type::iterator it(
        std::find_if(
          id_to_requests.begin(),
          id_to_requests.end(),
          id_equals(id.toString())
        )
      );

      if (it != id_to_requests.end()) {
        it->get<1>().push_back(
          boost::make_tuple(command, request_it, command_ad)
        );
      } else {
        id_to_requests.push_back(
          boost::make_tuple(
            id.toString(),
            requests_for_id_type()
          )
        );
        id_to_requests.back().get<1>().push_back(
          boost::make_tuple(command, request_it, command_ad)
        );
      }

      cleanup_guard.dismiss();

    } catch (utilities::ClassAdError& e) {
      Info(e.what());
    } catch (InvalidRequest& e) {
      Info(e.str());
    }
  }
}

bool is_match(requests_for_id_type::value_type const& v)
{
  std::string const& command = v.get<0>();
  return command == "match";
}

bool some_match(requests_for_id_type const& requests_for_id)
{
  return
    std::find_if(
      requests_for_id.begin(),
      requests_for_id.end(),
      is_match
    ) != requests_for_id.end();
}

bool is_not_resubmit(requests_for_id_type::value_type const& v)
{
  std::string const& command = v.get<0>();
  return command != "jobresubmit";
}

typedef boost::shared_ptr<edg_wll_JobStat> JobStatusPtr;

void delete_job_status(edg_wll_JobStat* p)
{
  edg_wll_FreeStatus(p);
  delete p;
}

JobStatusPtr job_status(jobid::JobId const& id)
{
  std::string const x509_proxy = common::get_user_x509_proxy(id);
  std::string const sequence_code;
  try {
    common::ContextPtr context(
      common::create_context_proxy(id, x509_proxy, sequence_code)
    );
    int const flags = 0;
    JobStatusPtr status(new edg_wll_JobStat, delete_job_status);
    if (!edg_wll_JobStatusProxy(
          context.get(),
          id.getId(),
          flags,
          status.get())
       ) {
      return status;
    } else {
      return JobStatusPtr();
    }
  } catch (common::CannotCreateLBContext const&) {
    return JobStatusPtr();
  }
}

bool is_waiting(JobStatusPtr const& status)
{
  return status && status->state == EDG_WLL_JOB_WAITING;
}

bool is_cancelled(JobStatusPtr const& status)
{
  return status && status->state == EDG_WLL_JOB_CANCELLED;
}

class clean_ignore
{
  boost::shared_ptr<extractor_type> m_extractor;
  std::string m_id;
public:
  clean_ignore(
    boost::shared_ptr<extractor_type> const& extractor,
    std::string const& id
  )
    : m_extractor(extractor), m_id(id)
  {
  }
  void operator()(requests_for_id_type::value_type const& v) const
  {
    Debug("ignoring " << v.get<0>() << " request for " << m_id);
    CleanUp(m_extractor, v.get<1>())();
  }
};

void single_request_recovery(
  id_requests_type const& id_requests,
  boost::shared_ptr<extractor_type> const& extractor,
  TaskQueue& tq
)
{
  std::string const& id = id_requests.get<0>();
  requests_for_id_type const& requests_for_id = id_requests.get<1>();
  assert(requests_for_id.size() == 1);
  requests_for_id_type::value_type const& req = requests_for_id.front();
  std::string const& command = req.get<0>();
 
  JobStatusPtr status(job_status(jobid::JobId(id)));

  bool valid = true;
  if (command == "match" && !status) {
    Info("matching");
  } else if (command == "jobsubmit" && is_waiting(status)) {
    Info("submitting");
  } else if (command == "jobcancel" && !is_cancelled(status)) {
    Info("cancelling");
  } else if (command == "jobresubmit" && is_waiting(status)) {
    Info("resubmitting");
  } else {
    assert(false && "invalid command");
    valid = false;
  }

  if (valid) {
    extractor_type::iterator const& request_it = req.get<1>();
    boost::function<void()> cleanup(CleanUp(extractor, request_it));
    ClassAdPtr const& command_ad = req.get<2>();
    RequestPtr request(new Request(*command_ad, cleanup, id));
    tq.insert(std::make_pair(id, request));
  } else {
    clean_ignore(extractor, id)(req);
  }
}

std::string summary(requests_for_id_type const& requests_for_id)
{
  std::string result;

  requests_for_id_type::const_iterator b = requests_for_id.begin();
  requests_for_id_type::const_iterator const e = requests_for_id.end();
  for ( ; b != e; ++b) {
    std::string const& command = b->get<0>();
    result += toupper(command[0]);
  }

  return result;
}

bool is_cancel(requests_for_id_type::value_type const& v)
{
  std::string const& command = v.get<0>();
  return command == "cancel";
}

void multiple_request_recovery(
  id_requests_type const& id_requests,
  boost::shared_ptr<extractor_type> const& extractor,
  TaskQueue& tq
)
{
  std::string const& id = id_requests.get<0>();
  requests_for_id_type const& requests_for_id = id_requests.get<1>();
  assert(requests_for_id.size() > 1);

  JobStatusPtr status(job_status(jobid::JobId(id)));

  std::string summary(summary(requests_for_id));
  std::string status_summary(" (status ");
  if (status) {
    status_summary += boost::lexical_cast<std::string>(status->state) + ')';
  } else {
    status_summary += "not available)";
  }
  Info("multiple requests [" << summary << "] for " << id << status_summary);

  // invalid patterns
  boost::regex const nonmatch_match_nonmatch_re("[^M]*M[^M]*");
  boost::regex const nonsubmit_submit_re("[^S]+S.*");
  boost::regex const more_submits_re("S.*S.*");

  // possible patterns
  boost::regex const more_matches_re("M+");
  boost::regex const a_cancel_re(".*C.*");
  boost::regex const no_cancel_re("[^C]*");

  if (boost::regex_match(summary, nonmatch_match_nonmatch_re)
      || boost::regex_match(summary, nonsubmit_submit_re)
      || boost::regex_match(summary, more_submits_re)
     ) {

    Info("invalid pattern; ignoring all requests");
    std::for_each(
      requests_for_id.begin(),
      requests_for_id.end(),
      clean_ignore(extractor, id)
    );

  } else if (boost::regex_match(summary, more_matches_re) && !status) {

    Info("matching");

    requests_for_id_type::value_type const& match_req(requests_for_id.back());
    
    extractor_type::iterator request_it = match_req.get<1>();
    ClassAdPtr const& command_ad = match_req.get<2>();

    boost::function<void()> cleanup(CleanUp(extractor, request_it));
    RequestPtr request(new Request(*command_ad, cleanup, id));
    tq.insert(std::make_pair(id, request));
    
    std::for_each(
      requests_for_id.begin(),
      boost::prior(requests_for_id.end()),
      clean_ignore(extractor, id)
    );

  } else if (boost::regex_match(summary, a_cancel_re)
             && !is_cancelled(status)) {

    Info("cancelling");

    // find the request corresponding to a cancel
    requests_for_id_type::const_iterator const cancel_it(
      std::find_if(requests_for_id.begin(), requests_for_id.end(), is_cancel)
    );
    assert(cancel_it != requests_for_id.end());
    extractor_type::iterator request_it = cancel_it->get<1>();
    ClassAdPtr const& command_ad = cancel_it->get<2>();

    boost::function<void()> cleanup(CleanUp(extractor, request_it));
    RequestPtr request(new Request(*command_ad, cleanup, id));
    tq.insert(std::make_pair(id, request));

    std::for_each(
      requests_for_id.begin(),
      cancel_it,
      clean_ignore(extractor, id)
    );
    std::for_each(
      boost::next(cancel_it),
      requests_for_id.end(),
      clean_ignore(extractor, id)
    );

  } else if (boost::regex_match(summary, no_cancel_re)
             && is_waiting(status)) {

    // take the last request, which must be a resubmit
    Info("resubmitting");

    requests_for_id_type::value_type const& last_resubmit(
      requests_for_id.back()
    );
    extractor_type::iterator request_it = last_resubmit.get<1>();
    ClassAdPtr const& command_ad = last_resubmit.get<2>();

    boost::function<void()> cleanup(CleanUp(extractor, request_it));
    RequestPtr request(new Request(*command_ad, cleanup, id));
    tq.insert(std::make_pair(id, request));

    std::for_each(
      requests_for_id.begin(),
      boost::prior(requests_for_id.end()),
      clean_ignore(extractor, id)
    );

  } else {

    Info("invalid pattern; ignoring all requests");
    std::for_each(
      requests_for_id.begin(),
      requests_for_id.end(),
      clean_ignore(extractor, id)
    );

  }
}

class recover
{
  boost::shared_ptr<extractor_type> m_extractor;
  TaskQueue* m_tq;
public:
  recover(
    boost::shared_ptr<extractor_type> const& extractor,
    TaskQueue& tq
  )
    : m_extractor(extractor), m_tq(&tq)
  {
  }
  void operator()(id_to_requests_type::value_type const& id_requests) const
  {
    std::string const& id = id_requests.get<0>();
    requests_for_id_type const& requests_for_id = id_requests.get<1>();
    assert(!requests_for_id.empty());
    Info("recovering " << id);
    if (requests_for_id.size() == 1) {
      single_request_recovery(id_requests, m_extractor, *m_tq);
    } else {

      multiple_request_recovery(id_requests, m_extractor, *m_tq);
    }
  }


};

void
recovery(
  boost::shared_ptr<extractor_type> const& extractor,
  TaskQueue& tq
)
{
  requests_type requests(extractor->get_all_available());
  id_to_requests_type id_to_requests;
  catalog_requests_by_id(extractor, requests, id_to_requests);

  std::for_each(
    id_to_requests.begin(),
    id_to_requests.end(),
    recover(extractor, tq)
  );
}

void
get_new_requests(
  boost::shared_ptr<extractor_type>& extractor,
  TaskQueue& tq
)
{
  requests_type new_requests(extractor->get_all_available());

  for (requests_type::iterator it = new_requests.begin();
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

      JobStatusPtr status(job_status(id));

      TaskQueue::iterator it = tq.find(id.toString());

      if (it == tq.end()) {

        if (((command == "jobsubmit" || command == "jobresubmit")
             && is_waiting(status))
            || (command == "jobcancel" && !is_cancelled(status))
           ) {

          Info("new " << command << " for " << id);

          cleanup_guard.dismiss();
          RequestPtr request(new Request(*command_ad, cleanup, id));
          tq.insert(std::make_pair(id.toString(), request));

          if (command == "jobsubmit" || command == "jobresubmit") {
            log_dequeued(request->lb_context());
          } else if (command == "jobcancel") {
            log_cancel_req(request->lb_context());
          }

        } if (command == "match" && !status) {

          cleanup_guard.dismiss();
          RequestPtr request(new Request(*command_ad, cleanup, id));
          tq.insert(std::make_pair(id.toString(), request));
          
        } else {

          Info("ignoring " << command << " for " << id);

        }

      } else {

        RequestPtr request = it->second;

        if (command == "jobsubmit") {

          if (request->marked_match()) {
            // abort the submit, via a fake request
            cleanup_guard.dismiss();
            Request fake_request(*command_ad, cleanup, id);
            std::string message("already existing match request");
            Info(message << ' ' << id);
            fake_request.state(Request::UNRECOVERABLE, message);
          } else {
            // ignore
            Info("ignoring submit " << id);
          }

        } else if (command == "jobresubmit") {

          if (request->marked_match()) {
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
              "jobresubmit " << id << " when in state " << request->state()
            );
            log_dequeued(request->lb_context());
            request->mark_resubmitted();
            cleanup_guard.dismiss();
            request->add_cleanup(cleanup);
          }

        } else if (command == "jobcancel") {

          if (request->marked_match()) {
            // ignore the cancel
            Info("ignoring cancel, already existing match request " << id);
          } else {
            log_cancel_req(request->lb_context());
            request->mark_cancelled();
            cleanup_guard.dismiss();
            request->add_cleanup(cleanup);
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

DispatcherFromFileList::DispatcherFromFileList(
  boost::shared_ptr<extractor_type> extractor
)
  : m_extractor(extractor)
{
}

void
DispatcherFromFileList::run(pipe_type::write_end_type& write_end)
try {

  Info("Dispatcher: starting");

  TaskQueue& tq(the_task_queue());

  if (is_recovery_enabled()) {
    Info("Dispatcher: doing recovery");
    recovery(m_extractor, tq);
  } else {
    Info("Dispatcher: recovery disabled");
  }

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
