// File: DispatcherFromFileList.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "DispatcherFromFileList.h"

#include <algorithm>
#include <cctype>
#include <boost/thread/xtime.hpp>
#include <boost/tuple/tuple.hpp>

#include "DispatcherFactory.h"
#include "dispatching_utils.h"
#include "signal_handling.h"

#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

#include "glite/wmsutils/exception/Exception.h"

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace task = glite::wms::common::task;
namespace configuration = glite::wms::common::configuration;
namespace utilities = glite::wms::common::utilities;

namespace {

dispatcher_type dispatcher_id("FileList");

DispatcherImpl* create_dispatcher()
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
  std::string file(wm_config->input());

  // the extractor is shared between the dispatcher and the cleanup functions
  // (see CleanUp below); the latter can live beyond the scope of the dispatcher
  // build the extractor outside the dispatcher to make this sharing more
  // apparent (i.e. the extractor does not belong to the dispatcher) even if it
  // could be built within the dispatcher ctor

  typedef DispatcherFromFileList::extractor_type extractor_type;
  boost::shared_ptr<extractor_type> extractor(new extractor_type(file));
  if (!extractor) {
    Fatal("cannot build FileList extractor");
  }

  Info("reading from " << file);

  return new DispatcherFromFileList(extractor);
}

dispatcher_type normalize(dispatcher_type const& id)
{
  dispatcher_type result(id);
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
    DispatcherFactory::instance()->unregister_dispatcher(normalize(dispatcher_id));
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

} // {anonymous} 

DispatcherFromFileList::DispatcherFromFileList(boost::shared_ptr<extractor_type> extractor)
  : m_extractor(extractor)
{
}

void
DispatcherFromFileList::run(task::PipeWriteEnd<pipe_value_type>& write_end)
try {

  Info("Dispatcher: starting");

  bool received_quit_command = false;
  bool was_sleeping = false;

  while (!(received_quit_command || received_quit_signal())) {

    try {

      extractor_type::iterator next;
      bool is_not_end;

      boost::tie(next, is_not_end) = m_extractor->try_get_one();

      if (is_not_end) {
        if (was_sleeping) {
          Debug("stop sleeping");
          was_sleeping = false;
        }

        received_quit_command = !process(*next, CleanUp(m_extractor, next), write_end);

      } else {

        if (!was_sleeping) {
          Debug("sleeping (checking every second for new input)...");
          was_sleeping = true;
        }

        boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += 1;
        boost::thread::sleep(xt);
      }

    } catch (InvalidRequest const& e) {
      Error("Invalid request (" << e.str() << ")");
    }
  }

  Info("Dispatcher: exiting");

} catch (task::SigPipe&) {
  Info("Dispatcher: no RequestHandler listening. Exiting...");
} catch (std::exception const& e) {
  Error("Dispatcher: " << e.what() << ". Exiting...");
} catch (...) {
  Error("Dispatcher: caught unknown exception. Exiting...");
}

} // server
} // manager
} // wms
} // glite

