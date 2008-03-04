
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

#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

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
      sprintf(buf, "%s --conf %s", 
	      "/opt/glite/bin/glite-wms-ice",
	      opt_conf_file.c_str());
      

    while(true) {
    //  cout << "Starting real ICE..." << endl;
    //  cout << "executing [" << buf << "]"<<endl;
      int ret = ::system( buf );
      int wret = WEXITSTATUS(ret);
    //  cout << "ret=["<< wret << "]" << endl;
      if( 2 == wret) { sleep(2); continue; }
      exit(wret);
    }
  }
}
