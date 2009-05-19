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
 * ICE status poller
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

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
#include "iceDb/RemoveJobByCid.h"
#include "iceDb/RemoveJobByGid.h"
#include "iceDb/GetStatusInfoByCompleteCreamJobID.h"
#include "iceDb/Transaction.h"
#include "iceDb/UpdateJobByGid.h"

// Cream Client API Headers
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"


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
using namespace glite::wms::ice::util;
using namespace std;

namespace { // begin anonymous namespace
    
  // See iceDb/GetJobsToPoll for definitions of type 'JobToPoll'
    bool insert_condition(const CreamJob& J) {        
      if( J.getCompleteCreamJobID().empty() ) 
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
	      //                boost::recursive_mutex::scoped_lock M( jobCache::mutex );
	      boost::recursive_mutex::scoped_lock M( CreamJob::globalICEMutex );

	      glite::wms::ice::db::GetJobByCid aJob( *it );
	      glite::wms::ice::db::Transaction tnx;
	      tnx.execute( &aJob );
	      if( !aJob.found() ) {
		continue; // nothing to do
	      }

//                 jobCache::iterator j = m_cache->lookupByCompleteCreamJobID( *it );
//                 if ( m_cache->end() == j )
//                     continue; // nothing to do

	      CreamJob job( aJob.get_job() );
	      CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
			     << "Removing job " << job.describe()
			     << " from the cache. Reason is: "
			     << m_reason);
                job.set_failure_reason( m_reason );
		{
		  list<pair<string, string> > params;
		  params.push_back( make_pair("failure_reason", job.get_failure_reason() ) );
		  glite::wms::ice::db::UpdateJobByGid updater( job.getGridJobID(), params );
		  glite::wms::ice::db::Transaction tnx;
		  tnx.execute( &updater );
		}
                m_lb_logger->logEvent( new job_aborted_event( job ) ); // ignore return value, the job will be removed from ICE cache anyway
                //m_cache->erase( j );
		{
		  glite::wms::ice::db::RemoveJobByGid remover( job.getGridJobID() );
		  glite::wms::ice::db::Transaction tnx2;
		  tnx.execute( &remover );
		}
            }
        }
    };


}; // end anonymous namespace

//____________________________________________________________________________
iceCommandStatusPoller::iceCommandStatusPoller( glite::wms::ice::Ice* theIce, bool poll_all_jobs ) :
    iceAbsCommand( "iceCommandStatusPoller" ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_lb_logger( iceLBLogger::instance() ),
    m_iceManager( theIce ),
    m_threshold( iceConfManager::getInstance()->getConfiguration()->ice()->poller_status_threshold_time() ),
    m_max_chunk_size( iceConfManager::getInstance()->getConfiguration()->ice()->bulk_query_size() ), 
    m_empty_threshold( 10*60 ), // 10 minutes
    m_poll_all_jobs( poll_all_jobs ),
    m_conf( iceConfManager::getInstance() ),
    m_stopped( false )
{
  m_empty_threshold = m_conf->getConfiguration()->ice()->ice_empty_threshold();
}


// See GetJobsToPoll.h source code for definition of type 'JobToPoll'
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
      glite::wms::ice::db::GetJobsToPoll getter( userdn, creamurl, m_poll_all_jobs, m_max_chunk_size );
      glite::wms::ice::db::Transaction tnx;
      tnx.execute( &getter );
      result = getter.get_jobs();
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
		       << thisJob->getCompleteCreamJobID() << "]"
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
	      soap_proxy::JobIdWrapper J( jobit->getCreamJobID(), 
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
                              &res).execute( 3 );
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
                               << ". Removing this job from the cache"
                               );
                {
		  boost::recursive_mutex::scoped_lock M( CreamJob::globalICEMutex );
		  db::RemoveJobByCid remover( infoIt->first );
		  db::Transaction tnx;
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
    // Lock the cache
    //boost::recursive_mutex::scoped_lock M( jobCache::mutex );

    if(m_stopped) {
      CREAM_SAFE_LOG(m_log_dev->debugStream() << "iceCommandStatusPoller::updateJobCache() - "
		     << "EMERGENCY CALLED STOP. Returning without polling..."
		     );
      return;
    }

    boost::recursive_mutex::scoped_lock M( CreamJob::globalICEMutex );

    vector< soap_proxy::JobStatusWrapper > status_changes;
    info_obj.getStatus( status_changes );
    
    // FIXME: must get cream_url from the JobInfoWrapper structure
    string completeJobID = info_obj.getCreamURL();
    boost::replace_all( completeJobID, 
			m_conf->getConfiguration()->ice()->cream_url_postfix(), "" );
    
    completeJobID += "/" + info_obj.getCreamJobID();
    
    CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                    << "Updating status for CREAM Job ID ["
                    << info_obj.getCreamJobID() << "] CREAM URL ["
                    << info_obj.getCreamURL() << "]"
                    );
    
