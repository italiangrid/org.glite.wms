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

#include "iceCommandStatusPoller.h"
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "iceLBEventFactory.h"
#include "iceConfManager.h"
#include "ice-core.h"
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "iceLBEventFactory.h"
#include "CreamProxyFactory.h"
#include "CreamProxyMethod.h"
#include "iceUtils.h"
#include "subscriptionManager.h"
#include "DNProxyManager.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include <boost/lexical_cast.hpp>

#include <set>
#include <algorithm>

#define STATUS_POLL_RETRY_COUNT 4

namespace cream_api  = glite::ce::cream_client_api;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;
namespace jobstat    = glite::ce::cream_client_api::job_statuses;

using namespace glite::wms::ice::util;
using namespace std;

namespace { // begin anonymous namespace

    /**
     * This class represents an iterator; the operator=(), instead of
     * inserting something into the container, removes an element from the
     * job cache.
     */
    class job_cache_remover {
    protected:
        jobCache* m_cache;
    public:
        typedef jobCache container_type;
        typedef output_iterator_tag iterator_category;
        typedef void value_type;
        typedef void difference_type;
        typedef void pointer;
        typedef void pointer;
        typedef void reference;
        
        job_cache_remover( ) : m_cache( jobCache::getInstance() ) { };
        /**
         * Removes an element from the jobCache. The cache is locked before
         * attempting to remove the element.
         *
         * @param job_id the Cream JobID to remove; if this Cream JobID is
         * not found in the cache, nothing is done.
         *
         * @return this iterator object.
         */
        job_cache_remover& operator=( const string& job_id ) {
            boost::recursive_mutex::scoped_lock M( jobCache::mutex );        
            jobCache::iterator it = m_cache->lookupByCreamJobID( job_id );
            m_cache->erase( it );
            return *this;
        };
        job_cache_remover& operator*() { return *this; }
        job_cache_remover& operator++() { return *this; }
        job_cache_remover& operator++(int) { return *this; }
    };    

    /**
     * Ad-hoc implementation of the copy_n algorithm, returning an
     * InputIterator. There actually is a copy_n algorithm defined in
     * GNU C++ implementation of the STL
     * (/usr/include/g++-3/stl_algobase.h), but it says that it is not
     * part of the C++ standard.
     *
     * This function copies at most n elements from the range
     * [InputIterator, InputIterator+n-1] (bounds included) into the
     * range [OutputIterator, OutputIterator+n-1] (bounds included).
     * If the source range is less than n elements wide, only the
     * elements in the range are copied.
     *
     * This function assumes that first and end are iterators to a
     * container of CreamJob objects. dest must be an iterator to a
     * container of string objects. This function copies the CREAM Job
     * ID from object referenced by the first iterator into the second
     * iterator.
     *
     * @param first the iterator of the first element in the source range
     *
     * @param end the iterator of the end of the source range. This
     * iterator is used to check if the input range is less than n
     * elements wide.
     *
     * @param n the maximum number of items to copy
     *
     * @param dest the iterator to the first element in the destination range
     *
     * @return an iterator to the input element PAST the last element copied.
     */
    template <class InputIterator, class Size, class OutputIterator>
    InputIterator copy_n_elements( InputIterator first, InputIterator end, Size n, OutputIterator dest ) {
        for ( ; n > 0 && first != end; --n ) {
            *dest = first->getCreamJobID();
            ++first;
            ++dest;
        }
        return first;
    }

    template <class InputIterator, class Size, class OutputIterator, class UnaryOperation>
    InputIterator transform_n_elements( InputIterator first, InputIterator end, Size n, OutputIterator dest, UnaryOperation f ) {
        for ( ; n > 0 && first != end; --n ) {
            *dest = f ( *first );
            ++first;
            ++dest;
        }
        return first;
    }

}; // end anonymous namespace

//____________________________________________________________________________
iceCommandStatusPoller::iceCommandStatusPoller( glite::wms::ice::Ice* theIce, bool poll_all_jobs ) :
  m_theProxy( CreamProxyFactory::makeCreamProxy( false ) ),
  m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
  m_lb_logger( iceLBLogger::instance() ),
  m_iceManager( theIce ),
  m_cache( jobCache::getInstance() ),
  m_threshold( iceConfManager::getInstance()->getConfiguration()->ice()->poller_status_threshold_time() ),
  m_max_chunk_size( 100 ), // FIXME: remove hardcoded default
  m_poll_all_jobs( poll_all_jobs )
{

}


