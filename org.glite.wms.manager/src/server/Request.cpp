#include "Request.hpp"
#include "lb_utils.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
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

}

Request::Request(
  classad::ClassAd const& command_ad,
  boost::function<void()> const& cleanup,
  jobid::JobId const& id
)
  : m_id(id),
    m_state(WAITING),
    m_last_processed(0),        // make it very old
    m_cancelled(false),
    m_resubmitted(false),
    m_expiry_time(std::time(0) + SECONDS_PER_DAY)
{
  if (!common::command_is_valid(command_ad)) {
    throw InvalidRequest(utilities::unparse_classad(command_ad));
  }
  std::string command = common::command_get_command(command_ad);

  if (command != "match") { //match doesn't need any context or jobid  ?!?!
    m_lb_context = aux_create_context(command_ad, command, m_id);
  }

  if (command == "jobsubmit") {

    m_jdl.reset(static_cast<classad::ClassAd*>(common::submit_command_get_ad(command_ad)->Copy()));
    m_jdl->SetParentScope(0);

    bool attribute_exists;
    int expiry_time = requestad::get_expiry_time(*m_jdl, attribute_exists);
    if (attribute_exists) {
      m_expiry_time = expiry_time;
    }

  } else if (command == "jobresubmit") {

    mark_resubmitted();

  } else if (command == "jobcancel") {

    state(DELIVERED);
    mark_cancelled();

  } else if (command == "match") {

    m_jdl.reset(static_cast<classad::ClassAd*>(common::match_command_get_ad(command_ad)->Copy()));
    m_jdl->SetParentScope(0);

    std::string filename = common::match_command_get_file(command_ad);
    int number = common::match_command_get_number_of_results(command_ad);
    bool brokerinfo = common::match_command_get_include_brokerinfo(command_ad);
    boost::tuple<std::string, int, bool> options(filename, number, brokerinfo);

    m_match_parameters = options;
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

}}}}
