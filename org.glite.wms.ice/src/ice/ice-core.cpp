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
 * ICE core class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "ice-core.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include "subscriptionManager.h"
//#include "cemonUrlCache.h"
#include "subscriptionManager.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "eventStatusListener.h"
#include "subscriptionUpdater.h"
#include "eventStatusPoller.h"
#include "leaseUpdater.h"
#include "proxyRenewal.h"
#include "jobKiller.h"
#include "iceLBEvent.h"
#include "iceLBLogger.h"
#include "CreamProxyFactory.h"

#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"

#ifdef GLITE_WMS_ICE_HAVE_PURGER
#include "glite/wms/purger/purger.h"
#include "glite/wmsutils/jobid/JobId.h"
#endif
#ifdef GLITE_WMS_ICE_HAVE_RENEWAL
#include "glite/security/proxyrenewal/renewal.h"
#endif
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include "classad_distribution.h"

#include <exception>
#include <unistd.h>
#include <cstdlib>

using namespace std;
using namespace glite::wms::ice;

namespace ice_util = glite::wms::ice::util;
namespace wmsutils_ns = glite::wms::common::utilities;
namespace cream_api = glite::ce::cream_client_api;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;
namespace cert_util = glite::ce::cream_client_api::certUtil;

typedef vector<string>::iterator vstrIt;

Ice* Ice::s_instance = 0;

//
// Begin Inner class definitions
//
Ice::IceThreadHelper::IceThreadHelper( const std::string& name ) :
    m_name( name ),
    m_thread( 0 ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() )
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

bool Ice::IceThreadHelper::is_started( void ) const 
{
    return ( 0 != m_thread );
}

void Ice::IceThreadHelper::stop( void )
{
    if( m_thread && m_ptr_thread->isRunning() ) {
        CREAM_SAFE_LOG( 
                       m_log_dev->infoStream()
                       << "Waiting for thread " << m_name 
                       << " termination..."
                       << log4cpp::CategoryStream::ENDLINE
                       );
        m_ptr_thread->stop();
        m_thread->join();
        CREAM_SAFE_LOG(
                       m_log_dev->infoStream()
                       << "Thread " << m_name << " finished"
                       << log4cpp::CategoryStream::ENDLINE
                       );
    }
}

//
// End inner class definitions
//

Ice* Ice::instance( void )
{
    log4cpp::Category* m_log_dev = cream_api::util::creamApiLogger::instance()->getLogger();

    if ( 0 == s_instance ) {
        try {
            s_instance = new Ice( ); // may throw iceInit_ex
        } catch(iceInit_ex& ex) {
            CREAM_SAFE_LOG(
                           m_log_dev->fatalStream() 
                           << "glite::wms::ice::Ice::instance() - " 
                           << ex.what()
                           << log4cpp::CategoryStream::ENDLINE
                           );
            exit(1);
        } catch(...) {
            CREAM_SAFE_LOG(
                           m_log_dev->fatalStream() 
                           << "glite::wms::ice::Ice::instance() - " 
                           << "Catched unknown exception"
                           << log4cpp::CategoryStream::ENDLINE
                           );
            exit(1);
        }
        
    }
    return s_instance;
}

//____________________________________________________________________________
Ice::Ice( ) throw(iceInit_ex&) : 
    m_listener_thread( "Event Status Listener" ),
    m_poller_thread( "Event Status Poller" ),
    m_updater_thread( "Subscription Updater" ),
    m_lease_updater_thread( "Lease Updater" ),
    m_proxy_renewer_thread( "Proxy Renewer" ),
    m_job_killer_thread( "Job Killer" ),

    // m_ns_filelist( ice_util::iceConfManager::getInstance()->getWMInputFile() ),
    // m_fle( ice_util::iceConfManager::getInstance()->getICEInputFile() ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_lb_logger( ice_util::iceLBLogger::instance() ),
    m_cache( ice_util::jobCache::getInstance() ),
    m_configuration( ice_util::iceConfManager::getInstance()->getConfiguration() ),
    m_ns_filelist( m_configuration->wm()->input() ),
    m_fle( m_configuration->ice()->input() )
{
    CREAM_SAFE_LOG( m_log_dev->infoStream()
                    << "Ice::CTOR() - Initializing File Extractor object..."
                    << log4cpp::CategoryStream::ENDLINE
                    );
    try {
        m_flns.open( m_ns_filelist );
    } catch( std::exception& ex ) {
        throw iceInit_ex( ex.what() );
    } catch( ... ) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream()
                        << "Ice::CTOR() - Catched unknown exception"
                        << log4cpp::CategoryStream::ENDLINE
                        );
        exit( 1 );
    }

}

