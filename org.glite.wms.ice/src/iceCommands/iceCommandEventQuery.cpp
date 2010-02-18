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

#include "glite/ce/cream-client-api-c/EventWrapper.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/scoped_timer.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include "iceCommandStatusPoller.h"
#include "iceCommandEventQuery.h"
#include "iceLBEventFactory.h"
#include "CreamProxyMethod.h"
#include "DNProxyManager.h"
#include "iceConfManager.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "ice-core.h"
#include "iceUtils.h"

#include "iceDb/GetDbID.h"
#include "iceDb/SetDbID.h"
#include "iceDb/GetJobs.h"
#include "iceDb/GetFields.h"
#include "iceDb/DNHasJobs.h"
#include "iceDb/InsertStat.h"
#include "iceDb/GetEventID.h"
#include "iceDb/SetEventID.h"
#include "iceDb/GetJobByGid.h"
#include "iceDb/Transaction.h"
#include "iceDb/GetJobsByDbID.h"
#include "iceDb/RemoveJobByCid.h"
#include "iceDb/UpdateJobByGid.h"
#include "iceDb/RemoveJobsByDbID.h"
#include "iceDb/RemoveJobByUserDN.h"

#include <boost/lexical_cast.hpp>
#include <boost/functional.hpp>
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"
#include "boost/format.hpp"

using namespace std;

using namespace glite::wms;


namespace cream_api  = glite::ce::cream_client_api;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;
namespace iceUtil    = glite::wms::ice::util;

namespace {
  class cleanup {
    list<soap_proxy::EventWrapper*>* m_toclean;
    
  public:
    cleanup( list<soap_proxy::EventWrapper*>* toClean ) : m_toclean( toClean ) {}
    ~cleanup() {
      if(m_toclean) {
        list<soap_proxy::EventWrapper*>::iterator it = m_toclean->begin();
        while( it != m_toclean->end() ) {
          delete( *it );
          ++it;
        }
      }
    }
  };
}

