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
#include <string>
#include <iostream>
#include <sys/types.h>          // getpid(), getpwnam()
#include <unistd.h>             // getpid(), usleep
#include <pwd.h>                // getpwnam()
#include <cstdio>               // popen()
#include <cstdlib>              // atoi()
#include <csignal>
#include <getopt.h>
#include <vector>

//#include "glite/ce/cream-client-api-c/creamApiLogger.h"
//#include "glite/ce/cream-client-api-c/job_statuses.h"
//#include "glite/ce/cream-client-api-c/VOMSWrapper.h"

#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"

#include "iceUtils/creamJob.h"
#include "iceDb/GetAllJobs.h"
#include "iceDb/GetJobByGid.h"
#include "iceDb/Transaction.h"
#include "CreamProxyMethod.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "iceLBEvent.h"
#include "iceLBLogger.h"
#include "iceLBEventFactory.h"

#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/ce/cream-client-api-c/ResultWrapper.h"


//#include "glite/ce/cream-client-api-c/certUtil.h"

#include <boost/scoped_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <list>



using namespace std;
using namespace glite::ce::cream_client_api;

namespace iceUtil   = glite::wms::ice::util;
namespace po        = boost::program_options;
namespace fs        = boost::filesystem;
namespace api_util  = glite::ce::cream_client_api::util;
namespace cream_api = glite::ce::cream_client_api::soap_proxy;


 

