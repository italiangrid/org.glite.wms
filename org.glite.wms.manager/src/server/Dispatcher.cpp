// File: Dispatcher.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "Dispatcher.h"
#include <algorithm>
#include <cctype>
#include "DispatcherImpl.h"
#include "DispatcherFactory.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

namespace configuration = glite::wms::common::configuration;

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

std::string normalize(std::string const& id)
{
  std::string result(id);
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

std::string
get_dispatcher_type()
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

  return normalize(wm_config->dispatcher_type());
}

} // {anonymous}

Dispatcher::Dispatcher()
  : m_impl(DispatcherFactory::instance()->create_dispatcher(get_dispatcher_type()))
{
}

Dispatcher::~Dispatcher()
{
}

void
Dispatcher::run()
{
  m_impl->run(write_end());     // the pipe write end is not available at
                                // construction time
}

} // namespace server
} // namespace manager
} // namespace wms
} // namespace glite
