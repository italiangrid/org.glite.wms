
#include "eventStatusPoller.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/BaseException.h"
#include "glite/ce/cream-client-api-c/InternalException.h"
#include "glite/ce/cream-client-api-c/DelegationException.h"
#include "abs-ice-core.h"
#include <vector>
#include <map>
#include <boost/thread/thread.hpp>

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::job_statuses;
using namespace glite::ce::cream_client_api::util;
using namespace std;

typedef vector<soap_proxy::JobInfoList*>::iterator JobInfoIt;
typedef vector<string>::iterator vstrIt;
typedef vector<string>::const_iterator cvstrIt;

//______________________________________________________________________________
eventStatusPoller::eventStatusPoller(
				     glite::wms::ice::absice* _iceManager,
				     const int& _d
				     )
  throw(eventStatusPoller_ex&)
  : endpolling(false), 
    delay(_d),
    iceManager(_iceManager),
    creamClient(NULL)
{
  jobs_to_query.reserve(1000);
  url_pieces.reserve(4);
  try {
    creamClient = new soap_proxy::CreamProxy(false);
    //creamClient->Authenticate( certfile );
  } catch(soap_proxy::soap_ex& ex) {
    throw eventStatusPoller_ex( ex.what() );
  } 
  oneJobToQuery.reserve(1);
  oneJobToPurge.reserve(1);

  //cout << "POLLER COSTRUITO !"<<endl;

}

//______________________________________________________________________________
// eventStatusPoller::eventStatusPoller(const eventStatusPoller& P) 
//   throw(eventStatusPoller_ex&)
// {
//   //cout << "COPY CTOR CALLED !!!" << endl;
//   try {
//     creamClient = new soap_proxy::CreamProxy(false);
//     //creamClient->Authenticate( certfile );
//   } catch(soap_proxy::soap_ex& ex) {
//     throw eventStatusPoller_ex( ex.what() );
//   }
// //   endpolling = P.endpolling;
// //   delay      = P.delay;
// //   iceManager = P.iceManager;
// //   jobs_to_query.reserve(1000);
// //   url_pieces.reserve(4);
// //   oneJobToQuery.reserve(1);
// //   oneJobToPurge.reserve(1);
// //   _jobinfolist.clear();
// }

