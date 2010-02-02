/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */


// ICE Headers
#include "iceCommandStatusPoller.h"
#include "subscriptionManager.h"
#include "iceLBEventFactory.h"
#include "iceLBEventFactory.h"
#include "CreamProxyMethod.h"
#include "iceConfManager.h"
#include "DNProxyManager.h"
#include "iceLBLogger.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "iceLBEvent.h"
#include "ice-core.h"
#include "iceUtils.h"
#include "iceDb/GetFields.h"
#include "iceDb/GetJobByCid.h"
#include "iceDb/InsertStat.h"
#include "iceDb/RemoveJobByCid.h"
#include "iceDb/RemoveJobByGid.h"
#include "iceDb/GetStatusInfoByCompleteCreamJobID.h"
#include "iceDb/Transaction.h"
#include "iceDb/UpdateJobByGid.h"
#include "iceDb/RemoveJobByUserDN.h"
#include "iceDb/GetJobs.h"

// Cream Client API Headers
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
//#include "glite/ce/cream-client-api-c/scoped_timer.h"

// WMS Headers
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/utilities/scope_guard.h"

// BOOST Headers
#include <boost/lexical_cast.hpp>
#include <boost/functional.hpp>

// STL Headers
#include <set>
#include <algorithm>
#include <cstdlib>

namespace cream_api  = glite::ce::cream_client_api;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;
namespace jobstat    = glite::ce::cream_client_api::job_statuses;
namespace wms_utils  = glite::wms::common::utilities;
namespace api_util   = glite::ce::cream_client_api::util;

using namespace glite::wms::ice::util;
using namespace std;

namespace { // begin anonymous namespace
    
  // See iceDb/GetJobsToPoll for definitions of type 'JobToPoll'
    bool insert_condition( const CreamJob& J) {        
      if( J.get_complete_cream_jobid().empty() ) 
	return false;
      else 
	return true;
    }
    
    /**
     * This class is used to remove a bunch of jobs from the job
     * cache.  It should be invoked by a scope_guard object, and is
     * used to remove a bunch of jobs if the polling fails due (i.e.)
     * to an expired user proxy.
     *
     * TODO: Remove this class, as it is not being used right now.
     */
    class remove_bunch_of_jobs {
    protected:
        vector< string > m_cream_job_ids;
        string m_reason;
        iceLBLogger* m_lb_logger;
      //        jobCache* m_cache;
        log4cpp::Category* m_log_dev;
    public:
        /**
         * jobs must be a vector of COMPLETE cream job ids.
         */
        remove_bunch_of_jobs( const vector< string>& jobs ) :
            m_cream_job_ids( jobs ),
            m_reason( "Removed by ICE status poller" ),
            m_lb_logger( iceLBLogger::instance() ),
            //m_cache( 0 /* jobCache::getInstance() */ ),
            m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() )
        { };
        void set_reason( const string& new_reason ) {
            m_reason = new_reason;
        };
        void operator()( void ) {
            static const char* method_name = "remove_bunch_of_jobs::operator() - ";
            vector< string >::const_iterator it;

            for ( it=m_cream_job_ids.begin(); it != m_cream_job_ids.end(); ++it ) {

	      glite::wms::ice::db::GetJobByCid getter( *it, "remove_bunch_of_jobs::operator" );
	      glite::wms::ice::db::Transaction tnx(false, false);
	      //tnx.Begin( );
	      tnx.execute( &getter );
	      //tnx.commit( );
	      if( !getter.found() ) {
		continue; // nothing to do
	      }

	      CreamJob job( getter.get_job() );
	      CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
			     << "Removing job " << job.describe()
			     << " from the database. Reason is: "
			     << m_reason);
                job.set_failure_reason( m_reason );
		string _gid( job.get_grid_jobid() );
		{
		  list<pair<string, string> > params;
		  params.push_back( make_pair("failure_reason", job.get_failure_reason() ) );
		  glite::wms::ice::db::UpdateJobByGid updater( _gid, params, "remove_bunch_of_jobs::operator" );
		  //glite::wms::ice::db::Transaction tnx;
		  //tnx.begin_exclusive( );
		  tnx.execute( &updater );
		}
                m_lb_logger->logEvent( new job_aborted_event( job ) ); // ignore return value, the job will be removed from ICE cache anyway
                //m_cache->erase( j );
		{
		  glite::wms::ice::db::RemoveJobByGid remover( _gid, "remove_bunch_of_jobs::operator" );
		  //glite::wms::ice::db::Transaction tnx;
		  //tnx.begin_exclusive( );
		  tnx.execute( &remover );
		}
		if(job.is_proxy_renewable())
		  DNProxyManager::getInstance()->decrementUserProxyCounter( job.get_user_dn(), job.get_myproxy_address() );
            }
        }
    };


}; // end anonymous namespace

