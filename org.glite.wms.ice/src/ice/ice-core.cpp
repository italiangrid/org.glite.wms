/* 
 * Copyright (C) Members Of The Egee Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed Under The Apache License, Version 2.0 (the "License"); 
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
#include "subscriptionManager.h"
#include "subscriptionManager.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "eventStatusListener.h"
#include "subscriptionUpdater.h"
#include "eventStatusPoller.h"
#include "leaseUpdater.h"
#include "proxyRenewal.h"
#include "iceLBEvent.h"
#include "iceLBLogger.h"
#include "CreamProxyMethod.h"
#include "iceCommandStatusPoller.h"
#include "Request_source_factory.h"
#include "Request_source.h"
#include "Request.h"
#include "DNProxyManager.h"
#include "iceUtils.h"
#include "creamJob.h"

#include "iceDb/GetJobsToPoll.h"
#include "iceDb/GetFieldsCount.h"
#include "iceDb/CheckGridJobID.h"
#include "iceDb/GetTerminatedJobs.h"
#include "iceDb/Transaction.h"
#include "iceDb/RemoveJobByGid.h"

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
#ifdef HAVE_GLITE_JOBID
#include "glite/jobid/JobId.h"
#else
#include "glite/wmsutils/jobid/JobId.h"
#endif
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
namespace config_ns = glite::wms::common::configuration;

Ice* Ice::s_instance = 0;
boost::recursive_mutex Ice::ClassAd_Mutex;
boost::recursive_mutex Ice::s_mutex;
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
                       
                       );
        m_ptr_thread->stop();
        m_thread->join();
        CREAM_SAFE_LOG(
                       m_log_dev->debugStream()
                       << "Ice::IceThreadHelper::stop() - Thread " 
		       << m_name << " finished"
                       
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
                           
                           );
            abort();
        } catch(...) {
            CREAM_SAFE_LOG(
                           m_log_dev->fatalStream() 
                           << "Ice::instance() - " 
                           << "Catched unknown exception"
                           
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
    m_reqnum(ice_util::iceConfManager::getInstance()->getConfiguration()->ice()->max_ice_threads()),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_lb_logger( ice_util::iceLBLogger::instance() ),
    m_configuration( ice_util::iceConfManager::getInstance()->getConfiguration() )
{
  /**
     Must check if ICE is starting from scratch. In that case all Event
     Query calls must be done with fromDate = 'now', because ICE is not
     intersted in old jobs/events that has been removed from its database
  */
  bool db_empty = false;
  {
    list<string> fields;
    fields.push_back( "gridjobid" );
    db::GetFieldsCount counter( fields, list<pair<string, string> >(), "Ice::Ice");
    db::Transaction tnx(false, false);
    tnx.execute( &counter );
    db_empty = ( counter.get_count() == 0 );
  }
  if( db_empty )
    m_start_time = time(0)-600;
  else
    m_start_time = -1;


  if(m_reqnum < 5) m_reqnum = 5;
   int thread_num_commands, thread_num = m_configuration->ice()->max_ice_threads();
   if(thread_num<1) thread_num=1;
