// File: workload_manager.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sys/types.h>          // getpid(), getpwnam()
#include <unistd.h>             // getpid()
#include <pwd.h>                // getpwnam()
#include <stdlib.h>             // getenv()
#include <dlfcn.h>              // dlopen(), dlclose()
#include <boost/shared_ptr.hpp>

#include "Dispatcher.h"
#include "RequestHandler.h"
#include "signal_handling.h"
#include "TaskQueue.hpp"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/common/configuration/exceptions.h"
#include "glite/wms/common/logger/logstream_ts.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wmsutils/tls/ssl_helpers/ssl_inits.h"
#include "glite/wmsutils/tls/ssl_helpers/ssl_pthreads.h"

namespace manager = glite::wms::manager::server;
namespace configuration = glite::wms::common::configuration;
namespace task = glite::wms::common::task;
namespace logger = glite::wms::common::logger;

namespace {

std::string program_name;
bool opt_daemon = false;
std::string opt_pid_file;
std::string opt_conf_file("edg_wl.conf");

void usage(std::ostream& os)
{
  os << "Usage: " << program_name << " --help\n"
     << "   or: " << program_name << " [--daemon PIDFILE] [--conf FILE]\n";
}

std::fstream edglog_stream;

std::ostream* out_stream_p = &std::cout;
std::ostream* err_stream_p = &std::cerr;

std::ostream& get_out_stream()
{
  return *out_stream_p;
}

void set_out_stream(std::ostream& os)
{
  out_stream_p = &os;
}

std::ostream& get_err_stream()
{
  return *err_stream_p;
}

void set_err_stream(std::ostream& os)
{
  err_stream_p = &os;
}

bool ssl_init()
{
  return edg_wlc_SSLInitialization() == 0 && edg_wlc_SSLLockingInit() == 0;
}

// change the uid and gid to those of user
// no-op if user corresponds to the current effective uid
// only root can set the uid
bool set_user(std::string const& user)
{
  uid_t euid = ::geteuid();

  if (euid == 0 && user.empty()) {
    return false;
  }

  ::passwd* pwd(::getpwnam(user.c_str()));
  if (pwd == 0) {
    return false;
  }
  ::uid_t uid = pwd->pw_uid;
  ::gid_t gid = pwd->pw_gid;

  return 
    euid == uid
    || (euid == 0 && ::setgid(gid) == 0 && ::setuid(uid) == 0);
}

} // {anonymous}

int
main(int argc, char* argv[])
try {

  program_name = argv[0];

  for (int i = 1; i < argc; ++i) {
    if (argv[i] == std::string("--help")) {
      usage(get_out_stream());
      return EXIT_SUCCESS;
    } else if (argv[i] == std::string("--daemon")) {
      ++i;
      if (i < argc) {
        opt_daemon = true;
        opt_pid_file = argv[i];
      } else {
        usage(get_err_stream());
        return EXIT_FAILURE;
      }
    } else if (argv[i] == std::string("--conf")) {
      ++i;
      if (i < argc) {
        opt_conf_file = argv[i];
      } else {
        usage(get_err_stream());
        return EXIT_FAILURE;
      }
    } else {
      usage(get_err_stream());
      return EXIT_FAILURE;
    }
  }

  manager::signal_handling_init();

  if (!ssl_init()) {
    get_err_stream() << program_name << ": "
                     << "cannot initialize SSL\n";
    return EXIT_FAILURE;
  }

  configuration::Configuration config(opt_conf_file,
                                      configuration::ModuleType::workload_manager);
  configuration::WMConfiguration const* const wm_config(config.wm());

  configuration::CommonConfiguration const* const common_config(config.common());

  std::string dguser(common_config->dguser());
  if (!set_user(dguser)) {
    get_err_stream() << program_name << ": "
                     << "cannot set the user id to " << dguser << "\n";
    return EXIT_FAILURE;
  }


  if (opt_daemon) {

    // init logger and adjust the err stream to use the same stream

    std::string log_file = wm_config->log_file();
    if (!log_file.empty()) {
      if (!std::ifstream(log_file.c_str())) {
        std::ofstream(log_file.c_str());
      }
      edglog_stream.open(log_file.c_str(), std::ios::in | std::ios::out | std::ios::ate);
    }
    if (edglog_stream) {
      set_err_stream(edglog_stream);
      logger::threadsafe::edglog.open(
        edglog_stream,
        static_cast<logger::level_t>(wm_config->log_level())
      );
      logger::threadsafe::edglog.activate_log_rotation(
        wm_config->log_file_max_size(),
        wm_config->log_rotation_base_file(),
        wm_config->log_rotation_max_file_number()
      );
    } else {
      get_err_stream() << program_name << ": "
                       << log_file << ": cannot open\n";
      return EXIT_FAILURE;
    }

    std::ofstream pid_file(opt_pid_file.c_str());
    if (!pid_file) {
      get_err_stream() << program_name << ": "
                       << opt_pid_file << ": cannot open for writing\n";
      return EXIT_FAILURE;
    }
    if (daemon(0, 0)) {
      get_err_stream() << program_name << ": "
                       << "cannot become daemon (errno = "<< errno << ")\n";
      return EXIT_FAILURE;
    }
    pid_file << ::getpid();

  } else { // not daemon

    logger::threadsafe::edglog.open(
      get_err_stream(),
      static_cast<logger::level_t>(wm_config->log_level())
    );

  }

  // FIXME: This a crude patch to cause the load of the appropriate
  // FIXME: dynamic broker helper. This should be replaced by a configurable
  // FIXME: Helper Factory.

  char *brlib = getenv("EDG_WL_BROKER_HELPER_LIB");
  if (brlib == NULL) {
    if (common_config->use_cache_instead_of_gris()) {
      brlib = "libedg_wl_helper_broker_prefetch.so.0";
    } else {
      brlib = "libedg_wl_helper_broker.so.0";
    }
  }

  boost::shared_ptr<void> broker_helper_handle(
    dlopen(brlib,RTLD_NOW|RTLD_GLOBAL),
    dlclose
  );

  if (!broker_helper_handle) {
    get_err_stream() << program_name << ": "
                     << "cannot load broker helper lib (" << brlib << ")\n";
    get_err_stream() << program_name << ": "
                     << "dlerror returns: " << dlerror() << "\n";
    return EXIT_FAILURE;
  }

  // force the creation of the TaskQueue before entering multithreading
  manager::the_task_queue();

  task::Pipe<boost::shared_ptr<manager::Request> > d2rh(wm_config->pipe_depth());

  manager::Dispatcher dispatcher;
  manager::RequestHandler request_handler;
  task::Task d(dispatcher, d2rh);
  task::Task r(request_handler, d2rh, wm_config->worker_threads());

} catch (std::exception& e) {
  get_err_stream() << "std::exception " << e.what() << "\n";
  return EXIT_FAILURE;
} catch (...) {
  get_err_stream() << "uknown exception\n";
  return EXIT_FAILURE;
}