//______________________________________________________________________________
ice::util::iceCommandEventQuery::iceCommandEventQuery( ice::Ice* theIce,
						       const std::string& dn,
						       const std::string& ce)
  : iceAbsCommand( "iceCommandEventQuery", "" ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_lb_logger( ice::util::iceLBLogger::instance() ),
    m_iceManager( theIce ),
    m_conf( ice::util::iceConfManager::getInstance() ),
    m_stopped( false ),
    m_dn( dn ),
    m_ce( ce )
{

}

//______________________________________________________________________________
void ice::util::iceCommandEventQuery::execute( const std::string& tid) throw()
{
 m_thread_id = tid;
  static const char* method_name = "iceCommandEventQuery::execute() - ";

    list<soap_proxy::EventWrapper*> events;
    cleanup cleaner( &events );

    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name << " TID=[" << getThreadID() << "] "
		   << "Retrieving last EVENT_ID for userdn ["
		   << m_dn << "] and ce url ["
		   << m_ce << "]..."
		   );
    
    long long thisEventID = this->getEventID( m_dn, m_ce );
    
    if( thisEventID == -1 ) {
      CREAM_SAFE_LOG(m_log_dev->warnStream() << method_name << " TID=[" << getThreadID() << "] "
		     << "Couldn't find any last EVENT_ID for current couple "
		     << "userdn [" 
		     << m_dn << "] and ce url ["
		     << m_ce << "]. Inserting 0."
		     );
      
      this->setEventID( m_dn, m_ce, 0 );
      
      thisEventID = 0;

    } else {

      CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name << " TID=[" << getThreadID() << "] "
		     << "Last EVENT_ID for current couple "
		     << "userdn [" 
		     << m_dn << "] and ce url ["
		     << m_ce << "] is ["
		     << thisEventID << "]"
		     );
    }
    
    boost::tuple<string, time_t, long long int> proxyinfo = DNProxyManager::getInstance()->getAnyBetterProxyByDN( m_dn );
    //string proxy( DNProxyManager::getInstance()->getAnyBetterProxyByDN( m_dn ).get<0>() );
    if ( proxyinfo.get<0>().empty() ) {
      CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name << " TID=[" << getThreadID() << "] "
		      << "A valid proxy file for DN [" << m_dn
		      << "] ce url ["
		      << m_ce << "] is not available. Skipping EventQuery. Going to remove all jobs of this user..."
		      );
      
      deleteJobsByDN( );
      
      return;
      
    }
    
    if ( proxyinfo.get<1>() < time(0) ) {
      CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name << " TID=[" << getThreadID() << "] "
		      << "The returned proxy ["
		      << proxyinfo.get<0>() << "] for DN [" << m_dn
		      << "] ce url ["
		      << m_ce << "] is not valid anymore. Skipping EventQuery. Going to remove all jobs of this user..."
		      );
      
      deleteJobsByDN( );
      
      return;
      
    }
    
    ostringstream from, to;
    from << thisEventID;
    
    string sdbid;
    time_t exec_time;
    
    string iceid = m_iceManager->getHostDN();
    boost::trim_if(iceid, boost::is_any_of("/"));
    boost::replace_all( iceid, "/", "_" );
    boost::replace_all( iceid, "=", "_" );
    
    try {
      api_util::scoped_timer Tot( "iceCommandEventQuery::execute() - SOAP Connection for QueryEvent" );
      vector<pair<string, string> > states;
      states.push_back( make_pair("STATUS", "RUNNING") );
      states.push_back( make_pair("STATUS", "REALLY-RUNNING") );
      states.push_back( make_pair("STATUS", "DONE-OK") );
      states.push_back( make_pair("STATUS", "DONE-FAILED") );
      states.push_back( make_pair("STATUS", "ABORTED") );
      states.push_back( make_pair("STATUS", "CANCELLED") );
      states.push_back( make_pair("STATUS", "HELD") );
      
      CreamProxy_QueryEvent( m_ce, 
			     proxyinfo.get<0>(), 
			     from.str(),
			     "-1",
			     m_iceManager->getStartTime(),
			     "JOB_STATUS",
			     500,
			     states,
			     sdbid,
			     exec_time,
			     events,
			     iceid,
			     false /* ignore blacklisted CE */).execute( 3 );
      
    } catch(soap_proxy::auth_ex& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name << " TID=[" << getThreadID() << "] "
		     << "Cannot query events for UserDN ["
		     << m_dn << "] CEUrl ["
		     << m_ce << "]. Exception auth_ex is [" << ex.what() << "]"
		     );

      
      return;
      
    } catch(soap_proxy::soap_ex& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name << " TID=[" << getThreadID() << "] "
		     << "Cannot query events for UserDN ["
		     << m_dn << "] CEUrl ["
		     << m_ce << "]. Exception soap_ex is [" << ex.what() << "]"
		     );
      
      
      return;

    } catch(cream_api::cream_exceptions::InternalException& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name << " TID=[" << getThreadID() << "] "
		     << "Cannot query events for UserDN ["
		     << m_dn << "] CEUrl ["
		     << m_ce << "]. Exception Internal ex is [" << ex.what() << "]"
		     );
      
      boost::regex pattern;
      boost::cmatch what;
      pattern = ".*No such operation 'QueryEventRequest'.*";
      if( boost::regex_match(ex.what(), what, pattern) ) {
	CREAM_SAFE_LOG(m_log_dev->warnStream() << method_name << " TID=[" << getThreadID() << "] "
		     << "Not present QueryEvent on CE ["
		     << m_ce << "]. Falling back to old-style StatusPoller."
		     );

	iceCommandStatusPoller( m_iceManager, make_pair( m_dn, m_ce), false ).execute( m_thread_id );
      }


      return ;

    } catch(exception& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name << " TID=[" << getThreadID() << "] "
		     << "Cannot query events for UserDN ["
		     << m_dn << "] CEUrl ["
		     << m_ce << "]. Exception ex is [" << ex.what() << "]"
		     );
      return;

    } catch(...) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name << " TID=[" << getThreadID() << "] "
		     << "Cannot query status job for UserDN ["
		     << m_dn << "] CEUrl ["
		     << m_ce << "]. Unknown Exception"
		     );
      return;
    }
    
    long long dbid = atoll(sdbid.c_str());
    
    if( !this->checkDatabaseID( m_ce, dbid ) ) {

      CREAM_SAFE_LOG(m_log_dev->warnStream() << method_name << " TID=[" << getThreadID() << "] "
		     << "*** CREAM HAS PROBABLY BEEN SCRATCHED. GOING TO ERASE"
		     << "ALL JOBS RELATED TO OLD DB_ID ["
		     << dbid << "] ***"
		     );

      list<CreamJob> jobs;
      this->getJobsByDbID( jobs, dbid );
      /**
	 TODO
	 Must enqueue them to log to LB
	 and then delete all of them.
      */
      {
	db::RemoveJobsByDbID remover( dbid, "iceCommandEventQuery::execute" );
	db::Transaction tnx(false, false);
	tnx.execute( &remover );
      }
      {
	db::SetEventID setter( m_dn, m_ce, 0, "iceCommandEventQuery::execute" );
	db::Transaction tnx(false, false);
	tnx.execute( &setter );
      }
    } // if( !this->checkDatabaseID..... )
    
    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name << " TID=[" << getThreadID() << "] "
		   << "There're [" << events.size() << "] event(s) "
		   << "for the couple DN ["
		   << m_dn <<"] CEUrl [" 
		   << m_ce <<"]"
		   );

    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name << " TID=[" << getThreadID() << "] "
		   << "Database  ID=[" << sdbid << "]"
		   );
    
    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name << " TID=[" << getThreadID() << "] "
		   << "Exec time ID=[" << exec_time << "]"
		   );
    
    if( events.size() ) {

      long long last_event_id = this->processEvents( events );
      
      this->setEventID( m_dn, m_ce, last_event_id+1 );
    }
}

