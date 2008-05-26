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
#include "iceCommandLeaseUpdater.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include "subscriptionManager.h"
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
#include "CreamProxyMethod.h"
#include "iceCommandStatusPoller.h"
#include "Request_source_factory.h"
#include "Request_source.h"
#include "Request.h"
#include "DNProxyManager.h"
#include "iceUtils.h"

#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/ResultWrapper.h"
#include "glite/ce/cream-client-api-c/JobIdWrapper.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/AbsCreamProxy.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/certUtil.h"

#include "glite/wms/purger/purger.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/security/proxyrenewal/renewal.h"
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
namespace cream_api = glite::ce::cream_client_api;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;

Ice* Ice::s_instance = 0;
boost::recursive_mutex Ice::ClassAd_Mutex;

//
// Begin Inner class definitions
//

//____________________________________________________________________________
Ice::IceThreadHelper::IceThreadHelper( const std::string& name ) :
    m_name( name ),
    m_thread( 0 ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() )
{
    
}

//____________________________________________________________________________
Ice::IceThreadHelper::~IceThreadHelper( ) 
{
    stop( );
    delete m_thread;
}

//____________________________________________________________________________
void Ice::IceThreadHelper::start( util::iceThread* obj ) throw( iceInit_ex& )
{
    m_ptr_thread = boost::shared_ptr< util::iceThread >( obj );
    try {
        m_thread = new boost::thread(boost::bind(&util::iceThread::operator(), m_ptr_thread) );
    } catch( boost::thread_resource_error& ex ) {
        throw iceInit_ex( ex.what() );
    }
}

//____________________________________________________________________________
bool Ice::IceThreadHelper::is_started( void ) const 
{
    return ( 0 != m_thread );
}

//____________________________________________________________________________
void Ice::IceThreadHelper::stop( void )
{
    if( m_thread && m_ptr_thread->isRunning() ) {
        CREAM_SAFE_LOG( 
                       m_log_dev->debugStream()
                       << "Ice::IceThreadHelper::stop() - Waiting for thread " 
		       << m_name 
                       << " termination..."
                       << log4cpp::CategoryStream::ENDLINE
                       );
        m_ptr_thread->stop();
        m_thread->join();
        CREAM_SAFE_LOG(
                       m_log_dev->debugStream()
                       << "Ice::IceThreadHelper::stop() - Thread " 
		       << m_name << " finished"
                       << log4cpp::CategoryStream::ENDLINE
                       );
    }
}

//
// End inner class definitions
//