//____________________________________________________________________________
iceCommandStatusPoller::iceCommandStatusPoller( glite::wms::ice::Ice* theIce, 
						const pair<string, string>& dnce,
						bool poll_all_jobs
						) :
  iceAbsCommand( "iceCommandStatusPoller", "" ),
  m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
  m_lb_logger( iceLBLogger::instance() ),
  m_iceManager( theIce ),
  m_threshold( iceConfManager::getInstance()->getConfiguration()->ice()->poller_status_threshold_time() ),
  m_max_chunk_size( iceConfManager::getInstance()->getConfiguration()->ice()->bulk_query_size() ), 
  m_empty_threshold( 10*60 ), // 10 minutes
  m_poll_all_jobs( poll_all_jobs ),
  m_conf( iceConfManager::getInstance() ),
  m_stopped( false ),
  m_dnce( dnce )
{
  m_empty_threshold = m_conf->getConfiguration()->ice()->ice_empty_threshold();
}

//____________________________________________________________________________
void iceCommandStatusPoller::get_jobs_to_poll( list< CreamJob >& result,
					       const std::string& userdn, 
					       const std::string& creamurl) throw()
{
    static const char* method_name = "iceCommandStatusPoller::get_jobs_to_poll() - ";

    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
                   << "Collecting jobs to poll for userdn=[" 
		   << userdn << "] creamurl=[" 
		   << creamurl << "]. LIMIT set to [" << m_max_chunk_size << "]..."
    		);
    {
      glite::wms::ice::db::GetJobsToPoll getter( &result, userdn, creamurl, m_poll_all_jobs,"iceCommandStatusPoller::get_jobs_to_poll", m_max_chunk_size );
      glite::wms::ice::db::Transaction tnx(false, false);
      //tnx.begin( );
      tnx.execute( &getter );
      //result = getter.get_jobs();
    }
    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
                   << "Finished collecting jobs to poll. [" << result.size() << "] jobs are to poll."
    		);
}

