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
//#define _GNU_SOURCE
#include <string>
#include <iostream>
#include <fstream>
#include <sys/types.h>          // getpid(), getpwnam()
#include <unistd.h>             // getpid(), usleep
#include <pwd.h>                // getpwnam()
#include <cstdio>               // popen()
#include <cstdlib>              // atoi()
#include <csignal>
#include <getopt.h>
#include <cerrno>
#include <vector>

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
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"

#include <boost/scoped_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <list>

using namespace std;
using namespace glite::ce::cream_client_api;

namespace iceUtil   = glite::wms::ice::util;
namespace iceDb     = glite::wms::ice::db;
namespace po        = boost::program_options;
namespace fs        = boost::filesystem;
namespace api_util  = glite::ce::cream_client_api::util;
namespace cream_api = glite::ce::cream_client_api::soap_proxy;

int main(int argc, char*argv[]) 
{
  char*   confile = "/opt/glite/etc/glite_wms.conf";
  bool    cancel_all = false;
  string  gridjobid;
  int     option_index = 0;
  bool    log_abort = false;
  string  abort_reason;
  char    c;
  bool    from_file = false;
  string  inputlist;

  while(1) {
    static struct option long_options[] = {
      {"conf", 1, 0, 'c'},
      {"all", 0, 0, 'a'},
      {"log-abort", 1, 0, 'l'},
      {"from-file", 1, 0, 'f'},
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
    case 'f':
      from_file = true;
      inputlist = optarg;
      break;
    default:
      cerr << "Type " << argv[0] << " -h for help" << endl;
      exit(1);
    }
  }
  
  if( ( argv[optind] && from_file ) ||
      ( argv[optind] && cancel_all) ||
      ( cancel_all   && from_file ) )
    {
      cerr << "Only one of --all, --from-file <pathfile>, <gridjobid> "
	   << "must be specified in the command line" << endl;
      return 1;
    }

  if (!argv[optind] && !from_file && !cancel_all )
    {
      cerr << "Must specify one of the options --all, --from-file <pathfile>, <gridjobid>" << endl;
      return 1;
    }

  if( argv[optind] )
    gridjobid = string(argv[optind]);

  iceUtil::iceConfManager::init( confile );
  try{
    iceUtil::iceConfManager::getInstance();
  }
  catch(iceUtil::ConfigurationManager_ex& ex) {
    cerr << "glite-wms-ice-rm::main - ERROR: " << ex.what() << endl;
    exit(1);
  }
  
  std::list< iceUtil::CreamJob > jobList;
  iceUtil::iceLBLogger *lb_logger( iceUtil::iceLBLogger::instance() );
  
  /**
     If specified by the user
     read gridjobids to cancel from file
  */
  if( from_file ) {
    
    FILE* in = fopen( inputlist.c_str(), "r" );
    if(!in) {
      cerr << "Error opening [" << inputlist << "]: " 
	   << strerror( errno ) << endl;
      return 1;
    }
    fclose( in );
    ifstream is(inputlist.c_str(), ios_base::in );
    string linestring;
    while( is >> linestring ) {
      //      cout << "gridjobid = [" << linestring << "]" << endl;
      iceDb::GetJobByGid getter( linestring, "glite-wms-ice-rm::main" );
      iceDb::Transaction tnx( false, false );
      tnx.execute( &getter );       
      if( getter.found( ) )
	jobList.push_back( getter.get_job( ) );
      else {
	cerr << "GriJobID ["<< linestring << "] is not found in the ICE database. Skipping cancel for it..." << endl;
      }
    }
  }
  
  //  return 0;
  
  if( cancel_all ) {     
    iceDb::GetAllJobs getter( &jobList, 0, 0, "glite-wms-ice-rm::main", false);
    iceDb::Transaction tnx( false, false );
    tnx.execute( &getter );       
  } 

  if( !gridjobid.empty() ) {
    
    iceDb::GetJobByGid getter( gridjobid, "glite-wms-ice-rm::main" );
    iceDb::Transaction tnx( false, false );
    tnx.execute( &getter );       
    if( getter.found( ) )
      jobList.push_back( getter.get_job( ) );
    else {
      cerr << "GriJobid "<< gridjobid << " is not found in the ICE database." << endl;
      return 1;
    }
  }
  
  std::list< iceUtil::CreamJob >::iterator jit;
  jit = jobList.begin( );
  while( jit != jobList.end( ) ) {
    if(log_abort) {
      jit->set_failure_reason( abort_reason );
      lb_logger->logEvent( new iceUtil::job_aborted_event( *jit /*, string("Cancel request by Admin through glite-wms-ice-rm") */) ); 
    }
    vector<cream_api::JobIdWrapper> toCancel;
    toCancel.push_back( cream_api::JobIdWrapper(jit->get_cream_jobid(), 
						jit->get_creamurl(), 
						std::vector<cream_api::JobPropertyWrapper>())
			);
    
    cream_api::JobFilterWrapper req( toCancel, vector<string>(), -1, -1, "", "");
    cream_api::ResultWrapper res;

    cout << "Sending JobCancel to CREAM for job [" 
	 << jit->get_grid_jobid( ) << "] -> ["
	 << jit->get_complete_cream_jobid( ) << "]" << endl;
    
    cream_api::VOMSWrapper V( jit->get_user_proxy_certificate(),  !::getenv("GLITE_WMS_ICE_DISABLE_ACVER") );

    if( !V.IsValid( ) ) {
      cerr <<"For job ["
	   << jit->get_grid_jobid() << "]"
	   << " the proxyfile ["
	   << jit->get_user_proxy_certificate() 
	   << "] is not valid: "
	   << V.getErrorMessage()
	   << ". Skipping cancellation of this job. "
	   << endl;
    } else {
      iceUtil::CreamProxy_Cancel( jit->get_creamurl(), jit->get_user_proxy_certificate( ), &req, &res ).execute( 3 );
    }
    
    ++jit;
  } 
  return 0;
}
