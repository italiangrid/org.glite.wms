// File: ism_utils.cpp
// Author: Francesco Giacomini
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/bind.hpp>
#include "ism_utils.h"
#include "signal_handling.h"
#include "dynamic_library.h"
#include "glite/wms/ism/purchaser/ism-purchaser.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wmsutils/classads/classad_utils.h"
#include "glite/wms/ism/ism.h"

#include "glite/wms/ism/purchaser/ism-ii-purchaser.h"
#include "glite/wms/ism/purchaser/ism-cemon-purchaser.h"
#include "glite/wms/ism/purchaser/ism-cemon-asynch-purchaser.h"
#include "glite/wms/ism/purchaser/ism-file-purchaser.h"
#include "glite/wms/ism/purchaser/ism-rgma-purchaser.h"

namespace configuration = glite::wms::common::configuration;
namespace ca = glite::wmsutils::classads;
namespace purchaser = glite::wms::ism::purchaser;
namespace ism = glite::wms::ism;

namespace glite {
namespace wms {
namespace manager {
namespace main {

namespace {

typedef boost::shared_ptr<server::DynamicLibrary> DynamicLibraryPtr;
typedef boost::shared_ptr<purchaser::ism_purchaser> PurchaserPtr;

DynamicLibraryPtr make_dll(std::string const& lib_name)
{
  return DynamicLibraryPtr(new server::DynamicLibrary(lib_name));
}

void load_dlls_and_create_purchasers(
  std::vector<DynamicLibraryPtr>& dlls,
  std::vector<PurchaserPtr>& purchasers
)
{
  using namespace glite::wms::ism;

  configuration::Configuration const& config(
    *configuration::Configuration::instance()
  );

  purchaser::exit_predicate_type const exit_predicate(
    server::received_quit_signal
  );
  purchaser::skip_predicate_type const skip_predicate(
    purchaser::is_in_black_list(config.wm()->ism_black_list())
  );

  if (config.wm()->enable_purchasing_from_rgma()) {

    Info("loading rgma purchaser");

    std::string const dll_name("libglite_wms_ism_rgma_purchaser.so");
    DynamicLibraryPtr dll(make_dll(dll_name));

    std::string const create_function_name("create_rgma_purchaser");
    purchaser::rgma::create_t* create_function;
    dll->lookup(create_function_name, create_function);

    std::string const destroy_function_name("destroy_rgma_purchaser");
    purchaser::rgma::destroy_t* destroy_function;
    dll->lookup(destroy_function_name, destroy_function);

    PurchaserPtr purchaser(
      create_function(
        config.wm()->rgma_query_timeout(),
        purchaser::loop,
        config.wm()->rgma_consumer_ttl(),
        config.wm()->rgma_consumer_life_cycle(),
        config.wm()->ism_rgma_purchasing_rate(),
        exit_predicate,
        skip_predicate
      ),
      destroy_function
    );

    dlls.push_back(dll);
    purchasers.push_back(purchaser);

  } else {

    Info("loading II purchaser");

    std::string const dll_name("libglite_wms_ism_ii_purchaser.so");
    DynamicLibraryPtr dll(make_dll(dll_name));

    std::string const create_function_name("create_ii_purchaser");
    purchaser::ii::create_t* create_function;
    dll->lookup(create_function_name, create_function);

    std::string const destroy_function_name("destroy_ii_purchaser");
    purchaser::ii::destroy_t* destroy_function;
    dll->lookup(destroy_function_name, destroy_function);

    PurchaserPtr purchaser(
      create_function(
        config.ns()->ii_contact(),
        config.ns()->ii_port(),
        config.ns()->ii_dn(),
        config.ns()->ii_timeout(),
        purchaser::loop,
        config.wm()->ism_ii_purchasing_rate(),
        exit_predicate,
        skip_predicate
      ),
      destroy_function
    );

    dlls.push_back(dll);
    purchasers.push_back(purchaser);
  }

  int const cemon_asynch_port(config.wm()->ce_monitor_asynch_port());

  if (cemon_asynch_port > 0) {

    Info("loading asynch cemon purchaser");

    std::string const dll_name("libglite_wms_ism_cemon_asynch_purchaser.so");
    DynamicLibraryPtr dll(make_dll(dll_name));

    std::string const create_function_name("create_cemon_asynch_purchaser");
    purchaser::cemon_asynch::create_t* create_function;
    dll->lookup(create_function_name, create_function);

    std::string const destroy_function_name("destroy_cemon_asynch_purchaser");
    purchaser::cemon_asynch::destroy_t* destroy_function;
    dll->lookup(destroy_function_name, destroy_function);

    PurchaserPtr purchaser(
      create_function(
        "CE_MONITOR",
        cemon_asynch_port,
        purchaser::loop,
        config.wm()->ism_cemon_asynch_purchasing_rate(),
        exit_predicate,
        skip_predicate
      ),
      destroy_function
    );

    dlls.push_back(dll);
    purchasers.push_back(purchaser);

  }

  std::vector<std::string> const cemon_services(
    config.wm()->ce_monitor_services()
  );

  if (!cemon_services.empty()) {

    Info("loading cemon purchaser");

    std::string const dll_name("libglite_wms_ism_cemon_purchaser.so");
    DynamicLibraryPtr dll(make_dll(dll_name));

    std::string const create_function_name("create_cemon_purchaser");
    purchaser::cemon::create_t* create_function;
    dll->lookup(create_function_name, create_function);

    std::string const destroy_function_name("destroy_cemon_purchaser");
    purchaser::cemon::destroy_t* destroy_function;
    dll->lookup(destroy_function_name, destroy_function);

    char const* certificate_path = ::getenv("GLITE_CERT_DIR");
    if (!certificate_path) {
      certificate_path = "/etc/grid-security/certificates";
    }

    PurchaserPtr purchaser(
      create_function(
        config.common()->host_proxy_file(),
        certificate_path,
        cemon_services,
        "CE_MONITOR",
        120,
        purchaser::loop,
        config.wm()->ism_cemon_purchasing_rate(),
        exit_predicate,
        skip_predicate
      ),
      destroy_function
    );

    dlls.push_back(dll);
    purchasers.push_back(purchaser);

  }

}

void
start_purchasing_threads(
  std::vector<PurchaserPtr> const& purchasers,
  boost::thread_group& purchasing_threads
)
{
  Info("starting purchaser threads");

  std::vector<PurchaserPtr>::const_iterator it = purchasers.begin();
  std::vector<PurchaserPtr>::const_iterator const end = purchasers.end();

  for ( ; it != end; ++it) {
    PurchaserPtr purchaser_ptr = *it;
    purchasing_threads.create_thread(
      boost::bind(
        &purchaser::ism_purchaser::do_purchase,
        purchaser_ptr
      )
    );
  }
}

}

struct ISM_Manager::Impl
{
  ism::ism_type m_ism;
  ism::ism_mutex_type m_ism_mutex;
  std::vector<DynamicLibraryPtr> m_dlls;
  std::vector<PurchaserPtr> m_purchasers;
  boost::thread_group m_purchasing_threads;
  boost::thread m_updater;
};

ISM_Manager::ISM_Manager()
  : m_impl(new Impl)
{
  Info("creating the ISM");

  ism::set_ism(m_impl->m_ism, m_impl->m_ism_mutex);

  load_dlls_and_create_purchasers(m_impl->m_dlls, m_impl->m_purchasers);
  //  load_dump_file(m_impl->m_purchasers, m_impl->m_ism);
  start_purchasing_threads(m_impl->m_purchasers, m_impl->m_purchasing_threads);

  // start update thread
  // start periodic-dump thread
}

ISM_Manager::~ISM_Manager()
{
  m_impl->m_purchasing_threads.join_all();
  m_impl->m_purchasers.clear();
  m_impl->m_dlls.clear();
  m_impl->m_ism.clear();
}

}}}} // glite::wms::manager::main