//    if(thread_num >= 4)
//      thread_num_commands = thread_num/2;
//    else
//      thread_num_commands = 2;

   int poll_tnum = 0;
   if(thread_num >=2)
     poll_tnum = thread_num/2;
   else
     poll_tnum = 2;

   m_requests_pool = new util::iceThreadPool("ICE Submission Pool", thread_num );
   m_ice_commands_pool = new util::iceThreadPool( "ICE Poller Pool", poll_tnum);
   

    try {

      m_hostdn = cream_api::certUtil::getDN( m_configuration->ice()->ice_host_cert() );

    } catch( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
      string hostcert = m_configuration->ice()->ice_host_cert();
        CREAM_SAFE_LOG( m_log_dev->fatalStream()
                        << "Ice::CTOR() - Unable to extract user DN from ["
                        <<  hostcert << "]"
                        << ". Cannot perform JobRegister and cannot start Listener. Stop!"
                        );
        exit(1);
    }

    try {
      m_myname = ice_util::getHostName();
    } catch( runtime_error& ex ) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() 
                        << "Ice::CTOR() - Couldn't determine hostname: ["
                        << ex.what() << "]"		     
                        );
        exit(1);
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
  list< glite::wms::ice::util::CreamJob > allJobs;
  {
    db::GetTerminatedJobs getter( &allJobs, "Ice::init" );
    db::Transaction tnx(false, false);
    tnx.execute( &getter );
    //    allJobs = getter.get_jobs();
  }
  
  for(list< glite::wms::ice::util::CreamJob >::iterator it = allJobs.begin();
      it != allJobs.end();
      ++it)
    {
      resubmit_or_purge_job( /*thisJob*/ *it );
    }
   
  if(m_configuration->ice()->start_lease_updater() ) {
    util::iceCommandLeaseUpdater l( true );
    l.execute();
  }
  //util::iceCommandStatusPoller p( this, true );
  //p.execute( );	
}

