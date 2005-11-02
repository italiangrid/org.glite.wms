
#include "eventStatusPoller.h"
#include "jobCache.h"
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
  : endpolling(false), 
    delay(_d),
    cream_service(_cream_service), 
    creamClient( NULL ), 
    _jobinfolist( NULL )
{
  try {
    creamClient = new CreamProxy(false);
    string VO = creamClient->Authenticate(certfile);
  } catch(soap_ex& ex) {
    throw eventStatusPoller_ex(ex.what());
  } catch(auth_ex& auth) {
    throw eventStatusPoller_ex(auth.what());
  }

  jobs_to_query.reserve(1000);
}

//______________________________________________________________________________
bool eventStatusPoller::getStatus(void) 
{
  /** 
   * _jobinfolist is assigned the return value of CreamProxy::Info(...)
   * This call allocates ( via new() ) the object that it returns;
   * the caller of this call must take care of memory deallocation.
   */
  if(_jobinfolist)
    delete(_jobinfolist);
  _jobinfolist = NULL;

  creamClient->clearSoap();

  jobs_to_query.clear();
  jobCache::getInstance()->getActiveCreamJobIDs(jobs_to_query);

  try {
    //cout << "Calling remote Cream jobInfo..."<<endl;
    _jobinfolist = creamClient->Info(cream_service.c_str(),
				     jobs_to_query, 
				     empty, 
				     -1, // SINCE
				     -1  // TO
				     );
  } catch(soap_ex& ex) { 
    cerr << "CreamProxy::Info raised a soap_ex exception: " 
	 << ex.what() << endl;
    return false; 
  } catch(BaseException& ex) {
    cerr << "CreamProxy::Info raised a BaseException exception: " 
	 << ex.what() << endl;
    return false; 
  } catch(InternalException& ex) {
    cerr << "CreamProxy::Info raised an InternalException exception: " 
	 << ex.what() << endl;
    return false; 
  } catch(DelegationException&) {
    cerr << "CreamProxy::Info raised a DelegationException exception\n";
    return false;
  }
//   if(_jobinfolist)
//     jobInfoVector = &(_jobinfolist->jobInfo);
  return true;
}

//______________________________________________________________________________
void eventStatusPoller::checkJobs() {
  for(unsigned int j=0; j<_jobinfolist->jobInfo.size(); j++) {

    glite::ce::cream_client_api::job_statuses::job_status 
      stNum = getStatusNum(_jobinfolist->jobInfo.at(j)->status);

    if((stNum == glite::ce::cream_client_api::job_statuses::DONE_FAILED) ||
       (stNum == glite::ce::cream_client_api::job_statuses::ABORTED))
      {
	// resubmit
      }

    if( api::job_statuses::isFinished( stNum ) )
      {
	// purge
      }
  }
}

//______________________________________________________________________________
void eventStatusPoller::updateJobCache() 
{
  if(!_jobinfolist) {
    cerr << "_jobinfolist internal variable is NULL. Wont update the job cache\n";
    return;
  }

  for(unsigned int j=0; j<_jobinfolist->jobInfo.size(); j++) {
    cerr << "Going to update jobcache with "
     	 << "[grid_jobid="<<_jobinfolist->jobInfo[j]->GridJobId
     	 <<", cream_jobid="<< _jobinfolist->jobInfo[j]->CREAMJobId
     	 << ", status="<< _jobinfolist->jobInfo[j]->status<<"]\n";
    
    //glite::ce::cream_client_api::job_statuses::job_status 
    glite::ce::cream_client_api::job_statuses::job_status 
      stNum = getStatusNum(_jobinfolist->jobInfo.at(j)->status);
//     if(stNum == -1)
//       stNum = glite::ce::cream_client_api::job_statuses::UNKNOWN;

    string gid = jobCache::getInstance()->get_grid_jobid_by_cream_jobid(_jobinfolist->jobInfo.at(j)->CREAMJobId);

    try {
      jobCache::getInstance()->put(gid,
				   _jobinfolist->jobInfo.at(j)->CREAMJobId,
				   (glite::ce::cream_client_api::job_statuses::job_status)stNum);
    } catch(std::exception& ex) {
      cerr << "eventStatusPoller::updateJobCache - jobCache::put(...) raised an ex: "
	   << ex.what()<<endl;
      delete(_jobinfolist);
      _jobinfolist = NULL;
      exit(1);
    }
  }
}

//______________________________________________________________________________
void eventStatusPoller::run() 
{
  endpolling = false;
  while(!endpolling) {
    if(getStatus()) {
      cout << "eventStatusPoller::getStatus OK - Updating jobCache..."<<endl;
      checkJobs(); // resubmits aborted/done-failed and/or purges terminated jobs
      try{
	updateJobCache();
      }
      catch(...) {
	cerr << "inside ::run - catched something"<<endl;
      }
    }

    sleep(delay);
  }
  cout << "eventStatusPoller::run - ending..." << endl;
}
