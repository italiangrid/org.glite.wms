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
//#include "iceDb/GetAllJobs.h"
#include "iceDb/GetFields.h"
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

bool get_job( const string& gid, vector<string>& );
void get_all_jobs( list<vector<string> >& jobs );

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
  
  std::list< vector<string> > jobList;
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
    
      vector<string> aJob;
      if( get_job( linestring, aJob ) )
        jobList.push_back( aJob );
      else
        cerr << "GriJobID ["<< linestring
	     << "] is not found in the ICE database. Skipping cancel for it..."
	     << endl;
    }
  }
  
  if( cancel_all ) {   
    get_all_jobs( jobList );       
  } 

  if( !gridjobid.empty() ) {
    vector<string> aJob;
    if( get_job( gridjobid, aJob ) )
      jobList.push_back( aJob );
    else {
      cerr << "GriJobid "<< gridjobid << " is not found in the ICE database." << endl;
      return 1;
    }
  }
  
  std::list< vector<string> >::iterator jit;
  jit = jobList.begin( );
  while( jit != jobList.end( ) ) {
//     if(log_abort) {
//       jit->set_failure_reason( abort_reason );
//       // lb_logger->logEvent( new iceUtil::job_aborted_event( *jit /*, string("Cancel request by Admin through glite-wms-ice-rm") */) ); 
//     }
    vector<cream_api::JobIdWrapper> toCancel;
    toCancel.push_back( cream_api::JobIdWrapper(jit->at(0)/*get_cream_jobid()*/, 
						jit->at(1)/*get_creamurl()*/, 
						std::vector<cream_api::JobPropertyWrapper>())
			);
    
    cream_api::JobFilterWrapper req( toCancel, vector<string>(), -1, -1, "", "");
    cream_api::ResultWrapper res;

    cout << "Sending JobCancel to CREAM for job [" 
	 << jit->at(3) << "] -> [" << jit->at(1) + "/" + jit->at(0) << "]" << endl;
    
    cream_api::VOMSWrapper V( jit->at(2)/*get_user_proxy_certificate()*/,  !::getenv("GLITE_WMS_ICE_DISABLE_ACVER") );

    if( !V.IsValid( ) ) {
      cerr <<"*** For job [" << jit->at(3) << "] -> ["
	   << jit->at(1) + "/" + jit->at(0)/*get_grid_jobid()*/ << "]"
	   << " the proxyfile ["
	   << jit->at(2)/*get_user_proxy_certificate()*/ 
	   << "] is not valid: "
	   << V.getErrorMessage()
	   << ". Skipping cancellation of this job. "
	   << endl;
    } else {
      try {
        iceUtil::CreamProxy_Cancel( jit->at(1), jit->at(2), &req, &res ).execute( 3 );
	cout << "CANCELLED JOB ["<< jit->at(3) << "]" << endl;
      } catch(exception& ex) {
        cerr << "*** Error cancelling job [" << jit->at(3) << "]: " << ex.what() << endl;
      }
    }
    
    ++jit;
  } 
  return 0;
}

//_______________________________________________________________________________
bool get_job( const string& gid, vector<string>& target )
{
  list<string> fields_to_get;
  list<vector<string> > result;
  list<pair<string, string> > clause;
  
  fields_to_get.push_back("creamjobid");	//0
  fields_to_get.push_back("creamurl");          //1
  fields_to_get.push_back("userproxy");		//2
  fields_to_get.push_back("gridjobid");         //3
  
  clause.push_back( make_pair("gridjobid", gid ) );
  
  iceDb::GetFields getter( fields_to_get, clause, result, "glite-wms-ice-rm::get_job" );
  iceDb::Transaction tnx( false, false );
  tnx.execute( &getter );
  
  if(result.begin() == result.end()) return false;
  
  target = *result.begin( );
  
  return true;
}

//_______________________________________________________________________________
void get_all_jobs( list<vector<string> >& jobs )
{
  list<string> fields_to_get;
  list<vector<string> > result;
  
  fields_to_get.push_back("creamjobid");	//0
  fields_to_get.push_back("creamurl");          //1
  fields_to_get.push_back("userproxy");		//2
  fields_to_get.push_back("gridjobid");		//3
  
  
  iceDb::GetFields getter( fields_to_get, list<pair<string,string> >(), result, "glite-wms-ice-rm::get_all_jobs" );
  iceDb::Transaction tnx( false, false );
  tnx.execute( &getter );
  
  if(result.begin() == result.end()) return;
  
  list<vector<string> >::iterator jit = result.begin( );
  while( jit != result.end( ) )
  {
    jobs.push_back( *jit );
    
    ++jit;
  }
}
