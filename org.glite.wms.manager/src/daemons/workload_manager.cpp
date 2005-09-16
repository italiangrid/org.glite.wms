// File: workload_manager.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sys/types.h>          // getpid(), getpwnam()
#include <unistd.h>             // getpid()
#include <pwd.h>                // getpwnam()
#include <stdlib.h>             // getenv()
#include <dlfcn.h>              // dlopen(), dlclose()
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include "Dispatcher.h"
#include "RequestHandler.h"
#include "signal_handling.h"
#include "TaskQueue.hpp"
#include "pipedefs.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/common/configuration/exceptions.h"
#include "glite/wms/common/logger/logstream_ts.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/ism/purchaser/ism-ii-purchaser.h"
#include "glite/wms/ism/purchaser/ism-ii-gris-purchaser.h"
#include "glite/wms/ism/purchaser/ism-cemon-purchaser.h"
#include "glite/wms/ism/purchaser/ism-cemon-asynch-purchaser.h"
#include "glite/wms/ism/purchaser/ism-file-purchaser.h"
#include "glite/wms/ism/purchaser/ism-rgma-purchaser.h"

namespace manager = glite::wms::manager::server;
namespace configuration = glite::wms::common::configuration;
namespace task = glite::wms::common::task;
namespace logger = glite::wms::common::logger;
namespace purchaser = glite::wms::ism::purchaser;
namespace ism = glite::wms::ism;

namespace {

std::string program_name;
bool opt_daemon = false;
std::string opt_pid_file;
std::string opt_conf_file("glite_wms.conf");

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

/*bool ssl_init()
  {
  return edg_wlc_SSLInitialization() == 0 && edg_wlc_SSLLockingInit() == 0;
  }*/

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
    
struct call_execute {
  call_execute(boost::shared_ptr<purchaser::ism_purchaser> p) { m_p = p; }
  void operator()() { m_p->do_purchase(); }
  boost::shared_ptr<purchaser::ism_purchaser> m_p;
}; 

//
// Function to run update ism entries and dump ism entries every n sec
//
class run_in_loop // non struct!
{
public:
  run_in_loop(boost::function<void ()> const& f, int sec)
    : m_fn(f), m_sec(sec)
  {}

  void operator()()
  {
    while (!manager::received_quit_signal()) {
      m_fn();
      sleep(m_sec);
     }
  }

private:
  boost::function<void()> m_fn;
  int m_sec;
};
 
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

  /* removed ssl_helpers */
  //  if (!ssl_init()) {
  //    get_err_stream() << program_name << ": "
  //                     << "cannot initialize SSL\n";
  //    return EXIT_FAILURE;
  //  }

  configuration::Configuration config(opt_conf_file,
                                      configuration::ModuleType::workload_manager);
  configuration::WMConfiguration const* const wm_config(config.wm());
  configuration::NSConfiguration const* const ns_config(config.ns());

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

  char *brlib = getenv("GLITE_WMS_BROKER_HELPER_LIB");
  if (brlib == NULL) {
    if (common_config->use_cache_instead_of_gris()) {
      brlib = "libglite_wms_helper_broker_ii_prefetch.so";
    } else {
      brlib = "libglite_wms_helper_broker_ism.so";
    }
  }

  void *h = dlopen(brlib,RTLD_NOW|RTLD_GLOBAL);
  if (!h) {
    get_err_stream() << program_name << ": "
                     << "cannot load broker helper lib (" << brlib << ")\n";
    std::string dlerr(dlerror());
    get_err_stream() << program_name << ": "
                     << "dlerror returns: " << dlerr << "\n";
    return EXIT_FAILURE;
  }

  boost::shared_ptr<void> broker_helper_handle(h, dlclose);

  // force the creation of the TaskQueue before entering multithreading
  manager::the_task_queue();

  purchaser::ii_gris::create_t* create_ii_gris_purchaser = 0;
  purchaser::ii_gris::destroy_t* destroy_ii_gris_purchaser = 0;

  purchaser::ii::create_t* create_ii_purchaser = 0;
  purchaser::ii::destroy_t* destroy_ii_purchaser = 0;

  purchaser::rgma::create_t* create_rgma_purchaser = 0;
  purchaser::rgma::destroy_t* destroy_rgma_purchaser = 0;

  purchaser::cemon::create_t* create_cemon_purchaser = 0;
  purchaser::cemon::destroy_t* destroy_cemon_purchaser = 0;

  purchaser::cemon_asynch::create_t* create_cemon_asynch_purchaser = 0;
  purchaser::cemon_asynch::destroy_t* destroy_cemon_asynch_purchaser = 0;

  purchaser::file::create_t* create_file_purchaser = 0;
  purchaser::file::destroy_t* destroy_file_purchaser = 0;
  purchaser::file::set_purchaser_entry_update_fns_t* set_purchaser_entry_update_fns = 0;