//______________________________________________________________________________
long long 
ice::util::iceCommandEventQuery::getEventID( const string& dn, const string& ce)
{
#ifdef ICE_PROFILE
  iceUtil::ice_timer timer("iceCommandEventQuery::getEventID");
#endif
  {
    db::GetEventID getter( dn, ce, "iceCommandEventQuery::getEventID" );
    db::Transaction tnx(false, false);
    tnx.execute( &getter );
    if( getter.found() )
      return getter.getEventID();
    else
      return -1;
  }
}

//______________________________________________________________________________
void 
ice::util::iceCommandEventQuery::setEventID( const std::string& dn, 
					     const std::string& ce, 
					     const long long id)
{
#ifdef ICE_PROFILE
  iceUtil::ice_timer timer("iceCommandEventQuery::setEventID");
#endif
  CREAM_SAFE_LOG(m_log_dev->debugStream() << "iceCommandEventQuery::setEventID() - " << " TID=[" << getThreadID() << "] "
		 << "Setting EventID for UserDN ["
		 << dn <<"] CEUrl ["
		 << ce << "] to ["
		 << id << "]"
		 );

  db::SetEventID setter( dn, ce, id, "iceCommandEventQuery::setEventID" );
  db::Transaction tnx(false, false);
  tnx.execute( &setter );
}

//______________________________________________________________________________
void 
ice::util::iceCommandEventQuery::getJobsByDbID( std::list<ice::util::CreamJob>& jobs, 
						const long long db_id )
{
#ifdef ICE_PROFILE
  iceUtil::ice_timer timer("iceCommandEventQuery::getJobsByDbID");
#endif
  db::GetJobsByDbID getter( jobs, db_id, "iceCommandEventQuery::getJobsByDbID" );
  db::Transaction tnx(false, false);
  tnx.execute( &getter );
}

//______________________________________________________________________________
bool
ice::util::iceCommandEventQuery::checkDatabaseID( const string& ceurl,
						  const long long dbid )
{
#ifdef ICE_PROFILE
  iceUtil::ice_timer timer("iceCommandEventQuery::checkDatabaseID");
#endif
  db::GetDbID getter( ceurl, "iceCommandEventQuery::checkDatabaseID" );
  {
    db::Transaction tnx(false, false);
    tnx.execute( &getter);
  }
  if( !getter.found() ) {
    {
      /**
	 There's not yet association CEUrl -> DB_ID
	 Must be created ex-novo.
      */
      db::SetDbID setter( ceurl, dbid, "iceCommandEventQuery::checkDatabaseID" );
      db::Transaction tnx(false, false);
      tnx.execute( &setter );
    }
    return true;

  } else {
    /** 
	The DB_ID for CE does exist.
    */
    if( getter.getDbID() != dbid )
      {
	/**
	   CREAM has been scratched because the 
	   previously saved DB_ID differs
	   from this one
	*/
	db::SetDbID setter( ceurl, dbid, "iceCommandEventQuery::checkDatabaseID" );
	db::Transaction tnx(false, false);
	tnx.execute( &setter );
	return false;
      }

    return true;
  }

  return true;
}

