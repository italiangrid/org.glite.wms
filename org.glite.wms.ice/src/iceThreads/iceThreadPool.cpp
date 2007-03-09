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
 * ICE thread pool class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceThreadPool.h"
#include "iceThreadPoolState.h"
#include "iceAbsCommand.h"
#include "ice-core.h"
#include "iceConfManager.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include "CreamProxyFactory.h"

#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>

namespace conf_ns=glite::wms::ice::util;
using namespace glite::wms::ice::util;
using namespace std;

/**
 * Definition of the inner worker thread class
 */
int iceThreadPool::iceThreadPoolWorker::s_threadNum = 0;

iceThreadPool::iceThreadPoolWorker::iceThreadPoolWorker( iceThreadPoolState* st ) :
    iceThread( boost::str( boost::format( "iceThreadPoolWorker(%1%)" ) % s_threadNum ) ),
    m_state( st ),
    m_threadNum( s_threadNum++ )
{
  log4cpp::Category* log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() );
  glite::ce::cream_client_api::soap_proxy::CreamProxy *proxy = CreamProxyFactory::makeCreamProxy(true);
  if(!proxy) {
    CREAM_SAFE_LOG( log_dev->fatalStream()
                    << "iceThreadPoolWorker::CTOR() - "
                    << "Failed CreamProxy creation. Abort!"
                    << log4cpp::CategoryStream::ENDLINE
                   );
    abort();
  }       
  //m_proxy.reset( proxy );
}

iceThreadPool::iceThreadPoolWorker::~iceThreadPoolWorker( )
{

}

void iceThreadPool::iceThreadPoolWorker::body( )
{
    log4cpp::Category* log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() );    

    while( !isStopped() ) {
        boost::scoped_ptr< iceAbsCommand > cmd;
        {
            boost::recursive_mutex::scoped_lock L( m_state->m_mutex );
            
            while ( m_state->m_requests_queue.end() == get_first_request() ) {
                try {
                    --m_state->m_num_running;

                    /* CREAM_SAFE_LOG(
                                   log_dev->debugStream()
                                   << "iceThreadPoolWorker::body() - "
                                   << "Worker Thread #" << m_threadNum 
                                   << " is waiting"
                                   << " (Number of running threads now: " 
                                   << m_state->m_num_running
                                   << ")"
                                   << log4cpp::CategoryStream::ENDLINE
                                   );        */

                    m_state->m_no_requests_available.wait( L );
                    ++m_state->m_num_running;
                } catch( boost::lock_error& err ) {
                    CREAM_SAFE_LOG( log_dev->fatalStream()
                                    << "iceThreadPoolWorker::body() - "
                                    << "Worker Thread #" << m_threadNum 
                                    << " raised the following lock_error "
                                    << "xception while waiting on the "
                                    << "command queue: " << err.what()
                                    << ". Giving up."
                                    << log4cpp::CategoryStream::ENDLINE
                                    );
                    abort();
                }
            } 
            CREAM_SAFE_LOG(
                           log_dev->debugStream()
                           << "iceThreadPoolWorker::body() - "
                           << "Worker Thread #" << m_threadNum 
                           << " started processing new request"
                           << " (Currently " << m_state->m_num_running
                           << " threads are running)" 
                           << log4cpp::CategoryStream::ENDLINE
                           );
            // Remove one request from the queue
            list< iceAbsCommand* >::iterator req_it = get_first_request( );
            assert( req_it != m_state->m_requests_queue.end() );
            iceAbsCommand* cmd_ptr = *req_it;
            cmd.reset( cmd_ptr );
            m_state->m_requests_queue.erase( req_it );
            m_state->m_pending_jobs.insert( cmd->get_grid_job_id() );
        } // releases lock

        try {
            cmd->execute( );
        } catch ( glite::wms::ice::iceCommandFatal_ex& ex ) {
            CREAM_SAFE_LOG( 
                           log_dev->errorStream()
                           << "Command execution got FATAL exception: "
                           << ex.what()
                           << log4cpp::CategoryStream::ENDLINE
                           );
        } catch ( glite::wms::ice::iceCommandTransient_ex& ex ) {
            CREAM_SAFE_LOG(
                           log_dev->errorStream()
                           << "Command execution got TRANSIENT exception: "
                           << ex.what()
                           << log4cpp::CategoryStream::ENDLINE
                           );
        } catch( exception& ex ) {
            CREAM_SAFE_LOG(
                           log_dev->errorStream()
                           << "Command execution got exception: "
                           << ex.what()
                           << log4cpp::CategoryStream::ENDLINE
                           );
        } catch( ... ) {
            CREAM_SAFE_LOG(
                           log_dev->errorStream()
                           << "Command execution got unknown exception"
                           << log4cpp::CategoryStream::ENDLINE
                           );
        }

        // Now, wake up another worker thread (just in case someone is
        // waiting for us to complede)
        boost::recursive_mutex::scoped_lock L( m_state->m_mutex );
        m_state->m_pending_jobs.erase( cmd->get_grid_job_id() );
        m_state->m_no_requests_available.notify_all();

    }
}

list< glite::wms::ice::iceAbsCommand* >::iterator iceThreadPool::iceThreadPoolWorker::get_first_request( void )
{
    list< glite::wms::ice::iceAbsCommand* >::iterator it = m_state->m_requests_queue.begin();
    while ( ( it != m_state->m_requests_queue.end() ) &&
            ( m_state->m_pending_jobs.end() != 
              m_state->m_pending_jobs.find( (*it)->get_grid_job_id() ) ) ) {
        ++it;
    }
    return it;
}


/**
 *
 * Implementation of the iceThreadPool class
 *
 */
iceThreadPool* iceThreadPool::s_instance = 0;

iceThreadPool::iceThreadPool( ) :
    m_state( new iceThreadPoolState() ),
    m_all_threads( ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{
    int n_threads = m_state->m_num_running = ( conf_ns::iceConfManager::getInstance()->getConfiguration()->ice()->max_ice_threads() < 1 ? 1 : conf_ns::iceConfManager::getInstance()->getConfiguration()->ice()->max_ice_threads() ); // FIXME: this check should be moved inside the iceConfManager
    CREAM_SAFE_LOG( m_log_dev->infoStream()
                    << "iceThreadPool::iceThreadPool() - "
                    << "Creating " << m_state->m_num_running 
                    << " worker threads"
                    << log4cpp::CategoryStream::ENDLINE
                    );            
    for ( int i=0; i<n_threads; i++ ) {
        boost::shared_ptr< util::iceThread > m_ptr_thread( new iceThreadPoolWorker( m_state.get() ) );
        boost::thread* thr;
        try {
            thr = new boost::thread(boost::bind(&util::iceThread::operator(), m_ptr_thread ) );
        } catch( boost::thread_resource_error& ex ) {
            CREAM_SAFE_LOG( m_log_dev->fatalStream()
                            << "iceThreadPool::iceThreadPool() -"
                            << "Unable to create worker thread. Giving up."
                            << log4cpp::CategoryStream::ENDLINE
                            );            
            abort();
        }
        m_all_threads.add_thread( thr );
    }
}

iceThreadPool::~iceThreadPool( )
{

}

iceThreadPool* iceThreadPool::instance( )
{
    if ( 0 == s_instance ) {
        s_instance = new iceThreadPool( );
    }
    return s_instance;
}

void iceThreadPool::add_request( glite::wms::ice::iceAbsCommand* req )
{
    boost::recursive_mutex::scoped_lock L( m_state->m_mutex );
    m_state->m_requests_queue.push_back( req );
    m_state->m_no_requests_available.notify_all(); // wake up one worker thread
}
