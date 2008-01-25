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

/**
 *
 * ICE Headers
 *
 */
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
#include "jobCache.h"
#include "ice-core.h"
#include "jobCache.h"
#include "iceUtils.h"

/**
 *
 * Cream Client API Headers
 *
 */
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"

/**
 *
 * WMS Headers
 *
 */
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

/**
 *
 * BOOST Headers
 *
 */
#include <boost/lexical_cast.hpp>
#include <boost/functional.hpp>

/**
 *
 * STL Headers
 *
 */
#include <set>
#include <algorithm>
#include <cstdlib>

namespace cream_api  = glite::ce::cream_client_api;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;
namespace jobstat    = glite::ce::cream_client_api::job_statuses;
namespace iceUtils   = glite::wms::ice::util;
using namespace std;

 namespace { // begin anonymous namespace

  bool insert_condition(const iceUtils::CreamJob& J) {
    if( J.getCompleteCreamJobID().empty() ) return false;
    
    return true;
  }
  
//     /**
//      * This class represents an iterator; the operator=(), instead of
//      * inserting something into the container, removes an element from the
//      * job cache.
//      */
//     class job_cache_remover {
//     protected:
//       iceUtils::jobCache* m_cache;
//     public:
//         typedef output_iterator_tag iterator_category;
//         typedef void value_type;
//         typedef void difference_type;
//         typedef void pointer;
//         typedef void pointer;
//         typedef void reference;
        
//       job_cache_remover( ) : m_cache( iceUtils::jobCache::getInstance() ) { };
//         /**
//          * Removes an element from the jobCache. The cache is locked before
//          * attempting to remove the element.
//          *
//          * @param job_id the Cream JobID to remove; if this Cream JobID is
//          * not found in the cache, nothing is done.
//          *
//          * @return this iterator object.
//          */
//         job_cache_remover& operator=( const string& job_id ) {
// 	    boost::recursive_mutex::scoped_lock M( iceUtils::jobCache::mutex );        
// 	    iceUtils::jobCache::iterator it = m_cache->lookupByCreamJobID( job_id );
//             m_cache->erase( it );
//             return *this;
//         };
//         job_cache_remover& operator*() { return *this; }
//         job_cache_remover& operator++() { return *this; }
//         job_cache_remover& operator++(int) { return *this; }
//     };    

 }; // end anonymous namespace

//____________________________________________________________________________
iceUtils::iceCommandStatusPoller::iceCommandStatusPoller( glite::wms::ice::Ice* theIce, bool poll_all_jobs ) :
  m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
  m_lb_logger( iceLBLogger::instance() ),
  m_iceManager( theIce ),
  m_cache( jobCache::getInstance() ),
  m_threshold( iceConfManager::getInstance()->getConfiguration()->ice()->poller_status_threshold_time() ),
  m_max_chunk_size( iceUtils::iceConfManager::getInstance()->getConfiguration()->ice()->bulk_query_size() ), 
  m_empty_threshold( 10*60 ), // 10 minutes
  m_poll_all_jobs( poll_all_jobs ),
  m_conf( iceUtils::iceConfManager::getInstance() )
{
  char* ice_empty_threshold_string = ::getenv( "ICE_EMPTY_THRESHOLD" );;
  if ( ice_empty_threshold_string ) {
    try {
      m_empty_threshold =  boost::lexical_cast< time_t >( ice_empty_threshold_string );
    } catch( boost::bad_lexical_cast & ) {
      m_empty_threshold = 10*60;
    }
  }
}