//____________________________________________________________________________
list< soap_proxy::JobInfoWrapper > 
iceCommandStatusPoller::check_multiple_jobs( const string& proxy,
					     const string& user_dn,
                                             const string& cream_url, 
                                             const list< CreamJob >& cream_job_ids ) 
  throw()
{
    

    static const char* method_name = "iceCommandStatusPoller::check_multiple_jobs() - ";

    for( list< CreamJob >::const_iterator thisJob = cream_job_ids.begin(); 
         thisJob != cream_job_ids.end(); 
         ++thisJob) 
      {
	CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		       << "Will poll job with CREAM job id = ["
		       << thisJob->get_complete_cream_jobid() << "]"
		       );
      }    

    // Following a discussion on 2008-11-12, we decided not to remove
    // jobs which cannot be polled. The reason being that errors may
    // be transient (so that the jobs could be polled in future), and
    // that the lease mechanism will take care of unreachable jobs
    // anyway.

    // Build the scope_guard object which will remove all jobs in the
    // vector from the job cache, if necessary.
    // remove_bunch_of_jobs remove_f( job_id_vector );
    // The following is needed because we want to pass to
    // remove_job_guard a reference to function remove_f, instead of a
    // copy of it.  Passing a reference here is needed because we can
    // later modify the object remove_f by changing the failure
    // reason, and we want the scope_guard to invoke the (modified)
    // function object.
    // boost::function<void()> remove_f_ref = boost::ref( remove_f );
    // wms_utils::scope_guard remove_jobs_guard( remove_f_ref );
    

    list<soap_proxy::JobInfoWrapper> the_job_status;
    
    try {
        
        CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
                       << "Connecting to [" << cream_url << "]"
                       );
        
        soap_proxy::AbsCreamProxy::InfoArrayResult res;        
        {
            list<CreamJob>::const_iterator jobit;
            vector< soap_proxy::JobIdWrapper > jobVec;
            
            for(jobit = cream_job_ids.begin(); jobit != cream_job_ids.end(); ++jobit) {
	      soap_proxy::JobIdWrapper J( jobit->get_cream_jobid(), 
					  cream_url, // we trust cream_url is the same for all jobs!!!
					  vector<soap_proxy::JobPropertyWrapper>());
	      jobVec.push_back( J );
            }
            
            soap_proxy::JobFilterWrapper req( jobVec, 
                                              vector<string>(), 
                                              -1, -1, 
                                              "", 
                                              "");
            
            
            if(m_stopped) {
	      CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
			     << "EMERGENCY CALLED STOP. Returning without polling..."
			     );
	      return list<soap_proxy::JobInfoWrapper>();
	    }
             CreamProxy_Info( cream_url, 
                              proxy,
                              &req,
                              &res,
			      false /* ignore blacklisted CE */).execute( 3 );
            // remove_jobs_guard.dismiss(); // dismiss the guard, we should be safe here...
            
        } // free some array



        map<string, boost::tuple<soap_proxy::JobInfoWrapper::RESULT, soap_proxy::JobInfoWrapper, string> >::const_iterator infoIt;
        
        for( infoIt = res.begin(); infoIt != res.end(); ++infoIt ) {
            boost::tuple<soap_proxy::JobInfoWrapper::RESULT, 
                soap_proxy::JobInfoWrapper, 
                string> thisInfo = infoIt->second;
            
            if ( thisInfo.get<0>() == soap_proxy::JobInfoWrapper::OK ) {
                
                the_job_status.push_back( thisInfo.get<1>() );
                
            } else {
                
                CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                               << "CREAM didn't return information for the Job=["
                               << infoIt->first << "] - DN=[" << user_dn
                               << "] - ProxyFile=[" << proxy
                               << "]. Error is: " << thisInfo.get<2>() 
                               << ". Removing this job from the database"
                               );
		/**
		   Must get the entire job by the Complete Cream JOB ID
		*/
		bool found = false;
		CreamJob theJob;
		{
		  db::GetJobByCid getter( infoIt->first, "iceCommandStatusPoller::check_multiple_jobs" );
		  db::Transaction tnx(false, false);
		  //tnx.begin( );
		  tnx.execute( &getter );
		  found = getter.found();
		  if( found ) {
		    theJob = getter.get_job();
		  }
		}
		if(found)
		  {
		    if( theJob.is_proxy_renewable() )
		      DNProxyManager::getInstance()->decrementUserProxyCounter( theJob.get_user_dn(), theJob.get_myproxy_address() );
		    db::RemoveJobByCid remover( infoIt->first, "iceCommandStatusPoller::check_multiple_jobs" );
		    db::Transaction tnx(false, false);
		    tnx.execute( &remover );
		  }
            }
        } // for

    } catch(soap_proxy::auth_ex& ex) {
      
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                       << "Cannot query status job for DN=["
                       << user_dn << "]. Exception is [" << ex.what() << "]"
                       );
        // remove_f.set_reason( ex.what() );
        sleep(1);

    } catch(soap_proxy::soap_ex& ex) {
        
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                       << "Cannot query status job for DN=["
                       << user_dn << "]. Exception is ["  << ex.what() << "]"
                       );
        // remove_f.set_reason( ex.what() );
        sleep(1);

    } catch(cream_api::cream_exceptions::InternalException& ex) {
      
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                       << "Cannot query status job for DN=["
                       << user_dn << "]. Exception is [" << ex.what() << "]"
                       );
        // remove_f.set_reason( ex.what() );
        
        // this ex can be raised if the remote service is not
        // reachable and scanJobs is called again immediately. Until
        // the service is down this could overload the cpu and the
        // logfile. So let's wait for a while before returning...
        sleep(1);
        
    } catch(exception& ex) {
        
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                       << "Cannot query status job for DN=["
                       << user_dn << "]. Exception is [" << ex.what() << "]"
                       );
        // remove_f.set_reason( ex.what() );

    } catch(...) {
        
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                       << "Cannot query status job for DN=["
                       << user_dn << "]. Unknown exception catched"
                       );
        
    }
    return the_job_status;
}

