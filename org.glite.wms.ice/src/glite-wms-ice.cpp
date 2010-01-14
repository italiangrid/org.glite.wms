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

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"

#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"

#include "ice-core.h"
#include "iceAbsCommand.h"
#include "iceCommandFactory.h"
#include "iceDb/UpdateJobByGid.h"
#include "iceDb/Transaction.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "iceThreadPool.h"
#include "DNProxyManager.h"
#include "Request.h"


#include "glite/ce/cream-client-api-c/certUtil.h"

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

#define MAX_ICE_MEM 550000LL

long long check_my_mem( const pid_t pid ) throw();

void sigusr1_handle(int x) { 
  exit(2);
}

void sigusr2_handle(int x) { 
  exit(3);
}

void sigpipe_handle(int x) { 
  //std::cerr << "glite-wms-ice::sigpipe_handle - PIPE handled; argument x=[" << x << "]" << std::endl;
  CREAM_SAFE_LOG(util::creamApiLogger::instance()->getLogger()->debugStream() 
		 << "glite-wms-ice::sigpipe_handle: Captured SIGPIPE. x argument=["
		 << x 
		 << "]" );
}

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
 

int main(int argc, char*argv[]) 
{
    string opt_pid_file;
    string opt_conf_file;

    int cache_dump_delay = 900;

    if( getenv("GLITE_WMS_ICE_CACHEDUMP_DELAY" ) ) {
      cache_dump_delay = atoi(getenv("GLITE_WMS_ICE_CACHEDUMP_DELAY" ));
      if(cache_dump_delay < 300)
	cache_dump_delay = 300;
    }

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

    

    /**
     * - creates an ICE object
     * - initializes the job cache
     * - starts the async event consumer and status poller
     * - opens the WM's and the NS's filelist
     */    

    /*****************************************************************************
     * Initializes configuration manager (that in turn loads configurations)
     ****************************************************************************/
    iceUtil::iceConfManager::init( opt_conf_file );
    try{
        iceUtil::iceConfManager::getInstance();
    }
    catch(iceUtil::ConfigurationManager_ex& ex) {
        cerr << "glite-wms-ice::main() - ERROR: " << ex.what() << endl;
        exit(1);
    }

    glite::wms::common::configuration::Configuration* conf = iceUtil::iceConfManager::getInstance()->getConfiguration();

    //
    // Change user id to become the "dguser" specified in the configuratoin file
    //
    string dguser( conf->common()->dguser() );
    if (!set_user(dguser)) {
        cerr << "glite-wms-ice::main() - ERROR: cannot set the user id to " 
             << dguser << endl;
        exit( 1 );
    }


    /*
     * Build all paths needed for files referred into the configuration file
     *
     * This code is taken from JC/LM (thanks to Ale)
     */
    std::list< fs::path > paths;

    try {
        paths.push_back(fs::path(conf->ice()->input(), fs::native));
        paths.push_back(fs::path(conf->ice()->persist_dir() + "/boh", fs::native));
        paths.push_back(fs::path(conf->ice()->logfile(), fs::native));
    } catch( ... ) {
        cerr << "glite-wms-ice::main() - ERROR: cannot create paths; "
             << "check ICE configuration file"
             << endl;
        exit( 1 );
    }
    
    for( std::list< fs::path >::iterator pathIt = paths.begin(); pathIt != paths.end(); ++pathIt ) {
        if( (!pathIt->native_file_string().empty()) && !fs::exists(pathIt->branch_path()) ) {
            try {
                fs::create_directories( pathIt->branch_path() );
            } catch( ... ) {
                cerr << "glite-wms-ice::main() - ERROR: cannot create path "
                     << pathIt->branch_path().string() 
                     << endl;
                exit( 1 );
            }
        }
    }

    /*****************************************************************************
     * Sets the log file
     ****************************************************************************/
    util::creamApiLogger* logger_instance = util::creamApiLogger::instance();
    log4cpp::Category* log_dev = logger_instance->getLogger();

    log_dev->setPriority( conf->ice()->ice_log_level() );
    logger_instance->setLogfileEnabled( conf->ice()->log_on_file() );
    logger_instance->setConsoleEnabled( conf->ice()->log_on_console() );
    logger_instance->setMaxLogFileSize( conf->ice()->max_logfile_size() );
    logger_instance->setMaxLogFileRotations( conf->ice()->max_logfile_rotations() );
    string logfile = conf->ice()->logfile();
    string hostcert = conf->ice()->ice_host_cert();

    logger_instance->setLogFile(logfile.c_str());
    CREAM_SAFE_LOG(log_dev->debugStream() 
		   << "ICE VersionID is [" << ICE_VERSIONID << "] ProcessID=["
		   << ::getpid() << "]"
		   );
    cout << "Logfile is [" << logfile << "]" << endl;

    
    signal(SIGPIPE, sigpipe_handle);

    signal(SIGUSR1, sigusr1_handle);
    signal(SIGUSR2, sigusr2_handle);

    /*****************************************************************************
     * Gets the distinguished name from the host proxy certificate
     ****************************************************************************/

    CREAM_SAFE_LOG(
                   log_dev->infoStream()
                   << method_name
                   << "Host certificate is [" << hostcert << "]" 
                   
                   );



    /**
     * Now the cache is ready and filled with all job's information
     * Let's create the DNProxyManager that also load all DN->ProxyFile mappping
     * by scanning the cache
     */
    iceUtil::DNProxyManager::getInstance();
    

    /*****************************************************************************
     * Initializes the database by invoking a fake query, and the ice manager
     ****************************************************************************/
    
    {
      list<pair<string, string> > params;
      params.push_back( make_pair("failure_reason", "" ));
      glite::wms::ice::db::UpdateJobByGid updater("FAKE QUERY TO INITIALISE DB", params, "glite-wms-ice::main");
      glite::wms::ice::db::Transaction tnx( false, true );
      tnx.execute( &updater );
    }
 
    glite::wms::ice::Ice* iceManager( glite::wms::ice::Ice::instance( ) );

  
    /*****************************************************************************
     * Prepares a listr that will contains requests fetched from input file
     * list.
     ****************************************************************************/
    list< iceUtil::Request* > requests;

    /*****************************************************************************
     * Starts status poller and/or listener if specified in the config file
     ****************************************************************************/
    iceManager->startListener( );    
    iceManager->startPoller( );  
    iceManager->startLeaseUpdater( );
    iceManager->startProxyRenewer( );
    iceManager->startJobKiller( );

    /*
     *
     * Starts the thread pool
     *
     */
    iceUtil::iceThreadPool* threadPool( iceManager->get_requests_pool() );
    iceUtil::iceThreadPool* threadPool_ice_cmds( iceManager->get_ice_commands_pool() ); 
    /*****************************************************************************
     * Main loop that fetch requests from input filelist, submit/cancel the jobs,
     * removes requests from input filelist.
     ****************************************************************************/
     
    pid_t myPid = ::getpid();
    int mem_threshold_counter = 0;

    long long max_ice_mem = conf->ice()->max_ice_mem();
    
    CREAM_SAFE_LOG(log_dev->debugStream()
		   << method_name
		   << "Max ICE memory threshold set to "
		   << max_ice_mem << " kB"
		   
		   );

    while(true) {
        
        //
        // BEWARE!! the get_command_count() method locks the
        // threadPool object. Hence, it is *extremely* dangerous to
        // call the get_command_count() method inside a CREAM_SAFE_LOG()
        // block, because it would result in a hold-and-wait potential
        // race condition.
        //
        unsigned int command_count = threadPool->get_command_count();
        if ( command_count > conf->ice()->max_ice_threads() ) {
            CREAM_SAFE_LOG(log_dev->debugStream()
                           << method_name
                           << "There are currently too many requests ("
                           << command_count
                           << ") in the internal command queue. "
                           << "Will check again in 1 second."
                           );
        } else {
	  
	  requests.clear();
	  iceManager->getNextRequests( requests );
	  
	  if( !requests.empty() )
            CREAM_SAFE_LOG(
                           log_dev->infoStream()
                           << method_name 
                           << "*** Found " << requests.size() << " new request(s)"
                           
                           );
	  
	  for( list< iceUtil::Request* >::iterator it = requests.begin();
	       it != requests.end(); ++it ) 
	    {
	      string reqstr = (*it)->to_string();

	      

	      CREAM_SAFE_LOG(
			     log_dev->debugStream()
			     << method_name
			     << "*** Unparsing request <"
			     << reqstr
			     << ">"
			     
			     );

	      try {
		glite::wms::ice::util::CreamJob theJob;
		string cmdtype;
		glite::wms::ice::util::full_request_unparse( *it,
							     theJob,
							     cmdtype );
		
		cream_api::VOMSWrapper V( theJob.get_user_proxy_certificate(),  !::getenv("GLITE_WMS_ICE_DISABLE_ACVER") );
		if( !V.IsValid( ) ) {
		  CREAM_SAFE_LOG( log_dev->errorStream()
				  << method_name
				  << "For job ["
				  << theJob.get_grid_jobid() << "]"
				  << " the proxyfile ["
				  << theJob.get_user_proxy_certificate() 
				  << "] is not valid: "
				  << V.getErrorMessage()
				  << ". Skipping processing of this job..."
				  );
		  continue;
		}

		theJob.set_userdn( V.getDNFQAN() );
		theJob.set_isbproxy_time_end( V.getProxyTimeEnd() );

		glite::wms::ice::iceAbsCommand* cmd 
		  = glite::wms::ice::iceCommandFactory::mkCommand( *it,
								   theJob,
								   cmdtype
								   );

		
		
		threadPool->add_request( cmd );

	      } catch( std::exception& ex ) {
		CREAM_SAFE_LOG( log_dev->errorStream()
				<< method_name
				<< "Got exception \"" << ex.what()
				<< "\". Removing BAD request..." 
				);

		iceManager->removeRequest( *it );
		delete( *it );
	      }
	    }
	}

	//usleep(500000);
	sleep(1);
	
	/**
	 *
	 * Every 2 minutes ICE checks its mem usage
	 *
	 */
	++mem_threshold_counter;


	if(mem_threshold_counter >= 120) { // every 30 seconds check the memory
	  mem_threshold_counter = 0;
	  long long mem_now = check_my_mem(myPid);
	  if(mem_now > max_ice_mem) {
	    
	    // let's lock the cache so no other thread try to do cache operations
	    iceManager->stopAllThreads(); // this return only when all threads have finished
	    threadPool->stopAllThreads();
	    threadPool_ice_cmds->stopAllThreads();

	    CREAM_SAFE_LOG( log_dev->fatalStream()
			    << method_name
			    << "glite-wms-ice::main - Max memory reached ["
			    << mem_now << " kB] ! EXIT!"
			    
			    );
		
	    return 2; // exit to shell with specific error code
	  }
	}
	
    }
    CREAM_SAFE_LOG( log_dev->fatalStream()
			    << method_name
			    << "glite-wms-ice::main - Returning '0' to the shell"
			    
			    );	
    return 0;
}

//______________________________________________________________________________
long long check_my_mem( const pid_t pid ) throw()
{
  char cmd[128];
  char used_rss_mem[64];
  memset((void*) cmd, 0, 64);
  
  sprintf( cmd, "/bin/ps --cols 200 -orss -p %d |/bin/grep -v RSS", pid);


  FILE * in = popen( cmd, "r");
  if(!in) return (long long)0;
  
  while (fgets(used_rss_mem, 64, in) != NULL)
    CREAM_SAFE_LOG( util::creamApiLogger::instance()->getLogger()->debugStream()
		    << "glite-wms-ice::main() - Used RSS Memory: "
		    << used_rss_mem 
		    );
  pclose(in);

  return atoll(used_rss_mem);
}