list< CreamJob > iceCommandStatusPoller::get_jobs_to_poll( void ) 
{
    list<CreamJob> result;
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );
    
    for(jobCache::iterator jit = m_cache->begin(); jit != m_cache->end(); ++jit) {

        if( jit->getCreamJobID().empty() ) {
            // This job doesn't have yet the CREAM Job ID. Skipping...
            continue;
        }
        
        time_t t_now( time(NULL) );
        time_t t_last_seen( jit->getLastSeen() ); // This can be zero for jobs which are being submitted right now. The value of the last_seen field of creamJob is set only before exiting from the execute() method of iceCommandSubmit.
        time_t oldness = t_now - t_last_seen;

        if ( m_poll_all_jobs ||
	     ( ( t_last_seen > 0 ) && oldness >= m_threshold ) ) {
            CREAM_SAFE_LOG(m_log_dev->debugStream() 
                           << "iceCommandStatusPoller::get_jobs_to_poll() - "
                           << jit->describe()
                           << " t_now=" << t_now
                           << " t_last_seen=" << t_last_seen
                           << " oldness=" << oldness 
                           << " threshold=" << m_threshold
                           << log4cpp::CategoryStream::ENDLINE);

            result.push_back( *jit );
        }
    }
    return result;
}


list< list< CreamJob > > iceCommandStatusPoller::create_chunks( const list< CreamJob >& jobs, unsigned int max_size )
{
    list< list< CreamJob > > result;
    list< CreamJob >::const_iterator it;
    list< CreamJob > current_chunk;
    list< CreamJob >::size_type chunk_size = 0;

    for ( it=jobs.begin(); it!=jobs.end(); ++it ) {
        // NOTE: According to STL documentation, current_chunk.size()
        // should not be assumed to take constant time; it may take
        // O(N) time, with N=list size. To make things constant, we
        // explicitly handle a chunk_size variable, so that the size
        // check can be done in constant time.
        if ( chunk_size >= max_size ) { 
            result.push_back( current_chunk );
            current_chunk.clear();
            chunk_size = 0;
        }
        current_chunk.push_back( *it );
        ++chunk_size;
    }
    // Push back any remaining chunk
    if ( !current_chunk.empty() ) {
        result.push_back( current_chunk );
    }
    return result;
}

//____________________________________________________________________________
// int iceCommandStatusPoller::get_jobs_to_poll_max_num( list<CreamJob>& result, const int max_num ) 
// {
//   result.clear();
//   boost::recursive_mutex::scoped_lock M( jobCache::mutex );
  
//   int counter = 0;

//   for(jobCache::iterator jit = m_cache->begin(); jit != m_cache->end(); ++jit) {
    
//     if( jit->getCreamJobID().empty() ) {
//       // This job doesn't have yet the CREAM Job ID. Skipping...
//       continue;
//     }
    
//     time_t t_now( time(NULL) );
//     time_t t_last_seen( jit->getLastSeen() ); // This can be zero for jobs which are being submitted right now. The value of the last_seen field of creamJob is set only before exiting from the execute() method of iceCommandSubmit.
//     time_t oldness = t_now - t_last_seen;
    
//     if ( m_poll_all_jobs ||
// 	 ( ( t_last_seen > 0 ) && oldness >= m_threshold ) ) {
//       CREAM_SAFE_LOG(m_log_dev->debugStream() 
// 		     << "iceCommandStatusPoller::get_jobs_to_poll_max_num() - "
// 		     << jit->describe()
// 		     << " t_now=" << t_now
// 		     << " t_last_seen=" << t_last_seen
// 		     << " oldness=" << oldness 
// 		     << " threshold=" << m_threshold
// 		     << log4cpp::CategoryStream::ENDLINE);
      
//       result.push_back( *jit );
//       if( (++counter) >= max_num )
// 	break;
//     }
//   }

//   return result.size();
  
// }

//____________________________________________________________________________
//  list< soap_proxy::JobInfo > iceCommandStatusPoller::check_jobs( const list< CreamJob > & job_list ) 
// {
//     list< soap_proxy::JobInfo > result;

//     for ( list< CreamJob >::const_iterator jit = job_list.begin(); jit != job_list.end(); ++jit ) {

//         vector< string > job_to_query;
//         vector< soap_proxy::JobInfo > the_job_status;

//         job_to_query.push_back( jit->getCreamJobID() );

//         CREAM_SAFE_LOG(m_log_dev->debugStream() 
//                        << "iceCommandStatusPoller::check_jobs() - "
//                        << "Polling job "
//                        << jit->describe()
//                        << log4cpp::CategoryStream::ENDLINE);
	
//         try {
//             m_theProxy->Authenticate( jit->getUserProxyCertificate() );
//             CreamProxy_Info( jit->getCreamURL(), job_to_query, vector<string>(), the_job_status, -1, -1).execute( m_theProxy.get(), 3 );