//____________________________________________________________________________
Ice::~Ice( )
{

}

//____________________________________________________________________________
void Ice::startListener( void )
{
    if ( ! m_configuration->ice()->start_listener() ) {
        CREAM_SAFE_LOG(
                       m_log_dev->infoStream()
                       << "Ice::startListener() - "
                       << "Listener not enabled, not started."
                       << log4cpp::CategoryStream::ENDLINE
                       );
        return;
    }

    // Checks host proxy file validity
    string hostdn;
    CREAM_SAFE_LOG(
                   m_log_dev->infoStream()
                   << "Host proxyfile is [" 
                   << m_configuration->common()->host_proxy_file()
                   << "]" 
                   << log4cpp::CategoryStream::ENDLINE
                   );
    try {
       hostdn = cert_util::getDN( m_configuration->common()->host_proxy_file() );
    } catch ( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
        CREAM_SAFE_LOG( 
                       m_log_dev->errorStream()
                       << "Unable to extract user DN from Proxy File "
                       << m_configuration->common()->host_proxy_file()
                       << ". Won't start Listener"
                       << log4cpp::CategoryStream::ENDLINE
                       );
        return;
    }

    if( (cert_util::getProxyTimeLeft( m_configuration->common()->host_proxy_file() ) <= 0) || ( hostdn.empty() ) ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream() 
                       << "Host proxy certificate is expired. "
                       << "Won't start Listener"
                       << log4cpp::CategoryStream::ENDLINE
                       );
        return;
    } else {
        CREAM_SAFE_LOG(
                       m_log_dev->infoStream() 
                       << "Host DN is [" << hostdn << "]"
                       << log4cpp::CategoryStream::ENDLINE
                       );
    }


    /**
     * The listener and the iceCommandSubmit need to subscribe to
     * CEMon in order to make ICE able to receive job status
     * notifications.  So now as preliminary operation it's the case
     * to check that the subscriptionManager singleton can be created
     * without problems.
     *
     * The subscriptionManager initialization also setups
     * authentication.
     */
    { 
        boost::recursive_mutex::scoped_lock M( util::subscriptionManager::mutex );
        util::subscriptionManager::getInstance();
    }
    /*if( !util::subscriptionManager::getInstance()->isValid() ) {
        CREAM_SAFE_LOG(
                       m_log_dev->fatalStream() 
                       << "Ice::CTOR() - "
                       << "Fatal error creating the subscriptionManager "
                       << "instance. Stop!"
                       << log4cpp::CategoryStream::ENDLINE
                       );
        exit(1);
    }*/
    
    util::eventStatusListener* listener;
    if( m_configuration->ice()->listener_enable_authn() ) {

        if( m_configuration->ice()->ice_host_cert().empty() ||
            m_configuration->ice()->ice_host_key().empty() ) {
            CREAM_SAFE_LOG(
                           m_log_dev->fatalStream()
                           << "Ice::startListener() - "
                           << "ice_host_cert and/or ice_host_key are not undefined. "
                           << "Cannot start Listener "
                           << "with authentication as requested. Stop."
                           << log4cpp::CategoryStream::ENDLINE
                           );
            exit(1);
        }

        CREAM_SAFE_LOG(
               m_log_dev->infoStream()
               << "Ice::startListener() - "
               << "Creating a CEMon listener object: port="
               << m_configuration->ice()->listener_port()
               << " proxyfile=" 
               << m_configuration->common()->host_proxy_file()
               << " hostkey="
               << m_configuration->ice()->ice_host_key()
               << " hostcert="
               << m_configuration->ice()->ice_host_cert()
               << log4cpp::CategoryStream::ENDLINE
               );
        
        listener = new util::eventStatusListener(m_configuration->ice()->listener_port(),
                                                 m_configuration->common()->host_proxy_file(),
                                                 m_configuration->ice()->ice_host_cert(),
                                                 m_configuration->ice()->ice_host_key() );
    } else {
        CREAM_SAFE_LOG(
                       m_log_dev->infoStream()
                       << "Ice::startListener() - "
                       << "Creating a CEMon listener object: port="
                       << m_configuration->ice()->listener_port()
                       << " proxyfile=" 
                       << m_configuration->common()->host_proxy_file()
                       << log4cpp::CategoryStream::ENDLINE
                       );
        
        listener = new util::eventStatusListener( m_configuration->ice()->listener_port(), m_configuration->common()->host_proxy_file() );
    }
    
    if( !listener->isOK() ) {
        
        CREAM_SAFE_LOG(        
                       m_log_dev->errorStream()
                       << "CEMon listener creation went wrong. Won't start it."
                       << log4cpp::CategoryStream::ENDLINE
                       );
        
        // this must be set because other pieces of code
        // have a behaviour that depends on the listener is running or not
        // confMgr->setStartListener( false ); FIXME
        return;
    }
    int bind_retry = 0;
    while( !listener->bind() ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream()
                       << "Ice::startListener() - Bind error: "
                       << listener->getErrorMessage()
                       << " - error code="
                       << listener->getErrorCode()
                       << "Retrying in 5 seconds..."
                       << log4cpp::CategoryStream::ENDLINE
                       );
	bind_retry++;
	if( bind_retry > 1000 ) {
            CREAM_SAFE_LOG(
                           m_log_dev->fatalStream()
                           << "Ice::startListener() - Too many bind retries (5000 secs). Giving up..."
                           << log4cpp::CategoryStream::ENDLINE
                           );
            exit(1);
	}  
        sleep(5);
    }
    
    m_listener_thread.start( listener );
    
    // Starts the subscription updater
    if( m_configuration->ice()->start_subscription_updater() ) {
        util::subscriptionUpdater* subs_updater = new util::subscriptionUpdater( );      
        m_updater_thread.start( subs_updater );
    }
}

