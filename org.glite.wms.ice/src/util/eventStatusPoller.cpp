
#include "eventStatusPoller.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/BaseException.h"
#include "glite/ce/cream-client-api-c/InternalException.h"
#include "glite/ce/cream-client-api-c/DelegationException.h"
#include "abs-ice-core.h"
#include "iceConfManager.h"
#include <vector>
#include <map>
#include <boost/thread/thread.hpp>

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::job_statuses;
using namespace glite::ce::cream_client_api::util;
using namespace std;

typedef vector<soap_proxy::JobStatusList*>::iterator JobStatusIt;
typedef vector<string>::iterator vstrIt;
typedef vector<string>::const_iterator cvstrIt;

//______________________________________________________________________________
eventStatusPoller::eventStatusPoller(
				     glite::wms::ice::absice* _iceManager,
				     const int _d
				     )
  throw(eventStatusPoller_ex&)
  : endpolling(false), 
    delay(_d),
    iceManager(_iceManager),
    creamClient(NULL),
    log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()),
    _ev_logger( iceEventLogger::instance() ),
    cache( jobCache::getInstance() )
{
  jobs_to_query.reserve(1000);
  url_pieces.reserve(4);
  try {
    creamClient = new soap_proxy::CreamProxy(false);
  } catch(soap_proxy::soap_ex& ex) {
    throw eventStatusPoller_ex( ex.what() );
  } 
  oneJobToQuery.reserve(1);
  oneJobToPurge.reserve(1);
}

//______________________________________________________________________________
eventStatusPoller::~eventStatusPoller()
{
  if(creamClient) delete(creamClient);
}

