
#include "eventStatusPoller.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/BaseException.h"
#include "glite/ce/cream-client-api-c/InternalException.h"
#include "glite/ce/cream-client-api-c/DelegationException.h"
#include "abs-ice-core.h"
#include <vector>

using namespace glite::wms::ice::util;
//using namespace glite::wms::ice;
using namespace glite::ce::cream_client_api;//::soap_proxy;

//using namespace glite::ce::cream_client_api::cream_exceptions;
using namespace glite::ce::cream_client_api::job_statuses;
using namespace glite::ce::cream_client_api::util;
using namespace std;

//______________________________________________________________________________
eventStatusPoller::eventStatusPoller(
				     const string& certfile,
				     const int& _d,
				     glite::wms::ice::absice* _iceManager
				     )
  throw(eventStatusPoller_ex&)
  : endpolling(false), 
    delay(_d),
    iceManager(_iceManager)
{
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
  
  soap_proxy::CreamProxyFactory::getProxy()->clearSoap();

  jobs_to_query.clear();
  try {
    jobCache::getInstance()->getActiveCreamJobIDs(jobs_to_query);
  } catch(exception& ex) {
    cerr << ex.what()<<endl;
    exit(1);
  }

  map< string, vector<string> > endpoint_hash;
  try {
    CEUrl::organise_by_endpoint(jobs_to_query, endpoint_hash, 
			     "/ce-cream/services/CREAM");
  } catch(CEUrl::ceid_syntax_ex& ex) {
    cerr << ex.what()<<endl;
    exit(1);
  }

  for(map< string, vector<string> >::iterator endpointIT=endpoint_hash.begin(); 
      endpointIT!=endpoint_hash.end();
      ++endpointIT)
    {
      try {
	cout << "Sending JobInfo request to ["<<endpointIT->first<<"]"<<endl;
	_jobinfolist.push_back( soap_proxy::CreamProxyFactory::getProxy()->Info(endpointIT->first.c_str(), endpointIT->second, empty, -1, -1 ));
	  
      } catch(soap_proxy::soap_ex& ex) { 
	cerr << "CreamProxy::Info raised a soap_ex exception: " 
	     << ex.what() << endl;
	return false; 
      } catch(cream_exceptions::BaseException& ex) {
	cerr << "CreamProxy::Info raised a BaseException exception: " 
	     << ex.what() << endl;
	return false; 
      } catch(cream_exceptions::InternalException& ex) {
	cerr << "CreamProxy::Info raised an InternalException exception: " 
	     << ex.what() << endl;
	return false; 
      } catch(cream_exceptions::DelegationException&) {
	cerr << "CreamProxy::Info raised a DelegationException exception\n";
	return false;
      }
    }

  return true;
}

