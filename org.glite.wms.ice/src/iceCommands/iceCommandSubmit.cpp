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
 * Council for the Central Laboratory of the Research Councils (CCLRfC), United Kingdom
 *
 * ICE command submit class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// Local includes
#include "iceCommandSubmit.h"
#include "subscriptionManager.h"
#include "subscriptionProxy.h"
#include "DNProxyManager.h"
#include "Delegation_manager.h"
#include "iceConfManager.h"
#include "iceSubscription.h"
#include "creamJob.h"
#include "ice-core.h"
#include "eventStatusListener.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "CreamProxyMethod.h"
#include "Request.h"
#include "Request_source_purger.h"
#include "iceUtils.h"
#include "eventStatusPoller.h" // for the proxymutex

#include "iceDb/RemoveJobByGid.h"
#include "iceDb/GetFields.h"
#include "iceDb/UpdateJobByGid.h"
#include "iceDb/Transaction.h"
#include "iceDb/CreateJob.h"

/**
 *
 * Cream Client API Headers
 *
 */
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/JobIdWrapper.h"
#include "glite/ce/cream-client-api-c/AbsCreamProxy.h"
#include "glite/ce/cream-client-api-c/ResultWrapper.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/ce/cream-client-api-c/JobDescriptionWrapper.h"
#include "glite/ce/cream-client-api-c/scoped_timer.h"

#include "glite/wms/common/utilities/scope_guard.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "Lease_manager.h"

// Boost stuff
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"
#include "boost/format.hpp"

// C++ stuff
#include <ctime>

using namespace std;
namespace ceurl_util    = glite::ce::cream_client_api::util::CEUrl;
namespace cream_api     = glite::ce::cream_client_api::soap_proxy;
namespace api_util      = glite::ce::cream_client_api::util;
namespace wms_utils     = glite::wms::common::utilities;
namespace iceUtil       = glite::wms::ice::util;
namespace configuration = glite::wms::common::configuration;

using namespace glite::wms::ice;

boost::recursive_mutex iceCommandSubmit::s_localMutexForSubscriptions;

//
//
//____________________________________________________________________________
namespace { // Anonymous namespace
    
    // 
    // This class is used by a scope_guard to delete a job from the
    // job cache if something goes wrong during the submission.
    //
    class remove_job_from_cache {
    protected:
        const std::string m_grid_job_id;
        
    public:
        /**
         * Construct a remove_job_from_cache object which will remove
         * the job with given grid_job_id from the cache.
         */
        remove_job_from_cache( const std::string& grid_job_id ) :
            m_grid_job_id( grid_job_id )
        { };
        /**
         * Actually removes the job from cache. If the job is no
         * longer in the job cache, nothing is done.
         */
        void operator()( void ) {
	 
	  db::RemoveJobByGid remover( m_grid_job_id, "remove_job_from_cache::operator()" );

	  db::Transaction tnx(false, false);

  tnx.execute( &remover );

        }
    };       
    
}; // end anonymous namespace

//
//
//____________________________________________________________________________
iceCommandSubmit::iceCommandSubmit( iceUtil::Request* request, 
				    const iceUtil::CreamJob& aJob )
  : iceAbsCommand( "iceCommandSubmit" ),
    m_theIce( Ice::instance() ),
    m_myname( m_theIce->getHostName() ),
    m_theJob( aJob ),
    m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
    m_configuration( iceUtil::iceConfManager::getInstance()->getConfiguration() ),
    m_lb_logger( iceUtil::iceLBLogger::instance() ),
    m_request( request )  
{

  if( m_configuration->ice()->listener_enable_authn() ) {
    m_myname_url = boost::str( boost::format("https://%1%:%2%") % m_myname % m_configuration->ice()->listener_port() );
  } else {
    m_myname_url = boost::str( boost::format("http://%1%:%2%") % m_myname % m_configuration->ice()->listener_port() );   
  }

}