//______________________________________________________________________________
eventStatusPoller::~eventStatusPoller()
{
  //cout << "POLLER DISTRUTTO !"<<endl;

  //  if(creamClient) delete(creamClient);
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
    for(JobInfoIt it = _jobinfolist.begin(); it != _jobinfolist.end(); ++it)
      if(*it) delete(*it);
    
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

//   map< string, vector<string> > endpoint_hash;
//   try {
//     CEUrl::organise_by_endpoint(jobs_to_query, endpoint_hash, 
// 			     "/ce-cream/services/CREAM");
//   } catch(CEUrl::ceid_syntax_ex& ex) {
//     cerr << ex.what()<<endl;
//     exit(1);
//   }


  for(vstrIt it = jobs_to_query.begin(); it != jobs_to_query.end(); ++it)
    {
      oneJobToQuery.clear();
      try {
	CreamJob theJob = jobCache::getInstance()->getJobByCreamJobID(*it);
	cout << "Sending JobInfo request for Job ["
	     << theJob.getJobID() << "]" << endl;
	oneJobToQuery.push_back(*it);
	creamClient->Authenticate( theJob.getUserProxyCertificate() );
	_jobinfolist.push_back( creamClient->Info(theJob.getCreamURL().c_str(), 
						  oneJobToQuery, 
						  empty, -1, -1 ));
	
      } catch (ClassadSyntax_ex& ex) {
	// this exception should not be raised because 
	// the CreamJob is created from another valid one
	cerr << "Fatal error: CreamJob creation failed from a valid one!!!" << endl;
	exit(1);
      } catch(soap_proxy::auth_ex& ex) {
	cerr << "Cannot query status job: " << ex.what() << endl;
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

  for(/* unsigned int k=0; k<_jobinfolist.size(); k++) { */
      JobInfoIt it = _jobinfolist.begin(); it != _jobinfolist.end(); ++it)
    {
      for(unsigned int j=0; j<(*it)->jobInfo.size(); j++) {
	stNum = getStatusNum((*it)->jobInfo[j]->status);
	cid = (*it)->jobInfo[j]->CREAMJobId;
	if((stNum == api::job_statuses::DONE_FAILED) ||
	   (stNum == api::job_statuses::ABORTED))
	  {
	    cout << "JobID ["
		 <<cid
		 <<"] is failed or aborted. Removing from cache and resubmitting..."
		 <<endl;
	    /**
	     * NOW MUST RESUBMIT THIS JOB
	     *
	     */
	    if(iceManager)
	      iceManager->doOnJobFailure(jobCache::getInstance()->get_grid_jobid_by_cream_jobid(cid));
	    jobs_to_purge.push_back(cid);
	    //continue;
	  }
	
	if( stNum == api::job_statuses::DONE_OK ||
	    stNum == api::job_statuses::CANCELLED) 
	  {
	    
	    // schedule this job for future purge
	    jobs_to_purge.push_back(cid);
	  }
	
	cout << jobs_to_purge.size() << " jobs to purge"<<endl;
	this->purgeJobs(jobs_to_purge);
	
	sleep(1); // sleep a little bit to not overload CREAM with too
	// many purges
	
	jobs_to_purge.clear();
      }
    }
}

//______________________________________________________________________________
void eventStatusPoller::updateJobCache() 
{
  for(JobInfoIt it = _jobinfolist.begin(); it != _jobinfolist.end(); ++it) {
    if(!*it) {
      //cerr << "_jobinfolist["<<k<<"] is NULL. Wont update the job cache\n";
      continue;
    }
    for(unsigned int j=0; j<(*it)->jobInfo.size(); j++) {
      
      glite::ce::cream_client_api::job_statuses::job_status 
	stNum = getStatusNum((*it)->jobInfo.at(j)->status);
      
      string gid;
      try {
	gid = jobCache::getInstance()->get_grid_jobid_by_cream_jobid((*it)->jobInfo.at(j)->CREAMJobId);
      } catch(exception& ex) {
	cerr << ex.what()<<endl;
	exit(1);
      }
      
      cerr << "Updating jobcache with\n"
	   << "\t\tgrid_jobid  = [" << gid<<"]\n"
	   << "\t\tcream_jobid = [" << (*it)->jobInfo[j]->CREAMJobId<<"]\n"
	   << "\t\tstatus      = [" << (*it)->jobInfo[j]->status<<"]"
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
// void eventStatusPoller::run() 
// {

// }

//______________________________________________________________________________
void eventStatusPoller::purgeJobs(const vector<string>& jobs_to_purge)
{
  if(!jobs_to_purge.size()) return;
  
//   map<string, vector<string> > endpoint_jobs;
//   CEUrl::organise_by_endpoint(jobs_to_purge, 
// 			      endpoint_jobs, 
// 			      "/ce-cream/services/CREAM");
  
  for(
      /* unsigned k=0; k < jobs_to_purge.size(); ++k) */
      cvstrIt it = jobs_to_purge.begin();
      it != jobs_to_purge.end();
      ++it
      )
    {
      oneJobToPurge.clear();
      string cid;
      try {
	cout << "Fetching Job for ID ["<<*it<<"]"<<endl;
	CreamJob theJob = jobCache::getInstance()->getJobByCreamJobID(*it);
        cid = theJob.getJobID();
	cout << "Calling JobPurge for host ["
	     << theJob.getCreamURL() << "]" <<endl;

	creamClient->Authenticate(theJob.getUserProxyCertificate());
	oneJobToPurge.push_back(theJob.getJobID());
	creamClient->Purge(theJob.getCreamURL().c_str(), oneJobToPurge);
        jobCache::getInstance()->remove_by_cream_jobid( cid );
      } catch (ClassadSyntax_ex& ex) {
	// this exception should not be raised because 
	// the CreamJob is created from another valid one
	cerr << "Fatal error: CreamJob creation failed from a valid one!!!" << endl;
	exit(1);
      } catch(soap_proxy::auth_ex& ex) {
	cerr << "Cannot purge job: " << ex.what() << endl;
      } catch(cream_exceptions::BaseException& s) {
	cerr << s.what()<<endl;
      } catch(cream_exceptions::InternalException& severe) {
	cerr << severe.what()<<endl;
	exit(1);
      } catch(elementNotFound_ex& ex) {                                   
          cerr << "Cannot remove ["<<cid                                    
               << "] from job cache: " << ex.what() << endl;  
      }
    }
}

//______________________________________________________________________________
void eventStatusPoller::operator()()
{
  //do_purchase();
  //cout << "eventStatusPoller::operator()() - calling run()"<<endl;
  //run();
    endpolling = false;
    //cout << "eventStatusPoller::() - entering while..."<<endl;
  while(!endpolling) {
    //    cout << "Inside while"<<endl;
    if(getStatus()) {
      cout << "eventStatusPoller::getStatus OK"<<endl;
      try{
	updateJobCache();
      }
      catch(...) {
	cerr << "inside ::() - catched something"<<endl;
      }
    }
    checkJobs(); // resubmits aborted/done-failed and/or purges terminated jobs
    /**
     * Let's dont use boost::thread::sleep because right not (18/11/2005)
     * the documentation says that it will be replaced by a more robust 
     * mechanism
     */
    //boost::thread::sleep(delay);
    sleep(delay);
  }
  cout << "eventStatusPoller::run - thread is ending..." << endl;
}
