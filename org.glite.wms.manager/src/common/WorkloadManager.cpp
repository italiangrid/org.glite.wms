// File: WorkloadManager.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "WorkloadManager.h"
#include <algorithm>
#include <cctype>
#include "WMFactory.h"
#include "WMImpl.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

namespace configuration = glite::wms::common::configuration;
namespace jobid = glite::wmsutils::jobid;

namespace glite {
namespace wms {
namespace manager {
namespace common {

namespace {

WMFactory::wm_type
normalize(
  WMFactory::wm_type const& id)
{
  WMFactory::wm_type result(id);
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

WMFactory::wm_type default_wm_id("Real");

WMFactory::wm_type
get_wm_type()
{
  WMFactory::wm_type result(default_wm_id);

  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  if (!config) {
    Fatal("empty or invalid configuration");
  }
  configuration::WMConfiguration const* const wm_config = config->wm();
  if (!wm_config) {
    Fatal("empty or invalid WM configuration");
  }

  if (config->get_module() != configuration::ModuleType::workload_manager) {
    Fatal("WM Proxy not implemented yet");
    //    result = wm_config->proxy_type();
  }

  return normalize(result);
}

}

WorkloadManager::WorkloadManager()
  : m_impl(WMFactory::instance()->create_wm(get_wm_type()))
{
}

WorkloadManager::~WorkloadManager()
{
}

void
WorkloadManager::submit(classad::ClassAd const* request_ad)
{
  m_impl->submit(request_ad);
}

void
WorkloadManager::cancel(jobid::JobId const& request_id)
{
  m_impl->cancel(request_id);
}

}}}} // glite::wms::manager::common
