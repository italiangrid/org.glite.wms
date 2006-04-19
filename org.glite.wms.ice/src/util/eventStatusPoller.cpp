// ICE includes
#include "ice-core.h"
#include "iceConfManager.h"
#include "eventStatusPoller.h"
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEventFactory.h"

// other glite includes
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/BaseException.h"
#include "glite/ce/cream-client-api-c/InternalException.h"
#include "glite/ce/cream-client-api-c/DelegationException.h"

// boost includes
#include <boost/thread/thread.hpp>

// system includes
#include <vector>
#include <map>

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api;
namespace jobstat=glite::ce::cream_client_api::job_statuses;
using namespace glite::ce::cream_client_api::util;
using namespace std;

typedef vector<soap_proxy::Status>::iterator JobStatusIt;
typedef vector<string>::iterator vstrIt;
typedef vector<string>::const_iterator cvstrIt;

/**
 * The following should organize the creamJobIDs in order to
 * group the maximum number of job related to the same CREAM URL and proxy
 * certificate, in order to send the status request for more jobs
 * to a single endpoint and with only one authentication call.
 */
//______________________________________________________________________________
void organizeJobs(const vector<CreamJob>& vec,
		  map<string, map<string, vector<string>  > >& target)
{
  for(vector<CreamJob>::const_iterator cit = vec.begin();
      cit != vec.end();
      cit++) {
	(target[cit->getEndpoint()])[cit->getUserProxyCertificate()].push_back( cit->getJobID() );
  }
}

//______________________________________________________________________________
eventStatusPoller::eventStatusPoller(
				     glite::wms::ice::Ice* _iceManager,
				     const int _d
				     )
  throw(eventStatusPoller_ex&)
  : iceThread( "event status poller" ),
    delay(_d),
    iceManager(_iceManager),
    creamClient( 0 ),
    log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()),
    _lb_logger( iceLBLogger::instance() ),
    cache( jobCache::getInstance() )
{
    try {
        soap_proxy::CreamProxy *p = new soap_proxy::CreamProxy(false);
        creamClient.reset( p );
    } catch(soap_proxy::soap_ex& ex) {
        throw eventStatusPoller_ex( ex.what() );
    } 
    oneJobToQuery.reserve(1);
    oneJobToPurge.reserve(1);
}

//______________________________________________________________________________
eventStatusPoller::~eventStatusPoller()
{
}