//______________________________________________________________________________
void eventStatusPoller::checkJobs() 
{

  /**
   * THIS PROCEDURE CAN BE OPTIMIZED: NOW FOR EACH JOB THERE'S A 
   * JobPurge REQUEST SENT TO CREAM; IN FUTURE MULTIPLE JOBIDS
   * TO PURGE ON THE SAME CREAM MUST BE PUT IN A VECTOR AND THE A
   * SINGLE REQUEST WITH THIS VECTOR MUST BE SENT TO THAT CREAM
   */

  glite::ce::cream_client_api::job_statuses::job_status stNum;
  string cid, addrURL;
  vector<string> pieces, jobs_to_purge;

  pieces.reserve(3);
  jobs_to_purge.reserve(100);

  for(unsigned int k=0; k<_jobinfolist.size(); k++) {
    for(unsigned int j=0; j<_jobinfolist[k]->jobInfo.size(); j++) {
      stNum = getStatusNum(_jobinfolist[k]->jobInfo[j]->status);
      cid = _jobinfolist[k]->jobInfo[j]->CREAMJobId;
      if((stNum == glite::ce::cream_client_api::job_statuses::DONE_FAILED) ||
	 (stNum == glite::ce::cream_client_api::job_statuses::ABORTED))
	{
	  // Removes jobid from cache
	  cout << "JobID ["
	       <<cid
	       <<"] is failed or aborted. Removing from cache and resubmitting..."
	       <<endl;
	  try {
	    jobCache::getInstance()->remove_by_cream_jobid(cid);
	  } catch(elementNotFound_ex& ex) {
	    cerr << "Cannot remove ["<<cid
		 << "] from job cache: " << ex.what() << endl;
	  } catch(exception& ex) {
	    cerr << ex.what()<<endl;
	    exit(1);
	  }
	  /**
	   * NOW MUST RESUBMIT THIS JOB
	   *
	   */
	  
	  // resubmit
	  if(iceManager)
	    iceManager->doOnJobFailure(jobCache::getInstance()->get_grid_jobid_by_cream_jobid(cid));
	}
      
      if( api::job_statuses::isFinished( stNum ) ) {

	// schedule this job for future purge
	jobs_to_purge.push_back(cid);

	// Removes jobid from cache
	cout << "JobID ["<<cid<<"] is finished. Removing from memory cache..."<<endl;
	try {
	  jobCache::getInstance()->remove_by_cream_jobid(cid);
	} catch(elementNotFound_ex& ex) {
	  cerr << "Cannot remove ["<<cid
	       << "] from job cache: " << ex.what() << endl;
	} catch(exception& ex) {
	  cerr << ex.what()<<endl;
	  exit(1);
	}
      }

      cout << jobs_to_purge.size() << " jobs to purge"<<endl;
      this->purgeJobs(jobs_to_purge);
      jobs_to_purge.clear();
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
      
      string gid;
      try {
	gid = jobCache::getInstance()->get_grid_jobid_by_cream_jobid(_jobinfolist[k]->jobInfo.at(j)->CREAMJobId);
      } catch(exception& ex) {
	cerr << ex.what()<<endl;
	exit(1);
      }
      
      cerr << "Updating jobcache with\n"
	   << "\t\tgrid_jobid  = [" << gid<<"]\n"
	   << "\t\tcream_jobid = [" << _jobinfolist[k]->jobInfo[j]->CREAMJobId<<"]\n"
	   << "\t\tstatus      = [" << _jobinfolist[k]->jobInfo[j]->status<<"]"
	   <<endl;
      
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
      cout << "eventStatusPoller::getStatus OK"<<endl;
      try{
	updateJobCache();
      }
      catch(...) {
	cerr << "inside ::run - catched something"<<endl;
      }
    }
    checkJobs(); // resubmits aborted/done-failed and/or purges terminated jobs
    sleep(delay);
  }
  cout << "eventStatusPoller::run - thread is ending..." << endl;
}

//______________________________________________________________________________
void eventStatusPoller::purgeJobs(const vector<string>& jobs_to_purge)
{
  if(!jobs_to_purge.size()) return;
  
  map<string, vector<string> > endpoint_jobs;
  endpoint_jobs.reserve(jobs_to_purge.size());
  
  CEUrl::organise_by_endpoint(jobs_to_purge, 
			      endpoint_jobs, 
			      "/ce-cream/services/CREAM");
  
  for(map<string, vector<string> >::iterator it=endpoint_jobs.begin();
      it!=endpoint_jobs.end();
      ++it) 
    {
      try {
	cout << "Calling JobPurge for host ["<<it->first<<"]"<<endl;
	for(unsigned k=0; k<it->second.size(); ++k)
	  cout << " -> Will purge [" << it->second[k] 
	       << "]" << endl;
	soap_proxy::CreamProxyFactory::getProxy()->Purge(it->first.c_str(), it->second );
      } catch(cream_exceptions::BaseException& s) {
	cerr << s.what()<<endl;
      } catch(cream_exceptions::InternalException& severe) {
	cerr << severe.what()<<endl;
	exit(1);
      }
    }
}