//
//
//____________________________________________________________________________
void iceCommandSubmit::execute( void ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
  
    static const char* method_name="iceCommandSubmit::execute() - ";

    CREAM_SAFE_LOG(
                   m_log_dev->infoStream()
                   << method_name
                   << "This request is a Submission..."
                   );   
    
    Request_source_purger purger_f( m_request );
    wms_utils::scope_guard remove_request_guard( purger_f );

    // We start by checking whether the job already exists in ICE
    // cache.  This may happen if ICE already tried to submit the job,
    // but crashed before removing the request from the filelist. If
    // we find the job already in ICE cache, we simply give up
    // (logging an information message), and the purge_f object will
    // take care of actual removal.
    string _gid( m_theJob.get_grid_jobid() );
    bool only_start = false;
    
    {
      list<string> fields_to_retrieve;
      fields_to_retrieve.push_back( "status" );
      list<pair<string, string> > clause;
      clause.push_back( make_pair("gridjobid",_gid) );
      list< vector<string> > results;
      
      db::GetFields getter(fields_to_retrieve, clause, results, "iceCommandSubmit::execute", false );
      db::Transaction tnx(false, false);
      tnx.execute( &getter );
      
      if( results.size() ) {
        int _status( atoi(results.begin()->at(0).c_str()) );
	
	switch( _status ) {
	case glite::ce::cream_client_api::job_statuses::UNKNOWN:
	  // ICE has been restarted/crahsed before JobRegister and after 
	  // new DB entry creation. Let's remove this entry and 
	  // regularly submit the job.
	  {
	    db::RemoveJobByGid remover( _gid, "iceCommandSubmit::execute" );
	    db::Transaction tnx(false, false);
	    tnx.execute( &remover );
	  }
	  break;
	case glite::ce::cream_client_api::job_statuses::REGISTERED:
	  // ICE has been restarted/crashed after a JobRegister and before a JobStart
	  // MUST ONLY start this job.
	  only_start = true;
	  break;
	default:
	  // ICE has been restarted/crashed after a JobStart has finished
	  // we shall ignore the request
	  CREAM_SAFE_LOG( m_log_dev->warnStream()
			<< method_name
			<< "Submit request for job GridJobID=["
			<< _gid
			<< "] is related to a job already in ICE's database that has been already submitted. "
			<< "Removing the request and going ahead."
			);
	  return;
	  break;
	}// switch
	
      }
    } 


    // This must be left AFTER the above code. The remove_job_guard
    // object will REMOVE the job from the cache when being destroied.
    remove_job_from_cache remove_f( _gid );
    wms_utils::scope_guard remove_job_guard( remove_f );

    /**
     * We now try to actually submit the job. Any exception raised by
     * the try_to_submit() method is catched, and triggers the
     * appropriate actions (logging to LB and resubmitting).
     */       
    if( !only_start ) //here only if the job is UNKNOWN. In this case it has been removed from DB (see above)
    {

      // Must set a mutex to not allow the eventStatusPoller 
      // to retrieve couples dn,ce before
      // a valid entry has been inserted into the 'proxy'
      // table
      //boost::recursive_mutex::scoped_lock proxy( glite::wms::ice::util::eventStatusPoller::s_proxymutex );
      {
	db::CreateJob creator( m_theJob, "iceCommandSubmit::execute" );
	db::Transaction tnx(false, false);
	tnx.execute( &creator );
      }
      // now the job is in cache and has been registered we can save its
      // proxy into the DN-Proxy Manager's cache
      if( !m_theJob.is_proxy_renewable() ) {
	iceUtil::DNProxyManager::getInstance()->setUserProxyIfLonger_Legacy( m_theJob.get_user_dn(), 
									     m_theJob.get_user_proxy_certificate(), 
									     m_theJob.get_isbproxy_time_end()
									     /*V.getProxyTimeEnd()*/ );
      } else {
	
	/**
	   MUST increment job counter of the 'super' better proxy table.
	*/
	iceUtil::DNProxyManager::getInstance()->incrementUserProxyCounter(m_theJob, m_theJob.get_isbproxy_time_end() );
      }
    } // releases eventStatusPoller's proxymutex
 
    try {
        try_to_submit( only_start );        
    } catch( const iceCommandFatal_ex& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream() 
                       << method_name 
                       << "Error during submission of jdl=" << m_jdl
                       << " Fatal Exception is:" << ex.what()
                       );
        m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, boost::str( boost::format( "Transfer to CREAM failed due to exception: %1%") % ex.what() ) ) );
	string reason = boost::str( boost::format( "Transfer to CREAM [%1%] failed due to exception: %2%") % m_theJob.get_creamurl() % ex.what() );
        m_theJob.set_failure_reason( reason );
	{
	  list< pair<string, string> > params;
	  params.push_back( make_pair("failure_reason", reason) );
	  db::UpdateJobByGid updater( _gid, params, "iceCommandSubmit::execute" );
	  db::Transaction tnx(false, false);
	  tnx.execute( &updater );
	}

        m_theJob = m_lb_logger->logEvent( new iceUtil::job_aborted_event( m_theJob ) );
        throw( iceCommandFatal_ex( /*ex.what()*/ reason ) );
    } catch( const iceCommandTransient_ex& ex ) {

        // The next event is used to show the failure reason in the
        // status info JC+LM log transfer-fail / aborted in case of
        // condor transfers fail
      string reason = boost::str( boost::format( "Transfer to CREAM failed due to exception: %1%" ) % ex.what() );
      m_theJob.set_failure_reason( reason );
      {
	list< pair<string, string> > params;
	params.push_back( make_pair("failure_reason", reason) );
	db::UpdateJobByGid updater( _gid, params, "iceCommandSubmit::execute" );
	db::Transaction tnx(false, false);
	tnx.execute( &updater );
      }
        m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, ex.what()  ) );
        m_theJob = m_lb_logger->logEvent( new iceUtil::job_done_failed_event( m_theJob ) );
        m_theIce->resubmit_job( m_theJob, boost::str( boost::format( "Resubmitting because of exception %1% CEUrl [%2%]" ) % ex.what() % m_theJob.get_creamurl() ) ); // Try to resubmit
        throw( iceCommandFatal_ex( string("Error submitting job to CE [") + m_theJob.get_creamurl() + "]: " + ex.what() ) ); // Yes, we throw an iceCommandFatal_ex in both cases

    }
    
    remove_job_guard.dismiss(); // dismiss guard, job will NOT be removed from cache
}

