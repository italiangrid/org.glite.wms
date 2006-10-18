/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * Event status poller
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// ICE includes
#include "ice-core.h"
#include "iceConfManager.h"
#include "eventStatusPoller.h"
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "iceLBEventFactory.h"
#include "CreamProxyFactory.h"

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
#include <boost/lexical_cast.hpp>

// system includes
#include <vector>
#include <map>

#define STATUS_POLL_RETRY_COUNT 4

using namespace glite::wms::ice::util;
namespace cream_api=glite::ce::cream_client_api;
namespace soap_proxy=glite::ce::cream_client_api::soap_proxy;
namespace jobstat=glite::ce::cream_client_api::job_statuses;
namespace cream_util=glite::ce::cream_client_api::util;
using namespace std;

typedef vector<soap_proxy::Status>::iterator JobStatusIt;
typedef vector<string>::iterator vstrIt;
typedef vector<string>::const_iterator cvstrIt;

boost::recursive_mutex eventStatusPoller::mutexJobStatusPoll;

/**
 * The following method will be used when the scenario with many jobs
 * and several CREAM urls will happen. Many jobs can be reorganized
 * in order to group the maximum number of job related to the same
 * CREAM Url AND the same proxy certificate, in order to reduce the
 * number of authentications on the same CREAM host
 */
//_____________________________________________________________________________
void organizeJobs( const vector<CreamJob> & vec,
                   map< string, map<string, vector<string> > >& target)
{
  for(vector<CreamJob>::const_iterator cit = vec.begin();
      cit != vec.end();
      ++cit) {
	( target[cit->getEndpoint()] )[ cit->getUserProxyCertificate() ].push_back( cit->getCreamJobID() );
  }
}

//____________________________________________________________________________
eventStatusPoller::eventStatusPoller( glite::wms::ice::Ice* manager, int d )
  throw(eventStatusPoller_ex&, ConfigurationManager_ex&)
  : iceThread( "event status poller" ),
    m_delay( d ),
    m_iceManager( manager ),
    m_creamClient( CreamProxyFactory::makeCreamProxy( false ) ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger()),
    m_lb_logger( iceLBLogger::instance() ),
    m_cache( jobCache::getInstance() ),
    m_threshold( iceConfManager::getInstance()->getPollerStatusThresholdTime() ) // may raise ConfigurationManager_ex
{

}

//____________________________________________________________________________
eventStatusPoller::~eventStatusPoller()
{

}

//____________________________________________________________________________
list< CreamJob > eventStatusPoller::get_jobs_to_poll( void ) 
{
    list<CreamJob> result;
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    
    for(jobCache::iterator jit = m_cache->begin(); jit != m_cache->end(); ++jit) {

        if( jit->getCreamJobID().empty() ) {
            // This job doesn't have yet the CREAM Job ID. Skipping...
            continue;
        }
        
        time_t oldness = time(NULL) - jit->getLastSeen();
	
        CREAM_SAFE_LOG(m_log_dev->debugStream() 
		       << "eventStatusPoller::scanJobs() - "
		       << "Job [" << jit->getCreamJobID() << "]"
		       << " oldness=" << oldness << " threshold=" << m_threshold
		       << log4cpp::CategoryStream::ENDLINE);
	
        if ( oldness >= m_threshold ) {
            result.push_back( *jit );
        }
    }
    return result;
}