//____________________________________________________________________________
void iceUtils::iceCommandStatusPoller::get_jobs_to_poll( list< iceUtils::CreamJob >& result ) throw()
{

    for(jobCache::iterator jit = m_cache->begin(); jit != m_cache->end(); ++jit) {

        if( jit->getCompleteCreamJobID().empty() ) {
            // This job doesn't have yet the CREAM Job ID. Skipping...
            continue;
        }
        
        time_t t_now( time(NULL) );
        time_t t_last_seen( jit->getLastSeen() ); // This can be zero for jobs which are being submitted right now. The value of the last_seen field of creamJob is set only before exiting from the execute() method of iceCommandSubmit.
        time_t t_last_empty_notification( jit->get_last_empty_notification() ); // The time ICE received the last empty notification for this job
        time_t oldness = t_now - t_last_seen;
        time_t empty_oldness = t_now - t_last_empty_notification;

        //
        // Q: When does a job get polled?
        //
        // A: A job gets polled if one of the following situations are true:
        //
        // 1. ICE starts. When ICE starts, it polls all the jobs it thinks
        // have not been purged yet.
        //
        // 2. ICE received the last non-empty status change
        // notification more than m_threshold seconds ago.
        //
        // 3. ICE received the last empty status change notification
        // more than 10*60 seconds ago (that is, 10 minutes).
        //
	// empty notification are effective to reduce the polling frequency if the m_threshold is much greater than 10x60 seconds
	
        if ( m_poll_all_jobs ||
	     ( ( t_last_seen > 0 ) && oldness >= m_threshold ) ||
             ( ( t_last_empty_notification > 0 ) && empty_oldness > m_empty_threshold ) ) { // empty_oldness must be greater than 10 minutes
            CREAM_SAFE_LOG(m_log_dev->debugStream() 
                           << "iceCommandStatusPoller::get_jobs_to_poll() - "
                           << "Adding job "
                           << jit->describe()
                           << " t_now=" << time_t_to_string( t_now )
                           << " t_last_nonempty_notification=" << time_t_to_string(t_last_seen)
                           << " oldness (t_last_nonempty_notification - t_now)=" << oldness 
                           << " threshold=" << m_threshold
                           << " t_last_empty_notification=" << time_t_to_string( t_last_empty_notification )
                           << " empty_oldness (t_last_empty_notification - t_now)=" << empty_oldness
                           << " empty_threshold=" << m_empty_threshold
                           << log4cpp::CategoryStream::ENDLINE);

            result.push_back( *jit );
        }
    }
}

