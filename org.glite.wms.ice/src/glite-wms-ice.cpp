/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE main daemon
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include <string>
#include <iostream>
#include <sys/types.h>          // getpid(), getpwnam()
#include <unistd.h>             // getpid()
#include <pwd.h>                // getpwnam()

//#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"

#include "ice-core.h"
#include "iceAbsCommand.h"
#include "iceCommandFactory.h"
#include "jobCache.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "iceThreadPool.h"
#include "CreamProxyFactory.h"

#include "glite/ce/cream-client-api-c/certUtil.h"

#include <boost/scoped_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace glite::ce::cream_client_api;
namespace iceUtil = glite::wms::ice::util;
namespace po = boost::program_options;

#define USE_STATUS_POLLER true
#define USE_STATUS_LISTENER false


// change the uid and gid to those of user no-op if user corresponds
// to the current effective uid only root can set the uid
// (this function was originally taken from org.glite.wms.manager/src/daemons/workload_manager.cpp
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

  //
  // Change user id to become the "dguser" specified in the configuratoin file
  //
  string dguser( iceUtil::iceConfManager::getInstance()->getDGuser() );
  if (!set_user(dguser)) {
      cerr << "glite-wms-ice::main() - ERROR: cannot set the user id to " 
	   << dguser << endl;
      exit( 1 );
  }



  /*****************************************************************************
   * Sets the log file
   ****************************************************************************/
  util::creamApiLogger* logger_instance = util::creamApiLogger::instance();
  log4cpp::Category* log_dev = logger_instance->getLogger();

  log_dev->setPriority( iceUtil::iceConfManager::getInstance()->getLogLevel() );
  logger_instance->setLogfileEnabled( iceUtil::iceConfManager::getInstance()->getLogOnFile() );
  logger_instance->setConsoleEnabled( iceUtil::iceConfManager::getInstance()->getLogOnConsole() );
  logger_instance->setMaxLogFileSize( iceUtil::iceConfManager::getInstance()->getMaxLogFileSize() );
  logger_instance->setMaxLogFileRotations( iceUtil::iceConfManager::getInstance()->getMaxLogFileRotations() );
  string logfile = iceUtil::iceConfManager::getInstance()->getLogFile();
  string hostcert = iceUtil::iceConfManager::getInstance()->getHostProxyFile();


  try {
    iceUtil::makePath( logfile );
  } catch(exception& ex) {
    cerr << "Error Creating path for logfile ["<<logfile<<"]: "<<ex.what()<<endl;
    exit( 1 );
  }

  logger_instance->setLogFile(logfile.c_str());
  log_dev->debugStream() << "ICE VersionID is [20061010-16:00]"<<log4cpp::CategoryStream::ENDLINE;
  cout << "Logfile is [" << logfile << "]" << endl;

