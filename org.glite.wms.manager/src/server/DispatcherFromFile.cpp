// File: DispatcherFromFile.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "DispatcherFromFile.h"

#include <fstream>

#include "DispatcherFactory.h"
#include "dispatching_utils.h"

#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace task = glite::wms::common::task;
namespace configuration = glite::wms::common::configuration;

namespace {

dispatcher_type dispatcher_id("file");

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

  return new DispatcherFromFile(file);
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

void CleanUp() {}

void load_requests(std::string const& filename,
                   std::vector<std::string>& requests)
{
  // commands are separated on file by a sigdash, ie. the string "-- " on a
  // line by itself

  std::ifstream is(filename.c_str());

  std::string line;
  while (getline(is, line) && line != "-- ")
    ;

  bool quit = false;

  while (!quit && is) {

    // get the next_request
    std::string ad_str;
    while (getline(is, line) && line != "-- ") {
      ad_str += line;
    }

    requests.push_back(ad_str);

  }
}

} // {anonymous}

DispatcherFromFile::DispatcherFromFile(std::string const& file)
  : m_file(file)
{
}

void
DispatcherFromFile::run(task::PipeWriteEnd<pipe_value_type>& write_end)
{
  try {

    Info("starting");

    std::vector<std::string> requests;
    load_requests(m_file, requests);

    bool quit = false;
    for (int i = 0;
         i < configuration::Configuration::instance()->wm()->input_iterations();
         ++i) {
      int r = 0;
      for (std::vector<std::string>::const_iterator it = requests.begin();
           !quit && it != requests.end(); ++it, ++r) {
        // process it
        try {
          quit = !process(*it, CleanUp, write_end);
          Debug("# " << i << ", " << r);
        } catch (InvalidRequest&) {
          Warning("Got invalid request");
        }
        //        boost::xtime xt;
        //        boost::xtime_get(&xt, boost::TIME_UTC);
        //        xt.sec += 1;
        //        boost::thread::sleep(xt);
      }
    }
  } catch (task::SigPipe&) {
    Info("SigPipe exception");
  } catch (std::exception& e) {
    Error("std::exception (" << e.what() << ")");
  } catch (...) {
    Error("unknown exception");
  }
  Info("exiting");
}

} // server
} // manager
} // wms
} // glite
