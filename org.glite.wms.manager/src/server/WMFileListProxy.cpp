// File: WMFileListProxy.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "WMFileListProxy.h"

#include <memory>
#include <boost/thread/mutex.hpp>
#include <boost/scoped_ptr.hpp>
#include <classad_distribution.h>

#include "../common/WMFactory.h"
#include "../common/CommandAdManipulation.h"
#include "../common/lb_utils.h"

#include "glite/wms/common/logger/logging_utils.h"

#include "glite/wmsutils/jobid/JobId.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

#include "glite/wms/jdl/JobAdManipulation.h"

#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/FileListLock.h"

#include "glite/lb/context.h"
#include "glite/lb/producer.h"

using namespace classad;

namespace utilities = glite::wms::common::utilities;
namespace jobid = glite::wmsutils::jobid;
namespace jdl = glite::wms::jdl;

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

WMFactory::wm_type wm_id("FileList");

WMFactory::wm_type normalize(WMFactory::wm_type const& id)
{
  WMFactory::wm_type result(id);
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

boost::scoped_ptr<utilities::FileList<std::string> > filelist;
boost::scoped_ptr<utilities::FileListMutex>          filelist_mutex;
boost::mutex                                         filelist_init_mutex;

common::WMImpl* create_wm()
{
  static char const* const function_name = "create_wm";

  namespace configuration = glite::wms::common::configuration;

  configuration::Configuration const* const config
    = configuration::Configuration::instance();

  if (!config) {
    Fatal(function_name, "empty or invalid configuration");
  }
  configuration::WMConfiguration const* const wm_config = config->wm();
  if (!wm_config) {
    Fatal(function_name, "empty WM configuration");
  }

  if (!filelist) {
    boost::mutex::scoped_lock l(filelist_init_mutex);
    if (!filelist) {
      filelist.reset(new utilities::FileList<std::string>(wm_config->input()));
      filelist_mutex.reset(new utilities::FileListMutex(*filelist));
    }
  }

  return new WMFileListProxy(*filelist, *filelist_mutex);
}

struct Register
{
  Register()
  {
    WMFactory::instance()->register_wm(normalize(wm_id), create_wm);
  }
  ~Register()
  {
    WMFactory::instance()->unregister_wm(normalize(wm_id));
  }
};

Register r;

} // {anonymous}

WMFileListProxy::WMFileListProxy(utilities::FileList<std::string>& filelist,
                                 utilities::FileListMutex& filelist_mutex)
  : m_filelist(filelist), m_filelist_mutex(filelist_mutex)
{
}

WMFileListProxy::~WMFileListProxy()
{
}

void
WMFileListProxy::submit(classad::ClassAd const* request_ad)
{
  static char const* const function_name = "WMFileListProxy::submit";

  Debug(function_name, utilities::unparse_classad(*request_ad));
  std::auto_ptr<ClassAd> request_ad_copy(new ClassAd(*request_ad));
  boost::scoped_ptr<ClassAd> command_ad(
    submit_command_create(request_ad_copy.release())
  );
  jobid::JobId request_id(jdl::get_dg_jobid(*request_ad));
  boost::shared_ptr<lb_context_adapter> context_ptr
    = ActiveRequests::instance()->find(request_id);
  if (!context_ptr) {
    Info(function_name, "unable to get lb context for " << request_id);
    return;
  }
  edg_wll_Context context = *context_ptr;
  try {
    m_filelist.push_back(utilities::unparse_classad(*command_ad));
    edg_wll_LogEnQueued(
      context,
      ("WM input queue " + m_filelist.filename()).c_str(),
      utilities::unparse_classad(*request_ad).c_str(),
      "OK",
      ""
    );
  } catch (std::exception& e) { // should discriminate between exception related
                                // to the request and exceptions related to the
                                // whole system
    Warning(
      function_name,
      "caugh exception (" << e.what() << ") for request ad "
      << utilities::unparse_classad(*request_ad)
    );
    edg_wll_LogEnQueued(
      context,
      "WM input queue",
      utilities::unparse_classad(*request_ad).c_str(),
      "FAIL",
      e.what()
    );
    throw;
  } catch (...) {
    Warning(
      function_name,
      "caugh uknown exception for request ad "
      << utilities::unparse_classad(*request_ad)
    );
    edg_wll_LogEnQueued(
      context,
      "WM input queue",
      utilities::unparse_classad(*request_ad).c_str(),
      "FAIL",
      "unknown exception"
    );
    throw;
  }
}

void
WMFileListProxy::resubmit(jobid::JobId const& request_id)
{
  static char const* const function_name = "WMFileListProxy::resubmit";

  Debug(function_name, request_id.toString());

  try {
    boost::shared_ptr<lb_context_adapter> context_ptr
      = ActiveRequests::instance()->find(request_id);
    if (!context_ptr) {
      Info(function_name, "unable to get lb context for " << request_id);
      return;
    }
    edg_wll_Context context = *context_ptr;
    std::string sequence_code = edg_wll_GetSequenceCode(context);
    boost::scoped_ptr<ClassAd> command_ad(
      resubmit_command_create(
        request_id.toString(),
        sequence_code
      )
    );
    m_filelist.push_back(utilities::unparse_classad(*command_ad));
    // should log resubmission request success?
  } catch (std::exception& e) {
    Warning(function_name,
            "caught exception (" << e.what() << ") for id " << request_id.toString());
    // should log resubmisson request failure?
    throw;
  } catch (...) {
    Warning(function_name,
            "caught unknown exception for id " << request_id.toString());
    // should log resubmission request failure?
    throw;
  }

}

void
WMFileListProxy::cancel(jobid::JobId const& request_id)
{
  static char const* const function_name = "WMFileListProxy::cancel";

  Debug(function_name, request_id.toString());

  boost::scoped_ptr<ClassAd> command_ad(cancel_command_create(request_id.toString()));
  try {
    m_filelist.push_back(utilities::unparse_classad(*command_ad));
    // should log cancellation request success?
  } catch (std::exception& e) {
    Warning(function_name,
            "caught exception (" << e.what() << ") for id " << request_id.toString());
    // should log cancellation request failure?
    throw;
  } catch (...) {
    Warning(function_name,
            "caught unknown exception for id " << request_id.toString());
    // should log cancellation request failure?
    throw;
  }
}

}}}} // edg::workload::planning::manager
