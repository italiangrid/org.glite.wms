#include "glite/wms/ism/purchaser/ism-cemon-purchaser.h"
#include "glite/ce/monitor-client-api-c/CEMonitorBinding.nsmap"
#include "glite/wms/common/logger/edglog.h"
#include <vector>
#include <string>
#include <exception>


using namespace std;
using namespace glite::wms::ism::purchaser;

namespace logger      = glite::wms::common::logger::threadsafe;

int main(void)
{
  logger::edglog.open(std::clog, glite::wms::common::logger::debug);
  vector<string> services;
  services.push_back("http://lxb2022.cern.ch:8080/ce-monitor/services/CEMonitor");
  
  ism_cemon_purchaser icp(services,"CE_MONITOR:ISM", 30, once);
  try { 
  icp();
  } catch(std::exception& e) {
  	cout << e.what() << endl;
  }
  return 0;	
}
