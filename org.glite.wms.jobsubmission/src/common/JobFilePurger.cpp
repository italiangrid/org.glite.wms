#include <memory>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>



#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/JCConfiguration.h"
#include "glite/wms/common/configuration/LMConfiguration.h"
#include "glite/wms/common/logger/logstream.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/jobid/manipulation.h"
#include "glite/wms/jobid/JobId.h"
#include "glite/wms/common/utilities/boost_fs_add.h"
#ifdef ENABLE_PURGER
#include "glite/wms/purger/purger.h"
#endif
#include "../jobcontrol_namespace.h"
#include "../common/files.h"

#include "JobFilePurger.h"

USING_COMMON_NAMESPACE;
using namespace std;
RenameLogStreamNS( elog );

JOBCONTROL_NAMESPACE_BEGIN {

namespace jccommon {

JobFilePurger::JobFilePurger( const glite::wms::jobid::JobId &id, bool isdag ) : jfp_isDag( isdag ), jfp_jobId( id ), jfp_dagId() {}

JobFilePurger::JobFilePurger( const glite::wms::jobid::JobId &dagid, const glite::wms::jobid::JobId &jobid ) : jfp_isDag( false ),
										       jfp_jobId( jobid ), jfp_dagId( dagid )
{}

JobFilePurger::~JobFilePurger( void ) {}

void JobFilePurger::do_purge( bool everything )
{
  const configuration::LMConfiguration    *lmconfig = configuration::Configuration::instance()->lm();
  logger::StatePusher                      pusher( elog::cedglog, "JobFilePurger::do_purge(...)" );
#ifdef ENABLE_PURGER
  bool     purge;
#endif

  if( lmconfig->remove_job_files() ) {
    unsigned long int            removed;
    auto_ptr<Files>              files( this->jfp_dagId.isSet() ? new Files(this->jfp_dagId, this->jfp_jobId) : 
					new Files(this->jfp_jobId) );

    try {
      elog::cedglog << logger::setlevel( logger::info ) << "Removing job directory: " << files->output_directory().file_path() << endl;
      removed = boost::filesystem::remove_all( files->output_directory() );
      elog::cedglog << logger::setlevel( logger::ugly ) << "Removed " << removed << " files." << endl;
    }
    catch( boost::filesystem::filesystem_error &err ) {
      elog::cedglog << logger::setlevel( logger::null ) << "Failed to remove job directory." << endl
		    << "Reason: " << err.what() << endl;
    }

    try {
      elog::cedglog << logger::setlevel( logger::info ) << "Removing submit file: " << files->submit_file().file_path() << endl;
      boost::filesystem::remove( files->submit_file() );
      elog::cedglog << logger::setlevel( logger::ugly ) << "Removed..." << endl;
    }
    catch( boost::filesystem::filesystem_error &err ) {
      elog::cedglog << logger::setlevel( logger::null ) << "Failed to remove submit file." << endl
		    << "Reason: " << err.what() << endl;
    }

    try {
      elog::cedglog << logger::setlevel( logger::info ) << "Removing classad file: " << files->classad_file().file_path() << endl;
      boost::filesystem::remove( files->classad_file() );
      elog::cedglog << logger::setlevel( logger::ugly ) << "Removed..." << endl;
    }
    catch( boost::filesystem::filesystem_error &err ) {
      elog::cedglog << logger::setlevel( logger::null ) << "Failed to remove classad file." << endl
		    << "Reason: " << err.what() << endl;
    }

    if( this->jfp_isDag ) {
      try {
	elog::cedglog << logger::setlevel( logger::info ) << "Removing DAG submit directory: "
		     << files->dag_submit_directory().file_path() << endl;
	removed = boost::filesystem::remove_all( files->dag_submit_directory() );
	elog::cedglog << logger::setlevel( logger::ugly ) << "Removed " << removed << " files." << endl;
      }
      catch( boost::filesystem::filesystem_error &err ) {
	elog::cedglog << logger::setlevel( logger::null ) << "Failed to remove DAG submit directory." << endl
		      << "Reason: " << err.what() << endl;
      }
    }
    else {
      try {
	elog::cedglog << logger::setlevel( logger::info ) << "Removing wrapper file: " << files->jobwrapper_file().file_path() << endl;
	boost::filesystem::remove( files->jobwrapper_file() );
	elog::cedglog << logger::setlevel( logger::ugly ) << "Removed..." << endl;
      }
      catch( boost::filesystem::filesystem_error &err ) {
	elog::cedglog << logger::setlevel( logger::null ) << "Failed to remove wrapper file." << endl
		      << "Reason: " << err.what() << endl;
      }
    }
  }
  else
    elog::cedglog << logger::setlevel( logger::info )
		  << "Job files not removed." << endl;

  if( everything ) {
    elog::cedglog << logger::setlevel( logger::ugly ) << "Going to purge job storage..." << endl;

#ifdef ENABLE_PURGER
    purge = purger::purgeStorage( this->jfp_jobId );

    elog::cedglog << logger::setlevel( logger::verylow ) << "Purging command returned " << (purge ? "ok" : "an error") << endl;
#else
    elog::cedglog << logger::setlevel( logger::null ) << "Job purging support not compiled." << endl;
#endif
  }

  return;
}

}; // jccommon namespace

} JOBCONTROL_NAMESPACE_END;
