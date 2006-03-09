// File: WMReal.cpp
// Authors: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
//          Marco Cecchi <Marco.Cecchi@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <algorithm>

#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "glite/lb/producer.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/JCConfiguration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileListLock.h"
#include "glite/wms/common/utilities/scope_guard.h"

#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAttributes.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"

#include "glite/wms/helper/Request.h"

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"

#include "glite/security/proxyrenewal/renewal.h"

#include "TaskQueue.hpp"
#include "plan.h"
#include "WMReal.h"
#include "lb_utils.h"

namespace utilities = glite::wms::common::utilities;
namespace configuration = glite::wms::common::configuration;
namespace jobid = glite::wmsutils::jobid;
namespace jdl = glite::wms::jdl;

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

std::string get_ice_input()
{
  configuration::Configuration const& config(
    *configuration::Configuration::instance()
  );
  return config.ice()->input();
}

std::string get_jc_input()
{
  configuration::Configuration const& config(
    *configuration::Configuration::instance()
  );
  return config.jc()->input();
}

} // {anonymous}

struct WMReal::Impl {

  Impl()
    : ice_output(get_ice_input()), to_ice(ice_output), to_ice_mx(to_ice),
      jc_output(get_jc_input()), to_jc(jc_output), to_jc_mx(to_jc)
  {
  }

  std::string ice_output;
  utilities::FileList<std::string> to_ice;
  utilities::FileListMutex to_ice_mx;

  std::string jc_output;
  utilities::FileList<std::string> to_jc;
  utilities::FileListMutex to_jc_mx;

};

WMReal::WMReal()
  : m_impl(new Impl)
{
}

namespace {

classad::ClassAd
submit_command_create(std::auto_ptr<classad::ClassAd> job_ad)
{
  classad::ClassAd result;

  result.InsertAttr("protocol", std::string("1.0.0"));
  result.InsertAttr("command", std::string("Submit"));
  result.InsertAttr("Source", 2);
  std::auto_ptr<classad::ClassAd> args(new classad::ClassAd);
  args->Insert("jobad", job_ad.get());
  job_ad.release();
  result.Insert("arguments", args.get());
  args.release();

  return result;
}

}

void
WMReal::submit(classad::ClassAd const* request_ad_p)
{
  if (!request_ad_p) {
    Error("request ad is null");
    return;
  }

  // we make a copy because we change the sequence code
  classad::ClassAd request_ad(*request_ad_p);

  glite::wmsutils::jobid::JobId jobid(jdl::get_edg_jobid(request_ad));
  ContextPtr context = get_context(jobid);
  std::string const sequence_code(get_lb_sequence_code(context));

  // before doing the planning, some helpers may need a
  // more recent sequence code than what appears in
  // the classad originally received
  jdl::set_lb_sequence_code(request_ad, sequence_code);

  std::auto_ptr<classad::ClassAd> planned_ad(Plan(request_ad));
  std::string const ce_id = jdl::get_ce_id(*planned_ad);
  log_match(context, ce_id);

  boost::regex const cream_ce_id(".+/cream-.+");
  bool const is_cream_ce = boost::regex_match(ce_id, cream_ce_id);

  std::string const output(
    is_cream_ce ? m_impl->ice_output : m_impl->jc_output
  );

  log_enqueued_start(context, output);

  std::string const sequence_code_(get_lb_sequence_code(context));
  jdl::set_lb_sequence_code(*planned_ad, sequence_code_);

  classad::ClassAd const cmd(submit_command_create(planned_ad));
  std::string const ad_str = utilities::unparse_classad(cmd);

  try {

    if (is_cream_ce) {
      utilities::FileListLock lock(m_impl->to_ice_mx);
      m_impl->to_ice.push_back(ad_str);
    } else {
      utilities::FileListLock lock(m_impl->to_jc_mx);
      m_impl->to_jc.push_back(ad_str);
    }

  } catch (utilities::FileContainerError const& e) {

    log_enqueued_fail(context, output, ad_str, e.string_error());

  }

  log_enqueued_ok(context, output, ad_str);
}

namespace {

classad::ClassAd
cancel_command_create(
  std::string const& job_id,
  std::string const& sequence_code,
  std::string const& user_x509_proxy
)
{
  classad::ClassAd result;

  result.InsertAttr("version", std::string("1.0.0"));
  result.InsertAttr("command", std::string("jobcancel"));
  std::auto_ptr<classad::ClassAd> args(new classad::ClassAd);
  args->InsertAttr(jdl::JDL::JOBID, job_id);
  args->InsertAttr(jdl::JDL::LB_SEQUENCE_CODE, sequence_code);
  args->InsertAttr(jdl::JDLPrivate::USERPROXY, user_x509_proxy);
  result.InsertAttr("arguments", args.get());
  args.release();

  return result;
}

}

void
WMReal::cancel(jobid::JobId const& id)
{
  ContextPtr context = get_context(id);
  std::string const sequence_code(get_lb_sequence_code(context));

  classad::ClassAd cmd(
    cancel_command_create(
      id.toString(),
      sequence_code,
      get_user_x509_proxy(id)
    )
  );

  std::string const ad_str(utilities::unparse_classad(cmd));

  try {
    utilities::FileListLock lock(m_impl->to_ice_mx);
    m_impl->to_ice.push_back(ad_str);
  } catch (utilities::FileContainerError&) {
  }

  try {
    utilities::FileListLock lock(m_impl->to_jc_mx);
    m_impl->to_jc.push_back(ad_str);
  } catch (utilities::FileContainerError&) {
  }
}

}}}}
