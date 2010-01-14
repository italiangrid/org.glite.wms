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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sys/wait.h>
#include <sys/types.h>          // getpid(), getpwnam()
#include <unistd.h>             // getpid()
#include <pwd.h>                // getpwnam()
#include <cerrno>

#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "iceConfManager.h"

#include <boost/program_options.hpp>

#include <libgen.h> // for dirname

using namespace std;
namespace po = boost::program_options;
namespace iceUtil = glite::wms::ice::util;

// change the uid and gid to those of user no-op if user corresponds
// to the current effective uid only root can set the uid (this
// function was originally taken from
// org.glite.wms.manager/src/daemons/workload_manager.cpp
bool set_user(std::string const& user)
{
    uid_t euid = ::geteuid();
 
    if (euid == 0 && user.empty()) {
        return false;
    }
 
    ::passwd* pwd(::getpwnam(user.c_str()));
    if (pwd == 0) {
        return false;
    }
    ::uid_t uid = pwd->pw_uid;
    ::gid_t gid = pwd->pw_gid;
 
    return
        euid == uid
        || (euid == 0 && ::setgid(gid) == 0 && ::setuid(uid) == 0);
}
 
//____________________________________________________________________
int main( int argc, char *argv[]) {

  string opt_pid_file;
  string opt_conf_file;
  static const char* method_name = "glite-wms-ice::main() - ";
  
  po::options_description desc("Usage");
  desc.add_options()
    ("help", "display this help and exit")
    (
     "daemon",
     po::value<string>(&opt_pid_file),
     "run in daemon mode and save the pid in this file"
     )
    (
     "conf",
     po::value<string>(&opt_conf_file)->default_value("glite_wms.conf"),
     "configuration file"
     )
    ;
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
  } catch( std::exception& ex ) {
    cerr << "There was an error parsing the command line. "
	 << "Error was: " << ex.what() << endl
	 << "Type " << argv[0] 
	 << " --help for the list of available options"
	 << endl;
    exit( 1 );
  }
  po::notify(vm);
  
  if ( vm.count("help") ) {
    cout << desc << endl;
    return 0;
  }

  iceUtil::iceConfManager::init( opt_conf_file );
  try{
    iceUtil::iceConfManager::getInstance();
  }
  catch(iceUtil::ConfigurationManager_ex& ex) {
    cerr << "glite-wms-ice-safe::main() - ERROR: " << ex.what() << endl;
    exit(1);
  }

  glite::wms::common::configuration::Configuration* conf = iceUtil::iceConfManager::getInstance()->getConfiguration();

  string logfile = conf->ice()->logfile();

  string logpath = dirname( (char*)logfile.c_str() );

  string consolelog = logpath + "/ice_console.log";

  if ( vm.count("daemon") ) {
    ofstream pid_file(opt_pid_file.c_str());
    if (!pid_file) {
      cerr << "the pid file " << opt_pid_file << " is not writable\n";
      return -1;
    }
    if (daemon(0, 0)) {
      cerr << "cannot become daemon (errno = "<< errno << ")\n";
      return -1;
    }
    pid_file << ::getpid();
  }
  

  //----------------------------------------------
  //cout << conf->ice( )->input_type( ) << endl;
  if( conf->ice( )->input_type( ) == "jobdir" ) {
    string jobdirpath( conf->ice( )->input( ) );
    char cmd[1024];
    memset((void*)cmd, 0, 1024);
    sprintf( cmd, "mv %s/old/* %s/new/ >/dev/null 2>&1", jobdirpath.c_str(),jobdirpath.c_str());
    cout << "Executing [" << cmd << "]" << endl;
    system( cmd );
  }
  
  if( argc>=3 ) {
    char buf[1024];
    memset((void*)buf, 0, 1024);
    
    char* mem = ::getenv( "MAX_ICE_MEM" );
    if(mem)
      sprintf(buf, "MAX_ICE_MEM=\"%s\" %s --conf %s", 
	      mem,
	      "/opt/glite/bin/glite-wms-ice",
	      opt_conf_file.c_str());
    else
      sprintf(buf, "%s --conf %s > %s 2>&1", 
	      "/opt/glite/bin/glite-wms-ice",
	      opt_conf_file.c_str(), consolelog.c_str() );
    
    
    
    
    while(true) {
      //  cout << "Starting real ICE..." << endl;
      //  cout << "executing [" << buf << "]"<<endl;
      int ret = ::system( buf );
      int wret = WEXITSTATUS(ret);
      //  cout << "ret=["<< wret << "]" << endl;
      /** 
       * Sleeping 5 minutes should ensure that the kernel has a sufficient time
       * to close a previously used bind-port. This to prevent an not
       * understood problem with soap_bind that causes a crash if the port is temporarily
       * unavailable
       */
      if( 2 == wret) { sleep(300); continue; }
      exit(wret);
    }
  } // if(argc>=3)
}
