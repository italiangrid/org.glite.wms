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
 * ICE lease updater
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "leaseUpdater.h"
#include "jobCache.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "CreamProxyFactory.h"
#include "CreamProxyMethod.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/BaseException.h"
#include "glite/ce/cream-client-api-c/InternalException.h"
#include "glite/ce/cream-client-api-c/DelegationException.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <boost/thread/thread.hpp>
#include <boost/functional.hpp>
#include <vector>
#include <string>
#include <list>

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::job_statuses;
using namespace glite::ce::cream_client_api::util;
using namespace std;

//____________________________________________________________________________
leaseUpdater::leaseUpdater( ) :
    iceThread( "ICE Lease Updater" ),
    m_threshold( iceConfManager::getInstance()->getConfiguration()->ice()->lease_threshold_time() ),
    m_delta( iceConfManager::getInstance()->getConfiguration()->ice()->lease_delta_time() ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    m_cache( jobCache::getInstance() ),
    m_creamClient( CreamProxyFactory::makeCreamProxy( false ) )
{
    double delta_time_for_lease = ((double)iceConfManager::getInstance()->getConfiguration()->ice()->lease_threshold_time())/4.0;
    m_delay = (time_t)(delta_time_for_lease);
}

//____________________________________________________________________________
leaseUpdater::~leaseUpdater( )
{

}

//____________________________________________________________________________
void leaseUpdater::update_lease( void )
{
    typedef list<CreamJob> cj_list_t;

    cj_list_t jobs_to_check;

    { // acquire lock on job cache
        boost::recursive_mutex::scoped_lock M( jobCache::mutex );
        for(jobCache::iterator jit = m_cache->begin(); jit != m_cache->end(); ++jit) {
            jobs_to_check.push_back( *jit );
        }
    } // releases lock on job cache
    
    for ( cj_list_t::iterator it = jobs_to_check.begin(); it != jobs_to_check.end(); ) {
        if ( it->getEndLease() <= time(0) ) {
            // Remove expired job from cache
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "leaseUpdater::update_lease() - "
                           << "Removing from cache lease-expired job [" 
                           << it->getCreamJobID() << "]"
                           << log4cpp::CategoryStream::ENDLINE);

            boost::recursive_mutex::scoped_lock M( jobCache::mutex );
            jobCache::iterator tmp( m_cache->lookupByGridJobID( it->getGridJobID() ) );
            it = m_cache->erase( tmp );              
        } else {
            if( ! it->getCreamJobID().empty() ) {
                CREAM_SAFE_LOG(m_log_dev->infoStream() 
                               << "leaseUpdater::update_lease() - "
                               << "Checking LEASE for Job ["
                               << it->getCreamJobID() << "] - " 
                               << " isActive=" << it->is_active()
                               << " - remaining=" << (it->getEndLease()-time(0))
                               << " - threshold=" << m_threshold
                               << log4cpp::CategoryStream::ENDLINE);
                
                if ( it->is_active() && 
                     ( it->getEndLease() - time(0) <= m_threshold ) ) {
                    update_lease_for_job( *it );
                }
            }
            ++it;
        }
    } // end for
}

//____________________________________________________________________________
void leaseUpdater::update_lease_for_job( CreamJob j )
{
    map< string, time_t > newLease;
    vector< string > jobids;
    
    jobids.push_back( j.getCreamJobID() );

    // Renew the lease
    
    CREAM_SAFE_LOG(m_log_dev->infoStream()
		   << "leaseUpdater::update_lease_for_job() - "
		   << "updating lease for cream jobid=["
		   << j.getCreamJobID()
		   << "] m_delta=" << m_delta
		   << log4cpp::CategoryStream::ENDLINE);
    
    try {

        m_creamClient->Authenticate( j.getUserProxyCertificate() );
        util::CreamProxy_Lease( j.getCreamURL(), jobids, m_delta, newLease ).execute( m_creamClient.get(), 3 );

    } catch(cream_exceptions::JobUnknownException& ex) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "leaseUpdater::update_lease_for_job() - "
                       << "CREAM doesn't know the current Job ["
                       << j.getCreamJobID()<<"]. Removing from cache."
                       << log4cpp::CategoryStream::ENDLINE);
        boost::recursive_mutex::scoped_lock M( jobCache::mutex );
        jobCache::iterator tmp = m_cache->lookupByCreamJobID( j.getCreamJobID() );
        m_cache->erase( tmp );
        return;
      
    } catch(cream_exceptions::BaseException& ex) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "leaseUpdater::update_lease_for_job() - "
                       << "Error updating lease for job ["
                       << j.getCreamJobID()<<"]: "<<ex.what()
                       << log4cpp::CategoryStream::ENDLINE);
        return;      
    } catch ( soap_proxy::soap_ex& ex ) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "leaseUpdater::update_lease_for_job() - "
                       << "Error updating lease for job ["
                       << j.getCreamJobID()<<"]: "<<ex.what()
                       << log4cpp::CategoryStream::ENDLINE);
        return;
    }

    if ( newLease.find( j.getCreamJobID() ) != newLease.end() ) {
        
        // The lease for this job has been updated
        // So update the job cache as well...

        CREAM_SAFE_LOG(m_log_dev->infoStream()
                       << "leaseUpdater::update_lease_for_job() - "
                       << "updating lease for cream jobid=["
                       << j.getCreamJobID()
                       << "]; old lease ends " << time_t_to_string( j.getEndLease() )
                       << " new lease ends " << time_t_to_string( newLease[ j.getCreamJobID() ] )
                       << log4cpp::CategoryStream::ENDLINE);
        
        boost::recursive_mutex::scoped_lock M( jobCache::mutex );
      
        // re-read the current job from the cache in order
        // to get modifications (if any) made by other threads
        jobCache::iterator tmpJob = m_cache->lookupByCreamJobID( j.getCreamJobID() );
        if(tmpJob != m_cache->end() ) {
            tmpJob->setEndLease( newLease[ j.getCreamJobID() ] );
            m_cache->put( *tmpJob ); // Be Careful!! This should not invalidate any iterator on the job cache, as the job j is guaranteed (in this case) to be already in the cache.            
        }

    } else {
        // there was an error updating the lease
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "leaseUpdater::update_lease_for_job() - "
                       << "unable to update lease for cream jobid=["
                       << j.getCreamJobID()
                       << "]; old lease ends " << time_t_to_string( j.getEndLease() )
                       << log4cpp::CategoryStream::ENDLINE);
    }
}

//______________________________________________________________________________
void leaseUpdater::body( void )
{
    while ( !isStopped() ) {
        CREAM_SAFE_LOG(m_log_dev->infoStream()
                       << "leaseUpdater::body() - new iteration"
                       << log4cpp::CategoryStream::ENDLINE);
        update_lease( );
        sleep( m_delay );
    }
}