//----------------------------------------------------------------------------
void iceCommandStatusPoller::updateJobCache( const list< soap_proxy::JobInfoWrapper >& info_list ) throw()
{
  if(m_stopped) {
    CREAM_SAFE_LOG(m_log_dev->debugStream() << "iceCommandStatusPoller::updateJobCache() - "
		   << "EMERGENCY CALLED STOP. Returning without polling..."
		   );
    return;
  }
    for_each( info_list.begin(), 
              info_list.end(), 
              boost::bind1st( boost::mem_fn( &iceCommandStatusPoller::update_single_job ), this ));
    
}

//____________________________________________________________________________
void iceCommandStatusPoller::update_single_job( const soap_proxy::JobInfoWrapper& info_obj ) throw()
{
    static const char* method_name = "iceCommandStatusPoller::update_single_job() - ";
    
    if(m_stopped) {
      CREAM_SAFE_LOG(m_log_dev->debugStream() << "iceCommandStatusPoller::updateJobCache() - "
		     << "EMERGENCY CALLED STOP. Returning without polling..."
		     );
      return;
    }

    vector< soap_proxy::JobStatusWrapper > status_changes;
    info_obj.getStatus( status_changes );
    
    
    string completeJobID = info_obj.getCreamURL();
    boost::replace_all( completeJobID, 
			m_conf->getConfiguration()->ice()->cream_url_postfix(), "" );
    
    completeJobID += "/" + info_obj.getCreamJobID();
    
    CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                    << "Updating status for CREAM Job ID ["
                    << info_obj.getCreamJobID() << "] CREAM URL ["
                    << info_obj.getCreamURL() << "]"
                    );
    
    int count;
    vector< soap_proxy::JobStatusWrapper >::const_iterator it;

    CreamJob tmp_job;
    {
      glite::wms::ice::db::GetJobByCid getter( completeJobID, "iceCommandStatusPoller::update_single_job" );
      glite::wms::ice::db::Transaction tnx(false, false);
      tnx.execute( &getter );
      if( !getter.found() )
      {
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name 
    		       << "cream_jobid [" << completeJobID 
		       << "] disappeared!"
		       );
	return;
      }
      
      tmp_job = getter.get_job();
      
    }
    
    tmp_job.set_worker_node( info_obj.getWorkerNode() );
    for ( it = status_changes.begin(), count = 1; 
	  it != status_changes.end(); 
	  ++it, ++count ) 
      {
	
	jobstat::job_status stNum( jobstat::getStatusNum( it->getStatusName() ) );
	
	/**
	   before doing anything, check if the job is "purged". If so,
	   remove from the cache and forget about it.
	*/
	if ( stNum == jobstat::PURGED ) {
	  CREAM_SAFE_LOG(m_log_dev->warnStream() << method_name
			 << "Job " << tmp_job.describe()
			 << " is reported as PURGED. Removing from database"
			 ); 
	  {
	    if( tmp_job.is_proxy_renewable() )
	      DNProxyManager::getInstance()->decrementUserProxyCounter( tmp_job.get_user_dn(), tmp_job.get_myproxy_address() );
	    glite::wms::ice::db::RemoveJobByCid remover( tmp_job.get_complete_cream_jobid(), "iceCommandStatusPoller::update_single_job" );
	    glite::wms::ice::db::Transaction tnx(false, false);
	    tnx.execute( &remover );
	  }
	  return;
	}
	
	
	
	/**
	   Update the job in the database only if the number of received states
	   is greater than the number of states of the current job.
	   In this case also check if the job must be purged or resubmitted
	*/ 
	if (  tmp_job.get_num_logged_status_changes() < count ) {
	  
	  string exitCode( it->getExitCode() );
	  
	  CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
			 << "Updating ICE's database for " << tmp_job.describe()
			 << " status = [" << it->getStatusName() << "]"
			 << " exit_code = [" << exitCode << "]"
			 << " failure_reason = [" << it->getFailureReason() << "]"
			 << " description = [" << it->getDescription() << "]"
			 );
	  tmp_job.set_status( stNum );
	  
#ifdef GLITE_WMS_ICE_ENABLE_STATS
	  {
	    db::InsertStat inserter( it->getTimestamp(), (short)stNum, "iceCommandEventQuery::processSingleEvent" );
	    db::Transaction tnx(false, false);
	    tnx.execute( &inserter );
	  }
#endif
	  
	  try {
	    tmp_job.set_exitcode( boost::lexical_cast< int >( exitCode ) );
	  } catch( boost::bad_lexical_cast & ) {
	    tmp_job.set_exitcode( 0 );
	  }
	  //
	  // See comment in normalStatusNotification.cpp
	  //
	  string reason = "";
	  if ( stNum == jobstat::CANCELLED ) {
	    tmp_job.set_failure_reason( it->getDescription() );
	  } else {
	    tmp_job.set_failure_reason( it->getFailureReason() );
	  }
	  tmp_job.set_numlogged_status_changes( count );
	  {
	    list<pair<string, string> > params;
	    params.push_back( make_pair("worker_node", info_obj.getWorkerNode()) );
	    /**
	       This update of releveat times is done outside, in the check_user_jobs 
	       method in order to prevent to re-poll always the same jobs 
	       if something goes wrong...
	       
	       params.push_back( make_pair("last_seen", int_to_string(time(0)  )) );
	       params.push_back( make_pair("last_empty_notification", int_to_string(time(0)  )));
	    */
	    params.push_back( make_pair("status", int_to_string(stNum)));
	    params.push_back( make_pair("exit_code", int_to_string(tmp_job.get_exit_code())));
	    params.push_back( make_pair("num_logged_status_changes", int_to_string(count)));
	    params.push_back( make_pair("failure_reason", it->getFailureReason()));
	    db::UpdateJobByGid updater( tmp_job.get_grid_jobid(), params, "iceCommandStatusPoller::update_single_job");
	    db::Transaction tnx(false, false);
	    tnx.execute( &updater );
	  }
	  // Log to L&B
	  iceLBEvent* ev = iceLBEventFactory::mkEvent( tmp_job );
	  if ( ev ) {
	    tmp_job = m_lb_logger->logEvent( ev );
	  }
	  
	  /**
	     Let's check if the job must be purged or resubmitted
	     only if a new status has been received
	  */
	  m_iceManager->resubmit_or_purge_job( tmp_job/*jit*/ );
	  
	} //  if (  tmp_job.get_num_logged_status_changes() < count ) {
	
	
      } // for over states
}

