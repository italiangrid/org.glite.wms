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

const time_t leaseUpdater::threshold = 60*30; // FIXME: hardcoded default 30 min
const time_t leaseUpdater::delay = 60*2; // FIXME: hardcoded default 2 min

leaseUpdater::leaseUpdater( ) :
    iceThread( "ICE Lease Updater" ),
    log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    cache( jobCache::getInstance() ),
    creamClient( 0 )
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

vector<CreamJob> leaseUpdater::getJobsToUpdate( void )
{
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    vector< CreamJob > jobs_to_update;

    for ( jobCache::const_iterator it = cache->begin(); it != cache->end(); it++ ) {
        if ( it->getEndLease() - time(0) > threshold ) {
            jobs_to_update.push_back( *it );
        }
    }

    return jobs_to_update;
}

void leaseUpdater::updateJobs( vector< CreamJob > jobs )
{
    const time_t delta = 60*60; // FIXME: hardcoded default of lease renewal: 1 hour
    for( vector< CreamJob >::iterator it = jobs.begin(); it != jobs.end(); it++ ) {
        time_t newEndLease( it->getEndLease() + delta );

        log_dev->infoStream()
            << "leaseUpdater:: updating lease for cream jobid=["
            << it->getJobID()
            << "]; old lease ends " << it->getEndLease()
            << " new lease ends " << newEndLease 
            << log4cpp::CategoryStream::ENDLINE;

        it->setEndLease( newEndLease );
        // FIXME: Here we actually should update the lease using the appropriate interface method. If lease renewal fails, check if the lease expired. If so, the job is lost and should be removed from the cache
        cache->put( *it );
    }    
}

void leaseUpdater::body( void )
{
    vector< CreamJob > _jobs;
    while ( !isStopped() ) {
        log_dev->infoStream()
            << "leastUpdater::body(): new iteration"
            << log4cpp::CategoryStream::ENDLINE;

        _jobs = getJobsToUpdate( );
        updateJobs( _jobs );

        sleep( delay );
    }
}
