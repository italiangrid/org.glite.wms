#include "glite/wms/ism/purchaser/ism-cemon-purchaser.h"
#include "glite/ce/monitor-client-api-c/CEMonitorBinding.nsmap"
#include "glite/wms/common/logger/edglog.h"
#include <vector>
#include <string>
#include <exception>


using namespace std;
using namespace glite::wms::ism;
using namespace glite::wms::ism::purchaser;

namespace logger      = glite::wms::common::logger::threadsafe;

int main(void)
{
  logger::edglog.open(std::clog, glite::wms::common::logger::debug);
  vector<string> services;
  services.push_back("https://lxde01.pd.infn.it:8443/ce-monitor/services/CEMonitor");
  vector<string> blacklist;
  
  // Get the certificate file name from the configuration file
  std::string certificate_file;
  //certificate_file.assign(common_config->host_proxy_file())

  // Try to get the certificate path from the evironment variable GLITE_CERT_DIR if
  // possible, otherwise defaults to /etc/grid-security/certificates.
  char* certificate_path = getenv("GLITE_CERT_DIR");
  if (!certificate_path) {
    certificate_path = "/etc/grid-security/certificates";
  }
  ism_cemon_purchaser icp(certificate_file, certificate_path, services,"CE_MONITOR", 30, once);
  icp.skip_predicate(is_in_black_list(blacklist));
  try { 
  icp();
  } catch(std::exception& e) {
  	cout << e.what() << endl;
  }
  boost::mutex::scoped_lock l(get_ism_mutex());

  for (ism_type::iterator pos=get_ism().begin();
       pos!= get_ism().end(); ++pos) {
    classad::ClassAd          ad_ism_dump;

    ad_ism_dump.InsertAttr("id", pos->first);
    ad_ism_dump.InsertAttr("update_time", boost::tuples::get<0>(pos->second));
    ad_ism_dump.InsertAttr("expiry_time", boost::tuples::get<1>(pos->second));
    ad_ism_dump.Insert("info", boost::tuples::get<2>(pos->second).get()->Copy());

   cout << ad_ism_dump;
  }

  return 0;	
}