//____________________________________________________________________________
void iceCommandStatusPoller::execute( const std::string& tid ) throw()
{

  m_thread_id = tid;

  static const char* method_name = "iceCommandStatusPoller::execute() - ";

  list< CreamJob > jobList;
  
  string userdn   = m_dnce.first;//it->at(0);
  string creamurl = m_dnce.second;//it->at(1);
  
  CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
		 << "Getting ["
		 << m_max_chunk_size
		 << "] jobs to poll for user ["
		 << userdn << "] creamurl ["
		 << creamurl << "]"
		 );
  
  /**
     This method locks ICE because interrogates the ICE's database.
     When it returns the lock is released
  */
  this->get_jobs_to_poll( jobList, userdn, creamurl );
  
  /**
     No jobs for this couple userdn/creamurl.
     Skip and go to the next couple userdn/creamurl
  */
  if( !jobList.size() ) return;//continue;
  
  /**
     Look for a valid proxy for the current userdn
  */
  string proxy( DNProxyManager::getInstance()->getAnyBetterProxyByDN( userdn ).get<0>() );
  if ( proxy.empty() ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		   << "A valid proxy file for DN [" << userdn
		   << "] CREAM-URL ["
		   << creamurl << "] is not available. Skipping polling for this user and "
		   << "deleting all his/her jobs" 
		   );
    deleteJobsByDN( userdn );
    return;//continue;
  }  
  if( !(isvalid( proxy ).first) ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		   << "Proxy ["
		   << proxy << "] for user ["
		   << userdn 
		   << "] is expired! Skipping polling for this user "
		   << "and deletegin all his/her jobs..."
		   );

    deleteJobsByDN( userdn );

    return;//continue;
  }
  
  CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
		 << "Authenticating with proxy [" << proxy << "] for userdn [" 
		 << userdn << "]..."
		 );
  
  /**
     The following method 'check_multiple_jobs' will connect to the CREAM service
     and then can hung for a while
  */
  list< soap_proxy::JobInfoWrapper > j_status( check_multiple_jobs( proxy, userdn, creamurl, jobList ) );
  
  /**
     Update the job's states in the ICE's database and for each job
     check if it must be resubmitted or purged.
  */
  updateJobCache( j_status );// modifies the cache, locks it job by job inside update_single_job
  
  for(list<CreamJob>::const_iterator it = jobList.begin();
      it != jobList.end();
      ++it)
    {
      list< pair< string, string > > params;
      params.push_back( make_pair("last_seen", int_to_string( time(0)) ));
      params.push_back( make_pair("last_empty_notification", int_to_string(time(0) )));
      params.push_back( make_pair("last_poller_visited", int_to_string(time(0) )));
      db::UpdateJobByGid updater( it->get_grid_jobid(), params, "iceCommandStatusPoller::execute" );
      db::Transaction tnx(false, false);
      //tnx.begin_exclusive( );
      tnx.execute( &updater );
    }
  //  }
}

//____________________________________________________________________________
void 
iceCommandStatusPoller::deleteJobsByDN( const string& dn ) throw( )
{

  list< CreamJob > results;
  {
    list<pair<string, string> > clause;
    clause.push_back( make_pair( "userdn", dn ) );
    
    db::GetJobs getter( clause, results, "iceCommandStatusPoller::deleteJobsForDN" );
    db::Transaction tnx( false, false );
    tnx.execute( &getter );
  }

  list< CreamJob >::iterator jit = results.begin();
  while( jit != results.end() ) {
    jit->set_failure_reason( "Job Aborted because proxy expired." );
    jit->set_status( cream_api::job_statuses::ABORTED ); 
    jit->set_exitcode( 0 );
    iceLBEvent* ev = iceLBEventFactory::mkEvent( *jit );
    if ( ev ) {
      m_lb_logger->logEvent( ev );
    }
  }

  {
    db::RemoveJobByUserDN remover( dn, "iceCommandStatusPoller::deleteJobsForDN" );
    db::Transaction tnx( false, false );
    tnx.execute( &remover );
  }
}
