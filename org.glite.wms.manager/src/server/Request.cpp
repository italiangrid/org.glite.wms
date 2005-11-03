// File: Request.cpp
// Author: Francesco Giacomini
// Copyright (c) Members of the EGEE Collaboration 2004
// For license conditions see http://public.eu-egee.org/license/license.html

// $Id$

#include "Request.hpp"
#include <map>
#include <boost/bind.hpp>
#include "lb_utils.h"
#include "CommandAdManipulation.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/lb/producer.h"

namespace jobid = glite::wmsutils::jobid;
namespace requestad = glite::wms::jdl;
namespace common = glite::wms::manager::common;
namespace utilities = glite::wms::common::utilities;
namespace configuration = glite::wms::common::configuration;

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

common::ContextPtr
aux_create_context(
  classad::ClassAd const& command_ad,
  std::string const& command,
  jobid::JobId const& id
)
{
  std::string x509_proxy;
  std::string sequence_code;

  if (command == "jobsubmit") {
    classad::ClassAd const* job_ad = common::submit_command_get_ad(command_ad);
    x509_proxy = requestad::get_x509_user_proxy(*job_ad);
    sequence_code = requestad::get_lb_sequence_code(*job_ad);
  } else if (command == "jobcancel") {
    x509_proxy = common::get_user_x509_proxy(id);
    sequence_code = common::cancel_command_get_lb_sequence_code(command_ad);
  } else if (command == "jobresubmit") {
    x509_proxy = common::get_user_x509_proxy(id);
    sequence_code = common::resubmit_command_get_lb_sequence_code(command_ad);
  }

  //  return common::create_context(id, x509_proxy, sequence_code);
  return common::create_context_proxy(id, x509_proxy, sequence_code);
}

time_t const SECONDS_PER_DAY = 86400;

glite::wmsutils::jobid::JobId
aux_get_id(classad::ClassAd const& command_ad, std::string const& command)
{
  namespace jobid = glite::wmsutils::jobid;
  namespace requestad = glite::wms::jdl;

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

}

std::pair<
  std::string,                  // command
  jobid::JobId                  // jobid
>
check_request(classad::ClassAd const& command_ad)
{
  std::string command;
  jobid::JobId id;

  if (common::command_is_valid(command_ad)) {
    command = common::command_get_command(command_ad);
    id = aux_get_id(command_ad, command);
  }

  return std::make_pair(command, id);
}

Request::Request(
  classad::ClassAd& command_ad,
  std::string const& command,
  jobid::JobId const& id,
  boost::function<void()> const& cleanup
)
  : m_id(id),
    m_state(WAITING),
    m_last_processed(0),
    m_cancelled(false),
    m_resubmitted(false),
    m_expiry_time(std::time(0) + SECONDS_PER_DAY)
{
  std::string x509_proxy;
  std::string sequence_code;

  if (command == "jobsubmit") {

    m_jdl.reset(common::submit_command_remove_ad(command_ad));

    bool exists = false;
    int expiry_time = requestad::get_expiry_time(*m_jdl, exists);
    if (exists) {
      m_expiry_time = expiry_time;
    }

    x509_proxy = requestad::get_x509_user_proxy(*m_jdl);
    sequence_code = requestad::get_lb_sequence_code(*m_jdl);
    m_lb_context = common::create_context_proxy(m_id, x509_proxy, sequence_code);

  } else if (command == "jobresubmit") {

    mark_resubmitted();

    x509_proxy = common::get_user_x509_proxy(m_id);
    sequence_code = common::resubmit_command_get_lb_sequence_code(command_ad);
    m_lb_context = common::create_context_proxy(m_id, x509_proxy, sequence_code);

  } else if (command == "jobcancel") {

    state(DELIVERED);
    mark_cancelled();

    x509_proxy = common::get_user_x509_proxy(m_id);
    sequence_code = common::cancel_command_get_lb_sequence_code(command_ad);
    m_lb_context = common::create_context_proxy(m_id, x509_proxy, sequence_code);

  } else if (command == "match") {

    m_jdl.reset(common::match_command_remove_ad(command_ad));

    std::string filename = common::match_command_get_file(command_ad);
    int number = common::match_command_get_number_of_results(command_ad);
    bool brokerinfo = common::match_command_get_include_brokerinfo(command_ad);

    m_match_parameters = boost::make_tuple(filename, number, brokerinfo);
  }

  m_input_cleaners.push_back(cleanup);

}  

namespace {

void log_abort(
  jobid::JobId const& id,
  common::ContextPtr const& context,
  std::string const& msg
)
{
  Error(msg << " for " << id.toString());
  int err; common::ContextPtr ctx;
  boost::tie(err, ctx) = common::lb_log(
    boost::bind(edg_wll_LogAbortProxy, _1, msg.c_str()),
    context
  );
  if (err) {
    Warning(
      common::get_logger_message(
        "edg_wll_LogAbortProxy",
        err,
        context,
        ctx
      )
    );
  }
}

void log_cancelled(common::ContextPtr const& context)
{
  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(edg_wll_LogCancelDONEProxy, _1, "SUCCESSFULLY CANCELLED"),
    context
  );
  if (lb_error) {
    Warning(
      common::get_logger_message(
        "edg_wll_LogCancelDONEProxy",
        lb_error,
        context,
        ctx
      )
    );
  }
}

}

namespace {

typedef std::vector<boost::function<void()> > cleaners_type;

void apply(cleaners_type const& input_cleaners)
{
  for (cleaners_type::const_iterator it = input_cleaners.begin();
       it != input_cleaners.end(); ++it) {
    (*it)();
  }
}

}

Request::~Request()
{
  try {
    switch (m_state) {
    case Request::UNRECOVERABLE:
      log_abort(m_id, m_lb_context, m_message);
      apply(m_input_cleaners);
      break;

    case Request::DELIVERED:
      assert(!marked_cancelled());
      apply(m_input_cleaners);
      break;

    case Request::CANCELLED:
      log_cancelled(m_lb_context);
      apply(m_input_cleaners);
      break;

    default:
      // something strange has happened, e.g. a Control-C
      // do nothing
      break;
    }
  } catch (...) { // don't allow exceptions to escape
  }
}

void Request::jdl(classad::ClassAd* jdl)
{
  m_jdl.reset(jdl);

  // reset the expiry time if the corresponding jdl attribute exists
  bool exists;
  int expiry_time = requestad::get_expiry_time(*m_jdl, exists);
  if (exists) {
    m_expiry_time = expiry_time;
  }
}

}}}} // glite::wms::manager::server
