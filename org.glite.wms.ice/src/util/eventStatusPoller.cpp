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
	( target[cit->getEndpoint()] )[ cit->getUserProxyCertificate() ].push_back( cit->getJobID() );
  }
}

//____________________________________________________________________________
eventStatusPoller::eventStatusPoller( glite::wms::ice::Ice* manager, int d )
  throw(eventStatusPoller_ex&)
  : iceThread( "event status poller" ),
    m_delay( d ),
    m_iceManager( manager ),
    m_creamClient( 0 ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger()),
    m_lb_logger( iceLBLogger::instance() ),
    m_cache( jobCache::getInstance() )
{
    try {
        soap_proxy::CreamProxy *p = new soap_proxy::CreamProxy(false);
        m_creamClient.reset( p );
    } catch(soap_proxy::soap_ex& ex) {
        throw eventStatusPoller_ex( ex.what() );
    } 
}

//____________________________________________________________________________
eventStatusPoller::~eventStatusPoller()
{

}

//____________________________________________________________________________
bool eventStatusPoller::getStatus( vector< soap_proxy::JobInfo > &job_status_list)
{
    boost::recursive_mutex::scoped_lock lockPoll( mutexJobStatusPoll );

    bool retval = true;

    // This clear is not needed beacuse the arg is created just before to call
    // this method.
    //job_status_list.clear();

    m_creamClient->clearSoap();

    boost::recursive_mutex::scoped_lock M( jobCache::mutex );

    time_t oldness;// = time(NULL)-jobIt->getLastUpdate();
    time_t threshold;// = iceConfManager::getInstance()->getPollerStatusThresholdTime();
    bool listener_started;

    jobCache::iterator jobIt( m_cache->begin() );

    while( jobIt != m_cache->end() ) {

        oldness = time(NULL)-jobIt->getLastSeen();

	{
            boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
            threshold = iceConfManager::getInstance()->getPollerStatusThresholdTime();
            listener_started = iceConfManager::getInstance()->getStartListener();
	}
                
        if( (oldness <  threshold) && listener_started ) {
            // This job is not old enough. Skip to next job
            ++jobIt;
            continue;
        }

        m_log_dev->infoStream()
            << "eventStatusPoller::getStatus() - "
            << "Sending JobStatus request for Job ["
            << jobIt->getJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;

        vector< string > job_to_query;
        vector< soap_proxy::JobInfo > the_job_status;

        job_to_query.push_back( jobIt->getJobID() );
        try {
            m_creamClient->Authenticate( jobIt->getUserProxyCertificate() );
            m_creamClient->Info(jobIt->getCreamURL().c_str(),
                                job_to_query,
                                vector<string>(),
                                the_job_status,
                                -1,
                                -1 );

            if ( the_job_status.empty() ) {
                // The job is unknown by ICE; remove from the jobCache
                m_log_dev->warnStream()
                    << "Job cream/grid ID=["
                    << jobIt->getJobID()
                    << "]/["
                    << jobIt->getGridJobID()
                    << "] was not found on CREAM; "
                    << "Removing from the job cache"
                    << log4cpp::CategoryStream::ENDLINE;

                jobIt = m_cache->erase( jobIt );
            } else {
                job_status_list.push_back( the_job_status[0] );
                jobIt++;
            }
        } catch (ClassadSyntax_ex& ex) { // FIXME: never thrown?
            // this exception should not be raised because
            // the CreamJob is created from another valid one
            m_log_dev->fatalStream()
                << "eventStatusPoller::getStatus() - "
                << "Fatal error: CreamJob creation failed from a valid one!"
                << " Exception is [" << ex.what() << "]"
                << log4cpp::CategoryStream::ENDLINE;

            exit(1);
        } catch(soap_proxy::auth_ex& ex) {
            m_log_dev->errorStream()
                << "eventStatusPoller::getStatus() - "
                << "Cannot query status job for JobId=["
                << jobIt->getJobID()
                << "]. Exception is [" << ex.what() << "]"
                << log4cpp::CategoryStream::ENDLINE;
            retval = false;
        } catch(soap_proxy::soap_ex& ex) {
            m_log_dev->errorStream()
                << "eventStatusPoller::getStatus() - "
                << "CreamProxy::Status() raised a soap_ex exception ["
                << ex.what() << "] for JobId=["
                << jobIt->getJobID() << "]"
                << log4cpp::CategoryStream::ENDLINE;

            retval = false;
        } catch(cream_api::cream_exceptions::BaseException& ex) {
            m_log_dev->errorStream()
                << "eventStatusPoller::getStatus() - "
                << "CreamProxy::Status() raised a BaseException ["
                << ex.what() << "] for JobId=["
                << jobIt->getJobID() << "]"
                << log4cpp::CategoryStream::ENDLINE;

            retval = false; 
        } catch(cream_api::cream_exceptions::InternalException& ex) {
            m_log_dev->errorStream()
                << "eventStatusPoller::getStatus() - "
                << "CreamProxy::Status() raised an InternalException ["
                << ex.what() << "] for JobId=["
                << jobIt->getJobID() << "]"
                << log4cpp::CategoryStream::ENDLINE;

            retval = false;
        } catch(cream_api::cream_exceptions::DelegationException& ex) {
            m_log_dev->errorStream()
                << "eventStatusPoller::getStatus() - "
                << "CreamProxy::Status() raised an DelegationException ["
                << ex.what() << "] for JobId=["
                << jobIt->getJobID() << "]"
                << log4cpp::CategoryStream::ENDLINE;

            retval = false;
        }
    }
    return retval;
}