//______________________________________________________________________________
long long
ice::util::iceCommandEventQuery::processEvents( list<soap_proxy::EventWrapper*>& events )
{
#ifdef ICE_PROFILE
  iceUtil::ice_timer timer("iceCommandEventQuery::processEvents");
#endif
  api_util::scoped_timer procTimeEvents( "iceCommandEventQuery::processEvents() - All Events Proc Time" );
  /**
     Group the events per GridJobID
  */
  map< string, list<soap_proxy::EventWrapper*> > mapped_events;

  long long last_id = 0;
  
  list<soap_proxy::EventWrapper*>::const_iterator it = events.begin();
  while( it != events.end() ) {

    long long tmpid = atoll((*it)->id.c_str() ) ;
    
    if( tmpid > last_id )
      last_id = tmpid;

    mapped_events[ (*it)->getPropertyValue("gridJobId") ].push_back( *it );
    ++it;
  }

  map< string, list<soap_proxy::EventWrapper*> >::const_iterator eit = mapped_events.begin();
  while( eit != mapped_events.end() ) {
    this->processEventsForJob( eit->first, eit->second );
    ++eit;
  }

  return last_id;
}

//______________________________________________________________________________
void
ice::util::iceCommandEventQuery::processEventsForJob( const string& GID, 
						      const list<soap_proxy::EventWrapper*>& ev )
{
#ifdef ICE_PROFILE
  iceUtil::ice_timer timer("iceCommandEventQuery::processEventsForJob");
#endif

  static const char* method_name = "iceCommandEventQuery::processEventsForJob() - ";

  if(GID=="N/A") return;

  CreamJob tmp_job;
  
  {
#ifdef ICE_PROFILE_ENABLE
    api_util::scoped_timer T2( "iceCommandEventQuery::processEventsForJob() - GETJOBBYGID" );
#endif
    glite::wms::ice::db::GetJobByGid getter( GID, "iceCommandEventQuery::processEventsForJob" );
    glite::wms::ice::db::Transaction tnx(false, false);
    tnx.execute( &getter );
    if( !getter.found() )
      {
	CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name  << " TID=[" << getThreadID() << "] "
		       << "GridJobID [" << GID
		       << "] disappeared!"
		       );
	return;
      }
    
    tmp_job = getter.get_job();
    
  }
  
  int num_events = ev.size();

  if( !num_events ) // this should not happen
    return;

  CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name << " TID=[" << getThreadID() << "] "
		 << "Processing [" << num_events << "] event(s) for Job ["
		 << tmp_job.describe() << "] userdn ["
		 << tmp_job.get_user_dn() << "] and ce url ["
		 << tmp_job.get_creamurl() << "]."
		 );
  
  list<soap_proxy::EventWrapper*>::const_iterator evt_it = ev.begin();
  int evt_counter = 0;
  while( evt_it != ev.end() ) {
    
    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name << " TID=[" << getThreadID() << "] "
		   << "EventID ["
		   << (*evt_it)->id << "] timestsamp ["
		   << (*evt_it)->timestamp << "]"
		 );
    
    bool removed = false;
    /**
       WRITE ON ICE's DATABASE ONLY IF THIS IS THE LAST EVENT
       FOR THE CURRENT JOB, i.e. if evt_counter == (num_events-1)
    */
    this->processSingleEvent( tmp_job, *evt_it, evt_counter == (num_events-1), removed );

    if(removed)
      return;
    
    ++evt_counter;
    ++evt_it;
    
  }

  //delete( ev );
}

