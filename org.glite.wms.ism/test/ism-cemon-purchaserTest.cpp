#include "glite/wms/ism/purchaser/ism-cemon-purchaser.h"
#include "glite/ce/monitor-client-api-c/CEMonitorBinding.nsmap"
#include <vector>
#include <string>
#include <exception>
using namespace std;
using namespace glite::wms::ism::purchaser;

int main(void)
{
  vector<string> services;
  services.push_back("http://lxb2022.cern.ch:8080/ce-monitor/services/CEMonitor");
  
  ism_cemon_purchaser icp(services,"CE_MONITOR:ISM", 30, ism_cemon_purchaser::once);
  try { 
  icp();
  } catch(std::exception& e) {
  	cout << e.what() << endl;
  }
  return 0;	
}
