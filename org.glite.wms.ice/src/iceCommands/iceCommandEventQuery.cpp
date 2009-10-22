#include "glite/ce/cream-client-api-c/EventWrapper.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/scoped_timer.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"


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
#include "iceDb/GetFields.h"
#include "iceDb/GetEventID.h"
#include "iceDb/SetEventID.h"
#include "iceDb/GetJobByGid.h"
#include "iceDb/Transaction.h"
#include "iceDb/GetJobsByDbID.h"
#include "iceDb/RemoveJobByCid.h"
#include "iceDb/UpdateJobByGid.h"
#include "iceDb/RemoveJobsByDbID.h"

#include <boost/lexical_cast.hpp>
#include <boost/functional.hpp>
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"
#include "boost/format.hpp"

using namespace std;

using namespace glite::wms;


namespace cream_api  = glite::ce::cream_client_api;
namespace soap_proxy = glite::ce::cream_client_api::soap_proxy;

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
ice::util::iceCommandEventQuery::iceCommandEventQuery( ice::Ice* theIce )
  : iceAbsCommand( "iceCommandEventQuery" ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_lb_logger( ice::util::iceLBLogger::instance() ),
    m_iceManager( theIce ),
    m_conf( ice::util::iceConfManager::getInstance() ),
    m_stopped( false )
{

}

//______________________________________________________________________________
void ice::util::iceCommandEventQuery::execute( ) throw()
{
  static const char* method_name = "iceCommandEventQuery::execute() - ";
  //#ifdef ICE_PROFILE_ENABLE
    api_util::scoped_timer Tot( "iceCommandEventQuery::execute() - Entire Command Time" );
    //#endif
  list< vector< string > > result;
  {
    /*
      SELECT DISTINCT (userdn, creamurl) from jobs;
    */
    list< string > fields;
    fields.push_back( "userdn" );
    fields.push_back( "creamurl" );
    
    db::GetFields getter( fields, list< pair< string, string > >(), true/*=DISTINCT*/ );
    db::Transaction tnx;
    tnx.execute( &getter );
    result = getter.get_values();
  }

  list< vector< string > >::const_iterator it = result.begin();
  while( it != result.end() ) {
    
    list<soap_proxy::EventWrapper*> events;
    
    cleanup cleaner( &events );

    string userdn = it->at(0);
    string ceurl  = it->at(1);
    
    api_util::scoped_timer Totdnce( string("iceCommandEventQuery::execute() - Proc Time for DN,CE [") + userdn + "],[" + ceurl + "]" );
    
    
    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		   << "Retrieving last EVENT_ID for userdn ["
		   << userdn << "] and ce url ["
		   << ceurl << "]..."
		   );
    
    long long thisEventID = this->getEventID( userdn, ceurl );
    
    if( thisEventID == -1 ) {
      CREAM_SAFE_LOG(m_log_dev->warnStream() << method_name
		     << "Couldn't find any last EVENT_ID for current couple "
		     << "userdn [" 
		     << userdn << "] and ce url ["
		     << ceurl << "]. Inserting 0."
		     );
      
      this->setEventID( userdn, ceurl, 0 );
      
      thisEventID = 0;

    } else {

      CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		     << "Last EVENT_ID for current couple "
		     << "userdn [" 
		     << userdn << "] and ce url ["
		     << ceurl << "] is ["
		     << thisEventID << "]"
		     );
    }
    
    string proxy( DNProxyManager::getInstance()->getAnyBetterProxyByDN( userdn ).get<0>() );
    if ( proxy.empty() ) {
      CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
		      << "A valid proxy file for DN [" << userdn
		      << "] ce url ["
		      << ceurl << "] is not available. Skipping EventQuery."
		      );
      it++;
      continue; // Next couple DN,CE
      
    }  
    if( !(isvalid( proxy ).first) ) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Proxy ["
		     << proxy << "] for userdn ["
		     << userdn << "] is expired! Skipping EventQuery."
		     );
      it++;
      continue; // Next couple DN,CE
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
      CreamProxy_QueryEvent( ceurl, 
			     proxy, 
			     from.str(),
			     "-1",
			     m_iceManager->getStartTime(),
			     "JOB_STATUS",
			     400,
			     sdbid,
			     exec_time,
			     events,
			     iceid).execute( 3 );
      
    } catch(soap_proxy::auth_ex& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Cannot query events for UserDN ["
		     << userdn << "] CEUrl ["
		     << ceurl << "]. Exception auth_ex is [" << ex.what() << "]"
		     );

      it++;
      continue;
      
    } catch(soap_proxy::soap_ex& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Cannot query events for UserDN ["
		     << userdn << "] CEUrl ["
		     << ceurl << "]. Exception soap_ex is [" << ex.what() << "]"
		     );
      
      it++;
      continue;

    } catch(cream_api::cream_exceptions::InternalException& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Cannot query events for UserDN ["
		     << userdn << "] CEUrl ["
		     << ceurl << "]. Exception Internal ex is [" << ex.what() << "]"
		     );
      
      it++;
      continue;

    } catch(exception& ex) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Cannot query events for UserDN ["
		     << userdn << "] CEUrl ["
		     << ceurl << "]. Exception ex is [" << ex.what() << "]"
		     );
      it++;
      continue;
      
    } catch(...) {
      
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Cannot query status job for UserDN ["
		     << userdn << "] CEUrl ["
		     << ceurl << "]. Unknown Exception"
		     );
      it++;
      continue;
    }
    
    long long dbid = atoll(sdbid.c_str());
    
    if( !this->checkDatabaseID( ceurl, dbid ) ) {

      CREAM_SAFE_LOG(m_log_dev->warnStream() << method_name
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
	db::RemoveJobsByDbID remover( dbid );
	db::Transaction tnx;
	tnx.execute( &remover );
      }
      {
	db::SetEventID setter( userdn, ceurl, 0 );
	db::Transaction tnx;
	tnx.execute( &setter );
      }
    } // if( !this->checkDatabaseID..... )
    
    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		   << "There're [" << events.size() << "] event(s) "
		   << "for the couple DN ["
		   << userdn <<"] CEUrl [" 
		   << ceurl <<"]"
		   );

    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		   << "Database  ID=[" << sdbid << "]"
		   );
    
    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		   << "Exec time ID=[" << exec_time << "]"
		   );
    