//
//
//______________________________________________________________________________
void iceCommandSubmit::try_to_submit( const bool only_start ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
  
  string _gid( m_theJob.get_grid_jobid() );
  
  static const char* method_name = "iceCommandSubmit::try_to_submit() - ";
  /**
   * Retrieve all usefull cert info.  In order to make the userDN an
   * index in the BDb's secondary database it must be available
   * already at the first put.  So the following block of code must
   * remain here.
   */
  
  string dbid, completeid, jobId, __creamURL, jobdesc;
  
//   cream_api::VOMSWrapper V( m_theJob.get_user_proxy_certificate() );
//   if( !V.IsValid( ) ) {
//     throw( iceCommandTransient_ex( "Authentication error: " + V.getErrorMessage() ) );
//   }
  
//   time_t proxy_time_end( V.getProxyTimeEnd() );
//   m_theJob.set_userdn( V.getDNFQAN() );

  if( m_theJob.get_isbproxy_time_end() <= time(0) ) {
    throw( iceCommandTransient_ex( "Authentication error: proxyfile [" 
				   + m_theJob.get_user_proxy_certificate() 
				   + "] is expired! Skipping submission of the job [" 
				   + m_theJob.get_grid_jobid() +"]"));
  }

  if(!only_start) {    
    
    {
      list< pair<string, string> > params;
      params.push_back( make_pair("userdn", m_theJob.get_user_dn()) );
      db::UpdateJobByGid updater( _gid, params, "iceCommandSubmit::try_to_submit" );
      db::Transaction tnx(false, false);
      tnx.execute( &updater );
    }
    
    //proxy_time_end = V.getProxyTimeEnd();
    
    m_theJob = m_lb_logger->logEvent( new iceUtil::wms_dequeued_event( m_theJob, m_configuration->ice()->input() ) );
    m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_start_event( m_theJob ) );
    
    
    
    string _ceurl( m_theJob.get_creamurl() );
    
    CREAM_SAFE_LOG(
                   m_log_dev->debugStream() 
                   << method_name << "Submitting JDL " 
		   << m_theJob.get_modified_jdl() << " to [" 
                   << _ceurl <<"] ["
                   << m_theJob.get_cream_delegurl() << "]"
                   );
    
    jobdesc = m_theJob.describe();
    
    CREAM_SAFE_LOG(
                   m_log_dev->debugStream()
                   << method_name
                   << "Sequence code for job ["
                   << jobdesc
                   << "] is "
                   << m_theJob.get_sequence_code()
                   );
    
    bool is_lease_enabled = ( m_configuration->ice()->lease_delta_time() > 0 );
    string  delegation;
    string lease_id; // empty delegation id
    bool force_delegation = false;
    bool force_lease = false;  
    bool retry = true;  
    cream_api::AbsCreamProxy::RegisterArrayResult res;        
    
    while( retry ) {
      //
      // Manage lease creation
      //
      if ( is_lease_enabled ) {
	
	process_lease( force_lease, jobdesc, _gid, lease_id );
        
      }
      
      // 
      // Delegate the proxy
      //
      
      handle_delegation( delegation, force_delegation, jobdesc, _gid, _ceurl );
      
      //
      // Registers the job (without autostart)
      //
      if( !register_job( is_lease_enabled,
			 jobdesc,
			 _gid,
			 delegation,
			 lease_id,
			 m_theJob.get_modified_jdl(),
			 force_delegation,
			 force_lease,
			 res) )
	{
	  continue;
	}
      
      process_result( retry, force_delegation, force_lease, is_lease_enabled, _gid, res );
      
    } // end while( retry )
    
    m_theJob.set_delegation_id( delegation );
    m_theJob.set_lease_id( lease_id );
    
    map< string, string > props;
    res.begin()->second.get<1>().getProperties( props );
    dbid       = props["DB_ID"];
    jobId      = res.begin()->second.get<1>().getCreamJobID();
    __creamURL = res.begin()->second.get<1>().getCreamURL();
    
    completeid = __creamURL;
    
    boost::replace_all( completeid, m_configuration->ice()->cream_url_postfix(), "" );
    
    completeid += "/" + jobId;
    
    // FIXME: should we check that __creamURL ==
    // m_theJob.getCreamURL() ?!?  If it is not it's VERY severe
    // server error, and I think it is not our businness
    
    CREAM_SAFE_LOG( m_log_dev->infoStream() << method_name
                    << "For GridJobID [" << _gid << "]" 
                    << " CREAM Returned CREAM-JOBID [" << completeid <<"] DB_ID ["
		    << dbid << "]"
		    );
    
    // TODO: put creamjobid and complete cream jobid and REGISTERED into database
    {
      list< pair<string, string> > fields_to_update;
      fields_to_update.push_back( make_pair( "creamjobid", jobId) );
      fields_to_update.push_back( make_pair( "complete_cream_jobid", completeid) );
      fields_to_update.push_back( make_pair( "creamurl", __creamURL) );
      fields_to_update.push_back( make_pair( "dbid", dbid) );
      fields_to_update.push_back( make_pair( "status", iceUtil::int_to_string(glite::ce::cream_client_api::job_statuses::REGISTERED) ) );
      fields_to_update.push_back( make_pair( "delegationid", delegation ) );
      fields_to_update.push_back( make_pair( "leaseid", lease_id) );
      db::UpdateJobByGid updater(_gid, fields_to_update, "iceCommandSubmit::try_to_submit");
      db::Transaction tnx( false, false );
      tnx.execute( &updater );
    }
    
  } // if(!only_start)
  else {
    
    // if the job is ONLY to start, means
    // that in the DB the needed info to start it are
    // there. Let's retrieve them...
    {
      list<string> fields_to_retrieve;
      fields_to_retrieve.push_back( "complete_cream_jobid" );
      fields_to_retrieve.push_back( "creamjobid" );
      fields_to_retrieve.push_back( "creamurl" );
      fields_to_retrieve.push_back( "dbid" );
      
      list< pair<string, string> > clause;
      clause.push_back( make_pair( "gridjobid", _gid ) );
      list<vector<string> > results;
      db::GetFields getter( fields_to_retrieve, clause, results, "iceCommandSubmit::try_to_submit", false);
      db::Transaction tnx( false, false );
      tnx.execute( &getter );
      completeid = results.begin()->at(0);
      jobId      = results.begin()->at(1);
      __creamURL = results.begin()->at(2);
      dbid       = results.begin()->at(3);
    }
    
    // completeid, __creamURL, proxy_time_end, dbid, jobId, jobdesc
    
    CREAM_SAFE_LOG( m_log_dev->infoStream() << method_name
                    << "GridJobID [" << _gid << "]" 
                    << " has already been REGISTERED. Will only START it..."
		    );

  } // else -> if(!only_start)
  
  cream_api::ResultWrapper startRes;
  
  try {
    CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
		    << "Going to START CreamJobID ["
		    << completeid <<"] related to GridJobID ["
		    << _gid << "]..."
		    );
    
    // FIXME: must create the request JobFilterWrapper for the Start operation
    vector<cream_api::JobIdWrapper> toStart;
    toStart.push_back( cream_api::JobIdWrapper( (const string&)jobId, 
						(const string&)__creamURL, 
						vector<cream_api::JobPropertyWrapper>()
						)
		       );
    
    cream_api::JobFilterWrapper jw(toStart, vector<string>(), -1, -1, "", "");
    
    iceUtil::CreamProxy_Start( __creamURL, 
			       m_theJob.get_user_proxy_certificate(), 
			       (const cream_api::JobFilterWrapper *)&jw, 
			       &startRes ).execute( 7 );
  } catch( exception& ex ) {
    throw iceCommandTransient_ex( boost::str( boost::format( "CREAM Start raised exception %1%") % ex.what() ) );
  }
  
  // FIXME: Very orrible. 
  // Must look for a better and more elegant way for doing the following
  // the following code accumulate all the results in tmp list
  list<pair<cream_api::JobIdWrapper, string> > tmp;
  startRes.getNotExistingJobs( tmp );
  if(tmp.empty())
    startRes.getNotMatchingStatusJobs( tmp );
  if(tmp.empty())
    startRes.getNotMatchingDateJobs( tmp );
  if(tmp.empty())
    startRes.getNotMatchingProxyDelegationIdJobs( tmp );
  if(tmp.empty())
    startRes.getNotMatchingLeaseIdJobs( tmp );
  
  // It is sufficient look for "empty-ness" because
  // we've started only one job
  if(!tmp.empty()) {
    pair<cream_api::JobIdWrapper, string> wrong = *( tmp.begin() ); // we trust there's only one element because we've started ONLY ONE job
    string errMex = wrong.second;
    
    CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		   << "Cannot start job [" << jobdesc
		   << "]. Reason is: " << errMex
		   );
    
    throw iceCommandTransient_ex( boost::str( boost::format( "CREAM Start failed due to error %1%") % errMex ) );
  }
  
  // no failure: put jobids and status in cache
  // and remove last request from WM's filelist
  
  
  m_theJob.set_cream_jobid( jobId );
  m_theJob.set_status(glite::ce::cream_client_api::job_statuses::PENDING);    
  // SET ABOVE... m_theJob.set_delegation_id( delegation );
  //m_theJob.set_delegation_expiration_time( delegation.get<1>() );
  //    m_theJob.set_delegation_duration( delegation.get<2>() );
  // SET ABOVE m_theJob.set_lease_id( lease_id ); // FIXME: redundant??
  //m_theJob.set_proxycert_mtime( time(0) ); // FIXME: should be the modification time of the proxy file?
  m_theJob.set_wn_sequencecode( m_theJob.get_sequence_code() );
  
  //if(!only_start)
  //{
  list< pair<string, string> > params;
  params.push_back( make_pair("creamjobid", jobId) );
  params.push_back( make_pair("complete_cream_jobid", m_theJob.get_complete_cream_jobid() ) );
  params.push_back( make_pair("status", iceUtil::int_to_string( glite::ce::cream_client_api::job_statuses::PENDING )));
  params.push_back( make_pair("delegationid", m_theJob.get_delegation_id() ));
  params.push_back( make_pair("leaseid", m_theJob.get_lease_id() ));
  params.push_back( make_pair("wn_sequence_code", m_theJob.get_sequence_code() ));
  params.push_back( make_pair("dbid", dbid ) );
  db::UpdateJobByGid updater( _gid , params,"iceCommandSubmit::try_to_submit" );
  db::Transaction tnx(false, false);
  tnx.execute( &updater );
  //     } else {
  //       list< pair<string, string> > params;
  //       params.push_back( make_pair("status", iceUtil::int_to_string( glite::ce::cream_client_api::job_statuses::PENDING )));
  //       params.push_back( make_pair("wn_sequence_code", m_theJob.get_sequence_code() ));
  //       db::UpdateJobByGid updater( _gid , params,"iceCommandSubmit::try_to_submit" );
  //       db::Transaction tnx(false, false);
  //       tnx.execute( &updater );
  //     }
  
  m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_ok_event( m_theJob ) );
  
  /*
   * here must check if we're subscribed to the CEMon service
   * in order to receive the status change notifications
   * of job just submitted. But only if listener is ON
   */
  if( m_theIce->is_listener_started() ) {	
    doSubscription( m_theJob );	
  }
  
  {
    m_theJob.set_last_seen( time(0) );
    list< pair<string, string> > params;
    params.push_back( make_pair("last_seen", iceUtil::int_to_string(m_theJob.get_last_seen())));
    db::UpdateJobByGid updater( _gid, params, "iceCommandSubmit::try_to_submit" ); 
    db::Transaction tnx(false, false);
    tnx.execute( &updater );
  }
} // try_to_submit



