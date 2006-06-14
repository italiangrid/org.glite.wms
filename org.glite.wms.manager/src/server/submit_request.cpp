// File: submit_request.cpp
// Author: Francesco Giacomini
// Copyright (c) Members of the EGEE Collaboration 2004
// For license conditions see http://public.eu-egee.org/license/license.html

// $Id$

#include "submit_request.h"

#include "glite/wms/common/utilities/wm_commands.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/jdl/JobAdManipulation.h"
#include "glite/jdl/PrivateAdManipulation.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/lb/producer.h"

#include "lb_utils.h"

namespace jobid = glite::wmsutils::jobid;
namespace jdl = glite::jdl;
namespace utilities = glite::wms::common::utilities;

namespace glite {
namespace wms {
namespace manager {
namespace server {

struct Submit::Pimpl
{
  boost::shared_ptr<classad::ClassAd> m_jdl;
  glite::wmsutils::jobid::JobId m_id;
  typedef std::vector<boost::function<void()> > input_cleaners_type;
  input_cleaners_type m_input_cleaners;
  State m_state;
  std::string m_message;
  std::time_t m_last_processed;
  ContextPtr m_lb_context;
  ContextPtr m_cancel_context;
  bool m_resubmitted;

  //match options: file name, number of results and include or not brokerinfo
  boost::tuple<std::string, int, bool> m_match_parameters;
  std::time_t m_expiry_time;
};

namespace {

static time_t const SECONDS_PER_DAY = 24*60*60;

std::string
aux_get_sequence_code(
  classad::ClassAd const& command_ad,
  std::string const& command
)
{
  classad::ClassAd const* job_ad =
    utilities::submit_command_get_ad(command_ad);
  return jdl::get_lb_sequence_code(*job_ad);
}

std::string
aux_get_x509_proxy(
  classad::ClassAd const& command_ad,
  std::string const& command,
  jobid::JobId const& id
)
{
  classad::ClassAd const* job_ad = utilities::submit_command_get_ad(command_ad);
  return jdl::get_x509_user_proxy(*job_ad);
}

glite::wmsutils::jobid::JobId
aux_get_id(classad::ClassAd const& command_ad, std::string const& command)
{
  if (command == "jobsubmit") {
    return jobid::JobId(
      jdl::get_edg_jobid(
        *utilities::submit_command_get_ad(command_ad)
      )
    );
  } else if (command == "jobresubmit") {
    return jobid::JobId(utilities::resubmit_command_get_id(command_ad));
  }
  // the following is just to avoid a warning about "control reaches end of
  // non-void function", but there is no possibility to arrive here
  return jobid::JobId();
}

}

boost::tuple<
  std::string,                  // command
  jobid::JobId,                 // jobid
  std::string,                  // sequence code
  std::string                   // x509_proxy
>
parse_request(classad::ClassAd const& command_ad)
{
  std::string command;
  jobid::JobId id;
  std::string sequence_code;
  std::string x509_proxy;

  command = utilities::command_get_command(command_ad);
  id = aux_get_id(command_ad, command);
  sequence_code = aux_get_sequence_code(command_ad, command);
  x509_proxy = aux_get_x509_proxy(command_ad, command, id);

  return boost::make_tuple(command, id, sequence_code, x509_proxy);
}

Submit::Submit(classad::ClassAd& command_ad,
    std::string const& command,
    jobid::JobId const& id,
    boost::function<void()> const& cleanup
)
  : m_pimpl(new Pimpl)
{
  m_pimpl->m_id = id;
  m_pimpl->m_state = WAITING;
  m_pimpl->m_last_processed = 0;        // make it very old
  m_pimpl->m_resubmitted = false;
  m_pimpl->m_expiry_time = std::time(0) + SECONDS_PER_DAY;

  std::string x509_proxy;
  std::string sequence_code;

  if (command == "jobsubmit") {

    std::auto_ptr<classad::ClassAd> ad(utilities::submit_command_remove_ad(command_ad));
    m_pimpl->m_jdl = ad;

    bool exists = false;
    int expiry_time = jdl::get_expiry_time(*(m_pimpl->m_jdl), exists);
    if (exists) {
      m_expiry_time = expiry_time;
    }

    x509_proxy = jdl::get_x509_user_proxy(*(m_pimpl->m_jdl));
    sequence_code = jdl::get_lb_sequence_code(*(m_pimpl->m_jdl));
    m_lb_context = create_context(m_pimpl->m_id, x509_proxy, sequence_code);

  } else if (command == "jobresubmit") {

    mark_resubmitted();

    x509_proxy = get_user_x509_proxy(m_pimpl->m_id);
    sequence_code = utilities::resubmit_command_get_lb_sequence_code(command_ad);
    m_lb_context = create_context(m_pimpl->m_id, x509_proxy, sequence_code);

  } 

  m_input_cleaners.push_back(cleanup);
}

Submit::~Submit()
{
  try {
    switch (m_state) {
    case Request::UNRECOVERABLE:
      log_abort(m_lb_context, m_message);
      apply(m_input_cleaners);
      break;

    case Request::DELIVERED:
      apply(m_input_cleaners);
      break;

    case Request::CANCELLED:
      log_cancelled(m_cancel_context);
      apply(m_input_cleaners);
      break;

    case Request::CANCEL_DELIVERED:
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

void
Submit::jdl(std::auto_ptr<classad::ClassAd> jdl)
{
  m_pimpl->m_jdl = jdl;

  // reset the expiry time if the corresponding jdl attribute exists
  bool exists;
  int expiry_time = jdl::get_expiry_time(*(m_pimpl->m_jdl), exists);
  if (exists) {
    m_expiry_time = expiry_time;
  }
}

std::time_t
Submit::expiry_time() const
{
  return m_expiry_time;
}

void
Submit::state(State s, std::string const& message = std::string())
{
  m_state = s;
  m_message = message;
}

void
Submit::mark_cancelled(ContextPtr cancel_context)
{
  m_cancel_context = cancel_context;
}

boost::tuple<std::string, int, bool>
Submit::match_parameters() const
{
  return m_match_parameters;
}

void
Submit::add_cleanup(boost::function<void()> const& cleanup);
{
  m_input_cleaners.push_back(cleanup);
}

}}}} // glite::wms::manager::server