//     bool found = false;
// 
//     CreamJob theJob;
//     {
//       glite::wms::ice::db::GetJobByCid getter( completeJobID );
//       glite::wms::ice::db::Transaction tnx;
//       tnx.execute( &getter );
//       found = getter.found();
//       theJob = getter.get_job();
//       //      info = getter.get_info();
//     }

//     if(found) {
//       CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
// 		      << " Managing Job " << theJob.describe()
// 		      << " for which I already processed "
// 		      << theJob.get_num_logged_status_changes()
// 		      << " status changes, and JobStatus contains "
// 		      << status_changes.size() << " status changes"
// 		      );
//     }

    int count;
    vector< soap_proxy::JobStatusWrapper >::const_iterator it;

    for ( it = status_changes.begin(), count = 1; it != status_changes.end(); ++it, ++count ) {

        //
        // Warning: the following block of code CANNOT be moved
        // outside the 'for' cycle; the reason is that ICE should be
        // tolerant to receiving inconsistent notifications, such that
        // two "DONE-FAILED" events. After the first DONE-FAILED, ICE
        // would purge the job. This means that the jit iterator would
        // no longer be valid. Hence, the necessity to check each
        // time.
        //
        //jobCache::iterator jit( m_cache->lookupByCompleteCreamJobID( completeJobID ) );    
	
      CreamJob tmp_job;
      {
	glite::wms::ice::db::GetJobByCid getter( completeJobID );
	glite::wms::ice::db::Transaction tnx;
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
      
      // Creates a temporary job
      
      // Update the worker node
      tmp_job.set_workernode( info_obj.getWorkerNode() );

      //
      // END block NOT to be moved outside the 'for' loop
      //

      

      tmp_job.set_last_seen( time(0) + iceConfManager::getInstance()->getConfiguration()->ice()->poller_delay()*5 );
      tmp_job.set_last_empty_notification_time( time(0) + iceConfManager::getInstance()->getConfiguration()->ice()->poller_delay()*5 );
      
      jobstat::job_status stNum( jobstat::getStatusNum( it->getStatusName() ) );
      // before doing anything, check if the job is "purged". If so,
      // remove from the cache and forget about it.
      if ( stNum == jobstat::PURGED ) {
	CREAM_SAFE_LOG(m_log_dev->warnStream() << method_name
		       << "Job " << tmp_job.describe()
		       << " is reported as PURGED. Removing from cache"
		       ); 
	{
	  glite::wms::ice::db::RemoveJobByCid remover( tmp_job.getCompleteCreamJobID() );
	  glite::wms::ice::db::Transaction tnx;
	  tnx.execute( &remover );
	}
	return;
      }
      
      string exitCode( it->getExitCode() );
      //      int exit_code = -1;
      if (  tmp_job.get_num_logged_status_changes() < count ) {
	
	CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		       << "Updating ICE's database for " << tmp_job.describe()
		       << " status = [" << it->getStatusName() << "]"
		       << " exit_code = [" << exitCode << "]"
		       << " failure_reason = [" << it->getFailureReason() << "]"
		       << " description = [" << it->getDescription() << "]"
		       );
	tmp_job.set_status( stNum );
	
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
	  db::UpdateJobByGid updater( tmp_job.getGridJobID(), params);
	  db::Transaction tnx;
	  tnx.execute( &updater );
	}
	// Log to L&B
	iceLBEvent* ev = iceLBEventFactory::mkEvent( tmp_job );
	if ( ev ) {
	  tmp_job = m_lb_logger->logEvent( ev );
	}
      } 
      
      m_iceManager->resubmit_or_purge_job( tmp_job/*jit*/ );
    }
}