//______________________________________________________________________________
bool eventStatusPoller::getStatus(void) 
{
    /** 
     * _jobstatuslist is filled with return values of CreamProxy::Status(...)
     * This call allocates ( via new() ) the object that it returns;
     * the caller of this call must take care of memory deallocation.
     */
    if(_jobstatuslist.size()) {
        for(JobStatusIt it = _jobstatuslist.begin(); it != _jobstatuslist.end(); ++it)
            if(*it) delete(*it);
    
    }
    _jobstatuslist.clear();
  
    creamClient->clearSoap();

    jobs_to_query.clear();

    boost::recursive_mutex::scoped_lock M( jobCache::mutex );

    try {
        cache->getActiveCreamJobIDs(jobs_to_query); // FIXME: does not throw anytning?
    } catch(exception& ex) {
        log_dev->log(log4cpp::Priority::ERROR,
                     string("eventStatusPoller::getStatus() - ")+ex.what());
        exit(1);
    }

    for(vstrIt it = jobs_to_query.begin(); it != jobs_to_query.end(); ++it) {
        oneJobToQuery.clear();
        jobCache::iterator jobIt = cache->lookupByCreamJobID(*it);
        try {
            if ( jobIt == cache->end() )
                throw( cream_exceptions::InternalException( "Invalid jobID" ) );

	    time_t oldness = time(NULL)-jobIt->getLastUpdate();
	    time_t threshold = iceConfManager::getInstance()->getPollerStatusThresholdTime();
//	    printf("\t*** oldness=%d threshold=%d\n", oldness, threshold);
//	    printf("\t*** listener=%d\n",iceConfManager::getInstance()->startListener());
            if( (oldness <  threshold) && iceConfManager::getInstance()->getStartListener() ) {
//	        printf("\t*** CONTINUE!!!\n");
		continue;
	    }
              //continue;

            log_dev->log(log4cpp::Priority::INFO,
                         string("eventStatusPoller::getStatus() - Sending JobStatus request for Job [")
                         + jobIt->getJobID() + "]");
            oneJobToQuery.push_back(*it);
            creamClient->Authenticate( jobIt->getUserProxyCertificate() );
            _jobstatuslist.push_back( creamClient->Status(jobIt->getCreamURL().c_str(),
                                                          oneJobToQuery,
                                                          empty, -1, -1 ) );
	
        } catch (ClassadSyntax_ex& ex) {
            // this exception should not be raised because
            // the CreamJob is created from another valid one
            log_dev->log(log4cpp::Priority::ERROR,
                         "eventStatusPoller::getStatus() - Fatal error: CreamJob creation failed from a valid one!!!");
            exit(1);
        } catch(soap_proxy::auth_ex& ex) {
            log_dev->log(log4cpp::Priority::ERROR,
                         string("eventStatusPoller::getStatus() - Cannot query status job: ")
                         + ex.what());
        } catch(soap_proxy::soap_ex& ex) {
            log_dev->log(log4cpp::Priority::ERROR,
                         string("eventStatusPoller::getStatus() - CreamProxy::Status() raised a soap_ex exception: ")
                         + ex.what());
            return false; 
        } catch(cream_exceptions::BaseException& ex) {
            log_dev->log(log4cpp::Priority::ERROR,
                         string("eventStatusPoller::getStatus() - CreamProxy::Status() raised a BaseException exception: ")
                         + ex.what());
            return false; 
        } catch(cream_exceptions::InternalException& ex) {
            log_dev->log(log4cpp::Priority::ERROR,
                         string("eventStatusPoller::getStatus() - CreamProxy::Status() raised a InternalException exception: ")
                         + ex.what());
            return false; 
        } catch(cream_exceptions::DelegationException&) {
            log_dev->log(log4cpp::Priority::ERROR,
                         string("eventStatusPoller::getStatus() - CreamProxy::Status() raised a DelegationException exception"));
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
    string addrURL;
    vector<string> pieces, jobs_to_purge;

    pieces.reserve(3);
    jobs_to_purge.reserve(100);

    for( JobStatusIt it = _jobstatuslist.begin(); 
         it != _jobstatuslist.end(); ++it) {

        vector<creamtypes__Status*> _statuslist = (*it)->jobStatus;
        
        // sit = Status ITerator
        for ( vector<creamtypes__Status*>::iterator sit = _statuslist.begin();
              sit != _statuslist.end(); sit++ ) {

            stNum = getStatusNum( (*sit)->name);
          
            if ( !(*sit)->jobId ) 
                continue;

            string cid( *((*sit)->jobId) );

            if ( stNum == api::job_statuses::DONE_FAILED ||
                 stNum == api::job_statuses::ABORTED ) {

                log_dev->infoStream()
                    << "eventStatusPoller::checkJobs() - JobID ["
                    << cid <<"] is failed or aborted. "
                    << "Removing from cache and resubmitting..."
                    << log4cpp::CategoryStream::ENDLINE;

                /**
                 * NOW MUST RESUBMIT THIS JOB
                 *
                 */
                if(iceManager) {

                    jobCache::iterator jobIt = cache->lookupByCreamJobID( cid );

                    if ( jobIt != cache->end() ) {
                        iceManager->doOnJobFailure( jobIt->getGridJobID() );
                    }
                    // FIXME: else????
                    // iceManager->doOnJobFailure(jobCache::getInstance()->get_grid_jobid_by_cream_jobid(cid));
                }
                jobs_to_purge.push_back(cid);
            }
          
            if ( stNum == api::job_statuses::DONE_OK ||
                 stNum == api::job_statuses::CANCELLED )
                {
                    // schedule this job for future purge
                    jobs_to_purge.push_back(cid);
                }
          
            log_dev->infoStream()
                << "eventStatusPoller::checkJobs() - "
                << jobs_to_purge.size() << " jobs to purge"
                << log4cpp::CategoryStream::ENDLINE;

          
            this->purgeJobs(jobs_to_purge);
          
            sleep(1); // sleep a little bit to not overload CREAM with too
            // many purges per second
          
            jobs_to_purge.clear();
        }
    }
}

//______________________________________________________________________________
void eventStatusPoller::updateJobCache()
{
    for ( JobStatusIt it = _jobstatuslist.begin(); 
          it != _jobstatuslist.end(); ++it) {

        soap_proxy::JobStatusList* theList = *it;
        
        if ( !theList )
            continue;

        vector<creamtypes__Status*> _statuslist = theList->jobStatus;
        
        // sit = Status ITerator
        for ( vector<creamtypes__Status*>::iterator sit = _statuslist.begin();
              sit != _statuslist.end(); sit++ ) {
            
            glite::ce::cream_client_api::job_statuses::job_status
                stNum = getStatusNum( (*sit)->name );
            
            try {
                // Locks the cache
                boost::recursive_mutex::scoped_lock M( jobCache::mutex );
                
                if ( ! (*sit)->jobId ) 
                    continue;
                
                string cid( *( (*sit)->jobId ) );
                
                // jit = Jobcache ITerator
                jobCache::iterator jit( cache->lookupByCreamJobID( cid ) );
                
                if ( jit != cache->end() ) {
                    // Check if the status changed after the last 
                    // status change as recorded by the jobCache
                    
                    if ( jit->getLastUpdate() < (*sit)->timestamp ) {
                        
                        log_dev->infoStream()
                            << "eventStatusPoller::updateJobCache() - "
                            << "Updating jobcache with "
                            << "grid_jobid = [" << jit->getGridJobID() << "] "
                            << "cream_jobid = [" << cid << "]"
                            << "status = [" << (*sit)->name << "]"
                            << log4cpp::CategoryStream::ENDLINE;
                        
                        jit->setStatus( stNum, (*sit)->timestamp );
                        cache->put( *jit );
                        // Log to L&B
                        _ev_logger->log_job_status_change( *jit ); // FIXME
                    }
                } else {
                    log_dev->errorStream() 
                        << "cream_jobid = ["<< cid << "] disappeared!"
                        << log4cpp::CategoryStream::ENDLINE;
                }
            } catch(exception& ex) {
                log_dev->log(log4cpp::Priority::ERROR, ex.what());
                exit(1);
            }
        }
    }
}

//______________________________________________________________________________
void eventStatusPoller::purgeJobs(const vector<string>& jobs_to_purge)
{
  if(!jobs_to_purge.size()) return;
  
  for(
      cvstrIt it = jobs_to_purge.begin();
      it != jobs_to_purge.end();
      ++it
      )
    {
      oneJobToPurge.clear();
      string cid;
      try {
          boost::recursive_mutex::scoped_lock M( jobCache::mutex );
          log_dev->debugStream()
              << "eventStatusPoller::purgeJobs() - "
              << "Fetching Job for ID [" << *it << "]"
              << log4cpp::CategoryStream::ENDLINE;

          jobCache::iterator jit = jobCache::getInstance()->lookupByCreamJobID( *it );
          cid = jit->getJobID();
          log_dev->infoStream() 
              << "eventStatusPoller::purgeJobs() - "
              << "Calling JobPurge for host ["
              << jit->getCreamURL() << "]"
              << log4cpp::CategoryStream::ENDLINE;
          creamClient->Authenticate( jit->getUserProxyCertificate());
          oneJobToPurge.push_back( jit->getJobID() );
          creamClient->Purge( jit->getCreamURL().c_str(), oneJobToPurge);
          jobCache::getInstance()->remove( jit );
      } catch (ClassadSyntax_ex& ex) {
	/**
	 * this exception should not be raised because
	 * the CreamJob is created from another valid one
	 */
	log_dev->errorStream() << "eventStatusPoller::purgeJobs() - "
			       << "Fatal error: CreamJob creation failed "
			       << "copying from a valid one!!!"
			       << log4cpp::CategoryStream::ENDLINE;
	exit(1);
      } catch(soap_proxy::auth_ex& ex) {
	log_dev->log(log4cpp::Priority::ERROR,
		     string("eventStatusPoller::purgeJobs() - Cannot purge job: ") + ex.what());
      } catch(cream_exceptions::BaseException& s) {
	log_dev->log(log4cpp::Priority::ERROR, s.what());
      } catch(cream_exceptions::InternalException& severe) {
	log_dev->log(log4cpp::Priority::ERROR, severe.what());
	exit(1);
      } catch(elementNotFound_ex& ex) {
	log_dev->log(log4cpp::Priority::ERROR, 
		     string("eventStatusPoller::purgeJobs() - Cannot remove [") + cid
		     + "] from job cache: "
		     + ex.what());
      }
    }
}

//______________________________________________________________________________
void eventStatusPoller::operator()()
{
  endpolling = false;
  while(!endpolling) {
    if(getStatus()) {
      try{
	updateJobCache();
      }
      catch(...) {
	log_dev->log(log4cpp::Priority::ERROR, 
		     "eventStatusPoller::operator() - catched unknown ex");
      }
    }
    checkJobs(); // resubmits aborted/done-failed and/or purges terminated jobs
    /**
     * Let's dont use boost::thread::sleep because right not (18/11/2005)
     * the documentation says that it will be replaced by a more robust 
     * mechanism
     */
    sleep(delay);
  }
  log_dev->log(log4cpp::Priority::INFO,
	       "eventStatusPoller::run() - thread is ending...");
}