//______________________________________________________________________________
void  iceCommandSubmit::doSubscription( const iceUtil::CreamJob& aJob )
{
  static const char* method_name = "iceCommandSubmit::doSubscription() - ";
  boost::recursive_mutex::scoped_lock cemonM( s_localMutexForSubscriptions );
  
  string cemon_url;
  iceUtil::subscriptionManager* subMgr( iceUtil::subscriptionManager::getInstance() );

  subMgr->getCEMonURL( aJob.get_user_proxy_certificate(), aJob.get_creamurl(), cemon_url ); // also updated the internal subMgr's cache cream->cemon
  
  CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                  << "For current CREAM, subscriptionManager returned CEMon URL ["
                  << cemon_url << "]"
                  
                  );
  
  // Try to determine if the current user userDN is subscribed to 'cemon_url' by
  // asking the cemonUrlCache
  
  bool foundSubscription;

  foundSubscription = subMgr->hasSubscription( aJob.get_user_proxy_certificate(), cemon_url );
  
  if ( foundSubscription )
    {
      // if this was a ghost subscription (i.e. it does exist in the cemonUrlCache's cache
      // but not actually in the CEMon
      // the subscriptionUpdater will fix it soon
      CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		     << "User [" << aJob.get_user_dn() << "] is already subsdcribed to CEMon ["
		     << cemon_url << "] (found in subscriptionManager's cache)"
		     );
      
      return;
    }	   
  
  
  
  
  // try to determine if the current user userProxy is subscribed to 'cemon_url'
  // with a direct SOAP query to CEMon (can block for SOAP_TIMETOUT seconds,
  // but executed only the first time)
  
  bool subscribed;
  iceUtil::iceSubscription sub;
  
  try {
    
    subscribed = iceUtil::subscriptionProxy::getInstance()->subscribedTo( aJob.get_user_proxy_certificate(), cemon_url, sub );
    
  } catch(exception& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                     << "Couldn't determine if user [" << aJob.get_user_dn() 
                     << "] is subscribed to [" << cemon_url 
                     << "]. Another job could trigger a successful subscription."
                     );
      return;
  }
  
  // OK: we're definitely subscribed to this CEMon with userProxy proxyfile, but the cached information is lost
  // for some reason (e.g. ICE has been recently re-started).
  
  if( subscribed ) {
    if( m_configuration->ice()->listener_enable_authz() ) {
      // If AUTHZ is ON, before caching the current CEMon to the list
      // of CEMons we're subscribed to, we must ask ot its DN that is 
      // an important information to cache in oder to authorize
      // notifications coming from this CEMon.
      string DN;

      if( subMgr->getCEMonDN( aJob.get_user_proxy_certificate(), cemon_url, DN ) ) {
	    
	subMgr->insertSubscription( aJob.get_user_proxy_certificate(), cemon_url, sub );
	//dnprxMgr->setUserProxyIfLonger( aJob.getUserDN(), aJob.getUserProxyCertificate() );
	
      } else {
          CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		       << "CEMon [" << cemon_url 
		       << "] reported that we're already subscribed to it, "
		       << "but couldn't get its DN. "
		       << "Will not authorize its job status "
		       << "notifications."
		       );
	return; 
      }
    } 
    else {

      subMgr->insertSubscription( aJob.get_user_proxy_certificate(), cemon_url, sub );

      //dnprxMgr->setUserProxyIfLonger( aJob.getUserDN(), aJob.getUserProxyCertificate() );

    }

    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		   << "User DN [" << aJob.get_user_dn() << "] is already subscribed to CEMon ["
		   << cemon_url << "] (asked to CEMon itself)"
		   );
  } else {
    // MUST subscribe
    
    bool can_subscribe = true;
    string DN;
    if ( m_configuration->ice()->listener_enable_authz() ) {
      if( !subMgr->getCEMonDN( aJob.get_user_proxy_certificate(), cemon_url, DN ) ) {
	// Cannot subscribe to a CEMon without it's DN
	can_subscribe = false;
	CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		       << "Notification authorization is enabled and couldn't "
		       << "get CEMon's DN. Will not subscribe to it."
		       );
      } 
    }
    
    if(can_subscribe) {
      iceUtil::iceSubscription sub;
      if( iceUtil::subscriptionProxy::getInstance()->subscribe( aJob.get_user_proxy_certificate(), cemon_url, sub ) ) {
	{
	  subMgr->insertSubscription( aJob.get_user_proxy_certificate(), cemon_url, sub );
	}
      } else {
          CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
		       << "Couldn't subscribe to [" 
		       << cemon_url << "] with userDN [" << aJob.get_user_dn()<< "]. Will not"
		       << " receive job status notification from it for this user. "
		       << "Hopefully the subscriptionUpdater will retry."
		       );
      }
    } // if(can_subscribe)
  } // else -> if(subscribedTo...)
  
} // end function, also unlocks the iceCommandSubmit's s_localMutexForSubscription mutex

