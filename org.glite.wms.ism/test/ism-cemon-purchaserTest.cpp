#include "glite/wms/ism/purchaser/ism-cemon-purchaser.h"
#include "glite/ce/monitor-client-api-c/CEMonitorBinding.nsmap"
#include "glite/ce/monitor-client-api-c/CEEvent.h"
#include <vector>
#include <string>

using namespace std;
using namespace glite::wms::ism::purchaser;

int main(void)
{
  vector<string> services;
  services.push_back("http://lxb2022.cern.ch:8080/ce-monitor/services/CEMonitor");
  ism_cemon_purchaser icp(services,"CE_MONITOR", 30);
   icp();
  return 0;	
}
