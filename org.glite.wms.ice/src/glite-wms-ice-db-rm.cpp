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

#include "iceUtils/CreamJob.h"
#include "iceDb/GetJobByGid.h"
#include "iceDb/Transaction.h"
#include "iceDb/RemoveJobByGid.h"
#include "iceUtils/IceConfManager.h"
#include "iceUtils/DNProxyManager.h"
#include "iceUtils/IceUtils.h"


#include <boost/scoped_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>

#include <list>

using namespace std;
using namespace glite::wms::ice;

bool get_job( const string& gid, util::CreamJob& );
void remove_job_fromdb( const util::CreamJob& );

int main(int argc, char*argv[]) 
{

  char*   confile = "glite_wms.conf";
  //bool    cancel_all = false;
  string  gridjobid;
  int     option_index = 0;
  //bool    log_abort = false;
  string  abort_reason;
  char    c;
  bool    from_file = false;
  string  inputlist;

  while(1) {
    static struct option long_options[] = {
      {"conf", 1, 0, 'c'},
      {0, 0, 0, 0}
    };
    c = getopt_long(argc, argv, "c:al:", long_options, &option_index);
    if ( c == -1 )
      break;
    
    switch(c) {
    case 'c':
      confile = optarg;//false;
      break;
    default:
      cerr << "Type " << argv[0] << " -h for help" << endl;
      exit(1);
    }
  }
  
  if( argv[optind] && from_file ) 
    {
      cerr << "Only one of --from-file <pathfile> or <gridjobid> "
	   << "must be specified in the command line" << endl;
      return 1;
    }

  if (!argv[optind] && !from_file )
    {
      cerr << "Must specify at least one of the options --from-file <pathfile> or <gridjobid>" << endl;
      return 1;
    }

  if( argv[optind] )
    gridjobid = string(argv[optind]);

  util::IceConfManager::init( confile );
  try{
    util::IceConfManager::instance();
  }
  catch(util::ConfigurationManager_ex& ex) {
    cerr << "glite-wms-ice-rm::main - ERROR: " << ex.what() << endl;
    exit(1);
  }
  
  std::list< util::CreamJob > jobList;
  
  /**
     If specified by the user
     read gridjobids, to delete from DB, from file
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
    
      util::CreamJob aJob;
      if( get_job( linestring, aJob ) )
        jobList.push_back( aJob );
      else
        cerr << "GriJobID ["<< linestring
	     << "] is not found in the ICE database. Skipping cancel for it..."
	     << endl;
    }
  }
  
  if( !gridjobid.empty() ) {
    util::CreamJob aJob;
    if( get_job( gridjobid, aJob ) )
      jobList.push_back( aJob );
    else {
      cerr << "GriJobid "<< gridjobid << " is not found in the ICE database." << endl;
      return 1;
    }
  }
  
  std::list< util::CreamJob >::iterator jit;
  jit = jobList.begin( );
  for( ; jit != jobList.end( ); ++jit ) {
    
    remove_job_fromdb( *jit );
    
    cout << "Removing from ICE's DB job [" 
	 << jit->grid_jobid() << "] -> [" << jit->complete_cream_jobid() << "]" << endl;
    
  } 
  return 0;
}

//_______________________________________________________________________________
bool get_job( const string& gid, util::CreamJob& target )
{

  db::GetJobByGid getter( gid, "glite-wms-ice-rm::get_job" );
  db::Transaction tnx( false, false );
  tnx.execute( &getter );
  
  if( !getter.found() ) return false;
  
  target = getter.get_job();
  
  return true;
}

//_______________________________________________________________________________
void remove_job_fromdb( const util::CreamJob& aJob ) {
  if( aJob.proxy_renewable() )
      util::DNProxyManager::getInstance()->decrementUserProxyCounter( aJob.user_dn(), aJob.myproxy_address() );
      
  {
    db::RemoveJobByGid remover( aJob.grid_jobid(), "glite-wms-ice-db-rm::remove_job_fromdb" );
    db::Transaction tnx(false, false);
    tnx.execute( &remover );
  }
}
