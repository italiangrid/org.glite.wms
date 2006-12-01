/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *    http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * ICE status poller thread
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// ICE includes
#include "ice-core.h"
#include "iceConfManager.h"
#include "eventStatusPoller.h"
#include "iceCommandStatusPoller.h"
#include "CreamProxyFactory.h"
//#include "CreamProxyMethod.h"

// other glite includes
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
//#include "glite/ce/cream-client-api-c/CEUrl.h"
//#include "glite/ce/cream-client-api-c/CreamProxy.h"
//#include "glite/ce/cream-client-api-c/soap_ex.h"
//#include "glite/ce/cream-client-api-c/BaseException.h"
//#include "glite/ce/cream-client-api-c/InternalException.h"
//#include "glite/ce/cream-client-api-c/DelegationException.h"
//#include "glite/wms/common/utilities/scope_guard.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

// boost includes
#include <boost/thread/thread.hpp>

#include <boost/functional.hpp>

// system includes
#include <vector>
#include <map>

#define STATUS_POLL_RETRY_COUNT 4

namespace cream_api  = glite::ce::cream_client_api;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;
namespace jobstat    = glite::ce::cream_client_api::job_statuses;
namespace cream_util = glite::ce::cream_client_api::util;
namespace wms_utils  = glite::wms::common::utilities;

using namespace glite::wms::ice::util;
using namespace std;

typedef vector<soap_proxy::Status>::iterator JobStatusIt;
typedef vector<string>::iterator vstrIt;
typedef vector<string>::const_iterator cvstrIt;

boost::recursive_mutex eventStatusPoller::mutexJobStatusPoll;

namespace { // anonymous namespace for local definitions

    /**
     * Handle a job which was not accessible through the
     * "jobInfo" method. We try a number of times before giving
     * up and removing the job from the jobCache.
     */
    class handle_job {
    protected:
        const string m_cream_job_id;
    public:

        /**
         * @param job the Job which ICE was unable to access. If the job
         * is not found in the job cache, this method does nothing
         */
        handle_job( const string& cream_job_id ) :
            m_cream_job_id( cream_job_id )
        { };

        void operator()( void ) {

            boost::recursive_mutex::scoped_lock M( jobCache::mutex );
            jobCache* cache( jobCache::getInstance() );
            log4cpp::Category* log_dev( cream_util::creamApiLogger::instance()->getLogger());
            jobCache::iterator jit( cache->lookupByCreamJobID( m_cream_job_id ) );

            if ( cache->end() == jit ) {
                return;
            }
            
            jit->incStatusPollRetryCount( );
            if( jit->getStatusPollRetryCount() < STATUS_POLL_RETRY_COUNT ) {
                
                CREAM_SAFE_LOG(log_dev->warnStream()
                               << "eventStatusPoller::handle_unreachable_job() - "
                               << "cream_job_id [" << jit->getCreamJobID()
                               << "], grid_job_id [" << jit->getGridJobID()
                               << "] was not found on CREAM; Retrying later..."
                               << log4cpp::CategoryStream::ENDLINE);
                
                cache->put( *jit );
                
            } else {
                
                CREAM_SAFE_LOG(log_dev->errorStream()
                               << "eventStatusPoller::handle_unreachable_job() - "
                               << "cream_job_id [" << jit->getCreamJobID()
                               << "], grid_job_id [" << jit->getGridJobID()
                               << "] was not found on CREAM after " 
                               << STATUS_POLL_RETRY_COUNT
                               << "retries; Removing from the job cache"
                               << log4cpp::CategoryStream::ENDLINE);
                cache->erase( jit );                
            }            
        }
    };

    /**
     * The following method will be used when the scenario with many
     * jobs and several CREAM urls will happen. Many jobs can be
     * reorganized in order to group the maximum number of job related
     * to the same CREAM Url AND the same proxy certificate, in order
     * to reduce the number of authentications on the same CREAM host
     */
    void organizeJobs( const vector<CreamJob> & vec, map< string, map<string, vector<string> > >& target)
    {
        for(vector<CreamJob>::const_iterator cit = vec.begin(); cit != vec.end(); ++cit) {
            ( target[cit->getEndpoint()] )[ cit->getUserProxyCertificate() ].push_back( cit->getCreamJobID() );
        }
    }


} // end anonymous namespace

//____________________________________________________________________________
eventStatusPoller::eventStatusPoller( glite::wms::ice::Ice* manager, int d )
  throw(eventStatusPoller_ex&, ConfigurationManager_ex&)
  : iceThread( "event status poller" ),
    m_delay( d ),
    m_iceManager( manager ),
    //m_creamClient( CreamProxyFactory::makeCreamProxy( false ) ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger())
    //m_lb_logger( iceLBLogger::instance() ),
    //m_cache( jobCache::getInstance() ),
    //m_threshold( iceConfManager::getInstance()->getConfiguration()->ice()->poller_status_threshold_time() ) // may raise ConfigurationManager_ex
{

}

//____________________________________________________________________________
eventStatusPoller::~eventStatusPoller()
{

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

        //list< CreamJob > j_list( get_jobs_to_poll() );

	iceCommandStatusPoller cmd( m_iceManager, CreamProxyFactory::makeCreamProxy( false ));
	cmd.execute();

//         list< soap_proxy::JobInfo > j_status( check_jobs( j_list ) );
//         try {
//             updateJobCache( j_status );
//         } catch(exception& ex) {
// 	  CREAM_SAFE_LOG(m_log_dev->errorStream()
// 			 << "eventStatusPoller::body() - "
// 			 << "catched std::exception: "
// 			 << ex.what()
// 			 << log4cpp::CategoryStream::ENDLINE);
// 	} catch(...) {
// 	  CREAM_SAFE_LOG(m_log_dev->errorStream()
// 			 << "eventStatusPoller::body() - "
// 			 << "catched unknown exception"
// 			 << log4cpp::CategoryStream::ENDLINE);
//         }

    }
}