//______________________________________________________________________________
void iceCommandSubmit::process_lease( const bool force_lease,
				      const std::string& jobdesc,
				      const std::string& _gid,
				      string& lease_id )
  throw( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
  const char* method_name = "iceCommandSubmit::process_lease() - ";

  if ( force_lease ) {
    CREAM_SAFE_LOG( m_log_dev->infoStream() << method_name
		    << "Lease is enabled, enforcing creation of a new lease "
		    << "for job [" << jobdesc << "]"
		    );        
  } else {
    CREAM_SAFE_LOG( m_log_dev->infoStream() << method_name
		    << "Lease is enabled, trying to get lease "
		    << "for job [" << jobdesc << "]"
		    );        
  }
  
  //
  // Get a (possibly existing) lease ID
  //
  try{
    lease_id = iceUtil::Lease_manager::instance()->make_lease( m_theJob, force_lease );
  } catch( const std::exception& ex ) {
    // something was wrong with the lease creation step. 
    string err_msg( boost::str( boost::format( "Failed to get lease_id for job %1%. Exception is %2%" ) % _gid % ex.what() ) );
    
    CREAM_SAFE_LOG( m_log_dev->errorStream()
		    << method_name << err_msg );
    throw( iceCommandTransient_ex( err_msg ) );

  } catch( ... ) {

    string err_msg( boost::str( boost::format( "Failed to get lease_id for job %1% due to unknown exception" ) % _gid ) );
    CREAM_SAFE_LOG( m_log_dev->errorStream()
		    << method_name << err_msg );
    throw( iceCommandTransient_ex( err_msg ) );

  }
  
  // lease creation OK
  CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
		  << "Using lease ID " << lease_id << " for job ["
		  << jobdesc << "]"
		  );
}