//____________________________________________________________________________
void Ice::startListener( void )
{
    if ( ! m_configuration->ice()->start_listener() ) {
        CREAM_SAFE_LOG(
                       m_log_dev->debugStream()
                       << "Ice::startListener() - "
                       << "Listener not enabled, not started."
                       
                       );
        return;
    }

    if( m_hostdn.empty() ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream() 
                       << "Ice::startListener() - Host certificate has an empty subject. "
                       << "Won't start Listener"
                       
                       );
        return;
    } else {
        CREAM_SAFE_LOG(
                       m_log_dev->debugStream() 
                       << "Ice::startListener() - Host DN is [" << m_hostdn << "]"
                       
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
                       
                       );
        
        listener = new util::eventStatusListener( m_configuration->ice()->listener_port() );
    }
    
    if( !listener->isOK() ) {
        
        CREAM_SAFE_LOG(        
                       m_log_dev->errorStream()
                       << "Ice::startListener() - CEMon listener creation went wrong. Won't start it."
                       
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
                       
                       );
	bind_retry++;
	if( bind_retry > 1000 ) {
            CREAM_SAFE_LOG(
                           m_log_dev->fatalStream()
                           << "Ice::startListener() - Too many bind retries (5000 secs). Giving up..."
                           
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
		    << "Delegation Renewal disabled in configuration file. "
		    << "Not started"
		    
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
/*    if ( !m_configuration->ice()->start_job_killer() ) {
        CREAM_SAFE_LOG( m_log_dev->warnStream()
                        << "Ice::startJobKiller() - "
                        << "Job Killer disabled in configuration file. "
                        << "Not started"
                        
                        );
        return;
    }
    util::jobKiller* jobkiller = new util::jobKiller( );
    m_job_killer_thread.start( jobkiller ); */
}

//____________________________________________________________________________
void Ice::getNextRequests( std::list< util::Request* >& ops) 
{
  //  int reqnum = ice_util::iceConfManager::getInstance()->getConfiguration()->ice()->max_ice_threads();
  //  if(reqnum < 5) reqnum = 5;
  ops = m_ice_input_queue->get_requests( m_reqnum );
}

//____________________________________________________________________________
void Ice::removeRequest( util::Request* req )
{
    m_ice_input_queue->remove_request( req );
}

//----------------------------------------------------------------------------
size_t Ice::get_input_queue_size( void )
{
    return m_ice_input_queue->get_size();
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

  //  string isbproxy( the_job.getUserProxyCertificate() );

  if( !::getenv( "GLITE_WMS_ICE_NORESUBMIT" ) ) {

     CREAM_SAFE_LOG( m_log_dev->warnStream() 
                    << "Ice::resubmit_job() - RESUBMISSION DISABLED."
                    );
     return;
  }

  cream_api::soap_proxy::VOMSWrapper V( the_job.get_user_proxy_certificate() );
  if( !V.IsValid( ) ) {
    //throw( iceCommandTransient_ex( "Authentication error: " + V.getErrorMessage() ) );
    CREAM_SAFE_LOG( m_log_dev->errorStream() 
		    << "Ice::resubmit_job() - Will NOT resubmit job ["
		    << the_job.describe() << "] " 
		    << "because it's Input Sandbox proxy file is not valid: "
		    << V.getErrorMessage()
		    );

    the_job.set_failure_reason( "Input sandbox's proxy is missing. Cannot resubmit job" );
    m_lb_logger->logEvent( new ice_util::job_aborted_event( the_job ) );

    return;
  }

    try {
      //boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
      boost::recursive_mutex::scoped_lock M( /*ice_util::CreamJob::s_GlobalICEMutex*/ s_mutex );
        
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
	  arguments.InsertAttr( "id", the_job.get_grid_jobid() );
	  arguments.InsertAttr( "lb_sequence_code", the_job.get_sequence_code() );
	  command.Insert( "arguments", arguments.Copy() );
	  
	  classad::ClassAdUnParser unparser;
	  unparser.Unparse( resub_request, &command );        
	} // releasing classad mutex

        CREAM_SAFE_LOG(
                       m_log_dev->infoStream()
                       << "Ice::resubmit_job() - Putting ["
                       << resub_request << "] to WM's Input file"
                       
                       );

        m_wms_input_queue->put_request( resub_request );
        the_job = m_lb_logger->logEvent( new ice_util::ns_enqueued_ok_event( the_job, m_wms_input_queue->get_name() ) );
    } catch(std::exception& ex) {
        CREAM_SAFE_LOG( m_log_dev->errorStream() 
			<< "Ice::resubmit_job() - "
                        << ex.what() 
                         );

        m_lb_logger->logEvent( new ice_util::ns_enqueued_fail_event( the_job, m_wms_input_queue->get_name(), ex.what() ) );
    }
}

//----------------------------------------------------------------------------
//ice_util::jobCache::iterator
void Ice::purge_job( const util::CreamJob& theJob , 
		     const string& reason )
  throw() 
{
    static const char* method_name = "Ice::purge_job() - ";

    string jobdesc( theJob.describe() );
    string _gid(  theJob.get_grid_jobid() );

    {
      db::CheckGridJobID checker( _gid, "Ice::purge_job" );
      db::Transaction tnx(false, false);
      tnx.execute( &checker );
      if( !checker.found() )
	return;
    }
    if ( !m_configuration->ice()->purge_jobs() ) {
      
      /**
	 PURGE on the CE is DISABLED, let's:
	 - decrement job counter
	 - remove job from ICE's database
	 - return
      */
      CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
		     << "JobPurge is DISABLED. Removing job ["
		     << jobdesc << "] from ICE database."
		     );
      if(theJob.is_proxy_renewable())
	ice_util::DNProxyManager::getInstance()->decrementUserProxyCounter( theJob.get_user_dn(), theJob.get_myproxy_address() );
      {
	db::RemoveJobByGid remover( _gid, "Ice::purge_job" );
	db::Transaction tnx(false, false);
	tnx.execute( &remover );
      }
      return;
    }
      
    vector<cream_api::soap_proxy::JobIdWrapper> target;
    cream_api::soap_proxy::ResultWrapper result;

    target.push_back(cream_api::soap_proxy::JobIdWrapper (theJob.get_cream_jobid(), 
							  theJob.get_creamurl(), 
							  std::vector<cream_api::soap_proxy::JobPropertyWrapper>()
							  ));
    
    cream_api::soap_proxy::JobFilterWrapper filter(target, vector<string>(), -1, -1, "", "");
    
    // Gets the proxy to use for authentication
    string better_proxy = util::DNProxyManager::getInstance()->getAnyBetterProxyByDN( theJob.get_user_dn() ).get<0>();

    if( better_proxy.empty() ) {
      CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
		      << "DNProxyManager returned an empty string for BetterProxy of user DN ["
		      << "] for job ["
		      << jobdesc
		      << "]. Using the Job's proxy." 
		      );

      better_proxy = theJob.get_user_proxy_certificate();
    }
    
    cream_api::soap_proxy::VOMSWrapper V( better_proxy );
    if( !V.IsValid( ) ) {

      /**
	 Cannot PURGE job on the CE;
	 let's leave it on the ICE's database
	 and DO NOT decrement the job counter
	 of the 'super' better proxy.
      */
      CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
		      << "Unable to purge job ["
		      << jobdesc
		      << "] due to authentication error: " 
		      << V.getErrorMessage()
		      );
      return;// jit;
    }

    try {
        
      CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
		     << "Calling JobPurge for job ["
		     << jobdesc << "]"
		     );
      // We cannot accumulate more jobs to purge in a
      // vector because we must authenticate different
      // jobs with different user certificates.
      
      glite::wms::ice::util::CreamProxy_Purge( theJob.get_creamurl(), 
					       better_proxy,
					       &filter, 
					       &result ).execute( 3 );
      
      
      
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
	
	CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		       << "Cannot purge job [" << jobdesc 
		       << "] - Reason is: " << errMex
		       );
	
	/**
	   Another poll will be tried again
	   later
	*/
	return;// jit;
      }
      
    } catch (ice_util::ClassadSyntax_ex& ex) {
      /**
       * this exception should not be raised because
       * the CreamJob is created from another valid one
       */
      CREAM_SAFE_LOG(m_log_dev->fatalStream() << method_name
		     << "Fatal error: CreamJob creation failed "
		     << "copying from a valid one!!!"
		     );
      abort();
    } catch(cream_api::soap_proxy::auth_ex& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Cannot purge job [" << jobdesc
		     << "]. Reason is: " << ex.what()
		     );
    } catch(cream_api::cream_exceptions::BaseException& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Cannot purge job [" << jobdesc
		     << "]. Reason is BaseException: " << ex.what()
		     );
    } catch(cream_api::cream_exceptions::InternalException& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Cannot purge job [" << jobdesc
		     << "]. Reason is InternalException: " << ex.what()
		     );
    } catch( std::exception& ex ) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Cannot purge job [" << jobdesc
		     << "]. Reason is an exception: " << ex.what()
		     
		     );
    } catch( ... ) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Cannot purge job [" << jobdesc
		     << "]. Reason is an unknown exception"
		     );
    }

    /**
       The remote purge to the CE went OK OR
       a fault has been raised by the CE
       Let's remove the job from the ICE's database and
       decrement the job counter of the 'super' better
       proxy.
    */
    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		   << "Removing purged job [" << jobdesc
		   << "] from ICE's database"
		   );
    if(theJob.is_proxy_renewable())
      ice_util::DNProxyManager::getInstance()->decrementUserProxyCounter( theJob.get_user_dn(), theJob.get_myproxy_address() );
    {
      db::RemoveJobByGid remover( _gid, "Ice::purge_job" );
      db::Transaction tnx(false, false);
      tnx.execute( &remover );
    }
}