//____________________________________________________________________________
list< soap_proxy::JobInfoWrapper > iceUtils::iceCommandStatusPoller::check_multiple_jobs( const string& user_dn, const string& cream_url, const vector< CreamJob >& cream_job_ids ) throw()
{
  
  CREAM_SAFE_LOG(m_log_dev->infoStream() 
                 << "iceCommandStatusPoller::check_multiple_jobs() - "
                 << "Will poll " << cream_job_ids.size() 
		 << " job(s) of the user ["
                 << user_dn << "] to CREAM ["
                 << cream_url << "]"
                 << log4cpp::CategoryStream::ENDLINE);

  for(vector< CreamJob >::const_iterator thisJob = cream_job_ids.begin(); thisJob != cream_job_ids.end(); ++thisJob) {
      CREAM_SAFE_LOG(m_log_dev->debugStream() 
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Will poll job with CREAM job id = ["
                     << thisJob->getCompleteCreamJobID() << "]"
                     << log4cpp::CategoryStream::ENDLINE);
  }

  string proxy( DNProxyManager::getInstance()->getBetterProxyByDN( user_dn ) );
  
  if ( proxy.empty() ) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() 
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "A Proxy file for DN ["
                     << user_dn
                     << "] is not yet available !!! Skipping..."
                     << log4cpp::CategoryStream::ENDLINE);
      // Cannot process the list of jobs submitted by this user;
      // skip over and hope for the best
      return list< soap_proxy::JobInfoWrapper >();
  }
  
  CREAM_SAFE_LOG(m_log_dev->infoStream() 
                 << "iceCommandStatusPoller::check_multiple_jobs() - "
                 << "Authenticating with proxy ["
                 << proxy
                 << "]"
                 << log4cpp::CategoryStream::ENDLINE);

  list<soap_proxy::JobInfoWrapper> the_job_status;

  try {
      
    soap_proxy::VOMSWrapper V( proxy );
    if( !V.IsValid( ) ) {
      throw cream_api::soap_proxy::auth_ex( V.getErrorMessage() );
    } else {
      if( V.getProxyTimeEnd() <= time(NULL) )
	throw cream_api::soap_proxy::auth_ex( string("Proxy [")+proxy+"] is expired!" );
    }
    
    
      CREAM_SAFE_LOG(m_log_dev->infoStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Connecting to ["
                     << cream_url
                     << "]"
                     << log4cpp::CategoryStream::ENDLINE);
      
      soap_proxy::AbsCreamProxy::InfoArrayResult res;

      {
	vector<CreamJob>::const_iterator jobit;
	vector< soap_proxy::JobIdWrapper > jobVec;
	
	for(jobit = cream_job_ids.begin(); jobit != cream_job_ids.end(); ++jobit) 
	  {
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
	
	
	
	iceUtils::CreamProxy_Info( cream_url, 
				   proxy,
				   &req,
				   &res).execute( 3 );
      } // free some array

      map<string, boost::tuple<soap_proxy::JobInfoWrapper::RESULT, soap_proxy::JobInfoWrapper, string> >::const_iterator infoIt;
      
      for( infoIt = res.begin(); infoIt != res.end(); ++infoIt )
	{
	  boost::tuple<soap_proxy::JobInfoWrapper::RESULT, 
	    soap_proxy::JobInfoWrapper, 
	    string> thisInfo = infoIt->second;

// 	  string completeCreamJobID = cream_url;
// 	  boost::replace_all( completeCreamJobID, 
// 			      m_conf->getConfiguration()->ice()->cream_url_postfix(), 
// 			      "" );

// 	  completeCreamJobID += "/" + infoIt->first;
	  

	  if( thisInfo.get<0>() == soap_proxy::JobInfoWrapper::OK ) {

	    the_job_status.push_back( thisInfo.get<1>() );

	  } else {

	    CREAM_SAFE_LOG(m_log_dev->errorStream()
			   << "iceCommandStatusPoller::check_multiple_jobs() - "
			   << "CREAM didn't return information for the job ["
			   << infoIt->first << "] for DN ["
			   << user_dn
			   << "]. Error is [" << thisInfo.get<2>() 
			   << "]. Removing this job from the cache"
			   << log4cpp::CategoryStream::ENDLINE);
	    {
	      boost::recursive_mutex::scoped_lock M( jobCache::mutex );
	      //remove_unknown_jobs_from_cache(cream_job_ids, the_job_status );
	      jobCache::iterator it = m_cache->lookupByCompleteCreamJobID( infoIt->first );
	      m_cache->erase( it );
	    }

	  }

	} // for

      // WARNING: the following code is not needed anymore
      // because for each job we ask the status of we receive an
      // answer with the status or with a fault indicating that
      // the job does not exist.

      // Runs over the returned status, and the missing jobs are removed from jobCache
      //       {
      //           boost::recursive_mutex::scoped_lock M( jobCache::mutex );
      // 	  // the following method removes from jobCache those jobs that
      // 	  // are in cream_job_ids but NOT are in the_job_status
      //           remove_unknown_jobs_from_cache( cream_job_ids, the_job_status );
      //       }
      
  } // catch( cream_api::cream_exceptions::JobUnknownException& ex) {
//       CREAM_SAFE_LOG(m_log_dev->errorStream()
//                      << "iceCommandStatusPoller::check_multiple_jobs() - "
//                      << "CREAM responded JobUnknown for DN=["
//                      << user_dn
//                      << "]. Exception is [" << ex.what() << "]. Removing it from the cache"
//                      << log4cpp::CategoryStream::ENDLINE);
//       {
//           boost::recursive_mutex::scoped_lock M( jobCache::mutex );
//           remove_unknown_jobs_from_cache(cream_job_ids, the_job_status );
//       }
//      
//  }
 catch(soap_proxy::auth_ex& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Cannot query status job for DN=["
                     << user_dn
                     << "]. Exception is [" << ex.what() << "]"
                     << log4cpp::CategoryStream::ENDLINE);
      sleep(1);
  } catch(soap_proxy::soap_ex& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Cannot query status job for DN=["
                     << user_dn
                     << "]. Exception is [" 
                     << ex.what() << "]"
                     << log4cpp::CategoryStream::ENDLINE);
      sleep(1);
  } // catch(cream_api::cream_exceptions::BaseException& ex) {
      
//       CREAM_SAFE_LOG(m_log_dev->errorStream()
//                      << "iceCommandStatusPoller::check_multiple_jobs() - "
//                      << "Cannot query status job for DN=["
//                      <<  user_dn
//                      << "]. Exception is [" 
//                      << ex.what() << "]"
//                      << log4cpp::CategoryStream::ENDLINE);

//   }
  catch(cream_api::cream_exceptions::InternalException& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Cannot query status job for DN=["
                     << user_dn
                     << "]. Exception is [" 
                     << ex.what() << "]"
                     << log4cpp::CategoryStream::ENDLINE);
	
      // this ex can be raised if the remote service is not
      // reachable and scanJobs is called again
      // immediately. Untill the service is down this could
      // overload the cpu and the logfile. So let's wait for a
      // while before returning...
      sleep(1);
  } // catch(cream_api::cream_exceptions::DelegationException& ex) {
      
//       CREAM_SAFE_LOG(m_log_dev->errorStream()
//                      << "iceCommandStatusPoller::check_multiple_jobs() - "
//                      << "Cannot query status job for DN=["
//                      << user_dn
//                      << "]. Exception is [" 
//                      << ex.what() << "]"
//                      << log4cpp::CategoryStream::ENDLINE);
//   }
  catch(exception& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Cannot query status job for DN=["
                     << user_dn
                     << "]. Exception is [" 
                     << ex.what() << "]"
                     << log4cpp::CategoryStream::ENDLINE);
  } catch(...) {
    
    CREAM_SAFE_LOG(m_log_dev->errorStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Cannot query status job for DN=["
                     << user_dn
                     << "]. Unknown exception catched"
                     << log4cpp::CategoryStream::ENDLINE);

  }

  //copy(the_job_status.begin(), the_job_status.end(), back_inserter(result) );

  return the_job_status;

  //return result;

}

//----------------------------------------------------------------------------
void iceUtils::iceCommandStatusPoller::updateJobCache( const list< soap_proxy::JobInfoWrapper >& info_list ) throw()
{

  for_each( info_list.begin(), 
	    info_list.end(), 
	    boost::bind1st( boost::mem_fn( &iceUtils::iceCommandStatusPoller::update_single_job ), this ));

}

//____________________________________________________________________________
void iceUtils::iceCommandStatusPoller::update_single_job( const soap_proxy::JobInfoWrapper& info_obj ) throw()
{
    static const char* method_name = "iceCommandStatusPoller::update_single_job() - ";
    // Lock the cache
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    vector< soap_proxy::JobStatusWrapper > status_changes;
    info_obj.getStatus( status_changes );

    // FIXME: must get cream_url from the JobInfoWrapper structure

    string completeJobID = info_obj.getCreamURL();
    boost::replace_all( completeJobID, 
			m_conf->getConfiguration()->ice()->cream_url_postfix(), "" );

    completeJobID += "/" + info_obj.getCreamJobID();
    
    //string cid( info_obj.getCompleteCreamJobID() ); // Cream job id

    jobCache::iterator job_pos( m_cache->lookupByCompleteCreamJobID( completeJobID ) );
    if ( m_cache->end() != job_pos ) {
        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << " Managing Job "
                        << job_pos->describe()
                        << " for which I already processed "
                        << job_pos->get_num_logged_status_changes()
                        << " status changes, and JobStatus contains "
                        << status_changes.size()
                        << " status changes"
                        << log4cpp::CategoryStream::ENDLINE);

        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name << "Job " << job_pos->describe()
                        << " has worker_node=" << info_obj.getWorkerNode()
                        << log4cpp::CategoryStream::ENDLINE);
    }

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
        jobCache::iterator jit( m_cache->lookupByCompleteCreamJobID( completeJobID ) );    
	
        if ( m_cache->end() == jit ) {
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << method_name 
                           << "cream_jobid ["
                           << completeJobID << "] disappeared!"
                           << log4cpp::CategoryStream::ENDLINE);
            return;
        }
        
        // Creates a temporary job
        CreamJob tmp_job( *jit );
        
        // Update the worker node
        tmp_job.set_worker_node( info_obj.getWorkerNode() );
        //
        // END block NOT to be moved outside the 'for' loop
        //
        tmp_job.setLastSeen( time(0) );
        tmp_job.set_last_empty_notification( time(0) );

        jobstat::job_status stNum( jobstat::getStatusNum( it->getStatusName() ) );
        // before doing anything, check if the job is "purged". If so,
        // remove from the cache and forget about it.
        if ( stNum == jobstat::PURGED ) {
            CREAM_SAFE_LOG(m_log_dev->warnStream()
                           << method_name
                           << "Job "
                           << tmp_job.describe()
                           << " is reported as PURGED. Removing from cache"
                           << log4cpp::CategoryStream::ENDLINE); 
            m_cache->erase( jit );
            return;
        }

        string exitCode( it->getExitCode() );

        if ( tmp_job.get_num_logged_status_changes() < count ) {
            
            CREAM_SAFE_LOG(m_log_dev->debugStream()
                           << method_name
                           << "Updating jobcache for "
                           << jit->describe()
                           << " status = [" << it->getStatusName() << "]"
                           << " exit_code = [" << exitCode << "]"
                           << " failure_reason = [" << it->getFailureReason() << "]"
                           << log4cpp::CategoryStream::ENDLINE);

            tmp_job.setStatus( stNum );
            try {
                tmp_job.set_exit_code( boost::lexical_cast< int >( exitCode ) );
            } catch( boost::bad_lexical_cast & ) {
                tmp_job.set_exit_code( 0 );
            }
            tmp_job.set_failure_reason( it->getFailureReason() );
            tmp_job.set_num_logged_status_changes( count );

            // Log to L&B
            iceLBEvent* ev = iceLBEventFactory::mkEvent( tmp_job );
            if ( ev ) {
                tmp_job = m_lb_logger->logEvent( ev );
            }
            jit = m_cache->put( tmp_job );
            
            m_iceManager->resubmit_or_purge_job( jit );
        }
    }
}