//____________________________________________________________________________
void Ice::startPoller( void )
{
    if ( ! m_configuration->ice()->start_poller() ) {
        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "Ice::startPoller() - "
                        << "Poller disabled in configuration file. "
                        << "Not started"
                        << log4cpp::CategoryStream::ENDLINE
                        );
        return;
    }
    
    util::eventStatusPoller* poller;
    
    // I removed the try/catch because in the new schema using the iceCommandStatusPoller
    // there is not anymore that exception
    
    //try {
        poller = new util::eventStatusPoller( this, m_configuration->ice()->poller_delay() );
        m_poller_thread.start( poller );
    /* } catch(util::eventStatusPoller_ex& ex) {
        CREAM_SAFE_LOG(
                       m_log_dev->fatalStream()
                       << "Ice::startPoller() - "
                       << "eventStatusPoller object creation failed "
                       << "due to exception \"" << ex.what()
                       << "\". Stop!"
                       << log4cpp::CategoryStream::ENDLINE
                       );
        exit(1);
    } */
}

//----------------------------------------------------------------------------
void Ice::startLeaseUpdater( void ) 
{
    if ( !m_configuration->ice()->start_lease_updater() ) {
        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "Ice::startLeaseUpdater() - "
                        << "Lease Updater disabled in configuration file. "
                        << "Not started"
                        << log4cpp::CategoryStream::ENDLINE
                        );
        return;
    }
    util::leaseUpdater* lease_updater = new util::leaseUpdater( );
    m_lease_updater_thread.start( lease_updater );
}