//____________________________________________________________________________
void Ice::deregister_proxy_renewal( const ice_util::CreamJob& job ) throw()
{
  string jobdesc( job.describe() );
    if ( !::getenv( "ICE_DISABLE_DEREGISTER") ) {
        // must deregister proxy renewal
        int      err = 0;
        
        CREAM_SAFE_LOG(
                       m_log_dev->infoStream()
                       << "Ice::deregister_proxy_renewal() - "
                       << "Unregistering Proxy for job ["
                       << jobdesc << "]"
                       );
        
        err = glite_renewal_UnregisterProxy( job.get_grid_jobid().c_str(), NULL );
        
        if ( err && (err != EDG_WLPR_PROXY_NOT_REGISTERED) ) {
	    const char* errmex = edg_wlpr_GetErrorText(err);
            CREAM_SAFE_LOG(
                           m_log_dev->errorStream()
                           << "Ice::deregister_proxy_renewal() - "
                           << "ICE cannot unregister the proxy " 
                           << "for job [" << jobdesc
                           << "]. Reason: \"" << errmex
                           << "\"."
                           );
        } else {
            if ( err == EDG_WLPR_PROXY_NOT_REGISTERED ) {
                CREAM_SAFE_LOG(
                               m_log_dev->warnStream()
                               << "Ice::deregister_proxy_renewal() - "
                               << "Job proxy not registered for job ["
                               << jobdesc 
                               << "]. Going ahead." 
                               );
            }
        }
    } else {
        CREAM_SAFE_LOG(
                       m_log_dev->warnStream()
                       << "Ice::deregister_proxy_renewal() - "
                       << "Proxy unregistration disable. To reenable, " 
                       << "unset the environment variable ICE_DISABLE_DEREGISTER"
                       );
    }
}


