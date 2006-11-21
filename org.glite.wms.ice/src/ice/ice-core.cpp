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
#include "cemonUrlCache.h"
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
#include "glite/wms/purger/purger.h"
#include "glite/wmsutils/jobid/JobId.h"
#ifdef GLITE_WMS_ICE_HAVE_RENEWAL
#include "glite/security/proxyrenewal/renewal.h"
#endif

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
    m_ns_filelist( ice_util::iceConfManager::getInstance()->getWMInputFile() ),
    m_fle( ice_util::iceConfManager::getInstance()->getICEInputFile() ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_is_purge_enabled( ice_util::iceConfManager::getInstance()->getPollerPurgesJobs() ),
    m_lb_logger( ice_util::iceLBLogger::instance() ),
    m_cache( ice_util::jobCache::getInstance() )
{
    CREAM_SAFE_LOG( 
                   m_log_dev->log(log4cpp::Priority::INFO, "Ice::CTOR() - Initializing File Extractor object...")
                   );
    try {
        m_flns.open( m_ns_filelist );
    }
    catch( std::exception& ex ) {
        throw iceInit_ex( ex.what() );
    } catch( ... ) {
        CREAM_SAFE_LOG(
                       m_log_dev->log(log4cpp::Priority::FATAL, "Ice::CTOR() - Catched unknown exception")
                       );
        exit( 1 );
    }

    //boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );

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
	{ 
          boost::recursive_mutex::scoped_lock M( util::subscriptionManager::mutex );
          util::subscriptionManager::getInstance();
	}
        if( !util::subscriptionManager::getInstance()->isValid() ) {
            CREAM_SAFE_LOG(
                           m_log_dev->fatalStream() 
                           << "Ice::CTOR() - "
                           << "Fatal error creating the subscriptionManager "
                           << "instance. Stop!"
                           << log4cpp::CategoryStream::ENDLINE
                           );
	    exit(1);
        }
        /**
         * cemonUrlCache is used to retrieve the list of cemon we're
         * subscribed. If it's creation failed, it is not the case (at 0-order)
         * to use the listener...
         *
         */
        {
            // this  check with NULL is useless
	    // becuase if something goes wrong in the
	    // cemonUrlCache's CTOR the program aborts !
            if( util::cemonUrlCache::getInstance() == NULL ) {
                CREAM_SAFE_LOG(
                               m_log_dev->fatalStream() 
                               << "Ice::CTOR() - "
                               << "Fatal error creating the subscriptionCache instance. "
                               << "Stop!."
                               << log4cpp::CategoryStream::ENDLINE
                               );
		exit(1);
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
    //boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
    util::iceConfManager* confMgr( util::iceConfManager::getInstance() );

    CREAM_SAFE_LOG(
                   m_log_dev->infoStream()
                   << "Ice::startListener() - "
                   << "Creating a CEMon listener object..."
                   << log4cpp::CategoryStream::ENDLINE
                   );

    util::eventStatusListener* listener;
    if( confMgr->getListenerEnableAuthN() ) {
      m_host_cert = confMgr->getIceHostCert(); //::getenv("GLITE_HOST_CERT");
      m_host_key  = confMgr->getIceHostKey();  //::getenv("GLITE_HOST_KEY");
      
      if( (m_host_cert == "") || (m_host_key == "") ) {
          CREAM_SAFE_LOG(
                         m_log_dev->fatalStream()
                         << "Ice::startListener() - cannot access to "
			 << "ice_host_cert and/or ice_host_key "
                         << "attributes. Cannot start Listener "
                         << "with authentication as requested. Stop."
                         << log4cpp::CategoryStream::ENDLINE
                         );
	exit(1);
      }
      listener = new util::eventStatusListener(listenPort, 
					       confMgr->getHostProxyFile(), 
					       m_host_cert, 
					       m_host_key);
    }
    else {
      listener = new util::eventStatusListener(listenPort, confMgr->getHostProxyFile());
    }
    
    if( !listener->isOK() ) {

        CREAM_SAFE_LOG(        
                       m_log_dev->errorStream()
                       << "CEMon listener creation went wrong. Won't start it."
                       << log4cpp::CategoryStream::ENDLINE
                       );
        
        // this must be set because other pieces of code
        // have a behaviour that depends on the listener is running or not
        confMgr->setStartListener( false );
        return;
    }
    int bind_retry=0;
    while( !listener->bind() ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream()
                       << "Ice::startListener() - Bind error: "
                       << listener->getErrorMessage()
                       << " - error code="
                       << listener->getErrorCode()
                       << "Retrying in 5 seconds..."
                       << log4cpp::CategoryStream::ENDLINE;
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
    
    //-----------------now is time to start subUpdater-------------------------
    bool tmp_start_sub_updater = confMgr->getStartSubscriptionUpdater();
    
    // The following subscriptionUpdater creation also triggers
    // creation of subscriptionManager (by invoking
    // subscriptionManager::getInstance(). But in this case the
    // creation of the subscriptionManager singleton already occurred
    // in the Ice::CTOR (see above).  Furthermore the CTOR of
    // subscriptionUpdater also sets of the subpscriptionManager's
    // internal variable (m_myname) that keeps the listener URL (of
    // this ICE) by calling
    // subscriptionManager::getInstance()->setConsumerURLName(...)
    if( tmp_start_sub_updater ) {
        util::subscriptionUpdater* subs_updater = new util::subscriptionUpdater( confMgr->getHostProxyFile());      
	if( !subs_updater->isValid() )
	{
            CREAM_SAFE_LOG(
                           m_log_dev->fatalStream()
                           << "Ice::startListener() - "
                           << "subscriptionUpdater object creation failed. Stop!"
                           << log4cpp::CategoryStream::ENDLINE;
                           );
	  exit(1);
	             
	}
        m_updater_thread.start( subs_updater );
    }
}

//____________________________________________________________________________
void Ice::startPoller( int poller_delay )
{
  util::eventStatusPoller* poller;
  try {
    poller = new util::eventStatusPoller( this, poller_delay );
    m_poller_thread.start( poller );
  } catch(util::eventStatusPoller_ex& ex) {
      CREAM_SAFE_LOG(
                     m_log_dev->fatalStream()
                     << "Ice::startPoller() - eventStatusPoller object creation failed. Stop!"
                     << log4cpp::CategoryStream::ENDLINE
                     );
    exit(1);
  }
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

//-----------------------------------------------------------------------------
void Ice::startJobKiller( void )
{
  util::jobKiller* jobkiller = new util::jobKiller( );
  m_job_killer_thread.start( jobkiller );
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
      CREAM_SAFE_LOG(
                     m_log_dev->fatalStream() 
                     << "Ice::getNextRequest() - " << ex.what()
                     << log4cpp::CategoryStream::ENDLINE
                     );
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
        
        if ( m_is_purge_enabled ) {
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

ice_util::jobCache::iterator Ice::resubmit_or_purge_job( ice_util::jobCache::iterator it )
{
    if ( it != m_cache->end() ) {

        if ( cream_api::job_statuses::CANCELLED == it->getStatus() ||
             cream_api::job_statuses::DONE_OK == it->getStatus() ||
             cream_api::job_statuses::DONE_FAILED == it->getStatus() ) {

#ifdef GLITE_WMS_ICE_HAVE_RENEWAL
            
            // must deregister proxy renewal
            int      err = 0;

            CREAM_SAFE_LOG(
                           m_log_dev->infoStream()
                           << "ice-core::resubmit_or_purge_job() - "
                           << "Unregistering Proxy"
                           << " for Grid job [" << it->getGridJobID() << "]"
                           << " CREAM job [" << it->getCreamJobID() << "]"
                           << log4cpp::CategoryStream::ENDLINE
                           );
            
            err = glite_renewal_UnregisterProxy( it->getUserProxyCertificate().c_str(), NULL );
            
            if ( err && (err != EDG_WLPR_PROXY_NOT_REGISTERED) ) {
                CREAM_SAFE_LOG(
                               m_log_dev->errorStream()
                               << "ice-core::resubmit_or_purge_job() - "
                               << "ICE cannot unregister the proxy " 
                               << "for Grid job [" << it->getGridJobID() << "] "
                               << " CREAM job [" << it->getCreamJobID() << "] "
                               << "Reason: \"" << edg_wlpr_GetErrorText(err) 
                               << "\"."
                               << log4cpp::CategoryStream::ENDLINE
                               );
            } else {
                if ( err == EDG_WLPR_PROXY_NOT_REGISTERED ) {
                    CREAM_SAFE_LOG(
                                   m_log_dev->errorStream()
                                   << "ice-core::resubmit_or_purge_job() - "
                                   << "Job proxy not registered for Grid job ["
                                   << it->getGridJobID() << "]"
                                   << " CREAM job [" << it->getCreamJobID() << "]."
                                   << " Going ahead." 
                                   << log4cpp::CategoryStream::ENDLINE
                                   );
                }
            }
#else
            CREAM_SAFE_LOG(
                           m_log_dev->warnStream()
                           << "ice-core::resubmit_or_purge_job() - "
                           << "Proxy unregistration support not compiled." 
                           << log4cpp::CategoryStream::ENDLINE
                           );
#endif
            
        }
        if ( cream_api::job_statuses::CANCELLED == it->getStatus() ) {
            // must purge job within WMS
            try {
                CREAM_SAFE_LOG(
                               m_log_dev->infoStream()
                               << "ice-core::resubmit_or_purge_job() - "
                               << "Purging storage for Grid job ["
                               << it->getGridJobID() << "]"
                               << " CREAM job [" << it->getCreamJobID() << "]"
                               << log4cpp::CategoryStream::ENDLINE
                               );

                glite::wmsutils::jobid::JobId j_id( it->getGridJobID() );
                wms::purger::purgeStorage( j_id );
            } catch( std::exception& ex ) {
                CREAM_SAFE_LOG(
                               m_log_dev->errorStream()
                               << "ice-core::resubmit_or_purge_job() - "
                               << "Cannot purge storage for Grid job [" 
                               << it->getGridJobID()
                               << " ] CREAM job ["
                               << it->getCreamJobID()
                               << "] due to exception: " << ex.what()
                               << log4cpp::CategoryStream::ENDLINE
                               );
                
            }
        }
        
        if ( it->can_be_resubmitted() ) {
            // resubmit job
            resubmit_job( *it, "Job resubmitted by ICE" );
        }
        if ( it->can_be_purged() ) {
            // purge the job
            it = purge_job( it, "Job purged by ICE" );
        }

    }
    return it;
}
