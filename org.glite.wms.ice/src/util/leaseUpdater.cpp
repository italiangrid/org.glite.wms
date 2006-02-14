#include "leaseUpdater.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/BaseException.h"
#include "glite/ce/cream-client-api-c/InternalException.h"
#include "glite/ce/cream-client-api-c/DelegationException.h"
//#include "abs-ice-core.h"
#include "iceConfManager.h"
#include <boost/thread/thread.hpp>

#include <vector>
#include <string>

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::job_statuses;
using namespace glite::ce::cream_client_api::util;
using namespace std;

leaseUpdater::leaseUpdater( ) :
    iceThread( "ICE Lease Updater" ),
    delay( 60 ), // FIXME: hardcoded, should be made user-configurable
    log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    cache( jobCache::getInstance() ),
    creamClient( 0 )
{
    try {
        creamClient = new soap_proxy::CreamProxy( false );
    } catch(soap_proxy::soap_ex& ex) {
        // FIXME: what to do??
    } 
}

leaseUpdater::~leaseUpdater( )
{
    if ( creamClient ) 
        delete creamClient;
}

vector<string> leaseUpdater::getJobsToUpdate( void )
{
    creamClient->clearSoap();
    vector<string> jobs_to_update;

    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    cache->getActiveCreamJobIDs(jobs_to_update); // FIXME: does not throw anything?
    
    return jobs_to_update;
}

void leaseUpdater::body( void )
{
    while ( !isStopped() ) {
        log_dev->infoStream()
            << "leastUpdater::body(): new iteration"
            << log4cpp::CategoryStream::ENDLINE;
        
        // updateLease( );
        sleep( delay );
    }
}
