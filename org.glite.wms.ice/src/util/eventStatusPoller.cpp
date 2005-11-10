
#include "eventStatusPoller.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include <vector>

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::cream_exceptions;
using namespace glite::ce::cream_client_api::job_statuses;
using namespace std;


//______________________________________________________________________________
eventStatusPoller::eventStatusPoller(
				     const std::string& certfile,
				     // const std::string& _cream_service,
				     const int& _d
				     )
  throw(eventStatusPoller_ex&)
  : endpolling(false), 
    delay(_d),
    creamClient( NULL )
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
  url_pieces.reserve(4);
}

//______________________________________________________________________________
bool eventStatusPoller::getStatus(void) 
{
  /** 
   * _jobinfolist is filled with return values of CreamProxy::Info(...)
   * This call allocates ( via new() ) the object that it returns;
   * the caller of this call must take care of memory deallocation.
   */
  if(_jobinfolist.size()) {
    for(unsigned k=0; k<_jobinfolist.size(); k++)
      if(_jobinfolist[k]) delete(_jobinfolist[k]);
    
  }
  _jobinfolist.clear();
  
  creamClient->clearSoap();

  jobs_to_query.clear();
  jobCache::getInstance()->getActiveCreamJobIDs(jobs_to_query);

//   for(unsigned l=0; l<jobs_to_query.size(); l++)
//     cout << "::geStatus - active job="<<jobs_to_query.at(l)<<endl;


  map< string, vector<string> > endpoint_hash;
  map< string, vector<string> >::iterator endpointIT;
  vector<string> pieces;
  string endpoint;
  for(unsigned int j=0; j<jobs_to_query.size(); j++) {
    string thisJob = jobs_to_query[j];
    pieces.clear();
    glite::ce::cream_client_api::util::CEUrl::parseJobID(jobs_to_query[j], pieces);
    endpoint = pieces[0]+"://"+pieces[1]+":"+pieces[2]+"/ce-cream/services/CREAM";
    endpoint_hash[endpoint].push_back(jobs_to_query[j]);
    
  }

  for(endpointIT=endpoint_hash.begin(); endpointIT!=endpoint_hash.end();endpointIT++)
    {
      try {
	cout << "Sending JobInfo request to ["<<endpointIT->first<<"]"<<endl;
	_jobinfolist.push_back( creamClient->Info(endpointIT->first.c_str(),
						  endpointIT->second, 
						  empty, 
						  -1, // SINCE
						  -1  // TO
						  )
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
    }

  //cout << "_jobinfolist size="<<_jobinfolist.size()<<endl;

  return true;
}

//______________________________________________________________________________
void eventStatusPoller::checkJobs() {
  glite::ce::cream_client_api::job_statuses::job_status stNum;
  for(unsigned int k=0; k<_jobinfolist.size(); k++) {
    for(unsigned int j=0; j<_jobinfolist[k]->jobInfo.size(); j++) {
      stNum = getStatusNum(_jobinfolist[k]->jobInfo[j]->status);
      
      if((stNum == glite::ce::cream_client_api::job_statuses::DONE_FAILED) ||
	 (stNum == glite::ce::cream_client_api::job_statuses::ABORTED))
	{
	  // resubmit
	}
      
      if( api::job_statuses::isFinished( stNum ) )
	{
	  // purge
	  //	vector<string> pieces;
	  // 	pieces.clear();
	  // 	string endpoint;
	  // 	glite::ce::cream_client_api::ceurl_util::parseJobID(jobinfolist->jobInfo[j]->CREAMJobId,pieces);
	  // 	string endpoint = pieces[1] + ":" + pieces[2];
	}
    }
  }
}

//______________________________________________________________________________
void eventStatusPoller::updateJobCache() 
{
  for(unsigned int k=0; k<_jobinfolist.size(); k++) {
    if(!_jobinfolist[k]) {
      cerr << "_jobinfolist["<<k<<"] is NULL. Wont update the job cache\n";
      continue;
    }
    for(unsigned int j=0; j<_jobinfolist[k]->jobInfo.size(); j++) {
      
      glite::ce::cream_client_api::job_statuses::job_status 
	stNum = getStatusNum(_jobinfolist[k]->jobInfo.at(j)->status);
      
      string gid = jobCache::getInstance()->get_grid_jobid_by_cream_jobid(_jobinfolist[k]->jobInfo.at(j)->CREAMJobId);
      
      cerr << "Updating jobcache with\n"
	   << "\t\tgrid_jobid  = [" << gid<<"]\n"
	   << "\t\tcream_jobid = [" << _jobinfolist[k]->jobInfo[j]->CREAMJobId<<"]\n"
	   << "\t\tstatus      = [" << _jobinfolist[k]->jobInfo[j]->status<<"]"<<endl;
      
      try {
	jobCache::getInstance()->updateStatusByGridJobID(gid, stNum);
      } catch(std::exception& ex) {
	cerr << "eventStatusPoller::updateJobCache - "
	     << "jobCache::updateStatusByGridJobID(...) raised an ex: "
	     << ex.what()<<endl;
	exit(1);
      }
    }
  }
}

//______________________________________________________________________________
void eventStatusPoller::run() 
{
  endpolling = false;
  while(!endpolling) {
    if(getStatus()) {
      cout << "eventStatusPoller::getStatus OK"<<endl;// - Updating jobCache..."<<endl;
      checkJobs(); // resubmits aborted/done-failed and/or purges terminated jobs
      try{
	//cout << "Updating jobCache..."<<endl;
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