//         } catch( cream_api::cream_exceptions::JobUnknownException& ex) {
//             CREAM_SAFE_LOG(m_log_dev->errorStream()
//                            << "iceCommandStatusPoller::check_jobs() - "
//                            << "CREAM responded JobUnknown for JobId=["
//                            << jit->getCreamJobID()
//                            << "]. Exception is [" << ex.what() << "]. Removing it from the cache"
//                            << log4cpp::CategoryStream::ENDLINE);
            
//             boost::recursive_mutex::scoped_lock M( jobCache::mutex );
//             jobCache::iterator thisJobToRemove = m_cache->lookupByCreamJobID( jit->getCreamJobID() );
//             m_cache->erase( thisJobToRemove );
            
//             continue;
	  
//         } catch(soap_proxy::auth_ex& ex) {

//             CREAM_SAFE_LOG(m_log_dev->errorStream()
//                            << "iceCommandStatusPoller::check_jobs() - "
//                            << "Cannot query status job for JobId=["
//                            << jit->describe()
//                            << "]. Exception is [" << ex.what() << "]"
//                            << log4cpp::CategoryStream::ENDLINE);
//             continue;
            
//         } catch(soap_proxy::soap_ex& ex) {

//             CREAM_SAFE_LOG(m_log_dev->errorStream()
//                            << "iceCommandStatusPoller::check_jobs() - "
//                            << "Cannot query status job for JobId=["
//                            << jit->describe()
//                            << "]. Exception is [" 
//                            << ex.what() << "]"
//                            << log4cpp::CategoryStream::ENDLINE);
//             continue;	  

//         } catch(cream_api::cream_exceptions::BaseException& ex) {

//             CREAM_SAFE_LOG(m_log_dev->errorStream()
//                            << "iceCommandStatusPoller::check_jobs() - "
//                            << "Cannot query status job for JobId=["
//                            << jit->describe()
//                            << "]. Exception is [" 
//                            << ex.what() << "]"
//                            << log4cpp::CategoryStream::ENDLINE);
//             continue;

//         } catch(cream_api::cream_exceptions::InternalException& ex) {
            
//             CREAM_SAFE_LOG(m_log_dev->errorStream()
//                            << "iceCommandStatusPoller::check_jobs() - "
//                            << "Cannot query status job for JobId=["
//                            << jit->describe()
//                            << "]. Exception is [" 
//                            << ex.what() << "]"
//                            << log4cpp::CategoryStream::ENDLINE);
//             continue;

//             // this ex can be raised if the remote service is not
//             // reachable and scanJobs is called again
//             // immediately. Untill the service is down this could
//             // overload the cpu and the logfile. So let's wait for a
//             // while before returning...
	  
//         } catch(cream_api::cream_exceptions::DelegationException& ex) {
            
//             CREAM_SAFE_LOG(m_log_dev->errorStream()
//                            << "iceCommandStatusPoller::check_jobs() - "
//                            << "Cannot query status job for JobId=["
//                            << jit->describe()
//                            << "]. Exception is [" 
//                            << ex.what() << "]"
//                            << log4cpp::CategoryStream::ENDLINE);
//             continue;
//         }

//         result.push_back( the_job_status.front() );

//     }
//     return result;
// }