//______________________________________________________________________________
void iceCommandSubmit::handle_delegation( string& delegation,
					  bool& force_delegation,
					  const string& jobdesc,
					  const string& _gid,
					  const string& _ceurl)
  throw( iceCommandTransient_ex& )
{
  const char* method_name = "iceCommandSubmit::handle_delegation() - ";
  boost::tuple<string, time_t, long long int> SBP;

  if( m_theJob.is_proxy_renewable() ) {
    SBP = iceUtil::DNProxyManager::getInstance()->getExactBetterProxyByDN( m_theJob.get_user_dn(), m_theJob.get_myproxy_address());
    
    if( SBP.get<0>() == "" ) {
      /**
	 NO SuperBetterProxy for DN.
	 must use that one in the ISB as SBP
      */
      CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
		      << "Setting new better proxy for userdn ["
		      << m_theJob.get_user_dn() << "] MyProxy server ["
		      << m_theJob.get_myproxy_address() << "] Job ["
		      << jobdesc 
		      << "]"
		      ); 
      iceUtil::DNProxyManager::getInstance()->setBetterProxy( m_theJob.get_user_dn(), 
							      m_theJob.get_user_proxy_certificate(),
							      m_theJob.get_myproxy_address(),
							      m_theJob.get_isbproxy_time_end(),
							      (unsigned long long)0);
      force_delegation = true;
      
    } else {
      /**
	 The SBP already exists. Let's check if the ISB one is more
	 long-living.
      */
      if( m_theJob.get_isbproxy_time_end() > SBP.get<1>() ) {
	boost::tuple<string, time_t, long long int> newPrx = boost::make_tuple( m_theJob.get_user_proxy_certificate(), m_theJob.get_isbproxy_time_end(), SBP.get<2>() );
	CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
			<< "Updating better proxy for userdn ["
			<< m_theJob.get_user_dn() << "] MyProxy server ["
			<< m_theJob.get_myproxy_address() << "] Job ["
			<< jobdesc
			<< "] Proxy Expiration time ["
			<< newPrx.get<1>() << "] Counter ["
			<< newPrx.get<2>()
			<< "] because this one is more long-living..."
			); 
	iceUtil::DNProxyManager::getInstance()->updateBetterProxy( m_theJob.get_user_dn(),
								   m_theJob.get_myproxy_address(),
								   newPrx );
	
      }
      /** 
	  Now check the duration of related delegation
      */
      iceUtil::Delegation_manager::table_entry deleg;
      deleg = iceUtil::Delegation_manager::instance()->getDelegation(m_theJob.get_user_dn(),
								     _ceurl,
								     m_theJob.get_myproxy_address()
								     );
      if( deleg.m_expiration_time < time(0) + 2*iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->proxy_renewal_frequency() ) // this means that the ICE's delegation renewal has failed. In fact it try to renew much before the exp time of the delegation itself.
	force_delegation = true;
      
    }
  }
  
  try {
    delegation = iceUtil::Delegation_manager::instance()->delegate( m_theJob, m_theJob.is_proxy_renewable(), force_delegation );
  } catch( const exception& ex ) {
    throw( iceCommandTransient_ex( boost::str( boost::format( "Failed to create a delegation id for job %1%: reason is %2%" ) % _gid % ex.what() ) ) );
  }
}

