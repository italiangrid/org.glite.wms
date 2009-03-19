#include <vector>
#include <string>
#include <exception>

#include "glite/wms/ism/purchaser/ism-ii-purchaser.h"
#include "glite/wms/ism/purchaser/ism-file-purchaser.h"
#include "glite/wms/ism/ism.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/exceptions.h"

#include "glite/wms/common/logger/edglog.h"

namespace purchaser = glite::wms::ism::purchaser;
namespace ism       = glite::wms::ism;
namespace logger    = glite::wms::common::logger::threadsafe;
namespace configuration = glite::wms::common::configuration;

std::string opt_conf_file("ism_wms.conf");

int main(void)
{
  logger::edglog.open(std::clog, glite::wms::common::logger::debug);

  configuration::Configuration config(opt_conf_file,
                                      configuration::ModuleType::workload_manager);

  configuration::WMConfiguration const* const wm_config(config.wm());
  configuration::NSConfiguration const* const ns_config(config.ns()); 
while (true) {
  purchaser::ism_ii_purchaser icp(ns_config->ii_contact(), 
				  ns_config->ii_port(),
				  ns_config->ii_dn(),
				  ns_config->ii_timeout(),
				  purchaser::once);

  try { 
    icp();

    sleep(1);
    ism::call_update_ism_entries()();
    ism::call_dump_ism_entries()();

    std::string filename(ism::get_ism_dump());
    purchaser::ism_file_purchaser ifp(filename);

  } catch(std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

  return 0;	
}
