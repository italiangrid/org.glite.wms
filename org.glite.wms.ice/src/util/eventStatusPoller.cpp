
#include "eventStatusPoller.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include <vector>

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace std;

//______________________________________________________________________________
eventStatusPoller::eventStatusPoller(const std::string& certfile,
				     const std::string& _cream_service)
  throw(eventStatusPoller_ex&)
  : jobs(NULL), grid_JOBID(""), cream_JOBID(""),
    status(glite::ce::cream_client_api::job_statuses::UNKNOWN),
    endpolling(false), cream_service(_cream_service), 
    creamClient( NULL ), jobinfolist( NULL )
{
  try {
    creamClient = new CreamProxy(false);
    string VO = creamClient->Authenticate(certfile);
  } catch(soap_ex& ex) {
    throw eventStatusPoller_ex(ex.what());
  } catch(auth_ex& auth) {
    throw eventStatusPoller_ex(auth.what());
  }
}

//______________________________________________________________________________
void eventStatusPoller::getStatus(void) 
{
//  vector<string> empty;
  jobinfolist = creamClient->Info(cream_service.c_str(),
				  empty, 
				  empty, 
				  -1, // SINCE
				  -1  // TO
				  );
}

//______________________________________________________________________________
void eventStatusPoller::updateJobCache() 
{
}

//______________________________________________________________________________
void eventStatusPoller::run() 
{
}
