#include <string>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <classad_distribution.h>

#if (CONDORG_VERSION >= 653)
#include <globus_gram_protocol_constants.h>
#endif

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/jobid/JobId.h"
#include "glite/wms/common/logger/logstream.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileListLock.h"
#include "../../jobcontrol_namespace.h"
#include "../../common/ProxyUnregistrar.h"
#include "../../common/EventLogger.h"
#include "../../logmonitor/exceptions.h"

#include "JobResubmitter.h"

USING_COMMON_NAMESPACE;
using namespace std;
RenameLogStreamNS( elog );

JOBCONTROL_NAMESPACE_BEGIN {

namespace logmonitor { namespace processer {

JobResubmitter::JobResubmitter( jccommon::EventLogger *logger ) : jr_list(), jr_logger( logger )
{
  const configuration::WMConfiguration       *config = configuration::Configuration::instance()->wm();
  logger::StatePusher                         pusher( elog::cedglog, "JobResubmitter::JobResubmitter(...)" );

  try {
    this->jr_list.open( config->input() );

    elog::cedglog << logger::setlevel( logger::info )
		  << "FileList to WM initialized." << endl;
  }
  catch( utilities::FileContainerError &error ) {
    elog::cedglog << logger::setlevel( logger::fatal )
		  << "Cannot open FileList to WM." << endl
		  << "Error: \"" << error.string_error() << "\"." << endl;

    throw CannotOpenFile( config->input() );
  }
}

JobResubmitter::~JobResubmitter( void )
{}

void JobResubmitter::resubmit( int laststatus, const string &edgid, const string &sequence_code )
{
  const configuration::WMConfiguration       *config = configuration::Configuration::instance()->wm();
  utilities::FileListDescriptorMutex          flmutex( this->jr_list );
  classad::ClassAd                            command, arguments;
  logger::StatePusher                         pusher( elog::cedglog, "JobResubmitter::resubmit(...)" );

  elog::cedglog << logger::setlevel( logger::info )
		<< "Last known status = " << laststatus << endl;

  switch( laststatus ) {
#if (CONDORG_VERSION >= 653)
    case GLOBUS_GRAM_PROTOCOL_ERROR_USER_PROXY_EXPIRED:
      elog::cedglog << logger::setlevel( logger::warning ) << "Job has an expiring proxy." << endl
		    << logger::setlevel( logger::info ) << "Must not resubmit, but abort." << endl
		    << "EDG id = " << edgid << endl;

      this->jr_logger->abort_on_error_event( string("Job proxy is expired.") );

      jccommon::ProxyUnregistrar( edgid ).unregister();

      break;
#endif
  default:
    elog::cedglog << logger::setlevel( logger::info ) << "Resubmitting job to WM." << endl
		  << logger::setlevel( logger::debug ) << "EDG id = " << edgid << endl;

    command.InsertAttr( "version", string("1.0.0") );
    command.InsertAttr( "command", string("jobresubmit") );
    arguments.InsertAttr( "id", edgid );
    arguments.InsertAttr( "lb_sequence_code", sequence_code );
    command.Insert( "arguments", arguments.Copy() );

    this->jr_logger->job_resubmitting_event();
    this->jr_logger->job_wm_enqueued_start_event( config->input(), command );
    try {
      utilities::FileListLock   lock( flmutex );
      this->jr_list.push_back( command );
    }
    catch( utilities::FileContainerError &error ) {
      this->jr_logger->job_wm_enqueued_failed_event( config->input(), command, error.string_error() );

      throw CannotExecute( error.string_error() );
    }

    this->jr_logger->job_wm_enqueued_ok_event( config->input(), command );
    break;
  }

  return;
}

}} // Namespace processer, logmonitor

} JOBCONTROL_NAMESPACE_END;
