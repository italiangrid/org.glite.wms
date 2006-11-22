// File: gpbox_utils.h
// Authors: 
//          Marco Cecchi <Marco.Cecchi@cnaf.infn.it>
//          Salvatore Monforte <Salvatore.Monforte@cnaf.infn.it>

#ifndef GLITE_WMS_BROKER_GPBOX_UTILS_H
#define GLITE_WMS_BROKER_GPBOX_UTILS_H

#include <string>

namespace glite {

namespace wmsutils {
namespace jobid {
class JobId;
}
}

namespace wms {
namespace common {
namespace configuration {
class Configuration;
}
}

namespace broker {
namespace gpbox {

namespace jobid = glite::wmsutils::jobid;
namespace configuration = glite::wms::common::configuration;

bool
interact(
  configuration::Configuration const& config,
  jobid::JobId const& jobid,
  std::string const& PBOX_host_name,
  matchtable& suitable_CEs);

bool
interact(
  configuration::Configuration const& config,
  std::string const& x509_user_proxy,
  std::string const& PBOX_host_name,
  matchtable& suitable_CEs);

}}}}
#endif
