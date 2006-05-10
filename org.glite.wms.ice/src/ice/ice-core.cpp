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
 * ICE core class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "ice-core.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include "subscriptionManager.h"
#include "subscriptionCache.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "iceThread.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <exception>
#include <unistd.h>
#include <cstdlib>

namespace wmsutils_ns = glite::wms::common::utilities;
using namespace std;

typedef vector<string>::iterator vstrIt;

namespace glite {
namespace wms {
namespace ice {

//
// Begin Inner class definitions
//
    Ice::IceThreadHelper::IceThreadHelper( const std::string& name ) :
        m_name( name ),
        m_thread( 0 ),
        m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
    {

    }

    Ice::IceThreadHelper::~IceThreadHelper( ) 
    {
        stop( );
        delete m_thread;
    }

    void Ice::IceThreadHelper::start( util::iceThread* obj ) throw( iceInit_ex& )
    {
        m_ptr_thread = boost::shared_ptr< util::iceThread >( obj );
        try {
            m_thread = new boost::thread(boost::bind(&util::iceThread::operator(), m_ptr_thread) );
        } catch( boost::thread_resource_error& ex ) {
            throw iceInit_ex( ex.what() );
        }
    }

    void Ice::IceThreadHelper::stop( void )
    {
        if( m_thread && m_ptr_thread->isRunning() ) {
            m_log_dev->infoStream()
                << "Waiting for thread " << m_name << " termination..."
                << log4cpp::CategoryStream::ENDLINE;
            m_ptr_thread->stop();
            m_thread->join();
            m_log_dev->infoStream()
                << "Thread " << m_name << " finished"
                << log4cpp::CategoryStream::ENDLINE;
        }
    }

//
// End inner class definitions
//

//____________________________________________________________________________
Ice::Ice( const string& NS_FL, const string& WM_FL ) throw(iceInit_ex&) : 
    m_listener_thread( "Event Status Listener" ),
    m_poller_thread( "Event Status Poller" ),
    m_updater_thread( "Subscription Updater" ),
    m_lease_updater_thread( "Lease Updater" ),
    m_proxy_renewer_thread( "Proxy Renewer" ),
    m_ns_filelist( NS_FL ),
    m_fle( WM_FL.c_str() ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{
    m_log_dev->log(log4cpp::Priority::INFO,
                   "Ice::CTOR() - Initializing File Extractor object...");
    try {
        m_flns.open( NS_FL.c_str() );
    }
    catch( std::exception& ex ) {
        throw iceInit_ex( ex.what() );
    } catch( ... ) {
        m_log_dev->log(log4cpp::Priority::FATAL,
                       "Ice::CTOR() - Catched unknown exception");
        exit( 1 );
    }

    boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );

    util::iceConfManager* confMgr( util::iceConfManager::getInstance() );
    bool tmp_start_listener = confMgr->getStartListener();
    
    // this could be useful if some other module need to quickly know if the 
    // subscription updater is running or not
    if( !tmp_start_listener )
      confMgr->setStartSubscriptionUpdater( false );

    if( tmp_start_listener ) {
        /**
         * The listener and the iceCommandSubmit need to subscribe to
         * CEMon in order to make ICE able to receive job status
         * notifications.  So now as preliminary operation it's the
         * case to check that the subscriptionManager singleton can be
         * created without problems.
         *
         * The subscriptionManager initialization also setups
         * authentication.
         */
        boost::recursive_mutex::scoped_lock M( util::subscriptionManager::mutex );
        util::subscriptionManager::getInstance();
        if( !util::subscriptionManager::getInstance()->isValid() ) {
            m_log_dev->errorStream() 
                << "Ice::CTOR() - "
                << "Fatal error creating the subscriptionManager instance. Will not start listener."
                << log4cpp::CategoryStream::ENDLINE;
            confMgr->setStartListener( false );
        }
        /**
         * subscriptionCache is used to retrieve the list of cemon we're
         * subscribed. If it's creation failed, it is not the case (at 0-order)
         * to use the listener...
         *
         */
        {
            boost::recursive_mutex::scoped_lock M( util::subscriptionCache::mutex );
            if( util::subscriptionCache::getInstance() == NULL ) {
                m_log_dev->errorStream() 
                    << "Ice::CTOR() - "
                    << "Fatal error creating the subscriptionCache instance. "
                    << "Will not start listener."
                    << log4cpp::CategoryStream::ENDLINE;
                confMgr->setStartListener( false );
            }
        }
    }
}

//____________________________________________________________________________
Ice::~Ice( )
{

}

//____________________________________________________________________________
void Ice::startListener( int listenPort )
{
    boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
    util::iceConfManager* confMgr( util::iceConfManager::getInstance() );

    m_log_dev->infoStream()
        << "Ice::startListener() - Creating a CEMon listener object..."
        << log4cpp::CategoryStream::ENDLINE;

    util::eventStatusListener* listener = new util::eventStatusListener(listenPort, confMgr->getHostProxyFile());
    
    if( !listener->isOK() ) {
        
        m_log_dev->errorStream()
            << "CEMon listener creation went wrong. Won't start it."
            << log4cpp::CategoryStream::ENDLINE;
        
        // this must be set because other pieces of code
        // have a behaviour that depends on the listener is running or not
        confMgr->setStartListener( false );
        return;
    }
    while( !listener->bind() ) {
        m_log_dev->errorStream()
            << "Ice::startListener() - Bind error: "
            << listener->getErrorMessage()
            << " - error code="
            << listener->getErrorCode()
            << "Retrying in 5 seconds..."
            << log4cpp::CategoryStream::ENDLINE;
        sleep(5);
    }
    
    m_listener_thread.start( listener );
    
    //-----------------now is time to start subUpdater-------------------------
    bool tmp_start_sub_updater = confMgr->getStartSubscriptionUpdater();
    
    if( tmp_start_sub_updater ) {
        util::subscriptionUpdater* subs_updater = new util::subscriptionUpdater( confMgr->getHostProxyFile());      
        m_updater_thread.start( subs_updater );
    }
}

//____________________________________________________________________________
void Ice::startPoller( int poller_delay )
{
    util::eventStatusPoller* poller = new util::eventStatusPoller( this, poller_delay );
    m_poller_thread.start( poller );
}

//----------------------------------------------------------------------------
void Ice::startLeaseUpdater( void ) 
{
    util::leaseUpdater* lease_updater = new util::leaseUpdater( );
    m_lease_updater_thread.start( lease_updater );
}

//-----------------------------------------------------------------------------
void Ice::startProxyRenewer( void ) 
{
    util::proxyRenewal* proxy_renewer = new util::proxyRenewal( );
    m_proxy_renewer_thread.start( proxy_renewer );
}

//____________________________________________________________________________
void Ice::clearRequests() 
{
    m_requests.clear();
}

//____________________________________________________________________________
void Ice::getNextRequests(vector<string>& ops) 
{
  try { 
      m_requests = m_fle.get_all_available();
  }
  catch( exception& ex ) {
      m_log_dev->log(log4cpp::Priority::FATAL, ex.what());
      exit(1);
  }
  for ( unsigned j=0; j < m_requests.size(); j++ ) {
      ops.push_back(*m_requests[j]);
  }
}

//____________________________________________________________________________
void Ice::removeRequest( unsigned int reqNum) 
{
    m_fle.erase(m_requests[reqNum]);
}

//____________________________________________________________________________
void Ice::ungetRequest( unsigned int reqNum)
{
    wmsutils_ns::FileListMutex mx(m_flns);
    wmsutils_ns::FileListLock  lock(mx);

    string toResubmit = *m_requests[reqNum];
    
    boost::replace_first( toResubmit, "jobsubmit", "jobresubmit");
    
    try {
        m_log_dev->infoStream()
            << "Ice::ungetRequest() - Putting ["
            << toResubmit << "] to WM's Input file"
            << log4cpp::CategoryStream::ENDLINE;
        
        m_flns.push_back(toResubmit);
    } catch(std::exception& ex) {
        m_log_dev->log(log4cpp::Priority::FATAL, ex.what());
        exit(1);
    }
}

//____________________________________________________________________________
void Ice::resubmit_job( util::CreamJob& j ) 
{
    util::iceLBLogger* lb_logger = util::iceLBLogger::instance();

    string resub_request = string("[ version = \"1.0.0\";")
        +" command = \"jobresubmit\"; arguments = [ id = \"" 
        + j.getGridJobID() + "\" ] ]";
    wmsutils_ns::FileListMutex mx(m_flns);
    wmsutils_ns::FileListLock  lock(mx);
    try {
        m_log_dev->infoStream()
            << "Ice::doOnJobFailure() - Putting ["
            << resub_request << "] to WM's Input file"
            << log4cpp::CategoryStream::ENDLINE;

        lb_logger->logEvent( new util::ns_enqueued_start_event( j, m_ns_filelist ) );
        m_flns.push_back(resub_request);
        lb_logger->logEvent( new util::ns_enqueued_ok_event( j, m_ns_filelist ) );
    } catch(std::exception& ex) {
        m_log_dev->log(log4cpp::Priority::FATAL, ex.what());
        lb_logger->logEvent( new util::ns_enqueued_fail_event( j, m_ns_filelist ) );
        exit(1); // FIXME: Should we keep going?
    }
}

} // namespace ice
} // namespace wms
} // namespace glite