//____________________________________________________________________________
void eventStatusPoller::checkJobs( const vector< soap_proxy::JobInfo >& status_list )
{

    /**
     * THIS PROCEDURE CAN BE OPTIMIZED: NOW FOR EACH JOB THERE'S A
     * JobPurge REQUEST SENT TO CREAM; IN FUTURE MULTIPLE JOBIDS
     * TO PURGE ON THE SAME CREAM MUST BE PUT IN A VECTOR AND THE A
     * SINGLE REQUEST WITH THIS VECTOR MUST BE SENT TO THAT CREAM
     */

    vector< string > jobs_to_purge;

    for( vector< soap_proxy::JobInfo >::const_iterator it = status_list.begin(); it != status_list.end(); ++it) {

        soap_proxy::JobInfo the_info( *it );

        vector< soap_proxy::Status > status_changes;
        the_info.getStatusList( status_changes );

        if ( status_changes.empty() ) {
            // If there are no job status changes, 
            continue;
        }
        soap_proxy::Status last_status( status_changes.back() );

        string cid( the_info.getCreamJobID() );

        api::job_statuses::job_status stNum( jobstat::getStatusNum( last_status.getStatusName() ) );
        string exitCode( last_status.getExitCode() );

        bool job_is_done =
            (stNum == cream_api::job_statuses::DONE_FAILED) ||
            (stNum == cream_api::job_statuses::DONE_OK);

        /**
         * We skip those jobs for which CREAM did not returned the
         * exit code yet.
         */
        if( job_is_done && (exitCode=="W") ) {
            m_log_dev->infoStream()
                << "eventStatusPoller::checkJobs() - WILL NOT Purge Job ["
                << cid <<"] because exitCode isn not available yet."
                << log4cpp::CategoryStream::ENDLINE;
            continue;
        }

        switch( stNum ) {

        case api::job_statuses::DONE_FAILED:
        case api::job_statuses::ABORTED:
            
            m_log_dev->infoStream()
                << "eventStatusPoller::checkJobs() - JobID ["
                << cid <<"] is failed or aborted. "
                << "Removing from cache and resubmitting..."
                << log4cpp::CategoryStream::ENDLINE;

            if( m_iceManager ) {
                
                jobCache::iterator jobIt = m_cache->lookupByCreamJobID( cid );
                
                if ( jobIt != m_cache->end() ) {
                    m_iceManager->resubmit_job( *jobIt );
                } else {
                    m_log_dev->errorStream()
                        << "eventStatusPoller::checkJobs() - JobID["
                        << cid << "] disappeared from the cache. "
                        << "Hope this is fine..."
                        << log4cpp::CategoryStream::ENDLINE;
                }
                
            }
            // do NOT break: fall to next case

        case api::job_statuses::DONE_OK:
        case api::job_statuses::CANCELLED:
            m_log_dev->infoStream()
                << "eventStatusPoller::checkJobs() - Scheduling JobID{"
                << cid << "] to be purged"
                << log4cpp::CategoryStream::ENDLINE;
            jobs_to_purge.push_back(cid);
            break;
        default:
            ; // nothing done
        }

    }

    purgeJobs(jobs_to_purge);

}

//----------------------------------------------------------------------------
void eventStatusPoller::updateJobCache( const vector< soap_proxy::JobInfo >& info_list )
{
    for ( vector< soap_proxy::JobInfo >::const_iterator it = info_list.begin(); it != info_list.end(); ++it ) {

        vector< soap_proxy::Status > status_changes;
        it->getStatusList( status_changes );
        
        update_single_job( status_changes );
    }
}