  // boost::shared_ptr<void> ism_purchasers_handle;
  boost::shared_ptr<void> ism_ii_purchaser_handle;
  boost::shared_ptr<void> ism_ii_gris_purchaser_handle;
  boost::shared_ptr<void> ism_rgma_purchaser_handle;
  boost::shared_ptr<void> ism_cemon_purchaser_handle;
  boost::shared_ptr<void> ism_cemon_asynch_purchaser_handle;
  boost::shared_ptr<void> ism_file_purchaser_handle;
  
  boost::shared_ptr<purchaser::ism_purchaser> ismp1;
  boost::shared_ptr<purchaser::ism_purchaser> ismp2;
  boost::shared_ptr<purchaser::ism_purchaser> ismp3;
  
  boost::thread_group purchaser_thread_group;
  
  // Try to execute ISM purchaser thread
  if (std::string(brlib).find("_ism") != std::string::npos) {
    
    bool enable_purchasing_from_rgma  = wm_config->enable_purchasing_from_rgma();
    bool disable_purchasing_from_gris = wm_config->disable_purchasing_from_gris();
       
    const char* prlib_ii      = "libglite_wms_ism_ii_purchaser.so"; 
    const char* prlib_ii_gris = "libglite_wms_ism_ii_gris_purchaser.so";
    
    void* prh_ii      = dlopen(prlib_ii,RTLD_NOW);
    void* prh_ii_gris = dlopen(prlib_ii_gris,RTLD_NOW);
    
    if (!prh_ii || !prh_ii_gris) {
      
      get_err_stream() << program_name << ": "
                       << "cannot load ism ii purchasers lib \n";
      std::string dlerr(dlerror());
      get_err_stream() << program_name << ": "
                       << "dlerror returns: " << dlerr << "\n";
      return EXIT_FAILURE;
    }
    const char* prlib_rgma = "libglite_wms_ism_rgma_purchaser.so";
    
    void* prh_rgma = 0;
    if (enable_purchasing_from_rgma && 
        !(prh_rgma=dlopen(prlib_rgma,RTLD_NOW))) {
      get_err_stream() << program_name << ": "
                       << "cannot load ism rgma purchasers lib \n";
      std::string dlerr(dlerror());
      get_err_stream() << program_name << ": "
                       << "dlerror returns: " << dlerr << "\n";
      return EXIT_FAILURE;
    }

    if (!enable_purchasing_from_rgma) {
      if (disable_purchasing_from_gris) {
        ism_ii_purchaser_handle.reset(prh_ii, dlclose);
   
        create_ii_purchaser  = (purchaser::ii::create_t*)  dlsym(prh_ii, "create_ii_purchaser");
        destroy_ii_purchaser = (purchaser::ii::destroy_t*) dlsym(prh_ii, "destroy_ii_purchaser");

        if ((!create_ii_purchaser) || (!destroy_ii_purchaser)) {
            get_err_stream() << "Cannot load " << prlib_ii
                             << " symbols: " << dlerror() << "\n";
            return EXIT_FAILURE;
        }
      }
      else {
        ism_ii_gris_purchaser_handle.reset(prh_ii_gris, dlclose);
   
        create_ii_gris_purchaser  = (purchaser::ii_gris::create_t*)  dlsym(prh_ii_gris, "create_ii_gris_purchaser");
        destroy_ii_gris_purchaser = (purchaser::ii_gris::destroy_t*) dlsym(prh_ii_gris, "destroy_ii_gris_purchaser");

        if ((!create_ii_gris_purchaser) || (!destroy_ii_gris_purchaser)) {
            get_err_stream() << "Cannot load " << prlib_ii_gris
                             << " symbols: " << dlerror() << "\n";
            return EXIT_FAILURE;
        }
      }
    }
    else {
      ism_rgma_purchaser_handle.reset(prh_rgma, dlclose);
      create_rgma_purchaser  = (purchaser::rgma::create_t*)  dlsym(prh_rgma, "create_rgma_purchaser");
      destroy_rgma_purchaser = (purchaser::rgma::destroy_t*) dlsym(prh_rgma, "destroy_rgma_purchaser");

      if ((!create_rgma_purchaser) || (!destroy_rgma_purchaser)) {
        get_err_stream() << "Cannot load " << prlib_rgma
                         << " symbols: " << dlerror() << "\n";
        return EXIT_FAILURE;
      }
    }
    char* prlib_cemon = "libglite_wms_ism_cemon_purchaser.so";
    void* prh_cemon = dlopen(prlib_cemon,RTLD_NOW);
    if (!prh_cemon) {
      get_err_stream() << program_name << ": "
                       << "cannot load ism purchaser lib (" << prlib_cemon << "\n";
      std::string dlerr(dlerror());
      get_err_stream() << program_name << ": "
                       << "dlerror returns: " << dlerr << "\n";
      return EXIT_FAILURE;
    }
    
    ism_cemon_purchaser_handle.reset(prh_cemon, dlclose);

    char* prlib_cemon_asynch = "libglite_wms_ism_cemon_asynch_purchaser.so";
    void* prh_cemon_asynch = dlopen(prlib_cemon_asynch,RTLD_NOW);
    if (!prh_cemon_asynch) {
      get_err_stream() << program_name << ": "
                       << "cannot load ism purchaser lib (" << prlib_cemon_asynch << "\n";
      std::string dlerr(dlerror());
      get_err_stream() << program_name << ": "
                       << "dlerror returns: " << dlerr << "\n";
      return EXIT_FAILURE;
    }
    
    // Try to enable purchasing from dump file
    // NOTE: This depends on the successful load of the II purchaser above
    if (wm_config->enable_ism_dump()) {
    
      char* prlib_file = "libglite_wms_ism_file_purchaser.so";
      void* prh_file = dlopen(prlib_file,RTLD_NOW);
      if (!prh_file) {
        get_err_stream() << program_name << ": "
                         << "cannot load ism purchaser lib (" << prlib_file << "\n";
        std::string dlerr(dlerror());
        get_err_stream() << program_name << ": "
                         << "dlerror returns: " << dlerr << "\n";
        return EXIT_FAILURE;
      }

      ism_file_purchaser_handle.reset(prh_file, dlclose);

      create_file_purchaser  = (purchaser::file::create_t*)  dlsym(prh_file, "create_file_purchaser");
      destroy_file_purchaser = (purchaser::file::destroy_t*) dlsym(prh_file, "destroy_file_purchaser");
      set_purchaser_entry_update_fns = (purchaser::file::set_purchaser_entry_update_fns_t*) dlsym(prh_file, "set_purchaser_entry_update_fns");

      if ((!create_file_purchaser) || (!destroy_file_purchaser) ||
           (!set_purchaser_entry_update_fns)) {
          get_err_stream() << "Cannot load " << prlib_file
			   << " symbols: " << dlerror() << "\n";
      } else {

        // Set the update entry function factory functions
        purchaser::ii::create_entry_update_fn_t*        create_ii_entry_update_fn      = 0;
        purchaser::ii_gris::create_entry_update_fn_t*   create_ii_gris_entry_update_fn = 0;
        purchaser::cemon::create_entry_update_fn_t*     create_cemon_entry_update_fn   = 0;
        purchaser::rgma::create_entry_update_fn_t* create_rgma_entry_update_fn    = 0;

        create_ii_entry_update_fn = (purchaser::ii::create_entry_update_fn_t*) 
          dlsym(prh_ii,"create_ii_entry_update_fn");
        create_ii_gris_entry_update_fn = (purchaser::ii_gris::create_entry_update_fn_t*) 
          dlsym(prh_ii_gris,"create_ii_gris_entry_update_fn");
        create_cemon_entry_update_fn = (purchaser::cemon::create_entry_update_fn_t*)
          dlsym(prh_cemon,"create_cemon_entry_update_fn");
        
        if(enable_purchasing_from_rgma) 
          create_rgma_entry_update_fn = (purchaser::rgma::create_entry_update_fn_t*)
            dlsym(prh_rgma,"create_rgma_entry_update_fn");

        if ((!create_ii_entry_update_fn) || (!create_ii_gris_entry_update_fn) ||
          (!create_cemon_entry_update_fn) ) {
          get_err_stream() << "Cannot load ism entry update function symbols: " << dlerror() << "\n";
          return EXIT_FAILURE;
        }

        set_purchaser_entry_update_fns(
          create_ii_entry_update_fn,
          create_ii_gris_entry_update_fn,
          create_cemon_entry_update_fn,
          create_rgma_entry_update_fn
        );

        // Try to load ISM status from dump file
        ismp1.reset(create_file_purchaser(wm_config->ism_dump()),
					  destroy_file_purchaser);
        ismp1->skip_predicate(purchaser::is_in_black_list(wm_config->ism_black_list()));
        try {
          ismp1->do_purchase();
        }
        catch(...) { // FIXME: Catch a specific exception
          get_err_stream() << "Cannot load ISM status from "
			   << wm_config->ism_dump() << "\n";
        }
      }
    }

    ism_cemon_asynch_purchaser_handle.reset(prh_cemon_asynch, dlclose);

    create_cemon_purchaser  = (purchaser::cemon::create_t*)  dlsym(prh_cemon, "create_cemon_purchaser");
    destroy_cemon_purchaser = (purchaser::cemon::destroy_t*) dlsym(prh_cemon, "destroy_cemon_purchaser");

    create_cemon_asynch_purchaser  = (purchaser::cemon_asynch::create_t*)  dlsym(prh_cemon_asynch, "create_cemon_asynch_purchaser");
    destroy_cemon_asynch_purchaser = (purchaser::cemon_asynch::destroy_t*) dlsym(prh_cemon_asynch, "destroy_cemon_asynch_purchaser");

    if ((!create_cemon_purchaser) || (!destroy_cemon_purchaser) ||
        (!create_cemon_asynch_purchaser) || (!destroy_cemon_asynch_purchaser)) {
      get_err_stream() << "Cannot load " << prlib_cemon << "/" << prlib_cemon_asynch 
  		       << " symbols: " << dlerror() << "\n";
      return EXIT_FAILURE;
    }
    if (enable_purchasing_from_rgma) {
      ismp1.reset(
        create_rgma_purchaser(
          wm_config->rgma_query_timeout(),
          purchaser::loop,
	  wm_config->rgma_consumer_ttl(),
	  wm_config->rgma_consumer_life_cycle(), 
          wm_config->ism_rgma_purchasing_rate(), 
          manager::received_quit_signal),
	  destroy_rgma_purchaser
      );
    }
    else
    if (disable_purchasing_from_gris) {     
      ismp1.reset(
        create_ii_purchaser(
          ns_config->ii_contact(), ns_config->ii_port(),
          ns_config->ii_dn(),ns_config->ii_timeout(),
          purchaser::loop, wm_config->ism_ii_purchasing_rate(), manager::received_quit_signal),
	  destroy_ii_purchaser
      );
    }
    else {
      ismp1.reset(
        create_ii_gris_purchaser(
          ns_config->ii_contact(), ns_config->ii_port(),
          ns_config->ii_dn(),ns_config->ii_timeout(),
          purchaser::loop, wm_config->ism_ii_purchasing_rate(), manager::received_quit_signal),
          destroy_ii_gris_purchaser
      );
    }

    ismp1->skip_predicate(purchaser::is_in_black_list(wm_config->ism_black_list()));
    
    // FIXME: It is not so nice but works...
    purchaser_thread_group.create_thread(call_execute(ismp1));
    
    // Try to execute ISM CEMON purchaser thread
    std::vector<std::string> cemonURLs = wm_config->ce_monitor_services();
    if (!cemonURLs.empty()) {
      
      // Get the certificate file name from the configuration file
      std::string certificate_file;
      certificate_file.assign(common_config->host_proxy_file());
      
      // Try to get the certificate path from the evironment variable GLITE_CERT_DIR if
      // possible, otherwise defaults to /etc/grid-security/certificates.
      char* certificate_path = getenv("GLITE_CERT_DIR");
      if (!certificate_path) {
        certificate_path = "/etc/grid-security/certificates";
      } 
      ismp2.reset(
        create_cemon_purchaser(certificate_file, certificate_path,
                               cemonURLs,"CE_MONITOR",
 			       120, purchaser::loop, wm_config->ism_cemon_purchasing_rate(),
			       manager::received_quit_signal),
         destroy_cemon_purchaser
      );
      ismp2->skip_predicate(purchaser::is_in_black_list(wm_config->ism_black_list()));
      // FIXME: It is not so nice but works...
      purchaser_thread_group.create_thread(call_execute(ismp2));
    }      
    // Try to execute ISM ASYNCH CEMON purchaser thread
    int cemon_asynch_port = wm_config->ce_monitor_asynch_port();
    if (cemon_asynch_port>0) {

      ismp3.reset(
        create_cemon_asynch_purchaser("CE_MONITOR", cemon_asynch_port,
                                      purchaser::loop, wm_config->ism_cemon_asynch_purchasing_rate(), 
 				      manager::received_quit_signal),
        destroy_cemon_asynch_purchaser
      );
      ismp3->skip_predicate(purchaser::is_in_black_list(wm_config->ism_black_list()));
      purchaser_thread_group.create_thread(call_execute(ismp3));
    }
    
//    purchaser_thread_group.join_all();
    
    ism::call_update_ism_entries cuie; 	
    boost::thread ism_update(run_in_loop(cuie,wm_config->ism_update_rate()));
//    ism_update.join();

    ism::call_dump_ism_entries cdie;
    boost::thread ism_dump(run_in_loop(cdie,wm_config->ism_dump_rate()));
//    ism_dump.join();
  }
  
  manager::Dispatcher dispatcher;
  manager::RequestHandler request_handler;

  manager::pipe_type d2rh(wm_config->pipe_depth());
  task::Task d(dispatcher, d2rh);
  task::Task r(request_handler, d2rh, wm_config->worker_threads());
  
  } catch (std::exception& e) {
    get_err_stream() << "std::exception " << e.what() << "\n";
    return EXIT_FAILURE;
  } catch (...) {
    get_err_stream() << "uknown exception\n";
    return EXIT_FAILURE;
  }