//-----------------------------------------------------------------------------
void Ice::startProxyRenewer( void ) 
{
    if ( !m_configuration->ice()->start_proxy_renewer() ) {
        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "Ice::startProxyRenewer() - "
                        << "Proxy Renewer disabled in configuration file. "
                        << "Not started"
                        << log4cpp::CategoryStream::ENDLINE
                        );
        return;
    }
    util::proxyRenewal* proxy_renewer = new util::proxyRenewal( );
    m_proxy_renewer_thread.start( proxy_renewer );
}

//-----------------------------------------------------------------------------
void Ice::startJobKiller( void )
{
    if ( !m_configuration->ice()->start_job_killer() ) {
        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "Ice::startJobKiller() - "
                        << "Job Killer disabled in configuration file. "
                        << "Not started"
                        << log4cpp::CategoryStream::ENDLINE
                        );
        return;
    }
    util::jobKiller* jobkiller = new util::jobKiller( );
    m_job_killer_thread.start( jobkiller );
}

//____________________________________________________________________________
// void Ice::clearRequests() 
// {
//     // m_requests.clear();
// }

//____________________________________________________________________________
void Ice::getNextRequests(vector< filelist_request >& ops) 
{
    std::vector<FLEit> m_requests;
  try { 
      m_requests = m_fle.get_all_available();
  }
  catch( exception& ex ) {
      CREAM_SAFE_LOG(
                     m_log_dev->fatalStream() 
                     << "Ice::getNextRequest() - " << ex.what()
                     << log4cpp::CategoryStream::ENDLINE
                     );
      exit(1);
  }
  for ( unsigned j=0; j < m_requests.size(); j++ ) {
      ops.push_back( filelist_request( m_requests[j] ) );
  }
}

//____________________________________________________________________________
void Ice::removeRequest( const filelist_request& req ) 
{
    m_fle.erase( req.get_iterator() );
    // m_fle.erase(m_requests[reqNum]);
}

bool Ice::is_listener_started( void ) const
{
    return m_listener_thread.is_started( );
}

bool Ice::is_poller_started( void ) const
{
    return m_poller_thread.is_started( );
}

bool Ice::is_lease_updater_started( void ) const
{
    return m_updater_thread.is_started( );
}

bool Ice::is_proxy_renewer_started( void ) const
{
    return m_proxy_renewer_thread.is_started( );
}

bool Ice::is_job_killer_started( void ) const
{
    return m_job_killer_thread.is_started( );
}

//____________________________________________________________________________
void Ice::resubmit_job( ice_util::CreamJob& the_job, const string& reason )
{
    wmsutils_ns::FileListMutex mx(m_flns);
    wmsutils_ns::FileListLock  lock(mx);
    try {
        boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
        
        the_job = m_lb_logger->logEvent( new ice_util::ice_resubmission_event( the_job, reason ) );
        
        the_job = m_lb_logger->logEvent( new ice_util::ns_enqueued_start_event( the_job, m_ns_filelist ) );
        
        classad::ClassAd command;
        classad::ClassAd arguments;
        
        command.InsertAttr( "version", string("1.0.0") );
        command.InsertAttr( "command", string("jobresubmit") );
        arguments.InsertAttr( "id", the_job.getGridJobID() );
        arguments.InsertAttr( "lb_sequence_code", the_job.getSequenceCode() );
        command.Insert( "arguments", arguments.Copy() );
        
        classad::ClassAdUnParser unparser;
        string resub_request;
        unparser.Unparse( resub_request, &command );        

        CREAM_SAFE_LOG(
                       m_log_dev->infoStream()
                       << "Ice::doOnJobFailure() - Putting ["
                       << resub_request << "] to WM's Input file"
                       << log4cpp::CategoryStream::ENDLINE
                       );

        m_flns.push_back(resub_request);
        the_job = m_lb_logger->logEvent( new ice_util::ns_enqueued_ok_event( the_job, m_ns_filelist ) );
    } catch(std::exception& ex) {
        CREAM_SAFE_LOG( m_log_dev->errorStream() 
                        << ex.what() 
                        << log4cpp::CategoryStream::ENDLINE );

        m_lb_logger->logEvent( new ice_util::ns_enqueued_fail_event( the_job, m_ns_filelist, ex.what() ) );
    }
}

