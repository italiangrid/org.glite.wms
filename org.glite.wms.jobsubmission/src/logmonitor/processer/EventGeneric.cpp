#include <cstring>
#include <cstdio>
#include <ctime>

#include <string>

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include <user_log.c++.h>

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wms/common/logger/logstream.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/LMConfiguration.h"
#include "../../common/EventLogger.h"
#include "../../common/IdContainer.h"
#include "../../common/JobFilePurger.h"
#include "../../common/ProxyUnregistrar.h"
#include "../../controller/JobController.h"
#include "../../logmonitor/exceptions.h"
#include "../../logmonitor/Timer.h"
#include "../../logmonitor/AbortedContainer.h"

#include "EventGeneric.h"
#include "MonitorData.h"

using namespace std;
USING_COMMON_NAMESPACE;
RenameLogStreamNS( elog );

JOBCONTROL_NAMESPACE_BEGIN {

namespace logmonitor { namespace processer {

namespace {

ULogEvent *instantiate_generic_event( jccommon::generic_event_t evn, int condorid, time_t epoch )
{
  ULogEvent           *event;
  GenericEvent        *generic_event;
  string               msg;

  event = instantiateEvent( ULOG_GENERIC );
  event->cluster = condorid;
  event->proc = event->subproc = 0;

  localtime_r( &epoch, &event->eventTime );

  switch( evn ) {
  case jccommon::force_remove:
    msg.assign( "LM: " );
    msg.append( boost::lexical_cast<string>(static_cast<int>(evn)) );
    msg.append( " - Must force job removal" );

    break;
  default:
    msg.assign( "LM: 0 - Null event" );

    break;
  }

  generic_event = dynamic_cast<GenericEvent *>( event );
  strncpy( generic_event->info, msg.c_str(), 128 );

  return event;
}

} // Anonymous namespace

EventGeneric::EventGeneric( ULogEvent *event, MonitorData *data ) : EventInterface( event, data ),
								    eg_event( dynamic_cast<GenericEvent *>(event) )
{}

EventGeneric::~EventGeneric( void )
{}

void EventGeneric::finalProcess( int cn, const string &message )
{
  const configuration::LMConfiguration   *conf = configuration::Configuration::instance()->lm();

  time_t                                  epoch;
  jccommon::generic_event_t               code( static_cast<jccommon::generic_event_t>(cn) );
  ULogEvent                              *event;
  char                                    wbuf[30];
  string                                  when;
  jccommon::IdContainer::iterator         position, dagposition;
  logger::StatePusher                     pusher( elog::cedglog, "EventGeneric::finalProcess(...)" );

  position = this->ei_data->md_container->position_by_condor_id( this->ei_condor );

  if( position == this->ei_data->md_container->end() )
    elog::cedglog << logger::setlevel( logger::warning )
		  << "Job has just been removed from the queue." << endl
		  << "Action not performed..." << endl;
  else {
    switch( code ) {
    case jccommon::null_event:
    default:
      elog::cedglog << logger::setlevel( logger::warning )
		    << "Null event ??? Hmmm... What is its meaning ???" << endl
		    << logger::setlevel( logger::info )
		    << "This event can be discarded..." << endl;

      break;
    case jccommon::user_cancelled_event:
      elog::cedglog << logger::setlevel( logger::info )
		    << "Job cancelled by the user, no resubmission should be triggered off. "  << endl;
      // no resubmission, the job has been cancelled by the user
      this->ei_data->md_container->update_pointer( position, this->ei_data->md_logger->sequence_code(), ULOG_GENERIC, jccommon::no_resubmission );
      break;
    case jccommon::cancelled_event:
      elog::cedglog << logger::setlevel( logger::info )
		    << "Attaching force remove timeout to cluster " << this->ei_condor << endl;

      epoch = time( NULL ) + conf->aborted_jobs_timeout();
      event = instantiate_generic_event( jccommon::force_remove, this->eg_event->cluster, epoch );

      asctime_r( &event->eventTime, wbuf );
      when.assign( wbuf, 24 );

      elog::cedglog << logger::setlevel( logger::info )
		    << "Timeout force removal will happen in " << conf->aborted_jobs_timeout() << " seconds." << endl
		    << logger::setlevel( logger::debug ) << "At: " << when << endl;

      this->ei_data->md_timer->start_timer( epoch, event );

      break;
    case jccommon::force_remove: {
      controller::JobController     controller( *this->ei_data->md_logger );

      elog::cedglog << logger::setlevel( logger::info )
		    << "Sending force removal request to JC for cluster " << this->ei_condor << endl
		    << logger::setlevel( logger::debug )
		    << "This is try n. " << position->retry_count() << endl;

      this->ei_data->md_container->increment_pointer_retry_count( position );
      controller.cancel( this->eg_event->cluster, this->ei_data->md_logfile_name.c_str() );

      break;
    }
    case jccommon::cannot_cancel_event: {
      int       retry = position->retry_count(), maxretries = conf->force_cancellation_retries();
      string    edgid( position->edg_id() );

      if( retry > maxretries ) {
	if( this->ei_data->md_isDagLog && (edgid != this->ei_data->md_dagId) ) {
	  controller::JobController   controller( *this->ei_data->md_logger );

	  elog::cedglog << logger::setlevel( logger::error )
			<< "Forced cancellation retries exceeded maximum (" << retry << '/' << maxretries << ')' << endl
			<< ei_s_subnodeof << this->ei_data->md_dagId << endl
			<< "As I cannot hope to remove such job anymore, I try to remove the DAG, sorry :(" << endl;

	  dagposition = this->ei_data->md_container->position_by_edg_id( this->ei_data->md_dagId );
	  if( this->ei_data->md_aborted->insert(dagposition->condor_id()) ) {
	    elog::cedglog << logger::setlevel( logger::fatal ) << ei_s_failedinsertion << endl;

	    throw CannotExecute( ei_s_failedinsertion );
	  }

	  this->ei_data->md_logger->reset_user_proxy( dagposition->proxy_file() );
	  this->ei_data->md_logger->reset_context( dagposition->edg_id(), dagposition->sequence_code() );
	  this->ei_data->md_logger->aborted_by_system_event( string("Forced cancellation of a node of the DAG failed,"
								    " removing the whole DAG.") );

	  controller.cancel( this->ei_data->md_dagId, this->ei_data->md_logfile_name.c_str(), true );
	  this->ei_data->md_container->update_pointer( dagposition, this->ei_data->md_logger->sequence_code(), ULOG_GENERIC );
	}
	else {
	  elog::cedglog << logger::setlevel( logger::severe )
			<< "Forced cancellation retries exceeded maximum (" << retry << '/' << maxretries << ')' << endl
			<< "Job will be removed from the queue and aborted." << endl;

	  this->ei_data->md_logger->reset_user_proxy( position->proxy_file() ).reset_context( edgid, position->sequence_code() );
	  this->ei_data->md_logger->abort_on_error_event( string("Removal retries exceeded.") );

	  jccommon::ProxyUnregistrar( edgid ).unregister();
	  jccommon::JobFilePurger( this->ei_data->md_dagId, edgid ).do_purge();

	  this->ei_data->md_container->remove( position );
	  this->ei_data->md_aborted->remove( this->ei_condor );
	  this->ei_data->md_timer->remove_all_timeouts( this->eg_event->cluster );
	}
      }
      else {
	elog::cedglog << logger::setlevel( logger::info )
		      << "Attaching force remove timeout to cluster " << this->ei_condor << endl
		      << logger::setlevel( logger::debug ) << "This is try n. " << (retry + 1) << endl;

	epoch = time( NULL ) + conf->aborted_jobs_timeout();

	event = instantiate_generic_event( jccommon::force_remove, this->eg_event->cluster, epoch );

	asctime_r( &event->eventTime, wbuf );
	when.assign( wbuf, 24 );

	elog::cedglog << logger::setlevel( logger::info ) 
		      << "Timeout force removal will happen in " << conf->aborted_jobs_timeout() << " seconds." << endl
		      << logger::setlevel( logger::debug ) << "At: " << when << endl;

	this->ei_data->md_container->increment_pointer_retry_count( position );
	this->ei_data->md_timer->start_timer( epoch, event );
      }

      break;
    }
    }
  }

  return;
}

void EventGeneric::process_event( void )
{
  string                                          info( this->eg_event->info ), code, message;
  boost::match_results<string::const_iterator>    match_pieces;
  logger::StatePusher                             pusher( elog::cedglog, "EventGeneric::process_event()" );

  static boost::regex  jcexpr( "^JC: ([0-9]+) - (.*)$" );
  static boost::regex  lmexpr( "^LM: ([0-9]+) - (.*)$" );

  elog::cedglog << logger::setlevel( logger::info ) << "Got a generic event." << endl
		<< "For cluster " << this->ei_condor << endl
		<< logger::setlevel( logger::debug ) << "Is this a message directed to me ??" << endl;

  if( boost::regex_match(info, match_pieces, jcexpr) ) {
    code.assign( match_pieces[1].first, match_pieces[1].second );
    message.assign( match_pieces[2].first, match_pieces[2].second );

    elog::cedglog << logger::setlevel( logger::debug ) << "Yes, it is a message from my beloved JobController !" << endl
		  << logger::setlevel( logger::info ) << "Message in the event says: \"" << message << "\"." << endl;

    this->finalProcess( boost::lexical_cast<int>(code), message );
  }
  else if( boost::regex_match(info, match_pieces, lmexpr) ) {
    code.assign( match_pieces[1].first, match_pieces[1].second );
    message.assign( match_pieces[2].first, match_pieces[2].second );

    elog::cedglog << logger::setlevel( logger::debug ) << "This seems to be a message coming from an expired timeout !" << endl
		  << logger::setlevel( logger::info ) << "Message in the event says: \"" << message << "\"." << endl;

    this->finalProcess( boost::lexical_cast<int>(code), message );
  }
  else
    elog::cedglog << logger::setlevel( logger::warning )
		  << "Unrecognized generic event: ignoring..." << endl;

  return;
}

}} // namespace processer, logmonitor

} JOBCONTROL_NAMESPACE_END;