//____________________________________________________________________________
void iceCommandStatusPoller::check_user_jobs( const string& userdn, const string& creamurl ) throw()
{
  static const char* method_name = "iceCommandStatusPoller::check_user_jobs() - ";
  list< CreamJob > j_list;
  //  do {

  j_list.clear();
  
  
  
  CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
		 << "Getting ["
		 << m_max_chunk_size
		 << "] jobs to poll for user ["
		 << userdn << "] creamurl ["
		 << creamurl << "]"
		 );
  
  this->get_jobs_to_poll( j_list, userdn, creamurl ); 
  
  if( !j_list.size() ) return;

  string proxy( DNProxyManager::getInstance()->getAnyBetterProxyByDN( userdn ).get<0>() );
  
  if ( proxy.empty() ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		   << "A valid proxy file for DN [" << userdn
		   << "] CREAM-URL ["
		   << creamurl << "] is not available. Skipping polling for this user ["
		   << userdn << "]."
		   );
    return;
  }
  
  if( !(isvalid( proxy ).first) ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		   << "Proxy ["
		   << proxy << "] for user ["
		   << userdn << "] is expired! Skipping polling for this user..."
		   );
    return;
  }
  
  CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
		 << "Authenticating with proxy [" << proxy << "] for userdn [" 
		 << userdn << "]..."
		 );
  
  list< soap_proxy::JobInfoWrapper > j_status( check_multiple_jobs( proxy, userdn, creamurl, j_list ) ); 
  
  //boost::recursive_mutex::scoped_lock M( glite::wms::ice::util::CreamJob::globalICEMutex );
  updateJobCache( j_status );// modifies the cache, locks it job by job
  
  boost::recursive_mutex::scoped_lock M( glite::wms::ice::util::CreamJob::globalICEMutex );
  for(list<CreamJob>::const_iterator it = j_list.begin();
      it != j_list.end();
      ++it)
    {
      list< pair< string, string > > params;
      params.push_back( make_pair("last_seen", int_to_string( time(0)) ));
      params.push_back( make_pair("last_empty_notification", int_to_string(time(0) )));
      params.push_back( make_pair("last_poller_visited", int_to_string(time(0) )));
      db::UpdateJobByGid updater( it->getGridJobID(), params );
      db::Transaction tnx;
      tnx.execute( &updater );
    }
  
  //  } while( j_list.size() != 0 );
}

//____________________________________________________________________________
void iceCommandStatusPoller::execute( ) throw()
{
  static const char* method_name = "iceCommandStatusPoller::execute() - ";

  list< vector< string > > result; // array of couples (userdn, creamurl)
  {
    /*
      SELECT DISTINCT (userdn, creamurl) from jobs;
     */
    list< string > fields;
    fields.push_back( "userdn" );
    fields.push_back( "creamurl" );
    
    db::GetFields getter( fields, list< pair< string, string > >(), true );
    db::Transaction tnx;
    tnx.execute( &getter );
    result = getter.get_values();
  }

  list< vector< string > >::const_iterator it;

  for( it = result.begin(); it != result.end(); ++it ) {

    /**
       Let's check if it is requested to exit before to start
       polling jobs to remote CREAM service...
    */
    if(m_stopped) {
      CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		     << "EMERGENCY CALLED STOP. Returning without polling..."
		     );
      return;
    }
    
    check_user_jobs( it->at(0), it->at(1) );

  }
}