//----------------------------------------------------------------------------
ice_util::jobCache::iterator Ice::purge_job( ice_util::jobCache::iterator jit, const string& reason )
{
    if ( jit == m_cache->end() )
        return jit;

    boost::scoped_ptr< soap_proxy::CreamProxy > creamClient( ice_util::CreamProxyFactory::makeCreamProxy(false) );

    try {
        boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
        
        string cid = jit->getCreamJobID();
        
        if ( m_configuration->ice()->poller_purges_jobs() ) {
            CREAM_SAFE_LOG(m_log_dev->infoStream()
                           << "ice-core::purge_job() - "
                           << "Calling JobPurge for JobId ["
                           << cid << "]"
                           << log4cpp::CategoryStream::ENDLINE);
            // We cannot accumulate more jobs to purge in a
            // vector because we must authenticate different
            // jobs with different user certificates.
            creamClient->Authenticate( jit->getUserProxyCertificate());
            vector< string > oneJobToPurge;
            oneJobToPurge.push_back( jit->getCreamJobID() );
            creamClient->Purge( jit->getCreamURL().c_str(), oneJobToPurge);
        } else {
            CREAM_SAFE_LOG(m_log_dev->warnStream()
                           << "ice-core::purge_job() - "
                           << "There'are jobs to purge, but PURGE IS DISABLED. "
                           << "Will not purge JobId ["
                           << cid << "] (but will be removed from ICE cache)"
                           << log4cpp::CategoryStream::ENDLINE);
        }
        
        jit = m_cache->erase( jit );
    } catch (ice_util::ClassadSyntax_ex& ex) {
        /**
         * this exception should not be raised because
         * the CreamJob is created from another valid one
         */
        CREAM_SAFE_LOG(m_log_dev->fatalStream() 
                       << "ice-core::purge_job() - "
                       << "Fatal error: CreamJob creation failed "
                       << "copying from a valid one!!!"
                       << log4cpp::CategoryStream::ENDLINE);
        exit(1);
    } catch(soap_proxy::auth_ex& ex) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "ice-core::purge_job() - "
                       << "Cannot purge job: " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE);
    } catch(cream_api::cream_exceptions::BaseException& s) {
        CREAM_SAFE_LOG(m_log_dev->log(log4cpp::Priority::ERROR, s.what()));
    } catch(cream_api::cream_exceptions::InternalException& severe) {
        CREAM_SAFE_LOG(m_log_dev->log(log4cpp::Priority::ERROR, severe.what()));
        //exit(1);
    } catch(ice_util::elementNotFound_ex& ex) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream()
                       << "ice-core::purge_job() - "
                       << "Cannot remove [" << jit->getCreamJobID()
                       << "] from job cache: " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
    }
    return jit;
}