//______________________________________________________________________________
void 
ice::util::iceCommandEventQuery::processSingleEvent( CreamJob& theJob, 
						     soap_proxy::EventWrapper* event,
						     const bool is_last_event,
						     bool& removed )
{
#ifdef ICE_PROFILE
  iceUtil::ice_timer timer("iceCommandEventQuery::processSingleEvent");
#endif
  static const char* method_name = "iceCommandEventQuery::processSingleEvent() - ";


  string jobdesc( theJob.describe() );

  cream_api::job_statuses::job_status status = (cream_api::job_statuses::job_status)atoi(event->getPropertyValue("type").c_str());

  string exit_code   = event->getPropertyValue("exitCode");
  string fail_reason = event->getPropertyValue("failureReason");
  string description = event->getPropertyValue("description");
  string worker_node = event->getPropertyValue("workerNode");

  theJob.set_worker_node( worker_node );

#ifdef GLITE_WMS_ICE_ENABLE_STATS
  {
    db::InsertStat inserter( /*event->timestamp*/ time(0), event->timestamp,(short)status, "iceCommandEventQuery::processSingleEvent" );
    db::Transaction tnx(false, false);
    tnx.execute( &inserter );
  }
#endif

  if ( status == cream_api::job_statuses::PURGED ) {
    CREAM_SAFE_LOG(m_log_dev->warnStream() << method_name << " TID=[" << getThreadID() << "] "
		   << "Job [" << jobdesc
		   << "] is reported as PURGED. Removing from database"
		   ); 
    {
      if( theJob.is_proxy_renewable() )
	DNProxyManager::getInstance()->decrementUserProxyCounter( theJob.get_user_dn(), theJob.get_myproxy_address() );
      db::RemoveJobByCid remover( theJob.get_complete_cream_jobid(), "iceCommandEventQuery::processSingleEvent" );
      db::Transaction tnx(false, false);
      tnx.execute( &remover );
    }
    removed = true;
    return;
  }

  

  theJob.set_status( status );
  
  try {
    theJob.set_exitcode( boost::lexical_cast< int >( exit_code ) );
  } catch( boost::bad_lexical_cast & ) {
    theJob.set_exitcode( 0 );
  }
  //
  // See comment in normalStatusNotification.cpp
  //
  if ( status == cream_api::job_statuses::CANCELLED ) {
    theJob.set_failure_reason( description );
  } else {
    theJob.set_failure_reason( fail_reason );
  }

  /**
     WRITE ON ICE's DATABASE ONLY IF THIS IS THE LAST EVENT
     FOR THE CURRENT JOB
  */
  if(is_last_event) {
    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name << " TID=[" << getThreadID() << "] "
		   << "Updating ICE's database for Job [" << jobdesc
		   << "] status = [" << cream_api::job_statuses::job_status_str[ status ] << "]"
		   << " exit_code = [" << exit_code << "]"
		   << " failure_reason = [" << fail_reason << "]"
		   << " description = [" << description << "]"
		   );
    list<pair<string, string> > params;
    params.push_back( make_pair("worker_node", worker_node ) );
    /**
       This update of releveat times is done outside, in the check_user_jobs 
       method in order to prevent to re-poll always the same jobs 
       if something goes wrong...
       
       params.push_back( make_pair("last_seen", int_to_string(time(0)  )) );
       params.push_back( make_pair("last_empty_notification", int_to_string(time(0)  )));
    */
    params.push_back( make_pair("status", int_to_string(status)));
    params.push_back( make_pair("exit_code", int_to_string(theJob.get_exit_code())));
    params.push_back( make_pair("failure_reason", theJob.get_failure_reason() ));
    
    db::UpdateJobByGid updater( theJob.get_grid_jobid(), params, "iceCommandEventQuery::processSingleEvent");
    db::Transaction tnx(false, false);
    tnx.execute( &updater );
  }

  // Log to L&B
#ifdef ICE_PROFILE_ENABLE
  api_util::scoped_timer T4( "iceCommandEventQuery::processSingleEvent - LOG_TO_LB+RESUBMIT_OR_PURGE" );
#endif

  

  iceLBEvent* ev = iceLBEventFactory::mkEvent( theJob );
  if ( ev ) {
    theJob = m_lb_logger->logEvent( ev );
  }
  
  /**
     Let's check if the job must be purged or resubmitted
     only if a new status has been received
  */
  if(is_last_event)
    removed = m_iceManager->resubmit_or_purge_job( theJob );
  else
    removed = false;
}



//______________________________________________________________________________
void 
ice::util::iceCommandEventQuery::deleteJobsByDN( void ) throw( )
{

  list< CreamJob > results;
  {
    list<pair<string, string> > clause;
    clause.push_back( make_pair( "userdn", m_dn ) );
    
    db::GetJobs getter( clause, results, "iceCommandEventQuery::deleteJobsForDN" );
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
    ++jit;
  }

  {
    db::RemoveJobByUserDN remover( m_dn, "iceCommandEventQuery::deleteJobsForDN" );
    db::Transaction tnx( false, false );
    tnx.execute( &remover );
  }

}
