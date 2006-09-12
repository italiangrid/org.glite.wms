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
  m_proxy.reset( proxy );
}

iceThreadPool::iceThreadPoolWorker::~iceThreadPoolWorker( )
{

}

void iceThreadPool::iceThreadPoolWorker::body( )
{
//    log4cpp::Category* m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() );
    log4cpp::Category* log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() );    
    while( !isStopped() ) {
        boost::scoped_ptr< iceAbsCommand > cmd;
        {
            boost::recursive_mutex::scoped_lock L( m_state->m_mutex );
            // The next while could be replaced by 
            while ( m_state->m_requests_queue.empty() ) {
                try {
                    --m_state->m_num_running;

                    CREAM_SAFE_LOG(
                                   log_dev->debugStream()
                                   << "iceThreadPoolWorker::body() - "
                                   << "Worker Thread #" << m_threadNum 
                                   << " is waiting"
                                   << " (Number of running threads now: " 
                                   << m_state->m_num_running
                                   << ")"
                                   << log4cpp::CategoryStream::ENDLINE
                                   );        

                    m_state->m_queue_empty.wait( L );
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
            iceAbsCommand* cmd_ptr = m_state->m_requests_queue.front();
            cmd.reset( cmd_ptr );
            m_state->m_requests_queue.pop_front();
        } // releases lock

        try {
            cmd->execute( glite::wms::ice::Ice::instance( ), m_proxy.get() );
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
            CREAM_SAFE_LOG( log_dev->log(log4cpp::Priority::INFO, "Request will be resubmitted" ) );            
        }
    }
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
    m_state->m_queue_empty.notify_one(); // wake up one worker thread
}