//____________________________________________________________________________
void eventStatusPoller::update_single_job( const vector< soap_proxy::Status >& status_list )
{
    if( status_list.empty() ) {
        return;
    }

    // All status changes must refer to the same job
    string cid( status_list.begin()->getJobID() );

    int count;
    vector< soap_proxy::Status >::const_iterator it;

    // Locks the cache
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
            
    jobCache::iterator jit( m_cache->lookupByCreamJobID( cid ) );

    if ( m_cache->end() == jit ) {
        m_log_dev->errorStream() 
            << "cream_jobid ["<< cid << "] disappeared!"
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }

    for ( it = status_list.begin(), count = 1; it != status_list.end(); ++it, ++count ) {

        glite::ce::cream_client_api::job_statuses::job_status
            stNum = jobstat::getStatusNum( it->getStatusName() );
        
        if ( jit->get_num_logged_status_changes() < count ) {
            
            m_log_dev->infoStream()
                << "eventStatusPoller::updateJobCache() - "
                << "Updating jobcache with "
                << "grid_jobid = [" << jit->getGridJobID() << "] "
                << "cream_jobid = [" << cid << "]"
                << " status = [" << it->getStatusName() << "]"
                << log4cpp::CategoryStream::ENDLINE;
            
            jit->setStatus( stNum );
            jit->set_num_logged_status_changes( count );

            // Log to L&B
            m_lb_logger->logEvent( iceLBEventFactory::mkEvent( *jit ) );
        }
    }
    jit->setLastSeen( time(0) );
    m_cache->put( *jit );
}


//____________________________________________________________________________
void eventStatusPoller::purgeJobs(const vector<string>& jobs_to_purge)
{
    string cid;
    for ( vector<string>::const_iterator it = jobs_to_purge.begin();
          it != jobs_to_purge.end(); ++it ) {

        try {
            boost::recursive_mutex::scoped_lock M( jobCache::mutex );
            //           m_log_dev->infoStream()
            //               << "eventStatusPoller::purgeJobs() - "
            //               << "Fetching Job for ID [" << *it << "]"
            //               << log4cpp::CategoryStream::ENDLINE;

            jobCache::iterator jit = m_cache->lookupByCreamJobID( *it );
            if( jit == m_cache->end() ) {
                m_log_dev->errorStream()
                    << "eventStatusPoller::purgeJobs() - "
                    << "NOT Found in chace job ["
                    << *it << "]"
                    << log4cpp::CategoryStream::ENDLINE;
            } else {
                cid = jit->getJobID();
                m_log_dev->infoStream()
                    << "eventStatusPoller::purgeJobs() - "
                    << "Calling JobPurge for JobId ["
                    << cid << "]"
                    << log4cpp::CategoryStream::ENDLINE;
                // We cannot accumulate more jobs to purge in a vector
                // because we must authenticate different jobs with different
                // user certificates.
                m_creamClient->Authenticate( jit->getUserProxyCertificate());
                vector< string > oneJobToPurge;
                oneJobToPurge.push_back( jit->getJobID() );
                m_creamClient->Purge( jit->getCreamURL().c_str(), oneJobToPurge);
                m_cache->erase( jit );
            }
        } catch (ClassadSyntax_ex& ex) {
            /**
             * this exception should not be raised because
             * the CreamJob is created from another valid one
             */
            m_log_dev->fatalStream() 
                << "eventStatusPoller::purgeJobs() - "
                << "Fatal error: CreamJob creation failed "
                << "copying from a valid one!!!"
                << log4cpp::CategoryStream::ENDLINE;
            exit(1);
        } catch(soap_proxy::auth_ex& ex) {
            m_log_dev->errorStream()
                << "eventStatusPoller::purgeJobs() - "
                << "Cannot purge job: " << ex.what()
                << log4cpp::CategoryStream::ENDLINE;
        } catch(cream_api::cream_exceptions::BaseException& s) {
            m_log_dev->log(log4cpp::Priority::ERROR, s.what());
        } catch(cream_api::cream_exceptions::InternalException& severe) {
            m_log_dev->log(log4cpp::Priority::FATAL, severe.what());
            exit(1);
        } catch(elementNotFound_ex& ex) {
            m_log_dev->errorStream()
                << "eventStatusPoller::purgeJobs() - Cannot remove [" << cid
                << "] from job cache: " << ex.what()
                << log4cpp::CategoryStream::ENDLINE;
        }
    }
}


//____________________________________________________________________________
void eventStatusPoller::body( void )
{
    while( !isStopped() ) {
        vector< soap_proxy::JobInfo > j_status;
        getStatus( j_status );
        try {
            updateJobCache( j_status );
        }
        catch(...) {
            m_log_dev->errorStream()
                << "eventStatusPoller::operator() - "
                << "catched unknown ex"
                << log4cpp::CategoryStream::ENDLINE;
        }
        checkJobs( j_status ); // resubmits aborted/done-failed and/or purges terminated jobs
        /**
         * We don't use boost::thread::sleep because right now
         * (18/11/2005) the documentation says it will be replaced by
         * a more robust mechanism in the future.
         */
        sleep( m_delay );
    }
}