//______________________________________________________________________________
bool iceCommandSubmit::register_job( const bool is_lease_enabled,
				     const string& jobdesc,
				     const string& _gid,
				     const string& delegation,
				     const string& lease_id,
				     const string& modified_jdl,
				     bool& force_delegation,
				     bool& force_lease,
				     cream_api::AbsCreamProxy::RegisterArrayResult& res)
  throw( iceCommandTransient_ex&)
{
  const char* method_name = "iceCommandSubmit::register_job() - ";

  try {
    
    CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
		    << "Going to REGISTER Job ["
		    << jobdesc << "] with delegation ID ["
		    << delegation << "] to CREAM [" << m_theJob.get_creamurl()
		    << "]..."
		    );
    
    cream_api::AbsCreamProxy::RegisterArrayRequest req;
    
    // FIXME: must check what to set the 3rd and 4th arguments
    // (delegationProxy, leaseID) last asrgument is irrelevant
    // now, because we register jobs one by one
    cream_api::JobDescriptionWrapper jd(modified_jdl, 
					delegation,
					"" /* delegPRoxy */, 
					lease_id /* leaseID */, 
					false, /* NO autostart */
					"foo");
    
    req.push_back( &jd );
    
    string iceid = m_theIce->getHostDN();
    boost::trim_if(iceid, boost::is_any_of("/"));
    boost::replace_all( iceid, "/", "_" );
    boost::replace_all( iceid, "=", "_" );
    
    string proxy = m_theJob.get_user_proxy_certificate();
    
    iceUtil::CreamProxy_Register( m_theJob.get_creamurl(),
				  proxy,
				  (const cream_api::AbsCreamProxy::RegisterArrayRequest*)&req,
				  &res,
				  iceid).execute( 3 );

  } catch ( glite::ce::cream_client_api::cream_exceptions::GridProxyDelegationException& ex ) {
    // Here CREAM tells us that the delegation ID is unknown.
    // We do not trust this fault, and try to redelegate the
    // _same_ delegation ID, hoping for the best.
    
    iceUtil::Delegation_manager::instance()->redelegate( m_theJob.get_user_proxy_certificate(), m_theJob.get_cream_delegurl(), delegation );
    // no exception is raised, we simply hope for the best
    return false;

  } catch ( glite::ce::cream_client_api::cream_exceptions::DelegationException& ex ) {
    if ( !force_delegation ) {
      CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
		      << "Cannot register GridJobID ["
		      << _gid 
		      << "] due to Delegation Exception: " 
		      << ex.what() << ". Will retry once..."
		      );
      force_delegation = true;
      return false;
    } else {
      throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register raised DelegationException %1%") % ex.what() ) ) ); // Rethrow
    }
  } catch ( glite::ce::cream_client_api::cream_exceptions::GenericException& ex ) {
    if ( is_lease_enabled && !force_lease ) {
      CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
		      << "Cannot register GridJobID ["
		      << _gid 
		      << "] due to Generic Fault: " 
		      << ex.what() << ". Will retry once by enforcing creation of a new lease ID..."
		      );
      force_lease = true;
      return false;//continue;
    } else {
      throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register raised GenericFault %1%") % ex.what() ) ) ); // Rethrow
    }                        
  } catch ( exception& ex ) {
    CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
		    << "Cannot register GridJobID ["
		    << _gid 
		    << "] due to std::exception: " 
		    << ex.what() << "."
		    );
    throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register raised std::exception %1%") % ex.what() ) ) ); // Rethrow
  } catch( ... ) {
    throw( iceCommandTransient_ex( "Unknown exception catched" ) );
  }
  return true;
}

