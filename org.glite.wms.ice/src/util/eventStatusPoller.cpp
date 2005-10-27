
#include "eventStatusPoller.h"
#include <vector>

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::cream_exceptions;
using namespace glite::ce::cream_client_api::job_statuses;
using namespace std;


//______________________________________________________________________________
eventStatusPoller::eventStatusPoller(const std::string& certfile,
				     const std::string& _cream_service,
				     const int& _d)
  throw(eventStatusPoller_ex&)
  : jobs(NULL),
    endpolling(false), 
    delay(_d),
    cream_service(_cream_service), 
    creamClient( NULL ), 
    jobInfoList( NULL )
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
bool eventStatusPoller::getStatus(void) 
{
  //  vector<string> empty;
  glite::ce::cream_client_api::soap_proxy::JobInfoList* jobinfolist = NULL;
  creamClient->clearSoap();
  try {
    jobinfolist = creamClient->Info(cream_service.c_str(),
				    empty, 
				    empty, 
				    -1, // SINCE
				    -1  // TO
				    );
  } catch(soap_ex&) { 
    cerr << "CreamProxy::Info raised a soap_ex exception\n";
    return false; 
  } catch(BaseException&) {
    cerr << "CreamProxy::Info raised a BaseException exception\n";
    return false; 
  } catch(InternalException&) {
    cerr << "CreamProxy::Info raised an InternalException exception\n";
    return false; 
  } catch(DelegationException&) {
    cerr << "CreamProxy::Info raised a DelegationException exception\n";
    return false;
  }
  if(jobinfolist)
    jobInfoList = &(jobinfolist->jobInfo);
  return true;
}

//______________________________________________________________________________
void eventStatusPoller::updateJobCache() 
{
  if(!jobs)
    {
      cerr << "Cache not initialized. Skipping update operation\n";
      return;
    }
  if(!jobInfoList) {
    cerr << "jobInfoList field is NULL. Wont update the job cache\n";
    return;
  }

  for(unsigned int j=0; j<jobInfoList->size(); j++) {
 //    cerr << "Going to update jobcache with "
// 	 << "[grid_jobid="<<jobInfoList->at(j)->GridJobId
// 	 <<", cream_jobid="<< jobInfoList->at(j)->CREAMJobId
// 	 << ", status="<<jobInfoList->at(j)->status<<"]\n";
    
    glite::ce::cream_client_api::job_statuses::job_status 
      stNum = getStatusNum(jobInfoList->at(j)->status);

    try {jobs->put(jobInfoList->at(j)->GridJobId,
		   jobInfoList->at(j)->CREAMJobId,
		   stNum);
    } catch(exception& ex) {
      cerr << "eventStatusPoller::updateJobCache - jobCache::put raised an ex: "<<ex.what()<<endl;
      exit(1);
    }
  }
}

//______________________________________________________________________________
void eventStatusPoller::run() 
{
  endpolling = false;
  while(!endpolling) {
    if(getStatus())
      updateJobCache();
    cout << "eventStatusPoller::run - called run" << endl;
    sleep(delay);
  }
  cout << "eventStatusPoller::run - ending..." << endl;
}