//____________________________________________________________________________
void Ice::purge_wms_storage( const ice_util::CreamJob& job ) throw()
{

  string jobdesc( job.describe() );
    if ( !::getenv( "ICE_DISABLE_PURGER" ) ) {
        try {
            CREAM_SAFE_LOG(
                           m_log_dev->infoStream()
                           << "Ice::purge_wms_storage() - "
                           << "Purging storage for job ["
                           << jobdesc << "]"
                           
                           );
#ifdef HAVE_GLITE_JOBID
            glite::jobid::JobId j_id( job.get_grid_jobid() );
#else        
            glite::wmsutils::jobid::JobId j_id( job.get_grid_jobid() );
#endif
            wms::purger::Purger the_purger( ice_util::iceConfManager::getInstance()->getConfiguration()->common()->lbproxy() );
            the_purger(j_id);

        } catch( std::exception& ex ) {
            CREAM_SAFE_LOG(
                           m_log_dev->errorStream()
                           << "Ice::purge_wms_storage() - "
                           << "Cannot purge storage for job ["
                           << jobdesc
                           << "]. Reason is: " << ex.what()
                           
                           );
            
        }
    } else {
        CREAM_SAFE_LOG(
                       m_log_dev->warnStream()
                       << "Ice::purge_wms_storage() - "
                       << "WMS job purger disabled in ICE. To reenable "
                       << "unset the environment variable ICE_DISABLE_PURGER"
                       
                       );
    }
}

//____________________________________________________________________________
//ice_util::jobCache::iterator 
bool Ice::resubmit_or_purge_job( util::CreamJob& tmp_job )
throw() 
{
  cream_api::job_statuses::job_status st = tmp_job.get_status();

  if ( cream_api::job_statuses::CANCELLED == st ||
       cream_api::job_statuses::DONE_OK == st ) {
    
    deregister_proxy_renewal( tmp_job );
    
  }
  if ( ( cream_api::job_statuses::DONE_FAILED == st ||
	 cream_api::job_statuses::ABORTED == st ) &&
       !tmp_job.is_killed_by_ice() ) {
    
    resubmit_job( tmp_job, "Job resubmitted by ICE" );
    
  }        
  if ( cream_api::job_statuses::DONE_OK == st ||
       cream_api::job_statuses::CANCELLED == st ||
       cream_api::job_statuses::DONE_FAILED == st ||
       cream_api::job_statuses::ABORTED == st ) {
    // WARNING: the next line removes the job from the job cache!
    
    purge_job( /*it*/tmp_job, "Job purged by ICE" );
    
    return true; // notify to the caller (EventQuery that the job has been removed
                 // from ICE's database

  }
  if ( cream_api::job_statuses::CANCELLED == st ) {
    
    purge_wms_storage( tmp_job );
    
  }
  return false;
}