//____________________________________________________________________________
void Ice::deregister_proxy_renewal( const ice_util::CreamJob& job )
{
#ifdef GLITE_WMS_ICE_HAVE_RENEWAL
    
    // must deregister proxy renewal
    int      err = 0;
    
    CREAM_SAFE_LOG(
                   m_log_dev->infoStream()
                   << "ice-core::deregister_proxy_renewal() - "
                   << "Unregistering Proxy"
                   << " for Grid job [" << job.getGridJobID() << "]"
                   << " CREAM job [" << job.getCreamJobID() << "]"
                   << log4cpp::CategoryStream::ENDLINE
                   );
    
    err = glite_renewal_UnregisterProxy( job.getUserProxyCertificate().c_str(), NULL );
    
    if ( err && (err != EDG_WLPR_PROXY_NOT_REGISTERED) ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream()
                       << "ice-core::deregister_proxy_renewal() - "
                       << "ICE cannot unregister the proxy " 
                       << "for Grid job [" << job.getGridJobID() << "] "
                       << " CREAM job [" << job.getCreamJobID() << "] "
                       << "Reason: \"" << edg_wlpr_GetErrorText(err) 
                       << "\"."
                       << log4cpp::CategoryStream::ENDLINE
                       );
    } else {
        if ( err == EDG_WLPR_PROXY_NOT_REGISTERED ) {
            CREAM_SAFE_LOG(
                           m_log_dev->warnStream()
                           << "ice-core::deregister_proxy_renewal() - "
                           << "Job proxy not registered for Grid job ["
                           << job.getGridJobID() << "]"
                           << " CREAM job [" << job.getCreamJobID() << "]."
                           << " Going ahead." 
                           << log4cpp::CategoryStream::ENDLINE
                           );
        }
    }
#else
    CREAM_SAFE_LOG(
                   m_log_dev->warnStream()
                   << "ice-core::deregister_proxy_renewal() - "
                   << "Proxy unregistration support not enabled." 
                   << log4cpp::CategoryStream::ENDLINE
                   );
#endif
}


//____________________________________________________________________________
void Ice::purge_wms_storage( const ice_util::CreamJob& job )
{
#ifdef GLITE_WMS_ICE_HAVE_PURGER
    try {
        CREAM_SAFE_LOG(
                       m_log_dev->infoStream()
                       << "ice-core::purge_storage_for_job() - "
                       << "Purging storage for Grid job ["
                       << job.getGridJobID() << "]"
                       << " CREAM job [" << job.getCreamJobID() << "]"
                       << log4cpp::CategoryStream::ENDLINE
                       );
        
        glite::wmsutils::jobid::JobId j_id( job.getGridJobID() );
        wms::purger::purgeStorage( j_id );
    } catch( std::exception& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream()
                       << "ice-core::purge_storage_for_job() - "
                       << "Cannot purge storage for Grid job [" 
                       << job.getGridJobID()
                       << " ] CREAM job ["
                       << job.getCreamJobID()
                       << "] due to exception: " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
        
    }
#else
    CREAM_SAFE_LOG(
                   m_log_dev->warnStream()
                   << "ice-core::purge_storage_for_job() - "
                   << "WMS job purger not enabled."
                   << log4cpp::CategoryStream::ENDLINE
                   );
#endif
}

ice_util::jobCache::iterator Ice::resubmit_or_purge_job( ice_util::jobCache::iterator it )
{
    if ( it != m_cache->end() ) {

        ice_util::CreamJob tmp_job( *it ); ///< This is necessary because the purge_job() method REMOVES the job from the cache; so we need to keep a local copy and work on that copy

        if ( cream_api::job_statuses::CANCELLED == tmp_job.getStatus() ||
             cream_api::job_statuses::DONE_OK == tmp_job.getStatus() ) {
            deregister_proxy_renewal( tmp_job );            
        }
        if ( cream_api::job_statuses::DONE_FAILED == tmp_job.getStatus() ||
             cream_api::job_statuses::ABORTED == tmp_job.getStatus() ) {
            resubmit_job( tmp_job, "Job resubmitted by ICE" );            
        }        
        if ( cream_api::job_statuses::DONE_OK == tmp_job.getStatus() ||
             cream_api::job_statuses::CANCELLED == tmp_job.getStatus() ||
             cream_api::job_statuses::DONE_FAILED == tmp_job.getStatus() ||
             cream_api::job_statuses::ABORTED == tmp_job.getStatus() ) {
            // WARNING: the next line removes the job from the job cache!
            purge_job( it, "Job purged by ICE" );
        }
        if ( cream_api::job_statuses::CANCELLED == tmp_job.getStatus() ) {
            purge_wms_storage( tmp_job );
        }
    }
    return it;
}