//______________________________________________________________________________
bool eventStatusPoller::getStatus(void)
{
    creamClient->clearSoap();

    boost::recursive_mutex::scoped_lock M( jobCache::mutex );

    time_t oldness;// = time(NULL)-jobIt->getLastUpdate();
    time_t threshold;// = iceConfManager::getInstance()->getPollerStatusThresholdTime();
    bool _tmp_start_listener;

    statusTarget.clear();

    for(jobCache::iterator jobIt = cache->begin(); jobIt != cache->end(); ++jobIt) {

        oneJobToQuery.clear();
        
	{
            boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
            oldness = time(NULL)-jobIt->getLastSeen();
            threshold = iceConfManager::getInstance()->getPollerStatusThresholdTime();
            _tmp_start_listener = iceConfManager::getInstance()->getStartListener();
	}
        
        log_dev->warnStream()
            << "eventStatusPoller::getStatus() - Checking job cream/grid ID=[" 
            << jobIt->getJobID()
            << "]/["
            << jobIt->getGridJobID()
            << "]; oldness="
            << oldness
            << " thrshold=" 
            << threshold
            << log4cpp::CategoryStream::ENDLINE;
        
        
        if( (oldness <  threshold) && _tmp_start_listener ) {
            continue;
        }

        log_dev->infoStream()
            << "eventStatusPoller::getStatus() - "
            << "Sending JobStatus request for Job ["
            << jobIt->getJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;

        oneJobToQuery.push_back(jobIt->getJobID());
	// statusTarget.clear();
        try {
            creamClient->Authenticate( jobIt->getUserProxyCertificate() );
            creamClient->Status(jobIt->getCreamURL().c_str(),
                                oneJobToQuery,
                                empty,
				statusTarget,
				-1,
				-1 );

            if ( statusTarget.empty() ) {
                // The job is unknown by ICE; remove from the jobCache
                log_dev->warnStream()
                    << "Job cream/grid ID=["
                    << jobIt->getJobID()
                    << "]/["
                    << jobIt->getGridJobID()
                    << "] was not found on CREAM; "
                    << "Removing from the job cache"
                    << log4cpp::CategoryStream::ENDLINE;

                jobIt = cache->erase( jobIt );
                jobIt--; // this is necessary to avoid skipping the next item
            } else {
                //_jobstatuslist.push_back( job_stat );
            }
        } catch (ClassadSyntax_ex& ex) { // FIXME: never thrown?
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
                         string("eventStatusPoller::getStatus() - CreamProxy::Status() raised a soap_ex exception: ") + ex.what());
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
    api::job_statuses::job_status stNum;
    string exitCode;
    string addrURL;
    vector<string> pieces, jobs_to_purge;

    pieces.reserve(3);
    jobs_to_purge.reserve(100);

    for( JobStatusIt it = statusTarget.begin(); it != statusTarget.end(); ++it) {

        stNum = jobstat::getStatusNum( it->getStatusName() );
        exitCode = it->getExitCode();
        string cid( it->getJobID() );
        
        bool DONE = (stNum == api::job_statuses::DONE_FAILED) ||
            (stNum == api::job_statuses::DONE_OK);
        
        /**
         * Let's wait if the job has done but CREAM didn't
         * retrieve yet the exit code
         */
        if( DONE && (exitCode=="W") ) {
            log_dev->infoStream()
                << "eventStatusPoller::checkJobs() - WILL NOT Purge Job ["
                << cid <<"] because exitCode isn't available yet. "
                << log4cpp::CategoryStream::ENDLINE;
            return;
        }
        
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
                    iceManager->resubmit_job( *jobIt );
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

//______________________________________________________________________________
void eventStatusPoller::updateJobCache()
{
    int count;
    JobStatusIt it;
    for ( it = statusTarget.begin(), count = 1; it != statusTarget.end(); ++it, ++count ) {

        glite::ce::cream_client_api::job_statuses::job_status
            stNum = jobstat::getStatusNum( it->getStatusName() );
        
        try {
            // Locks the cache
            boost::recursive_mutex::scoped_lock M( jobCache::mutex );
            
            string cid( it->getJobID() );
            
            jobCache::iterator jit( cache->lookupByCreamJobID( cid ) );
            
            if ( jit != cache->end() ) {
                // Check if the status changed after the last 
                // status change as recorded by the jobCache
                
                if ( jit->get_num_logged_status_changes() < count ) {
                    
                    log_dev->infoStream()
                        << "eventStatusPoller::updateJobCache() - "
                        << "Updating jobcache with "
                        << "grid_jobid = [" << jit->getGridJobID() << "] "
                        << "cream_jobid = [" << cid << "]"
                        << " status = [" << it->getStatusName() << "]"
                        << log4cpp::CategoryStream::ENDLINE;
                    
                    jit->setStatus( stNum );
                    jit->set_num_logged_status_changes( count );
                    
                    // Log to L&B
                    _lb_logger->logEvent( iceLBEventFactory::mkEvent( *jit ) );
                }
                jit->setLastSeen( time(0) );
                cache->put( *jit );
            } else {
                log_dev->errorStream() 
                    << "cream_jobid ["<< cid << "] disappeared!"
                    << log4cpp::CategoryStream::ENDLINE;
            }
        } catch(exception& ex) {
            log_dev->log(log4cpp::Priority::ERROR, ex.what());
            exit(1);
        }
    }
}


//______________________________________________________________________________
void eventStatusPoller::purgeJobs(const vector<string>& jobs_to_purge)
{
  if(!jobs_to_purge.size()) return;
  string cid;
  for(
      cvstrIt it = jobs_to_purge.begin();
      it != jobs_to_purge.end();
      ++it
      )
    {
      oneJobToPurge.clear();

      try {
          boost::recursive_mutex::scoped_lock M( jobCache::mutex );
          log_dev->debugStream()
              << "eventStatusPoller::purgeJobs() - "
              << "Fetching Job for ID [" << *it << "]"
              << log4cpp::CategoryStream::ENDLINE;

          jobCache::iterator jit = cache->lookupByCreamJobID( *it );
	  if( jit == cache->end() )
	  {
	    log_dev->errorStream() 
                << "eventStatusPoller::purgeJobs() - "
                << "NOT Found in chace job ["
                << *it << "]"
                << log4cpp::CategoryStream::ENDLINE;
	    return;
	  }
	  cid = jit->getJobID();
          log_dev->infoStream()
              << "eventStatusPoller::purgeJobs() - "
              << "Calling JobPurge for host ["
              << jit->getCreamURL() << "]"
              << log4cpp::CategoryStream::ENDLINE;
	  // We cannot accumulate more jobs to purge in a vector
	  // because we must authenticate different jobs with different
	  // user certificates.
	  creamClient->Authenticate( jit->getUserProxyCertificate());
          oneJobToPurge.push_back( jit->getJobID() );
          creamClient->Purge( jit->getCreamURL().c_str(), oneJobToPurge);
          jit = cache->erase( jit );
          jit--;
      } catch (ClassadSyntax_ex& ex) {
	/**
	 * this exception should not be raised because
	 * the CreamJob is created from another valid one
	 */
	log_dev->errorStream() 
            << "eventStatusPoller::purgeJobs() - "
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
          log_dev->errorStream()
              << "eventStatusPoller::purgeJobs() - Cannot remove [" << cid
              << "] from job cache: " << ex.what()
              << log4cpp::CategoryStream::ENDLINE;
      }
    }
}

//______________________________________________________________________________
void eventStatusPoller::body( void )
{
    while( !isStopped() ) {
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
}

