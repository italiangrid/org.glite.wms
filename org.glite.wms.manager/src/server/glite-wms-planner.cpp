// File: edg-wl-planner.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2003 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <iostream>
#include <fstream>
#include <cstring>
#include <dlfcn.h>              // dlopen()
#include <boost/scoped_ptr.hpp>
#include <classad_distribution.h>

#include "signal_handling.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wmsutils/tls/ssl_helpers/ssl_inits.h"
#include "glite/wmsutils/tls/ssl_helpers/ssl_pthreads.h"

#include "glite/wms/helper/Request.h"
#include "plan.h"

namespace manager = glite::wms::manager::server;
namespace utilities = glite::wms::common::utilities;
namespace requestad = glite::wms::jdl;
namespace configuration = glite::wms::common::configuration;

namespace {

bool ssl_init()
{
  return edg_wlc_SSLInitialization() == 0 && edg_wlc_SSLLockingInit() == 0;
}

std::string program_name;

void usage(std::ostream& os)
{
  os
    << "Usage: " << program_name << " --help\n"
    << "   or: " << program_name << " SOURCE DEST\n";
}

} // {anonymous}

int
main(int argc, char* argv[])
try {
  program_name = argv[0];

  if (argc == 2 && std::string(argv[1]) == "--help") {

    usage(std::cout);
    return EXIT_SUCCESS;

  } else if (argc != 3) {

    usage(std::cerr);
    return EXIT_FAILURE;

  }

  manager::signal_handling_init();

  if (!ssl_init()) {
    std::cerr << "Cannot initialize SSL\n";
    return EXIT_FAILURE;
  }

  configuration::Configuration config("glite_wms.conf",
                                      configuration::ModuleType::workload_manager);
  configuration::CommonConfiguration const* const common_config(config.common());

  // FIXME: This a crude patch to cause the load of the appropriate
  // FIXME: dynamic broker helper. This should be replaced by a configurable
  // FIXME: Helper Factory.

  char *brlib = getenv("GLITE_WMS_BROKER_HELPER_LIB");
  if (brlib == NULL) {
    if (common_config->use_cache_instead_of_gris()) {
      brlib = "libglite_wms_helper_broker_ii_prefetch.so";
    } else {
      brlib = "libglite_wms_helper_broker_ii.so";
    }
  }

  void *m_broker_helper_handle = dlopen(brlib,RTLD_NOW|RTLD_GLOBAL);
  if (m_broker_helper_handle == NULL) {
    std::cerr << program_name << ": "
                     << "cannot load broker helper lib (" << brlib << ")\n";
    std::cerr << program_name << ": "
                     << "dlerror returns: " << dlerror() << "\n";
    return EXIT_FAILURE;
  }

  std::string input_file(argv[1]);
  std::ifstream is(input_file.c_str());
  if (!is) {
    std::cerr << "Cannot open input file " << input_file << "\n";
    return EXIT_FAILURE;
  }
  std::string output_file(argv[2]);
  std::ofstream os(output_file.c_str());
  if (!os) {
    std::cerr << "Cannot open output file " << output_file << "\n";
    return EXIT_FAILURE;
  }

  boost::scoped_ptr<classad::ClassAd> input_ad(utilities::parse_classad(is));
  boost::scoped_ptr<classad::ClassAd> output_ad(Plan(*input_ad));

  if (!output_ad) {
    std::cerr << "Planning failed for unknown reason\n";
    return EXIT_FAILURE;
  }

  os << utilities::unparse_classad(*output_ad) << "\n";

  std::cout << requestad::get_ce_id(*output_ad) << '\n';

} catch (std::exception& e) {

  std::cerr << e.what() << '\n';
  return EXIT_FAILURE;

} catch (...) {

  std::cerr << "unknown exception\n";
  return EXIT_FAILURE;

}
