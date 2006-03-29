#include "leaseUpdater.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/BaseException.h"
#include "glite/ce/cream-client-api-c/InternalException.h"
#include "glite/ce/cream-client-api-c/DelegationException.h"
#include "iceConfManager.h"
#include <boost/thread/thread.hpp>

#include <vector>
#include <string>

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::job_statuses;
using namespace glite::ce::cream_client_api::util;
using namespace std;

namespace {
    /**
     * Utility function to convert a time_t value into a string.
     * This function uses the thread-safe ctime_r function.
     *
     * @param tval the timestamp to convert
     * @return the string representation of the argument
     */
    string time_t_to_string( time_t tval ) 
    {
        char buf[26]; // ctime_r wants a buffer of at least 26 bytes
        ctime_r( &tval, buf );
        return string( buf );
    }
}

leaseUpdater::leaseUpdater( ) :
    iceThread( "ICE Lease Updater" ),
    log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    cache( jobCache::getInstance() ),
    creamClient( 0 ),
    threshold( iceConfManager::getInstance()->getLeaseThresholdTime() * 60 ),
    delay( 1*60 ), // lease updater wakes up every minute
    delta( iceConfManager::getInstance()->getLeaseDeltaTime() * 60 )
{
    try {        
        soap_proxy::CreamProxy* p( new soap_proxy::CreamProxy( false ) );
        creamClient.reset( p ); // boost::scoped_ptr<>.reset() requires its argument not to throw anything, IIC
    } catch(soap_proxy::soap_ex& ex) {
        // FIXME: what to do??
    } 
}

leaseUpdater::~leaseUpdater( )
{

}

void leaseUpdater::update_lease( void )
{
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    jobCache::iterator it = cache->begin();
    while ( it != cache->end() ) {
        if ( it->getEndLease() < time(0) ) {
            // Purge expired job
            it = cache->erase( it );
        } else {
            if ( it->is_active() && 
                 ( it->getEndLease() - time(0) < threshold ) ) 
                update_lease_for_job( *it );
            it++;
        }
    }
}

void leaseUpdater::update_lease_for_job( CreamJob& j )
{
    creamClient->clearSoap();

    map< string, time_t > newLease;
    vector< string > jobids;
    
    jobids.push_back( j.getJobID() );

    // Renew the lease
    try {
        creamClient->Authenticate( j.getUserProxyCertificate() );
        creamClient->Lease( j.getCreamURL().c_str(), jobids, delta, newLease );
    } catch ( soap_proxy::soap_ex& ex ) {
        log_dev->errorStream()
            << "leaseUpdater::update_lease_for_job() returned an exception: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        // FIXME: what to do?
    }

    if ( newLease.find( j.getJobID() ) != newLease.end() ) {
        
        // The lease for this job has been updated
        // So update the job cache as well...

        log_dev->infoStream()
            << "leaseUpdater:: updating lease for cream jobid=["
            << j.getJobID()
            << "]; old lease ends " << time_t_to_string( j.getEndLease() )
            << " new lease ends " << time_t_to_string( newLease[ j.getJobID() ] )
            << log4cpp::CategoryStream::ENDLINE;
        
        j.setEndLease( newLease[ j.getJobID() ] );
        cache->put( j ); // Be Careful!! This should not invalidate any iterator on the job cache, as the job j is guaranteed (in this case) to be already in the cache.            
    } else {
        // there was an error updating the lease
        log_dev->errorStream()
            << "leaseUpdater:: unable to update lease for cream jobid=["
            << j.getJobID()
            << "]; old lease ends " << time_t_to_string( j.getEndLease() )
            << log4cpp::CategoryStream::ENDLINE;
    }
}

void leaseUpdater::body( void )
{
    while ( !isStopped() ) {
        log_dev->infoStream()
            << "leastUpdater::body() - new iteration"
            << log4cpp::CategoryStream::ENDLINE;

        update_lease( );

        sleep( delay );
    }
}
