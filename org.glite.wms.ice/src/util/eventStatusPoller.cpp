
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
				     const string& certfile,
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
  try {
    jobCache::getInstance()->getActiveCreamJobIDs(jobs_to_query);
  } catch(exception& ex) {
    cerr << ex.what()<<endl;
    exit(1);
  }

  //  cout << "Active jobs to query status of: "<<jobs_to_query.size()<<endl;

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
void eventStatusPoller::checkJobs() 
{

  /**
   * THIS PROCEDURE CAN BE OPTIMIZED: NOW FOR EACH JOB THERE'S A 
   * JobPurge REQUEST SENT TO CREAM; IN FUTURE MULTIPLE JOBIDS
   * TO PURGE ON THE SAME CREAM MUST BE PUT IN A VECTOR AND THE A
   * SINGLE REQUEST WITH THIS VECTOR MUST BE SENT TO THAT CREAM
   */

  glite::ce::cream_client_api::job_statuses::job_status stNum;
  string cid, addrURL ;
  vector<string> pieces, jobs_to_purge;

  pieces.reserve(3);
  jobs_to_purge.reserve(1);

  for(unsigned int k=0; k<_jobinfolist.size(); k++) {
    for(unsigned int j=0; j<_jobinfolist[k]->jobInfo.size(); j++) {
      stNum = getStatusNum(_jobinfolist[k]->jobInfo[j]->status);
      cid = _jobinfolist[k]->jobInfo[j]->CREAMJobId;
      if((stNum == glite::ce::cream_client_api::job_statuses::DONE_FAILED) ||
	 (stNum == glite::ce::cream_client_api::job_statuses::ABORTED))
	{
	  // Removes jobid from cache
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
	}
      
      if( api::job_statuses::isFinished( stNum ) )
	{
	  // Removes jobid from cache
	  
	  try {
	    jobCache::getInstance()->remove_by_cream_jobid(cid);
	  } catch(elementNotFound_ex& ex) {
	    cerr << "Cannot remove ["<<cid
		 << "] from job cache: " << ex.what() << endl;
	  } catch(exception& ex) {
	    cerr << ex.what()<<endl;
	    exit(1);
	  }
	  // Purges jobid from CREAM server
	  pieces.clear();
	  glite::ce::cream_client_api::util::CEUrl::parseJobID( cid, pieces );
	  addrURL = pieces[0] + "://" + pieces[1] + ":" + pieces[2] +
	    "/ce-cream/services/CREAM";
	  jobs_to_purge.clear();
	  jobs_to_purge.push_back( cid );
	  try {
	    cout << "Calling JobPurge for jobid ["<<cid<<"]"<<endl;
	    creamClient->Purge(addrURL.c_str(), jobs_to_purge );
	  } catch(BaseException& s) {
	    cerr << s.what()<<endl;
	  } catch(InternalException& severe) {
	    cerr << severe.what()<<endl;
	    exit(1);
	  }
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