//     for(list<soap_proxy::EventWrapper*>::const_iterator eit = events.begin();
// 	eit != events.end();
// 	++eit)
//       {
// 	cout << endl;
// 	CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
// 		       << "EventID  =[" << (*eit)->id << "]"
// 		       );
// 	CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
// 		       << "Timestamp=[" << (*eit)->timestamp << "]"
// 		       );
	
// 	map<string, string> properties;
// 	(*eit)->get_event_properties( properties );
// 	for(map<string, string>::const_iterator pit = properties.begin();
// 	    pit != properties.end();
// 	    ++pit)
// 	  {
// 	    if(pit->first == "type") {
// 	      //	      log_dev->info( string("  \t[status]=[") + job_statuses::job_status_str[atoi(pit->second.c_str())] + "]");
// 	      CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
// 			     << "  \t[status]=[" << cream_api::job_statuses::job_status_str[atoi(pit->second.c_str())] << "]"
// 			     );
	      
// 	    }
// 	    else {
// 	      CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
// 			     << "  \t[" << pit->first 
// 			     << "]=[" << pit->second << "]"
// 			     );
// 	    }
// 	  }
//       }
    
    if( events.size() ) {

      long long last_event_id = this->processEvents( events );
      
      this->setEventID( userdn, ceurl, last_event_id+1 );
    }
    it++;

  }  // while( it != result.end() ) { LOOP OVER COUPLES  DN,CE
}