//____________________________________________________________________________
// void iceUtils::iceCommandStatusPoller::remove_unknown_jobs_from_cache(const vector<string>& all_jobs, const list< soap_proxy::JobInfoWrapper >& jobs_found ) throw()
// {
//     if ( all_jobs.size() == jobs_found.size() )
//         return; // for all jobs in jobVec there's a corresponding JobInfo object in to_check

//     // Ok, now lets try to be smart

//     // Step 1. Build a vector of job ids which are in the to_check vector
//     set< string > jobs_found_ids;
// //     for ( vector< soap_proxy::JobInfo >::const_iterator it=jobs_found.begin(); it != jobs_found.end(); ++it ) {
// //         jobs_found_ids.insert( it->getCreamJobID() );
// //     }

//     transform( jobs_found.begin(), 
// 	       jobs_found.end(), 
// 	       inserter( jobs_found_ids, jobs_found_ids.begin() ), 
// 	       mem_fun_ref(&soap_proxy::JobInfoWrapper::getCreamJobID));

//     set< string > all_job_ids;
//     copy( all_jobs.begin(), all_jobs.end(), inserter( all_job_ids, all_job_ids.begin() ) );
//     // copy( all_jobs.begin(), all_jobs.end(), insert_iterator( all_job_ids ) );