//____________________________________________________________________________
 list< soap_proxy::JobInfo > eventStatusPoller::check_jobs( const list< CreamJob > & job_list ) 
{
    list< soap_proxy::JobInfo > result;

    for ( list< CreamJob >::const_iterator jit = job_list.begin(); jit != job_list.end(); jit++ ) {

        vector< string > job_to_query;
        vector< soap_proxy::JobInfo > the_job_status;
        
        job_to_query.push_back( jit->getCreamJobID() );
	
        try {
            m_creamClient->Authenticate( jit->getUserProxyCertificate() );
            m_creamClient->Info(jit->getCreamURL().c_str(),
                                job_to_query,
                                vector<string>(),
                                the_job_status,
                                -1,
                                -1 );
            
        } catch( cream_api::cream_exceptions::JobUnknownException& ex) {
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::scanJobs() - "
                           << "CREAM responded JobUnknown for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" << ex.what() << "]. Removing it from the cache"
                           << log4cpp::CategoryStream::ENDLINE);
            {
                boost::recursive_mutex::scoped_lock M( jobCache::mutex );
                jobCache::iterator thisJobToRemove = m_cache->lookupByCreamJobID( jit->getCreamJobID() );
                m_cache->erase( thisJobToRemove );
            }
            continue;
	  
        } catch(soap_proxy::auth_ex& ex) {

            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::scanJobs() - "
                           << "Cannot query status job for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" << ex.what() << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            handle_unreachable_job( jit->getCreamJobID() );
            continue;
            
        } catch(soap_proxy::soap_ex& ex) {

            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::scanJobs() - "
                           << "Cannot query status job for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" 
                           << ex.what() << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            handle_unreachable_job( jit->getCreamJobID() );
            continue;	  

        } catch(cream_api::cream_exceptions::BaseException& ex) {

            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::scanJobs() - "
                           << "Cannot query status job for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" 
                           << ex.what() << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            handle_unreachable_job( jit->getCreamJobID() );
            continue;

        } catch(cream_api::cream_exceptions::InternalException& ex) {
            
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::scanJobs() - "
                           << "Cannot query status job for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" 
                           << ex.what() << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            handle_unreachable_job( jit->getCreamJobID() );
            continue;

            // this ex can be raised if the remote service is not
            // reachable and scanJobs is called again
            // immediately. Untill the service is down this could
            // overload the cpu and the logfile. So let's wait for a
            // while before returning...
	  
        } catch(cream_api::cream_exceptions::DelegationException& ex) {
            
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::scanJobs() - "
                           << "Cannot query status job for JobId=["
                           << jit->getCreamJobID()
                           << "]. Exception is [" 
                           << ex.what() << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            handle_unreachable_job( jit->getCreamJobID() );
            continue;
        }


        result.push_back( the_job_status.front() );

        //
        // The following is DEAD code, and is not compiled
        //


#ifdef DO_NOT_USE

        if ( the_job_status.empty() ) {
            // The job is unknown to ICE; remove from the jobCache

            jit->incStatusPollRetryCount();
            if( jit->getStatusPollRetryCount() < STATUS_POLL_RETRY_COUNT ) {
	      CREAM_SAFE_LOG(m_log_dev->warnStream()
			     << "Job cream/grid ID=[" << jit->getCreamJobID()
			     << "]/[" << jit->getGridJobID()
			     << "] was not found on CREAM; Retrying later..."
			     << log4cpp::CategoryStream::ENDLINE);
                
	      //jobIt = m_cache->put( *jobIt );
	      {
		boost::recursive_mutex::scoped_lock M( jobCache::mutex );
		//jobCache::iterator tmp = m_cache->lookupByCreamJobID( jit->getCreamJobID() );
		m_cache->put( *jit );
	      }
	      
            } else {
	      CREAM_SAFE_LOG(m_log_dev->errorStream()
			     << "Job cream/grid ID=[" << jit->getCreamJobID()
			     << "]/[" << jit->getGridJobID()
			     << "] was not found on CREAM after " 
			     << STATUS_POLL_RETRY_COUNT
			     << "retries; Removing from the job cache"
			     << log4cpp::CategoryStream::ENDLINE);
	      {
		boost::recursive_mutex::scoped_lock M( jobCache::mutex );
		jobCache::iterator tmp = m_cache->lookupByCreamJobID( jit->getCreamJobID() );
		m_cache->erase( tmp );
                //jobIt = m_cache->erase( jobIt );
	      }
            }
        } else {
            job_status_list.push_back( the_job_status[0] );
            jit->resetStatusPollRetryCount();
            jit++;
        }
#endif
        //
        // end of DEAD code
        //
    }
    return result;
}


//____________________________________________________________________________
void eventStatusPoller::handle_unreachable_job( const std::string& cream_job_id )
{
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    jobCache::iterator jit = m_cache->lookupByCreamJobID( cream_job_id );
    
    if ( m_cache->end() == jit ) {
        return;
    }

    jit->incStatusPollRetryCount( );
    if( jit->getStatusPollRetryCount() < STATUS_POLL_RETRY_COUNT ) {

        CREAM_SAFE_LOG(m_log_dev->warnStream()
                       << "Job cream/grid ID=[" << jit->getCreamJobID()
                       << "]/[" << jit->getGridJobID()
                       << "] was not found on CREAM; Retrying later..."
                       << log4cpp::CategoryStream::ENDLINE);

        m_cache->put( *jit );
        
    } else {

        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "Job cream/grid ID=[" << jit->getCreamJobID()
                       << "]/[" << jit->getGridJobID()
                       << "] was not found on CREAM after " 
                       << STATUS_POLL_RETRY_COUNT
                       << "retries; Removing from the job cache"
                       << log4cpp::CategoryStream::ENDLINE);
        m_cache->erase( jit );

    }

}


//----------------------------------------------------------------------------
void eventStatusPoller::updateJobCache( const list< soap_proxy::JobInfo >& info_list )
{
    for ( list< soap_proxy::JobInfo >::const_iterator it = info_list.begin(); it != info_list.end(); ++it ) {

        update_single_job( *it );

    }
}

