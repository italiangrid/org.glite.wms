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
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

using namespace glite::wms::ice::util;
using namespace std;

/**
 * Definition of the inner worker thread class
 */
int iceThreadPool::iceThreadPoolWorker::s_threadNum = 0;

iceThreadPool::iceThreadPoolWorker::iceThreadPoolWorker( iceThreadPoolState* st ) :
    iceThread( ),
    m_state( st ),
    m_threadNum( ++s_threadNum )
{

}

iceThreadPool::iceThreadPoolWorker::~iceThreadPoolWorker( )
{

}

void iceThreadPool::iceThreadPoolWorker::body( )
{
    log4cpp::Category* m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() );

    while( !isStopped() ) {
        boost::scoped_ptr< iceAbsCommand > cmd;
        {
            boost::recursive_mutex::scoped_lock L( m_state->m_mutex );
            // The next while could be replaced by 
            while ( m_state->m_requests_queue.empty() ) {
                try {
                    --m_state->m_num_running;
                    m_state->m_queue_empty.wait( L );
                    ++m_state->m_num_running;
                } catch( boost::lock_error& err ) {
                    abort(); // FIXME
                }
            } 
            CREAM_SAFE_LOG(
                           m_log_dev->debugStream()
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
            cmd->execute( glite::wms::ice::Ice::instance( ) );            
        } catch ( glite::wms::ice::iceCommandFatal_ex& ex ) {
            CREAM_SAFE_LOG( 
                           m_log_dev->errorStream()
                           << "Command execution got FATAL exception: "
                           << ex.what()
                           << log4cpp::CategoryStream::ENDLINE
                           );
        } catch ( glite::wms::ice::iceCommandTransient_ex& ex ) {
            CREAM_SAFE_LOG(
                           m_log_dev->errorStream()
                           << "Command execution got TRANSIENT exception: "
                           << ex.what()
                           << log4cpp::CategoryStream::ENDLINE
                           );
            CREAM_SAFE_LOG( m_log_dev->log(log4cpp::Priority::INFO, "Request will be resubmitted" ) );
            
        }

        CREAM_SAFE_LOG(
                       m_log_dev->debugStream()
                       << "iceThreadPoolWorker::body() - "
                       << "Worker Thread #" << m_threadNum 
                       << " finished processing request"
                       << " (Currently " << m_state->m_num_running
                       << " threads are running)" 
                       << log4cpp::CategoryStream::ENDLINE
                       );


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
    m_state->m_num_running = 10; // FIXME: To be moved in iceThreadPoolState constructor?
    for ( int i=0; i<10; i++ ) { // FIXME: hardcoded default of 10 threads
        boost::shared_ptr< util::iceThread > m_ptr_thread = 
            boost::shared_ptr< util::iceThread >( new iceThreadPoolWorker( m_state.get() ) );
        boost::thread* thr;
        try {
            thr = new boost::thread(boost::bind(&util::iceThread::operator(), m_ptr_thread ) );
        } catch( boost::thread_resource_error& ex ) {
            abort(); // FIXME
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