int main(int argc, char*argv[]) 
{
  char* confile = "/opt/glite/etc/glite_wms.conf";
  bool  cancel_all = false;
  string gridjobid;
  int option_index = 0;
  bool log_abort = false;
  string abort_reason;
  char c;
  
  while(1) {
    static struct option long_options[] = {
      {"conf", 1, 0, 'c'},
      {"all", 0, 0, 'a'},
      {"log-abort", 1, 0, 'l'},
      {0, 0, 0, 0}
    };
    c = getopt_long(argc, argv, "c:al:", long_options, &option_index);
    if ( c == -1 )
      break;

    switch(c) {
    case 'c':
      confile = optarg;//false;
      break;
    case 'a':
      cancel_all = true;
      break;
    case 'l':
      log_abort = true;
      abort_reason = optarg;
      break;
    default:
        cerr << "Type " << argv[0] << " -h for help" << endl;
        exit(1);
    }
  }
  if( argv[optind] )
    gridjobid = string(argv[optind]);
  else {
    cerr << "glite-wms-ice-rm::main - ERROR: must specify at least one between argument <gridjobid> or option '--all'. Stop." << endl;
    return 1; 
  }

    iceUtil::iceConfManager::init( confile );
    try{
        iceUtil::iceConfManager::getInstance();
    }
    catch(iceUtil::ConfigurationManager_ex& ex) {
        cerr << "glite-wms-ice-rm::main - ERROR: " << ex.what() << endl;
        exit(1);
    }

    //glite::wms::common::configuration::Configuration* conf = glite::wms::ice::util::iceConfManager::getInstance()->getConfiguration();

    


    /*
     * Build all paths needed for files referred into the configuration file
     *
     * This code is taken from JC/LM (thanks to Ale)
     */
    std::list< fs::path > paths;

//     try {
//         paths.push_back(fs::path(conf->ice()->input(), fs::native));
//         paths.push_back(fs::path(conf->ice()->persist_dir() + "/boh", fs::native));
//         paths.push_back(fs::path(conf->ice()->logfile(), fs::native));
//     } catch( ... ) {
//         cerr << "glite-wms-ice::main() - ERROR: cannot create paths; "
//              << "check ICE configuration file"
//              << endl;
//         exit( 1 );
//     }
//     
//     for( std::list< fs::path >::iterator pathIt = paths.begin(); pathIt != paths.end(); ++pathIt ) {
//         if( (!pathIt->native_file_string().empty()) && !fs::exists(pathIt->branch_path()) ) {
//             try {
//                 fs::create_directories( pathIt->branch_path() );
//             } catch( ... ) {
//                 cerr << "glite-wms-ice::main() - ERROR: cannot create path "
//                      << pathIt->branch_path().string() 
//                      << endl;
//                 exit( 1 );
//             }
//         }
//     }

    /*****************************************************************************
     * Sets the log file
     ****************************************************************************/
//     util::creamApiLogger* logger_instance = util::creamApiLogger::instance();
//     log4cpp::Category* log_dev = logger_instance->getLogger();
// 
//     log_dev->setPriority( conf->ice()->ice_log_level() );
//     logger_instance->setLogfileEnabled( conf->ice()->log_on_file() );
//     logger_instance->setConsoleEnabled( conf->ice()->log_on_console() );
//     logger_instance->setMaxLogFileSize( conf->ice()->max_logfile_size() );
//     logger_instance->setMaxLogFileRotations( conf->ice()->max_logfile_rotations() );
//     string logfile = conf->ice()->logfile();
//     string hostcert = conf->ice()->ice_host_cert();
// 
//     logger_instance->setLogFile(logfile.c_str());
//     CREAM_SAFE_LOG(log_dev->debugStream() 
// 		   << "ICE VersionID is [" << ICE_VERSIONID << "] ProcessID=["
// 		   << ::getpid() << "]"
// 		   );
//     cout << "Logfile is [" << logfile << "]" << endl;
// 
//     
//     signal(SIGPIPE, sigpipe_handle);
// 
//     signal(SIGUSR1, sigusr1_handle);
//     signal(SIGUSR2, sigusr2_handle);
// 
//     /*****************************************************************************
//      * Gets the distinguished name from the host proxy certificate
//      ****************************************************************************/
// 
//     CREAM_SAFE_LOG(
//                    log_dev->infoStream()
//                    << method_name
//                    << "Host certificate is [" << hostcert << "]" 
//                    
//                    );
// 
// 
// 
//     /**
//      * Now the cache is ready and filled with all job's information
//      * Let's create the DNProxyManager that also load all DN->ProxyFile mappping
//      * by scanning the cache
//      */
//     iceUtil::DNProxyManager::getInstance();
    

    /*****************************************************************************
     * Initializes the database by invoking a fake query, and the ice manager
     ****************************************************************************/
     
     std::list< glite::wms::ice::util::CreamJob > jobList;
     glite::wms::ice::util::iceLBLogger *lb_logger( glite::wms::ice::util::iceLBLogger::instance() );
     
     if( cancel_all ) {     
       glite::wms::ice::db::GetAllJobs getter( &jobList, 0, 0, "glite-wms-ice-rm::main", false);
       glite::wms::ice::db::Transaction tnx( false, false );
       tnx.execute( &getter );       
     } else {
     
       glite::wms::ice::db::GetJobByGid getter( gridjobid, "glite-wms-ice-rm::main" );
       glite::wms::ice::db::Transaction tnx( false, false );
       tnx.execute( &getter );       
       if( getter.found( ) )
         jobList.push_back( getter.get_job( ) );
       else {
         cerr << "GriJobid "<< gridjobid << " is not found in the ICE database." << endl;
	 return 1;
       }
     }
     
     std::list< glite::wms::ice::util::CreamJob >::const_iterator jit;
     jit = jobList.begin( );
     while( jit != jobList.end( ) ) {
         lb_logger->logEvent( new glite::wms::ice::util::cream_cancel_request_event( *jit, string("Cancel request by Admin through glite-wms-ice-rm") ) ); 
	 vector<cream_api::JobIdWrapper> toCancel;
         toCancel.push_back( cream_api::JobIdWrapper(jit->get_cream_jobid(), 
						     jit->get_creamurl(), 
						     std::vector<cream_api::JobPropertyWrapper>())
			    );
      
         cream_api::JobFilterWrapper req( toCancel, vector<string>(), -1, -1, "", "");
         cream_api::ResultWrapper res;
      
         glite::wms::ice::util::CreamProxy_Cancel( jit->get_creamurl(), jit->get_user_proxy_certificate( ), &req, &res ).execute( 3 );
	 ++jit;
	}
     
     
    return 0;
}
