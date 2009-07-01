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
#include "iceCommandStatusPoller2.h"
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
#include "iceDb/Transaction.h"
#include "iceDb/RemoveJobByCid.h"
#include "iceDb/RemoveJobByGid.h"
#include "iceDb/UpdateJobByGid.h"
#include "iceDb/GetAllDNCE.h"
//#include "iceDb/GetOldestPollTimeForUserDNCE.h"
#include "iceDb/UpdateOldestPollTimeForUserDNCE.h"
#include "iceDb/GetStatusInfoByCompleteCreamJobID.h"
//#include "iceDb/InsertOldestPollTimeForUserDNCE.h"


// Cream Client API Headers
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/ce/cream-client-api-c/JobInfoWrapper.h"
#include "glite/ce/cream-client-api-c/scoped_timer.h"

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
namespace db         = glite::wms::ice::db;
using namespace glite::wms::ice::util;
using namespace std;

//time_t iceCommandStatusPoller2::s_last_seen = -1;

//______________________________________________________________________________
class processJobInfo {

  iceConfManager       *m_conf;//::getInstance()
  log4cpp::Category    *m_log_dev;
  iceLBLogger          *m_lb_logger;
  glite::wms::ice::Ice *m_iceManager;
  time_t               *m_last_time;

public:

  processJobInfo( glite::wms::ice::Ice* theIce, time_t* T ) 
    : m_conf(iceConfManager::getInstance()),
      m_log_dev(cream_api::util::creamApiLogger::instance()->getLogger()),
      m_lb_logger(iceLBLogger::instance()),
      m_iceManager( theIce ),
      m_last_time( T ) {}
		   
  void operator()( const soap_proxy::JobInfoWrapper& jobinfo ) 
  {
    
#ifdef ICE_PROFILE_ENABLE
    api_util::scoped_timer T0( "iceCommandStatusPoller2::processorJobInfo()() - ENTIRE METHOD" );
#endif
    static const char* method_name = "iceCommandStatusPoller2::processorJobInfo()() - ";

    vector< soap_proxy::JobStatusWrapper > status_changes;
    jobinfo.getStatus( status_changes );
    
    string completeJobID = jobinfo.getCreamURL();
    boost::replace_all( completeJobID, 
			m_conf->getConfiguration()->ice()->cream_url_postfix(), "" );
    
    completeJobID += "/" + jobinfo.getCreamJobID();
    
    CreamJob tmp_job;
    {
#ifdef ICE_PROFILE_ENABLE
      api_util::scoped_timer T2( "iceCommandStatusPoller2::processorJobInfo()() - GETJOBBYCID" );
#endif
      boost::recursive_mutex::scoped_lock M( CreamJob::s_globalICEMutex );
      glite::wms::ice::db::GetJobByCid getter( completeJobID );
      glite::wms::ice::db::Transaction tnx;
      tnx.execute( &getter );
      if( !getter.found() )
	{
	  CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name 
			 << "CREAM Job ID [" << completeJobID 
			 << "] disappeared from ICE database !"
			 );
	  return;
	}
      
      tmp_job = getter.get_job();
      
    }

//     if( tmp_job.get_num_logged_status_changes() >= status_changes.size()) {
//       CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
//                     << "No new states for CREAM Job ID ["
//                     << completeJobID << "]. Skipping"
// 		      );
//       return;
//     }

    CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                    << "Updating status for CREAM Job ID ["
                    << completeJobID << "]"
                    );
    
    tmp_job.set_workernode( jobinfo.getWorkerNode() );
    
    int count;
    vector< soap_proxy::JobStatusWrapper >::const_iterator it;
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
			 << " is reported as PURGED. Removing from cache"
			 ); 
	  {
	    if( tmp_job.is_proxy_renewable() )
	      DNProxyManager::getInstance()->decrementUserProxyCounter( tmp_job.getUserDN(), tmp_job.getMyProxyAddress() );
	    
	    glite::wms::ice::db::RemoveJobByCid remover( tmp_job.getCompleteCreamJobID() );
	    boost::recursive_mutex::scoped_lock M( CreamJob::s_globalICEMutex );
	    glite::wms::ice::db::Transaction tnx;
	    tnx.execute( &remover );
	  }
	  return;
	}
      
      
      
	/**
	   Update the job in the database only if the number of received states
	   is greater than the number of states of the current job.
	   In this case also check if the job must be purged or resubmitted
	*/ 
	if((*m_last_time) <= it->getTimestamp() )
	  (*m_last_time) = it->getTimestamp();

	//	if (  tmp_job.get_num_logged_status_changes() < count ) {
	  
	  /**
	     save the time of the most recent status in order to
	     return it to the caller.
	  */
	  
	  string exitCode( it->getExitCode() );
	  
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
	  { // DB UPDATE
	    list<pair<string, string> > params;
	    params.push_back( make_pair("worker_node", jobinfo.getWorkerNode()) );
	    /**
	       This update of releveat times is done outside, in the check_user_jobs 
	       method in order to prevent to re-poll always the same jobs 
	       if something goes wrong...
	       
	       params.push_back( make_pair("last_seen", int_to_string(time(0)  )) );
	       params.push_back( make_pair("last_empty_notification", int_to_string(time(0)  )));
	    */
	    params.push_back( make_pair("status", int_to_string(stNum)));
	    params.push_back( make_pair("exit_code", int_to_string(tmp_job.get_exit_code())));
	    //	    params.push_back( make_pair("num_logged_status_changes", int_to_string(count)));
	    params.push_back( make_pair("failure_reason", it->getFailureReason()));

#ifdef ICE_PROFILE_ENABLE
	    api_util::scoped_timer tmp_timer( "iceCommandStatusPoller2::processorJobInfo()() - UPDATEJOBBYGID" );
#endif
	    db::UpdateJobByGid updater( tmp_job.getGridJobID(), params);
	    boost::recursive_mutex::scoped_lock M( CreamJob::s_globalICEMutex );
	    db::Transaction tnx;
	    tnx.execute( &updater );
	  }
	  // Log to L&B
#ifdef ICE_PROFILE_ENABLE
	  api_util::scoped_timer T4( "iceCommandStatusPoller2::processorJobInfo()() - LOG_TO_LB+RESUBMIT_OR_PURGE" );
#endif
	  iceLBEvent* ev = iceLBEventFactory::mkEvent( tmp_job );
	  if ( ev ) {
	    tmp_job = m_lb_logger->logEvent( ev ); // also DB UPDATE
	  }
	  
	  /**
	     Let's check if the job must be purged or resubmitted
	     only if a new status has been received
	  */
#ifdef ICE_PROFILE_ENABLE
	  api_util::scoped_timer T5( "iceCommandStatusPoller2::processorJobInfo()() - RESUBMIT_OR_PURGE" );
#endif
	  m_iceManager->resubmit_or_purge_job( tmp_job/*jit*/ );
	  
	  //	} //  if (  tmp_job.get_num_logged_status_changes() < count ) {
	
	
      } // for over states
  } // operator()()
};