//______________________________________________________________________________
void iceCommandSubmit::process_result( bool& retry, 
				       bool& force_delegation, 
				       bool& force_lease,
				       const bool is_lease_enabled,
				       const string& _gid,
				       const cream_api::AbsCreamProxy::RegisterArrayResult& res )
  throw( iceCommandTransient_ex& )
{
  const char* method_name = "iceCommandSubmit::process_result() - ";

  cream_api::JobIdWrapper::RESULT result = res.begin()->second.get<0>();
  string err = res.begin()->second.get<2>();
  
  switch( result ) {
  case cream_api::JobIdWrapper::OK: // nothing to do
    retry = false;
    break;
  case cream_api::JobIdWrapper::DELEGATIONIDMISMATCH:
  case cream_api::JobIdWrapper::DELEGATIONPROXYERROR:
    if ( !force_delegation ) {
      CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
		      << "Cannot register GridJobID ["
		      << _gid 
		      << "] due to Delegation Error: " 
		      << err << ". Will retry once..."
		      );
      force_delegation = true;
    } else {
      throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register returned delegation error \"%1%\"") % err ) ) );
    }            
    break;
  case cream_api::JobIdWrapper::LEASEIDMISMATCH:
    if ( is_lease_enabled && !force_lease ) {
      CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
		      << "Cannot register GridJobID ["
		      << _gid 
		      << "] due to Lease Error: " 
		      << err << ". Will retry once by enforcing creation of a new lease ID..."
		      );
      force_lease = true;
    } else {
      throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register returned lease id mismatch \"%1%\"") % err ) ) );
    }     
    break;                               
  default:
    CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
		    << "Error while registering GridJobID ["
		    << _gid 
		    << "] due to Error: " 
		    << err
		    );
    throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register returned error \"%1%\"") % err ) ) ); 
  }
}
