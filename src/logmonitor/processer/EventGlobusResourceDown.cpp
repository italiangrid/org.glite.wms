#include <cstdio>
#include <ctime>

#include <user_log.c++.h>

#include "glite/wms/common/logger/logstream.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "../../common/EventLogger.h"
#include "../../common/IdContainer.h"
#include "../../controller/JobController.h"
#include "../../logmonitor/exceptions.h"
#include "../../logmonitor/AbortedContainer.h"

#include "EventGlobusResourceDown.h"
#include "MonitorData.h"

using namespace std;
USING_COMMON_NAMESPACE;
RenameLogStreamNS( elog );

JOBCONTROL_NAMESPACE_BEGIN {

namespace logmonitor { namespace processer {

EventGlobusResourceDown::EventGlobusResourceDown( ULogEvent *event, MonitorData *data ) : EventInterface( event, data ), 
  egrd_event( dynamic_cast<GlobusResourceDownEvent *>(event) )
{}

EventGlobusResourceDown::~EventGlobusResourceDown( void )
{}

void EventGlobusResourceDown::process_event( void )
{
  jccommon::IdContainer::iterator        position;
  controller::JobController              controller( *this->ei_data->md_logger );
  logger::StatePusher                    pusher( elog::cedglog, "EventGlobusResourceDown::process_event()" );

  elog::cedglog << logger::setlevel( logger::info ) << "Got a Globus resource down event." << endl
		<< "For cluster: " << this->ei_condor << endl
		<< "On ce: " << this->egrd_event->rmContact << endl;

  position = this->ei_data->md_container->position_by_condor_id( this->ei_condor );

  if( position == this->ei_data->md_container->end() )
    elog::cedglog << logger::setlevel( logger::warning ) << ei_s_notsub << endl;
  else {
    elog::cedglog << logger::setlevel( logger::info ) << ei_s_edgideq << position->edg_id() << endl;

    if( this->ei_data->md_isDagLog )
      elog::cedglog << ei_s_subnodeof << this->ei_data->md_dagId << endl;

    if( this->ei_data->md_aborted->insert(this->ei_condor) ) {
      elog::cedglog << logger::setlevel( logger::fatal ) << ei_s_failedinsertion << endl;

      throw CannotExecute( ei_s_failedinsertion );
    }

    this->ei_data->md_logger->reset_user_proxy( position->proxy_file() ).reset_context( position->edg_id(), position->sequence_code() );
    this->ei_data->md_logger->globus_resource_down_event();

    elog::cedglog << logger::setlevel( logger::info ) << "Forwarding remove request to JC." << endl;

    if( this->ei_data->md_isDagLog ) {
      elog::cedglog << logger::setlevel( logger::debug )
		    << "Forwarding request by Condor Id..." << endl;

      controller.cancel( this->egrd_event->cluster, this->ei_data->md_logfile_name.c_str(), false );
    }
    else {
      elog::cedglog << logger::setlevel( logger::debug )
		    << "Forwarding request by EDG Id..." << endl;

      controller.cancel( glite::wmsutils::jobid::JobId(position->edg_id()), this->ei_data->md_logfile_name.c_str() );
    }

    this->ei_data->md_container->update_pointer( position, this->ei_data->md_logger->sequence_code(), this->egrd_event->eventNumber );
  }

  return;
}

}} // namespace processer, logmonitor

} JOBCONTROL_NAMESPACE_END;