//____________________________________________________________________________
list< soap_proxy::JobInfo > iceCommandStatusPoller::check_multiple_jobs( const string& user_dn, const string& cream_url, const vector< string >& cream_job_ids ) 
{
  list< soap_proxy::JobInfo > result;

  vector< soap_proxy::JobInfo > the_job_status; 

  CREAM_SAFE_LOG(m_log_dev->infoStream() 
                 << "iceCommandStatusPoller::check_multiple_jobs() - "
                 << "Will poll jobs of the user ["
                 << user_dn << "] to CREAM ["
                 << cream_url << "]"
                 << log4cpp::CategoryStream::ENDLINE);

  // vector< string > cream_job_ids;
  for(vector< string >::const_iterator thisJob = cream_job_ids.begin(); thisJob != cream_job_ids.end(); ++thisJob) {
      CREAM_SAFE_LOG(m_log_dev->infoStream() 
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Will poll job with CREAM job id=["
                     << *thisJob << "]"
                     << log4cpp::CategoryStream::ENDLINE);
      // cream_job_ids.push_back( *thisJob );
  }

  the_job_status.clear();

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
      return result;
  }
  
  CREAM_SAFE_LOG(m_log_dev->debugStream() 
                 << "iceCommandStatusPoller::check_multiple_jobs() - "
                 << "Authenticating with proxy ["
                 << proxy
                 << "]"
                 << log4cpp::CategoryStream::ENDLINE);

  try {
      
      m_theProxy->Authenticate( proxy );
      
      CREAM_SAFE_LOG(m_log_dev->debugStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Connecting to ["
                     << cream_url
                     << "]"
                     << log4cpp::CategoryStream::ENDLINE);
      
      CreamProxy_Info( cream_url, cream_job_ids, vector<string>(), the_job_status, -1, -1).execute( m_theProxy.get(), 3 );
      
      // Runs over the returned status, and the missing jobs are removed from jobCache
      boost::recursive_mutex::scoped_lock M( jobCache::mutex );
      remove_unknown_jobs_from_cache( cream_job_ids, the_job_status );
      
  } catch( cream_api::cream_exceptions::JobUnknownException& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "CREAM responded JobUnknown for DN=["
                     << user_dn
                     << "]. Exception is [" << ex.what() << "]. Removing it from the cache"
                     << log4cpp::CategoryStream::ENDLINE);
      
      boost::recursive_mutex::scoped_lock M( jobCache::mutex );
      remove_unknown_jobs_from_cache(cream_job_ids, the_job_status );
      
  } catch(soap_proxy::auth_ex& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Cannot query status job for DN=["
                     << user_dn
                     << "]. Exception is [" << ex.what() << "]"
                     << log4cpp::CategoryStream::ENDLINE);
      
  } catch(soap_proxy::soap_ex& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Cannot query status job for DN=["
                     << user_dn
                     << "]. Exception is [" 
                     << ex.what() << "]"
                     << log4cpp::CategoryStream::ENDLINE);
      
  } catch(cream_api::cream_exceptions::BaseException& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Cannot query status job for DN=["
                     <<  user_dn
                     << "]. Exception is [" 
                     << ex.what() << "]"
                     << log4cpp::CategoryStream::ENDLINE);

  } catch(cream_api::cream_exceptions::InternalException& ex) {
      
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
      
  } catch(cream_api::cream_exceptions::DelegationException& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream()
                     << "iceCommandStatusPoller::check_multiple_jobs() - "
                     << "Cannot query status job for DN=["
                     << user_dn
                     << "]. Exception is [" 
                     << ex.what() << "]"
                     << log4cpp::CategoryStream::ENDLINE);
  }

  for(vector< soap_proxy::JobInfo >::const_iterator sit=the_job_status.begin();
      sit != the_job_status.end();
      ++sit)
      result.push_back( *sit );

  return result;

}

//----------------------------------------------------------------------------
void iceCommandStatusPoller::updateJobCache( const list< soap_proxy::JobInfo >& info_list ) throw()
{
    for ( list< soap_proxy::JobInfo >::const_iterator it = info_list.begin(); it != info_list.end(); ++it ) {

        update_single_job( *it );

    }
}