//____________________________________________________________________________
iceCommandStatusPoller2::iceCommandStatusPoller2(glite::wms::ice::Ice* theIce) 
  : iceAbsCommand( "iceCommandStatusPoller" ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_iceManager( theIce ),
    m_conf( iceConfManager::getInstance() ),
    m_stopped( false )
{
}

//____________________________________________________________________________
void iceCommandStatusPoller2::execute() throw( )
{
  static const char* method_name = "iceCommandStatusPoller2::execute() - ";

  list< boost::tuple<string, string, time_t> > userdn_ce;
  this->getUserDN_CreamURL( userdn_ce );

  list< boost::tuple<string, string, time_t> >::const_iterator it = userdn_ce.begin();
  
  /*
    Loop over all couples
    <UserDN,CreamURL>; for each
    couple get the last poller visited time;
    then and invoke real polling from that time to 'now'.
  */
  while( it != userdn_ce.end() ) {
    time_t last_poll_time;


    time_t last_poll_time_for_userdn_ce = 
      poll_userdn_ce( it->get<0>(), it->get<1>(), it->get<2>() );
    
    /**
       Update the table dn_ce_polltime with the new
       last poll time
    */
    if(last_poll_time_for_userdn_ce != -1 )
      {
	db::UpdateOldestPollTimeForUserDNCE updater( it->get<0>(), it->get<1>(), last_poll_time_for_userdn_ce );
	boost::recursive_mutex::scoped_lock M( CreamJob::s_globalICEMutex );
	db::Transaction tnx;
	tnx.execute( &updater );
      }
    
    ++it; // next tuple userdn/creamurl/last_seen_poll
  }
}

//____________________________________________________________________________
/**
 *
 *  This method returns a list of couples
 *  of <Userdn,CreamURL>
 *
 */
void
iceCommandStatusPoller2::getUserDN_CreamURL(list< boost::tuple<string, string, time_t> >& result) const
{
  static const char* method_name = "iceCommandStatusPoller2::getUserDN_CreamURL() - ";
  /**
     Obtain an array of different couples of userdn,creamurl
  */

#ifdef ICE_PROFILE_ENABLE
  api_util::scoped_timer tmp_timer2( "iceCommandStatusPoller2::getUserDN_CreamURL() - SQL SELECT couples userdn,CE + MUTEX" );
#endif
  
  boost::recursive_mutex::scoped_lock M( CreamJob::s_globalLastTimePollMutex );
  
#ifdef ICE_PROFILE_ENABLE
  api_util::scoped_timer tmp_timer3( "iceCommandStatusPoller2::getUserDN_CreamURL() - SQL SELECT couples userdn,CE" );
#endif
  /*
    SELECT userdn,creamurl FROM dn_ce_polltime;
  */
  db::GetAllDNCE getter;
  db::Transaction tnx;
  tnx.execute( &getter );
  //return getter.get();
  result = getter.get();
  
}

//____________________________________________________________________________
/**
 *
 *  This method connect to CREAM invoking
 *  the JobQuery method in order to retrieve
 *  all JobInfo objects related to jobs
 *  whose status changed since last_seen to now.
 *
 */
time_t 
iceCommandStatusPoller2::poll_userdn_ce( const std::string& userdn, 
					 const std::string& cream_url,
					 const time_t last_seen)
{
  static const char* method_name = "iceCommandStatusPoller2::poll_userdn_ce() - ";

#ifdef ICE_PROFILE_ENABLE
  api_util::scoped_timer tmp_timer2( "iceCommandStatusPoller2::poll_userdn_ce - ENTIRE METHOD" );
#endif

  CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		 << "Asking for job status changes for userdn ["
		 << userdn << "] to Cream URL ["
		 << cream_url << "] from [" 
		 << time_t_to_string( last_seen ) << "] to now."
		 );

  soap_proxy::JobFilterWrapper filter(std::vector< soap_proxy::JobIdWrapper>(), 
				      vector<string>(), 
				      -1, -1, 
				      "", "", 
				      last_seen, time(0), 
				      1, 0, 
				      "");
  
  soap_proxy::AbsCreamProxy::InfoArrayResult Iresult;
  


  /**
     Look for a valid proxy for the current userdn
  */
  string proxy( DNProxyManager::getInstance()->getAnyBetterProxyByDN( userdn ).get<0>() );
  if ( proxy.empty() ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		   << "A valid proxy file for DN [" << userdn
		   << "] is not available. Skipping polling for this user."
		   );
    return -1;
  }  
  if( !(isvalid( proxy ).first) ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		   << "Proxy ["
		   << proxy << "] for user ["
		   << userdn << "] is expired! Skipping polling for this user..."
		   );
    return -1;
  }
  
  CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
		 << "Authenticating with proxy [" << proxy << "] for userdn [" 
		 << userdn << "]..."
		 );
  
  string iceid = m_iceManager->getHostDN();
  boost::trim_if(iceid, boost::is_any_of("/"));
  boost::replace_all( iceid, "/", "_" );
  boost::replace_all( iceid, "=", "_" );

// #ifdef ICE_PROFILE_ENABLE
//   api_util::scoped_timer tmp_timer3( "iceCommandStatusPoller2::poll_userdn_ce - CONNECT TO CREAM" );
// #endif

  CreamProxy_Query( cream_url,
		    proxy,
		    &filter,
		    &Iresult,
		    iceid).execute( 3 );
  
  /**
   *
   * Process the information returned by CREAM
   *
   */
  time_t last_poll_time = -1;
  processJobInfo processor( m_iceManager, &last_poll_time );
  map< string, boost::tuple<soap_proxy::JobInfoWrapper::RESULT, soap_proxy::JobInfoWrapper, string> >::const_iterator it;
  it = Iresult.begin();
  /**
     Loop over all jobs returned for the couple userdn,cream_url
  */
  while( it != Iresult.end() ) {
    processor( it->second.get<1>() );
    ++it;
  }
  
  return last_poll_time;
}