//______________________________________________________________________________
long long 
ice::util::iceCommandEventQuery::getEventID( const string& dn, const string& ce)
{
  {
    db::GetEventID getter( dn, ce );
    db::Transaction tnx;
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
  CREAM_SAFE_LOG(m_log_dev->debugStream() << "iceCommandEventQuery::setEventID() - "
		 << "Setting EventID for UserDN ["
		 << dn <<"] CEUrl ["
		 << ce << "] to ["
		 << id << "]"
		 );

  db::SetEventID setter( dn, ce, id );
  db::Transaction tnx;
  tnx.execute( &setter );
}

//______________________________________________________________________________
void 
ice::util::iceCommandEventQuery::getJobsByDbID( std::list<ice::util::CreamJob>& jobs, 
						const long long db_id )
{
  db::GetJobsByDbID getter( jobs, db_id );
  db::Transaction tnx;
  tnx.execute( &getter );
}

//______________________________________________________________________________
bool
ice::util::iceCommandEventQuery::checkDatabaseID( const string& ceurl,
						  const long long dbid )
{
  db::GetDbID getter( ceurl );
  {
    db::Transaction tnx;
    tnx.execute( &getter);
  }
  if( !getter.found() ) {
    {
      /**
	 There's not yet association CEUrl -> DB_ID
	 Must be created ex-novo.
      */
      db::SetDbID setter( ceurl, dbid );
      db::Transaction tnx;
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
	db::SetDbID setter( ceurl, dbid );
	db::Transaction tnx;
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
    it++;
  }

  map< string, list<soap_proxy::EventWrapper*> >::const_iterator eit = mapped_events.begin();
  while( eit != mapped_events.end() ) {
    this->processEventsForJob( eit->first, eit->second );
    eit++;
  }

  return last_id;
}

//______________________________________________________________________________
void
ice::util::iceCommandEventQuery::processEventsForJob( const string& GID, 
						      const list<soap_proxy::EventWrapper*>& ev )
{
  static const char* method_name = "iceCommandEventQuery::processEventsForJob() - ";

  if(GID=="N/A") return;

  CreamJob tmp_job;
  
  {
#ifdef ICE_PROFILE_ENABLE
    api_util::scoped_timer T2( "iceCommandEventQuery::processEventsForJob() - GETJOBBYGID" );
#endif
    glite::wms::ice::db::GetJobByGid getter( GID );
    glite::wms::ice::db::Transaction tnx;
    tnx.execute( &getter );
    if( !getter.found() )
      {
	CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name 
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

  CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		 << "Processing [" << num_events << "] event(s) for Job ["
		 << tmp_job.describe() << "] userdn ["
		 << tmp_job.getUserDN() << "] and ce url ["
		 << tmp_job.getCreamURL() << "]."
		 );
  
  list<soap_proxy::EventWrapper*>::const_iterator evt_it = ev.begin();
  int evt_counter = 0;
  while( evt_it != ev.end() ) {
    
    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
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
    
    evt_counter++;
    evt_it++;
    
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
  static const char* method_name = "iceCommandEventQuery::processSingleEvent() - ";


//   CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
// 		 << "Dumping EVENT properties..."
// 		 ); 
  
//   map<string, string> pros;
//   event->get_event_properties( pros );
//   map<string, string>::const_iterator it = pros.begin();
//   while( it != pros.end() ) {
//     CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
// 		   << "[" << it->first << "]=["
// 		   << it->second << "]"
// 		   ); 
//     it++;
//   }

//  boost::scoped_ptr< soap_proxy::EventWrapper > evt_safe_ptr( event );

  cream_api::job_statuses::job_status status = (cream_api::job_statuses::job_status)atoi(event->getPropertyValue("type").c_str());

  string exit_code   = event->getPropertyValue("exitCode");
  string fail_reason = event->getPropertyValue("failureReason");
  string description = event->getPropertyValue("description");
  string worker_node = event->getPropertyValue("workerNode");

  theJob.set_workernode( worker_node );

  if ( status == cream_api::job_statuses::PURGED ) {
    CREAM_SAFE_LOG(m_log_dev->warnStream() << method_name
		   << "Job " << theJob.describe()
		   << " is reported as PURGED. Removing from database"
		   ); 
    {
      if( theJob.is_proxy_renewable() )
	DNProxyManager::getInstance()->decrementUserProxyCounter( theJob.getUserDN(), theJob.getMyProxyAddress() );
      db::RemoveJobByCid remover( theJob.getCompleteCreamJobID() );
      db::Transaction tnx;
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
    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		   << "Updating ICE's database for Job [" << theJob.describe()
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
    
    db::UpdateJobByGid updater( theJob.getGridJobID(), params);
    db::Transaction tnx;
    tnx.execute( &updater );
  }

  // Log to L&B
#ifdef ICE_PROFILE_ENABLE
  api_util::scoped_timer T4( "iceCommandEventQuery::processSingleEvent - LOG_TO_LB+RESUBMIT_OR_PURGE" );
#endif

  

  iceLBEvent* ev = iceLBEventFactory::mkEvent( theJob );
  if ( ev ) {
//     CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
// 		   << "Going to log to LB... Fail reason=["
// 		   << theJob.get_failure_reason() << "] WN=["
// 		   << theJob.get_worker_node() << "]";
// 		   );
    theJob = m_lb_logger->logEvent( ev );
  }
  
  /**
     Let's check if the job must be purged or resubmitted
     only if a new status has been received
  */
  removed = m_iceManager->resubmit_or_purge_job( theJob );
}