//____________________________________________________________________________
Ice* Ice::instance( void )
{
    log4cpp::Category* m_log_dev = cream_api::util::creamApiLogger::instance()->getLogger();

    if ( 0 == s_instance ) {
        try {
            s_instance = new Ice( ); // may throw iceInit_ex
        } catch(iceInit_ex& ex) {
            CREAM_SAFE_LOG(
                           m_log_dev->fatalStream() 
                           << "Ice::instance() - " 
                           << ex.what()
                           << log4cpp::CategoryStream::ENDLINE
                           );
            abort();
        } catch(...) {
            CREAM_SAFE_LOG(
                           m_log_dev->fatalStream() 
                           << "Ice::instance() - " 
                           << "Catched unknown exception"
                           << log4cpp::CategoryStream::ENDLINE
                           );
            abort();
        }
        s_instance->init();    
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
    m_wms_input_queue( util::Request_source_factory::make_source_input_wm() ),
    m_ice_input_queue( util::Request_source_factory::make_source_input_ice() ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_lb_logger( ice_util::iceLBLogger::instance() ),
    m_cache( ice_util::jobCache::getInstance() ),
    m_configuration( ice_util::iceConfManager::getInstance()->getConfiguration() ),
    m_requests_pool( new util::iceThreadPool("ICE Requests Pool", m_configuration->ice()->max_ice_threads() ) ),
    m_ice_commands_pool( new util::iceThreadPool( "ICE Internal Commands Pool", 5 ) ) // FIXME: remove hardcoded default
{
  /*  CREAM_SAFE_LOG( m_log_dev->debugStream()
                    << "Ice::CTOR() - Done"
                    << log4cpp::CategoryStream::ENDLINE
                    );*/

    try {

      m_hostdn = cream_api::certUtil::getDN( m_configuration->ice()->ice_host_cert() );

    } catch( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
      CREAM_SAFE_LOG(
                     m_log_dev->errorStream()
                     << "Ice::CTOR() - Unable to extract user DN from ["
                     <<  m_configuration->ice()->ice_host_cert() << "]"
                     << ". Cannot perform JobRegister and cannot start Listeneri. Stop!"
                     << log4cpp::CategoryStream::ENDLINE
                     );
      exit(1);
    }

    try {
      m_myname = ice_util::getHostName();
    } catch( runtime_error& ex ) {
      CREAM_SAFE_LOG(
		     m_log_dev->fatalStream() 
		     << "Ice::CTOR() - Couldn't determine hostname: ["
		     << ex.what() << "]"
		     << log4cpp::CategoryStream::ENDLINE
		     );
      abort();
    }

}

//____________________________________________________________________________
Ice::~Ice( )
{

}

//____________________________________________________________________________
void Ice::stopAllThreads( void ) 
{
  /**
   * The following call to stop() method of IteThreadHelper
   * causes the call of iceThread::stop() and boost::thread::join()
   */
  if(m_poller_thread.is_started())
    m_poller_thread.stop();

  if(m_listener_thread.is_started())
  m_listener_thread.stop();

  if(m_updater_thread.is_started())
    m_updater_thread.stop();

  if(m_lease_updater_thread.is_started())
    m_lease_updater_thread.stop();

  if(m_job_killer_thread.is_started())
    m_job_killer_thread.stop();

  if(m_proxy_renewer_thread.is_started())
    m_proxy_renewer_thread.stop();
}

//____________________________________________________________________________
void Ice::init( void )
{    
    // Handle resubmitted/purged jobs
    for ( util::jobCache::iterator it=m_cache->begin(); it != m_cache->end() ; ) {
        util::jobCache::iterator old_it( it );
        it = resubmit_or_purge_job( it );
        // If resubmit_or_purge_job fails, for whatever reason, the
        // iterator if NOT incremented to the next element.
        if ( old_it == it ) {
            ++it;
        }
    }

    if(m_configuration->ice()->start_lease_updater() ) {
        util::iceCommandLeaseUpdater l( true );
        l.execute();
    }
    util::iceCommandStatusPoller p( this, true );
    p.execute( );	
}

//____________________________________________________________________________
void Ice::startListener( void )
{
    if ( ! m_configuration->ice()->start_listener() ) {
        CREAM_SAFE_LOG(
                       m_log_dev->debugStream()
                       << "Ice::startListener() - "
                       << "Listener not enabled, not started."
                       << log4cpp::CategoryStream::ENDLINE
                       );
        return;
    }

    if( m_hostdn.empty() ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream() 
                       << "Ice::startListener() - Host certificate has an empty subject. "
                       << "Won't start Listener"
                       << log4cpp::CategoryStream::ENDLINE
                       );
        return;
    } else {
        CREAM_SAFE_LOG(
                       m_log_dev->debugStream() 
                       << "Ice::startListener() - Host DN is [" << m_hostdn << "]"
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
      //boost::recursive_mutex::scoped_lock M( util::subscriptionManager::mutex );
      util::subscriptionManager::getInstance();
    }
    
    util::eventStatusListener* listener;
    if( m_configuration->ice()->listener_enable_authn() ) {

        if( m_configuration->ice()->ice_host_cert().empty() ||
            m_configuration->ice()->ice_host_key().empty() ) {
            CREAM_SAFE_LOG(
                           m_log_dev->fatalStream()
                           << "Ice::startListener() - "
                           << "ice_host_cert and/or ice_host_key are not defined. "
                           << "Cannot start Listener "
                           << "with authentication as requested. Stop."
                           << log4cpp::CategoryStream::ENDLINE
                           );
            abort();
        }

        CREAM_SAFE_LOG(
               m_log_dev->debugStream()
               << "Ice::startListener() - "
               << "Creating a CEMon listener object: port=["
               << m_configuration->ice()->listener_port() << "]"
               << " hostkey=["
               << m_configuration->ice()->ice_host_key() << "]"
               << " hostcert=["
               << m_configuration->ice()->ice_host_cert() << "]"
               << log4cpp::CategoryStream::ENDLINE
               );
        
        listener = new util::eventStatusListener(m_configuration->ice()->listener_port(),
                                                 m_configuration->ice()->ice_host_cert(),
                                                 m_configuration->ice()->ice_host_key() );
    } else {
        CREAM_SAFE_LOG(
                       m_log_dev->debugStream()
                       << "Ice::startListener() - "
                       << "Creating a CEMon listener object: port=["
                       << m_configuration->ice()->listener_port() <<"]"
                       << log4cpp::CategoryStream::ENDLINE
                       );
        
        listener = new util::eventStatusListener( m_configuration->ice()->listener_port() );
    }
    
    if( !listener->isOK() ) {
        
        CREAM_SAFE_LOG(        
                       m_log_dev->errorStream()
                       << "Ice::startListener() - CEMon listener creation went wrong. Won't start it."
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
            abort();
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
        CREAM_SAFE_LOG( m_log_dev->warnStream()
                        << "Ice::startPoller() - "
                        << "Poller disabled in configuration file. "
                        << "Not started"
                        << log4cpp::CategoryStream::ENDLINE
                        );
        return;
    }
    
    util::eventStatusPoller* poller;
    
    // I removed the try/catch because in the new schema using the
    // iceCommandStatusPoller there is not anymore that exception
    
    poller = new util::eventStatusPoller( this, m_configuration->ice()->poller_delay() );
    m_poller_thread.start( poller );

}

//----------------------------------------------------------------------------
void Ice::startLeaseUpdater( void ) 
{
    if ( !m_configuration->ice()->start_lease_updater() ) {
        CREAM_SAFE_LOG( m_log_dev->warnStream()
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
    CREAM_SAFE_LOG( m_log_dev->warnStream()
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
    // FIXME: uncomment this method to activate the jobKiller    
    if ( !m_configuration->ice()->start_job_killer() ) {
        CREAM_SAFE_LOG( m_log_dev->warnStream()
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
void Ice::getNextRequests( std::list< util::Request* >& ops) 
{
    ops = m_ice_input_queue->get_requests( 5 );
}

//____________________________________________________________________________
void Ice::removeRequest( util::Request* req )
{
    m_ice_input_queue->remove_request( req );
}

//____________________________________________________________________________
bool Ice::is_listener_started( void ) const
{
    return m_listener_thread.is_started( );
}

//____________________________________________________________________________
bool Ice::is_poller_started( void ) const
{
    return m_poller_thread.is_started( );
}

//____________________________________________________________________________
bool Ice::is_lease_updater_started( void ) const
{
    return m_updater_thread.is_started( );
}

//____________________________________________________________________________
bool Ice::is_proxy_renewer_started( void ) const
{
    return m_proxy_renewer_thread.is_started( );
}

//____________________________________________________________________________
bool Ice::is_job_killer_started( void ) const
{
    return m_job_killer_thread.is_started( );
}

//____________________________________________________________________________
void Ice::resubmit_job( ice_util::CreamJob& the_job, const string& reason ) throw()
{
    try {
        boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
        
        the_job = m_lb_logger->logEvent( new ice_util::ice_resubmission_event( the_job, reason ) );
        
        the_job = m_lb_logger->logEvent( new ice_util::ns_enqueued_start_event( the_job, m_wms_input_queue->get_name() ) );
        
	string resub_request;


	{ /**
	   * ClassAd-mutex protected region
	   */
	  boost::recursive_mutex::scoped_lock M_classad( Ice::ClassAd_Mutex );
	  
	  classad::ClassAd command;
	  classad::ClassAd arguments;
	  
	  command.InsertAttr( "version", string("1.0.0") );
	  command.InsertAttr( "command", string("jobresubmit") );
	  arguments.InsertAttr( "id", the_job.getGridJobID() );
	  arguments.InsertAttr( "lb_sequence_code", the_job.getSequenceCode() );
	  command.Insert( "arguments", arguments.Copy() );
	  
	  classad::ClassAdUnParser unparser;
	  unparser.Unparse( resub_request, &command );        
	} // releasing classad mutex

        CREAM_SAFE_LOG(
                       m_log_dev->infoStream()
                       << "Ice::resubmit_job() - Putting ["
                       << resub_request << "] to WM's Input file"
                       << log4cpp::CategoryStream::ENDLINE
                       );

        m_wms_input_queue->put_request( resub_request );
        the_job = m_lb_logger->logEvent( new ice_util::ns_enqueued_ok_event( the_job, m_wms_input_queue->get_name() ) );
    } catch(std::exception& ex) {
        CREAM_SAFE_LOG( m_log_dev->errorStream() 
			<< "Ice::resubmit_job() - "
                        << ex.what() 
                        << log4cpp::CategoryStream::ENDLINE );

        m_lb_logger->logEvent( new ice_util::ns_enqueued_fail_event( the_job, m_wms_input_queue->get_name(), ex.what() ) );
    }
}

//----------------------------------------------------------------------------
ice_util::jobCache::iterator Ice::purge_job( ice_util::jobCache::iterator jit, const string& reason )
throw() 
{
    if ( jit == m_cache->end() )
      return jit;

    vector<cream_api::soap_proxy::JobIdWrapper> target;
    cream_api::soap_proxy::ResultWrapper result;

    target.push_back(cream_api::soap_proxy::JobIdWrapper (jit->getCreamJobID(), 
				   jit->getCreamURL(), 
				   std::vector<cream_api::soap_proxy::JobPropertyWrapper>()
				   ));
    
    cream_api::soap_proxy::JobFilterWrapper filter(target, vector<string>(), -1, -1, "", "");
    
    //boost::scoped_ptr< soap_proxy::AbsCreamProxy > creamClient( soap_proxy::CreamProxyFactory::make_CreamProxy_Purge( &filter, &result, ) );

    // Gets the proxy to use for authentication
    string proxy;
    //{
    //    boost::recursive_mutex::scoped_lock M( ice_util::DNProxyManager::mutex );
    proxy = util::DNProxyManager::getInstance()->getBetterProxyByDN( jit->getUserDN() );
	//}
    
    cream_api::soap_proxy::VOMSWrapper V( jit->getUserProxyCertificate() );
    if( !V.IsValid( ) ) {
      //      throw cream_api::auth_ex( V.getErrorMessage() );
      CREAM_SAFE_LOG(
                     m_log_dev->errorStream()
                     << "Ice::::purge_job() - "
                     << "Unable to purge gridJobID=" 
                     << jit->getGridJobID()
                     << " due to authentication error: " << V.getErrorMessage()
                     << log4cpp::CategoryStream::ENDLINE
                     );
      return jit;
    }

    try {
        boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex ); // this can be called by eventStatusListener::handleEvent that already acquired this mutex; this is not a problem 'cause the mutexes are recursive

        //string cid = jit->getCreamJobID();
        
        if ( m_configuration->ice()->purge_jobs() ) {
            CREAM_SAFE_LOG(m_log_dev->infoStream()
                           << "Ice::purge_job() - "
                           << "Calling JobPurge for job "
                           << jit->describe()
                           << log4cpp::CategoryStream::ENDLINE);
            // We cannot accumulate more jobs to purge in a
            // vector because we must authenticate different
            // jobs with different user certificates.

            //creamClient->Authenticate( proxy );

//             vector< string > oneJobToPurge;
//             oneJobToPurge.push_back( jit->getCreamJobID() );

	    glite::wms::ice::util::CreamProxy_Purge( jit->getCreamURL(), 
						     jit->getUserProxyCertificate(), 
						     &filter, 
						     &result ).execute( 3 );

        } else {
            CREAM_SAFE_LOG(m_log_dev->warnStream()
                           << "Ice::purge_job() - "
                           << "There'are jobs to purge, but PURGE IS DISABLED. "
                           << "Will not purge job "
                           << jit->describe()
                           << " (but will be removed from ICE cache)"
                           << log4cpp::CategoryStream::ENDLINE);
        }
	
	// the following code accumulate all the results in tmp list
	// If an error occurred tmp should contain 1 (and only 1) element
	// (because we sent purge for only 1 job)
	list<pair<cream_api::soap_proxy::JobIdWrapper, string> > tmp;
	result.getNotExistingJobs( tmp );
	result.getNotMatchingStatusJobs( tmp );
	result.getNotMatchingDateJobs( tmp );
	result.getNotMatchingProxyDelegationIdJobs( tmp );
	result.getNotMatchingLeaseIdJobs( tmp );
	
	// It is sufficient look for "empty-ness" because
	// we've started only one job
	if( !tmp.empty() ) {
	  pair<cream_api::soap_proxy::JobIdWrapper, string> wrong = *( tmp.begin() ); // we trust there's only one element because we've purged ONLY ONE job
	  string errMex = wrong.second;
	  
	  CREAM_SAFE_LOG(m_log_dev->errorStream()
			 << "Ice::purge_job() - "
			 << "Cannot purge job " 
			 << jit->describe()
			 << ". Reason is: " << errMex
			 << log4cpp::CategoryStream::ENDLINE);
	  
	  return jit;
	}
	CREAM_SAFE_LOG(m_log_dev->debugStream()
			 << "Ice::purge_job() - "
			 << "Removing purged job " 
			 << jit->describe()
			 << " from cache"
			 << log4cpp::CategoryStream::ENDLINE);
        jit = m_cache->erase( jit );

    } catch (ice_util::ClassadSyntax_ex& ex) {
        /**
         * this exception should not be raised because
         * the CreamJob is created from another valid one
         */
        CREAM_SAFE_LOG(m_log_dev->fatalStream() 
                       << "Ice::purge_job() - "
                       << "Fatal error: CreamJob creation failed "
                       << "copying from a valid one!!!"
                       << log4cpp::CategoryStream::ENDLINE);
        abort();
    } catch(cream_api::soap_proxy::auth_ex& ex) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "Ice::purge_job() - "
                       << "Cannot purge job " 
                       << jit->describe()
                       << ". Reason is: " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE);
    } catch(cream_api::cream_exceptions::BaseException& ex) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "Ice::purge_job() - "
                       << "Cannot purge job " 
                       << jit->describe()
                       << ". Reason is BaseException: " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE);
    } catch(cream_api::cream_exceptions::InternalException& ex) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << "Ice::purge_job() - "
                       << "Cannot purge job " 
                       << jit->describe()
                       << ". Reason is InternalException: " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE);
    } catch(ice_util::elementNotFound_ex& ex) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream()
                       << "Ice::purge_job() - "
                       << "Cannot purge job " << jit->describe()
                       << ". Reason is elementNotFound_ex: " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
    } catch( std::exception& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream()
                       << "Ice::purge_job() - "
                       << "Cannot purge job " << jit->describe()
                       << ". Reason is an exception: " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
    } catch( ... ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream()
                       << "Ice::purge_job() - "
                       << "Cannot purge job " << jit->describe()
                       << ". Reason is an unknown exception"
                       << log4cpp::CategoryStream::ENDLINE
                       );
    }
    return jit;
}


//____________________________________________________________________________
void Ice::deregister_proxy_renewal( const ice_util::CreamJob& job ) throw()
{
    if ( !::getenv( "ICE_DISABLE_DEREGISTER") ) {
        // must deregister proxy renewal
        int      err = 0;
        
        CREAM_SAFE_LOG(
                       m_log_dev->infoStream()
                       << "Ice::deregister_proxy_renewal() - "
                       << "Unregistering Proxy for job "
                       << job.describe()
                       << log4cpp::CategoryStream::ENDLINE
                       );
        
        err = glite_renewal_UnregisterProxy( job.getUserProxyCertificate().c_str(), NULL );
        
        if ( err && (err != EDG_WLPR_PROXY_NOT_REGISTERED) ) {
            CREAM_SAFE_LOG(
                           m_log_dev->errorStream()
                           << "Ice::deregister_proxy_renewal() - "
                           << "ICE cannot unregister the proxy " 
                           << "for job " << job.describe()
                           << ". Reason: \"" << edg_wlpr_GetErrorText(err) 
                           << "\"."
                           << log4cpp::CategoryStream::ENDLINE
                           );
        } else {
            if ( err == EDG_WLPR_PROXY_NOT_REGISTERED ) {
                CREAM_SAFE_LOG(
                               m_log_dev->warnStream()
                               << "Ice::deregister_proxy_renewal() - "
                               << "Job proxy not registered for job "
                               << job.describe() 
                               << ". Going ahead." 
                               << log4cpp::CategoryStream::ENDLINE
                               );
            }
        }
    } else {
        CREAM_SAFE_LOG(
                       m_log_dev->warnStream()
                       << "Ice::deregister_proxy_renewal() - "
                       << "Proxy unregistration disable. To reenable, " 
                       << "unset the environment variable ICE_DISABLE_DEREGISTER"
                       << log4cpp::CategoryStream::ENDLINE
                       );
    }
}


//____________________________________________________________________________
void Ice::purge_wms_storage( const ice_util::CreamJob& job ) throw()
{
    if ( !::getenv( "ICE_DISABLE_PURGER" ) ) {
        try {
            CREAM_SAFE_LOG(
                           m_log_dev->infoStream()
                           << "Ice::purge_storage_for_job() - "
                           << "Purging storage for job "
                           << job.describe()
                           << log4cpp::CategoryStream::ENDLINE
                           );
        
            glite::wmsutils::jobid::JobId j_id( job.getGridJobID() );
            wms::purger::Purger()(j_id);
        } catch( std::exception& ex ) {
            CREAM_SAFE_LOG(
                           m_log_dev->errorStream()
                           << "Ice::purge_storage_for_job() - "
                           << "Cannot purge storage for job "
                           << job.describe()
                           << ". Reason is: " << ex.what()
                           << log4cpp::CategoryStream::ENDLINE
                           );
            
        }
    } else {
        CREAM_SAFE_LOG(
                       m_log_dev->warnStream()
                       << "Ice::purge_storage_for_job() - "
                       << "WMS job purger disabled in ICE. To reenable "
                       << "unset the environment variable ICE_DISABLE_PURGER"
                       << log4cpp::CategoryStream::ENDLINE
                       );
    }
}

//____________________________________________________________________________
ice_util::jobCache::iterator Ice::resubmit_or_purge_job( ice_util::jobCache::iterator it )
throw() 
{
    if ( it != m_cache->end() ) {

        ice_util::CreamJob tmp_job( *it ); ///< This is necessary because the purge_job() method REMOVES the job from the cache; so we need to keep a local copy and work on that copy

        if ( cream_api::job_statuses::CANCELLED == tmp_job.getStatus() ||
             cream_api::job_statuses::DONE_OK == tmp_job.getStatus() ) {

            deregister_proxy_renewal( tmp_job );

        }
        if ( ( cream_api::job_statuses::DONE_FAILED == tmp_job.getStatus() ||
               cream_api::job_statuses::ABORTED == tmp_job.getStatus() ) &&
             !tmp_job.is_killed_by_ice() ) {

            resubmit_job( tmp_job, "Job resubmitted by ICE" );

        }        
        if ( cream_api::job_statuses::DONE_OK == tmp_job.getStatus() ||
             cream_api::job_statuses::CANCELLED == tmp_job.getStatus() ||
             cream_api::job_statuses::DONE_FAILED == tmp_job.getStatus() ||
             cream_api::job_statuses::ABORTED == tmp_job.getStatus() ) {
            // WARNING: the next line removes the job from the job cache!

            it = purge_job( it, "Job purged by ICE" );

        }
        if ( cream_api::job_statuses::CANCELLED == tmp_job.getStatus() ) {

            purge_wms_storage( tmp_job );

        }
    }
    return it;
}