//   cout << "Poller Threshold time="<<iceUtil::iceConfManager::getInstance()->getPollerStatusThresholdTime()<<endl;
  /*****************************************************************************
   * Gets the distinguished name from the host proxy certificate
   ****************************************************************************/

  //string hostcert = iceUtil::iceConfManager::getInstance()->getHostProxyFile();

  // Set the creation of CreamProxy with automatic delegetion ON

  string hostdn;
  CREAM_SAFE_LOG(
                 log_dev->infoStream()
                 << "Host proxyfile is [" << hostcert << "]" 
                 << log4cpp::CategoryStream::ENDLINE
                 );
  try {
    hostdn = certUtil::getDN(hostcert);
    boost::trim_if(hostdn, boost::is_any_of("/"));
    while( hostdn.find("/", 0) != string::npos ) {
      boost::replace_first(hostdn, "/", "_");
    }

    while( hostdn.find("=", 0) != string::npos ) {
      boost::replace_first(hostdn, "=", "_");
    }

    if((certUtil::getProxyTimeLeft(hostcert)<=0) || (hostdn=="") ) {
        CREAM_SAFE_LOG(
                       log_dev->errorStream() 
                       << "Host proxy certificate is expired. Won't start Listener"
                       << log4cpp::CategoryStream::ENDLINE
                       );

        // this must be set because other pieces of code
        // have a behaviour that depends on the listener is running or not
	//boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
	iceUtil::iceConfManager::getInstance()->setStartListener( false );
    } else {
        CREAM_SAFE_LOG(
                       log_dev->log(log4cpp::Priority::INFO,
                                    string( "Host DN is [" + hostdn + "]") )
                       );
    }
  } catch ( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
    logger_instance->log(log4cpp::Priority::ERROR,
			 "Unable to extract user DN from Proxy File "
			 + hostcert + ". Won't start Listener", true, true, true);

    // this must be set because other pieces of code
    // have a behaviour that depends on the listener is running or not
    //boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
    iceUtil::iceConfManager::getInstance()->setStartListener( false );
  }
  
  glite::wms::ice::util::CreamProxyFactory::setHostDN( hostdn );

  /*****************************************************************************
   * Initializes job cache
   ****************************************************************************/
  //string jcachefile;
  string jcachedir;
  {
//    boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
    //jcachefile = iceUtil::iceConfManager::getInstance()->getCachePersistFile();
    jcachedir = iceUtil::iceConfManager::getInstance()->getCachePersistDirectory();
  }
  //string jsnapfile  = jcachefile+".snapshot";
  CREAM_SAFE_LOG(
                 log_dev->infoStream() 
                 //<< "Initializing jobCache with journal file ["
                 << "Initializing jobCache with persistency directory ["
		 //<< jcachefile
		 << jcachedir
                 //<< "] and snapshot file ["
                 //<< jsnapfile
                 << "]..."
                 << log4cpp::CategoryStream::ENDLINE
                 );

  //iceUtil::jobCache::setJournalFile(jcachefile);
  //iceUtil::jobCache::setSnapshotFile(jsnapfile);

  iceUtil::jobCache::setPersistDirectory( jcachedir );
  iceUtil::jobCache::setRecoverableDb( true );

  try {
      iceUtil::jobCache::getInstance();
  }
  catch(exception& ex) {
      CREAM_SAFE_LOG( log_dev->log( log4cpp::Priority::FATAL, ex.what() ) );
      exit( 1 );
  }


  /*****************************************************************************
   * Initializes ice manager
   ****************************************************************************/ 
  glite::wms::ice::Ice* iceManager( glite::wms::ice::Ice::instance( ) );

  
  /*****************************************************************************
   * Prepares a vector that will contains requests fetched from input file
   * list. Its initial capacity is set large enough... to tune...
   ****************************************************************************/
  vector<string> requests;
  requests.reserve(1000);

  /*****************************************************************************
   * Starts status poller and/or listener if specified in the config file
   ****************************************************************************/
  {
    if(iceUtil::iceConfManager::getInstance()->getStartListener())
      iceManager->startListener(iceUtil::iceConfManager::getInstance()->getListenerPort());

    if(iceUtil::iceConfManager::getInstance()->getStartPoller())
      iceManager->startPoller(iceUtil::iceConfManager::getInstance()->getPollerDelay());
  }
  
  if( iceUtil::iceConfManager::getInstance()->getStartLeaseUpdater() )
    iceManager->startLeaseUpdater( );
    
  if( iceUtil::iceConfManager::getInstance()->getStartProxyRenewer() )
    iceManager->startProxyRenewer( );
    
  if( iceUtil::iceConfManager::getInstance()->getStartJobKiller() )
    iceManager->startJobKiller( );

  vector<string> url_jid;
  url_jid.reserve(2);

  /*
   *
   * Starts the thread pool
   *
   */
  iceUtil::iceThreadPool* threadPool( iceUtil::iceThreadPool::instance() );
  
  /*
   * Initializes the L&B logger
   */
  // iceUtil::iceEventLogger* iceLogger = iceUtil::iceEventLogger::instance();

  /*****************************************************************************
   * Main loop that fetch requests from input filelist, submit/cancel the jobs,
   * removes requests from input filelist.
   ****************************************************************************/
  while(true) {
    
    iceManager->getNextRequests(requests);
    
    if( requests.size() )
        CREAM_SAFE_LOG(
                       log_dev->infoStream()
                       << "*** Found " << requests.size() << " new request(s)"
                       << log4cpp::CategoryStream::ENDLINE
                       );

    for(unsigned int j=0; j < requests.size( ); j++) {
        CREAM_SAFE_LOG(
                       log_dev->infoStream()
                       << "*** Unparsing request <"
                       << requests[j] 
                       << ">"
                       << log4cpp::CategoryStream::ENDLINE
                       );
        glite::wms::ice::iceAbsCommand* cmd;
        try {
            cmd = glite::wms::ice::iceCommandFactory::mkCommand( requests[j] );
        } catch( std::exception& ex ) {
            CREAM_SAFE_LOG( log_dev->log(log4cpp::Priority::ERROR, ex.what() ) );
            CREAM_SAFE_LOG( log_dev->log(log4cpp::Priority::INFO, "Removing BAD request..." ) );
            iceManager->removeRequest(j);
            continue;
        }

//       try {
//           cmd->execute( iceManager );
//       } catch ( glite::wms::ice::iceCommandFatal_ex& ex ) {
//           CREAM_SAFE_LOG( 
//                          log_dev->errorStream()
//                          << "Command execution got FATAL exception: "
//                          << ex.what()
//                          << log4cpp::CategoryStream::ENDLINE
//                          );
//       } catch ( glite::wms::ice::iceCommandTransient_ex& ex ) {
//           CREAM_SAFE_LOG(
//                          log_dev->errorStream()
//                          << "Command execution got TRANSIENT exception: "
//                          << ex.what()
//                          << log4cpp::CategoryStream::ENDLINE
//                          );
//           CREAM_SAFE_LOG( log_dev->log(log4cpp::Priority::INFO, "Request will be resubmitted" ) );
          
//       }
      CREAM_SAFE_LOG( log_dev->log(log4cpp::Priority::INFO, "Removing submitted request from WM/ICE's filelist..." ) );
      try { 
	iceManager->removeRequest(j);
      } catch(exception& ex) {
          CREAM_SAFE_LOG(
                         log_dev->fatalStream() 
                         << "glite-wms-ice::main() - "
                         << "Error removing request from FL: "
                         << ex.what()
                         << log4cpp::CategoryStream::ENDLINE
                         );
	exit(1);
      }

      // Submit to the thread pool
      threadPool->add_request( cmd );

    }
    sleep(1);
    requests.clear();
  }
  return 0;
}