//     // Step 2: do the difference on the sorted vectors
//     // set< string > missing_job_ids;
//     set_difference( all_job_ids.begin(), all_job_ids.end(),
//                     jobs_found_ids.begin(), jobs_found_ids.end(),
//                     job_cache_remover() );
// }

//____________________________________________________________________________
void iceUtils::iceCommandStatusPoller::execute( ) throw()
{

  list< CreamJob > j_list;
  {
    // moved mutex here (from the get_jobs_to_poll's body) because of more
    // code readability
    boost::recursive_mutex::scoped_lock M( iceUtils::jobCache::mutex );
    this->get_jobs_to_poll( j_list ); // this method locks the cache
  }

  if ( j_list.empty() ) 
    return; // give up if no job to check

    
    
  // Step 1. Build the mapping between ( UserDN, CreamURL ) -> list<CreamJobId>
  map< pair<string, string>, list< CreamJob >, ltstring> jobMap;
 
    
    jobMap_appender appender( jobMap, &insert_condition );
    for_each( j_list.begin(),
 	      j_list.end(),
 	      appender);

    // Step 2. Iterates over the map which has just been constructed.
    for(map< pair<string, string>, list<CreamJob> >::const_iterator jit = jobMap.begin(); jit != jobMap.end(); ++jit) {
        
        const string user_dn( jit->first.first );
        const string cream_url( jit->first.second );

 	

        list< iceUtils::CreamJob >::const_iterator it = (jit->second).begin();
        list< iceUtils::CreamJob >::const_iterator list_end = (jit->second).end();
        while ( it != list_end ) {
	  //vector< string > jobs_to_poll;
	  vector<CreamJob> jobs_to_poll;
	  
	  //it = iceUtils::transform_n_elements( it, 
	  it = iceUtils::copy_n_elements( it, 
					       list_end, 
					       m_max_chunk_size, 
					       back_inserter( jobs_to_poll )
					  /*, mem_fun_ref( &iceUtils::CreamJob::getPairIdCreamURL() ) */
					  );
	  
	  list< soap_proxy::JobInfoWrapper > j_status( check_multiple_jobs( user_dn, cream_url, jobs_to_poll ) ); // doesn't lock the cache

	  updateJobCache( j_status );// modifies the cache, locks it job by job
        }        
    }
}