//____________________________________________________________________________
void iceCommandStatusPoller::update_single_job( const soap_proxy::JobInfo& info_obj ) throw()
{
    // Locks the cache
    boost::recursive_mutex::scoped_lock M( jobCache::mutex );

    vector< soap_proxy::Status > status_changes;
    info_obj.getStatusList( status_changes );
    string cid( info_obj.getCreamJobID() ); // Cream job id

    jobCache::const_iterator job_pos( m_cache->lookupByCreamJobID( cid ) );
    if ( m_cache->end() != job_pos ) {
        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "iceCommandStatusPoller::update_single_job() - "
                        << " Managing Job "
                        << job_pos->describe()
                        << " for which I already processed "
                        << job_pos->get_num_logged_status_changes()
                        << " status changes, and JobStatus contains "
                        << status_changes.size()
                        << " status changes"
                        << log4cpp::CategoryStream::ENDLINE);
    }

    int count;
    vector< soap_proxy::Status >::const_iterator it;

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
        jobCache::iterator jit( m_cache->lookupByCreamJobID( cid ) );    
        
        if ( m_cache->end() == jit ) {
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "iceCommandStatusPoller::update_single_job() -  cream_jobid ["
                           << cid << "] disappeared!"
                           << log4cpp::CategoryStream::ENDLINE);
            return;
        }
        
        // Update the worker node
        jit->set_worker_node( info_obj.getWorkerNode() );
        //
        // END block NOT to be moved outside the 'for' loop
        //

        jit->setLastSeen( time(0) );

        jobstat::job_status stNum( jobstat::getStatusNum( it->getStatusName() ) );
        // before doing anything, check if the job is "purged". If so,
        // remove from the cache and forget about it.
        if ( stNum == jobstat::PURGED ) {
            CREAM_SAFE_LOG(m_log_dev->infoStream()
                           << "iceCommandStatusPoller::update_single_job() - "
                           << "Job "
                           << jit->describe()
                           << " is reported as PURGED. Removing from cache"
                           << log4cpp::CategoryStream::ENDLINE); 
            m_cache->erase( jit );
            return;
        }

        string exitCode( it->getExitCode() );

        if ( jit->get_num_logged_status_changes() < count ) {
            
            CREAM_SAFE_LOG(m_log_dev->infoStream()
                           << "iceCommandStatusPoller::update_single_job() - "
                           << "Updating jobcache for "
                           << jit->describe()
                           << " status = [" << it->getStatusName() << "]"
                           << " exit_code = [" << exitCode << "]"
                           << " failure_reason = [" << it->getFailureReason() << "]"
                           << log4cpp::CategoryStream::ENDLINE);

            // Creates a temporary job
            CreamJob tmp_job( *jit );

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
void iceCommandStatusPoller::execute( ) throw()
{
    /**
     * OLD CODE TO POLL JOB BY JOB
     */
    //list< CreamJob > j_list( get_jobs_to_poll() );
    //list< soap_proxy::JobInfo > j_status( check_jobs( j_list ) );
    

    list< CreamJob > j_list( get_jobs_to_poll() );

    if ( j_list.empty() ) 
        return; // give up if no job to check

    
    CREAM_SAFE_LOG(m_log_dev->debugStream()
                   << "iceCommandStatusPoller::execute() - "
                   << "********** Number of jobs to check ["
                   << j_list.size() << "]"
                   << log4cpp::CategoryStream::ENDLINE);
    
    // Step 1. Build the mapping between ( UserDN, CreamUDL ) -> list<CreamJobId>
    map< pair<string, string>, list< CreamJob >, ltstring> jobMap;
    
    for ( list< CreamJob >::const_iterator jit = j_list.begin(); jit != j_list.end(); ++jit ) {
        jobMap[ make_pair( jit->getUserDN(), jit->getCreamURL()) ].push_back( *jit );
    }
    
    // Step 2. Iterates over the map which has just been constructed.
    for(map< pair<string, string>, list<CreamJob> >::const_iterator jit = jobMap.begin(); jit != jobMap.end(); ++jit) {
        
        const string user_dn( jit->first.first );
        const string cream_url( jit->first.second );
        // list< list< CreamJob > > chunks( create_chunks( jit->second, m_max_chunk_size ) );
        
        // Step 2a. Interates over each chunk
        // for ( list< list< CreamJob > >::iterator chunk_it = chunks.begin();
        // chunk_it != chunks.end(); ++chunk_it ) {

        list< CreamJob >::const_iterator it = (jit->second).begin();
        list< CreamJob >::const_iterator list_end = (jit->second).end();
        while ( it != list_end ) {
            vector< string > jobs_to_poll;
            it = copy_n_elements( it, list_end, m_max_chunk_size, back_inserter( jobs_to_poll ) );
            // it = transform_n_elements( it, list_end, m_max_chunk_size, back_inserter( jobs_to_poll ), mem_fun_ref( &CreamJob::getCreamJobID ) );

            list< soap_proxy::JobInfo > j_status( check_multiple_jobs( user_dn, cream_url, jobs_to_poll ) );

            updateJobCache( j_status );      
        }        
    }
}


//____________________________________________________________________________
void iceCommandStatusPoller::remove_unknown_jobs_from_cache(const vector<string>& all_jobs, const vector< soap_proxy::JobInfo >& jobs_found ) throw()
{
    if ( all_jobs.size() == jobs_found.size() )
        return; // for all jobs in jobVec there's a corresponding JobInfo object in to_check

    // Ok, now lets try to be smart

    // Step 1. Build a vector of job ids which are in the to_check vector
    set< string > jobs_found_ids;
    for ( vector< soap_proxy::JobInfo >::const_iterator it=jobs_found.begin(); it != jobs_found.end(); ++it ) {
        jobs_found_ids.insert( it->getCreamJobID() );
    }

    set< string > all_job_ids;
    copy( all_jobs.begin(), all_jobs.end(), inserter( all_job_ids, all_job_ids.begin() ) );
    // copy( all_jobs.begin(), all_jobs.end(), insert_iterator( all_job_ids ) );

    // Step 2: do the difference on the sorted vectors
    // set< string > missing_job_ids;
    set_difference( all_job_ids.begin(), all_job_ids.end(),
                    jobs_found_ids.begin(), jobs_found_ids.end(),
                    job_cache_remover() );
}