//____________________________________________________________________________
void eventStatusPoller::update_single_job( const soap_proxy::JobInfo& info_obj )
{
    // Locks the cache
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );

    vector< soap_proxy::Status > status_changes;
    info_obj.getStatusList( status_changes );
    string cid( info_obj.getCreamJobID() ); // Cream job id

    int count;
    vector< soap_proxy::Status >::const_iterator it;

    for ( it = status_changes.begin(), count = 1; it != status_changes.end(); ++it, ++count ) {

        //
        // Warning: the following block of code CANNOT be moved
        // outside the 'for' cycle; the reason is that ICE should be
        // tolerant to receiving inconsistent notifications, such that
        // two "DONE-FAILED" events. After the first DONE-FAILED, ICE
        // would purge the job. This means that the jit iterator would
        // no longer be valid. Hence, the necessity to check each
        // time.
        //
        jobCache::iterator jit( m_cache->lookupByCreamJobID( cid ) );    
        
        if ( m_cache->end() == jit ) {
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "eventStatusPoller::update_single_job() - cream_jobid ["
                           << cid << "] disappeared!"
                           << log4cpp::CategoryStream::ENDLINE);
            return;
        }
        
        // Update the worker node
        jit->set_worker_node( info_obj.getWorkerNode() );
        //
        // END block NOT to be moved outside the 'for' loop
        //

        jit->setLastSeen( time(0) );

        jobstat::job_status stNum( jobstat::getStatusNum( it->getStatusName() ) );
        // before doing anything, check if the job is "purged". If so,
        // remove from the cache and forget about it.
        if ( stNum == jobstat::PURGED ) {
            CREAM_SAFE_LOG(m_log_dev->infoStream()
                           << "eventStatusPoller::update_single_job() - "
                           << "Job with cream_job_id = ["
                           << jit->getCreamJobID()
                           << "], grid_job_id = ["
                           << jit->getGridJobID()
                           << "] is reported as PURGED. Removing from cache"
                           << log4cpp::CategoryStream::ENDLINE); 
            m_cache->erase( jit );
            return;
        }

        string exitCode( it->getExitCode() );

        if ( jit->get_num_logged_status_changes() < count ) {
            
            CREAM_SAFE_LOG(m_log_dev->infoStream()
                           << "eventStatusPoller::update_single_job() - "
                           << "Updating jobcache with "
                           << "grid_jobid = [" << jit->getGridJobID() << "] "
                           << "cream_jobid = [" << cid << "]"
                           << " status = [" << it->getStatusName() << "]"
                           << " exit_code = [" << exitCode << "]"
                           << " failure_reason = [" << it->getFailureReason() << "]"
                           << log4cpp::CategoryStream::ENDLINE);            

            // Creates a temporary job
            CreamJob tmp_job( *jit );

            tmp_job.setStatus( stNum );
            try {
                tmp_job.set_exit_code( boost::lexical_cast< int >( exitCode ) );
            } catch( boost::bad_lexical_cast & ) {
                tmp_job.set_exit_code( 0 );
            }
            tmp_job.set_failure_reason( it->getFailureReason() );
            tmp_job.set_num_logged_status_changes( count );

            // Log to L&B
            iceLBEvent* ev = iceLBEventFactory::mkEvent( tmp_job );
            if ( ev ) {
                tmp_job = m_lb_logger->logEvent( ev );
            }
            jit = m_cache->put( tmp_job );
            
            m_iceManager->resubmit_or_purge_job( jit );
        }
    }
}


//____________________________________________________________________________
void eventStatusPoller::body( void )
{
    while( !isStopped() ) {

        /**
         * We don't use boost::thread::sleep because right now
         * (18/11/2005) the documentation says it will be replaced by
         * a more robust mechanism in the future.
         */
        sleep( m_delay );

        // Thread wakes up

        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "eventStatusPoller::body() - New iteration"
                        << log4cpp::CategoryStream::ENDLINE );

        list< CreamJob > j_list( get_jobs_to_poll() );
        list< soap_proxy::JobInfo > j_status( check_jobs( j_list ) );
        try {
            updateJobCache( j_status );
        } catch(exception& ex) {
	  CREAM_SAFE_LOG(m_log_dev->errorStream()
			 << "eventStatusPoller::body() - "
			 << "catched std::exception: "
			 << ex.what()
			 << log4cpp::CategoryStream::ENDLINE);
	} catch(...) {
	  CREAM_SAFE_LOG(m_log_dev->errorStream()
			 << "eventStatusPoller::body() - "
			 << "catched unknown exception"
			 << log4cpp::CategoryStream::ENDLINE);
        }

    }
}

