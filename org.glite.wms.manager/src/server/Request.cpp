#include "Request.hpp"
#include "lb_utils.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/lb/producer.h"
#include <map>
#include <boost/bind.hpp>

#include "CommandAdManipulation.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"

namespace jobid = glite::wmsutils::jobid;
namespace requestad = glite::wms::jdl;
namespace common = glite::wms::manager::common;
namespace utilities = glite::wms::common::utilities;

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

  return common::create_context(id, x509_proxy, sequence_code);
}

}

Request::Request(
  classad::ClassAd const& command_ad,
  boost::function<void()> const& cleanup, 
  jobid::JobId const& id
)
  : m_id(id),
    m_state(WAITING),
    m_cancelled(false),
    m_resubmitted(false)
{
  if (!common::command_is_valid(command_ad)) {
    throw InvalidRequest(utilities::unparse_classad(command_ad));
  }
  std::string command = common::command_get_command(command_ad);
  
  if (command != "match") { //match doesn't need any context or jobid  ?!?!
    m_lb_context = aux_create_context(command_ad, command, m_id);
  }
  
  #warning TODO better error handling
  // assert(m_lb_context); There isn't any context for match
  
  if (command == "jobsubmit") {
    m_jdl.reset(static_cast<classad::ClassAd*>(common::submit_command_get_ad(command_ad)->Copy()));
    m_jdl->SetParentScope(0);
  } else if (command == "jobresubmit") {
    mark_resubmitted();
  } else if (command == "jobcancel") {
    state(DELIVERED); 
    mark_cancelled();
  } else if (command == "match") {
    m_jdl.reset(static_cast<classad::ClassAd*>(common::match_command_get_ad(command_ad)->Copy()));
    m_jdl->SetParentScope(0);
    mark_match(common::match_command_get_file(command_ad)); 
  }
  
  m_input_cleaners.push_back(cleanup);
  m_last_processed.sec = 0;
  m_last_processed.nsec = 0;
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
    boost::bind(edg_wll_LogAbort, _1, msg.c_str()),
    context
  );
  if (err) {
    Warning(common::get_logger_message("edg_wll_LogAbort", err, context, ctx));
  }
}

void log_cancelled(common::ContextPtr const& context)
{
  int lb_error;
  common::ContextPtr ctx;
  boost::tie(lb_error, ctx) = common::lb_log(
    boost::bind(edg_wll_LogCancelDONE, _1, "SUCCESSFULLY CANCELLED"),
    context
  );
  if (lb_error) {
    Warning(common::get_logger_message("edg_wll_LogCancelDONE", lb_error, context, ctx));
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

}}}}
