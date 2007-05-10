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

#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"

#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"

#include "ice-core.h"
#include "iceAbsCommand.h"
#include "iceCommandFactory.h"
#include "jobCache.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "iceThreadPool.h"
#include "DNProxyManager.h"
#include "CreamProxyFactory.h"
#include "Request.h"

#include "glite/ce/cream-client-api-c/certUtil.h"

#include <boost/scoped_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <list>



using namespace std;
using namespace glite::ce::cream_client_api;
namespace iceUtil = glite::wms::ice::util;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

void sigpipe_handle(int x) { 
  //std::cerr << "glite-wms-ice::sigpipe_handle - PIPE handled; argument x=[" << x << "]" << std::endl;
  CREAM_SAFE_LOG(util::creamApiLogger::instance()->getLogger()->debugStream() 
		 << "glite-wms-ice::sigpipe_handle: Captured SIGPIPE. x argument=["
		 << x 
		 << "]" << log4cpp::CategoryStream::ENDLINE);
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
    CREAM_SAFE_LOG(log_dev->debugStream() << "ICE VersionID is [20070510-15:00]"<<log4cpp::CategoryStream::ENDLINE);
    cout << "Logfile is [" << logfile << "]" << endl;

    
    signal(SIGPIPE, sigpipe_handle);

    /*****************************************************************************
     * Gets the distinguished name from the host proxy certificate
     ****************************************************************************/

    CREAM_SAFE_LOG(
                   log_dev->infoStream()
                   << "Host certificate is [" << hostcert << "]" 
                   << log4cpp::CategoryStream::ENDLINE
                   );

    try {
        string hostdn( certUtil::getDN(hostcert) );
        glite::wms::ice::util::CreamProxyFactory::setHostDN( hostdn );
    } catch ( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
        CREAM_SAFE_LOG(log_dev->errorStream()
                       << "Unable to extract user DN from Proxy File "
                       << hostcert 
                       << ". Won't set SOAP header"
                       << log4cpp::CategoryStream::ENDLINE);
    }
  

    /*****************************************************************************
     * Initializes job cache
     ****************************************************************************/
    string jcachedir( conf->ice()->persist_dir() );

    CREAM_SAFE_LOG(
                   log_dev->infoStream() 
                   << "Initializing jobCache with persistency directory ["
                   << jcachedir
                   << "]..."
                   << log4cpp::CategoryStream::ENDLINE
                   );

    iceUtil::jobCache::setPersistDirectory( jcachedir );
    iceUtil::jobCache::setRecoverableDb( true );

    try {
        iceUtil::jobCache::getInstance();
    }
    catch(exception& ex) {
        CREAM_SAFE_LOG( log_dev->log( log4cpp::Priority::FATAL, ex.what() ) );
        exit( 1 );
    }

    /**
     * Now the cache is ready and filled with all job's information
     * Let's create the DNProxyManager that also load all DN->ProxyFile mappping
     * by scanning the cache
     */
    iceUtil::DNProxyManager::getInstance();
    

    /*****************************************************************************
     * Initializes ice manager
     ****************************************************************************/ 
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
  
    /*****************************************************************************
     * Main loop that fetch requests from input filelist, submit/cancel the jobs,
     * removes requests from input filelist.
     ****************************************************************************/
    while(true) {

        //
        // BEWARE!! the get_command_count() method locks the
        // threadPool object. Hence, it is *extremely* dangerous to
        // call the get_command_count() method inside a CREAM_SAFE_LOG()
        // block, because it would result in a hold-and-wait potential
        // race condition.
        //
        int command_count = threadPool->get_command_count();

        if ( command_count > 100 ) {
            CREAM_SAFE_LOG(log_dev->infoStream()
                           << "glite-wms-ice::main() - "
                           << "There are currently too many requests ("
                           << command_count
                           << ") in the internal command queue. "
                           << "Will check again in 30 seconds."
                           << log4cpp::CategoryStream::ENDLINE
                           );
            sleep( 30 );
            continue;
        }

        requests.clear();
        iceManager->getNextRequests(requests);
    
        if( requests.size() )
            CREAM_SAFE_LOG(
                           log_dev->infoStream()
                           << "*** Found " << requests.size() << " new request(s)"
                           << log4cpp::CategoryStream::ENDLINE
                           );

        for( list< iceUtil::Request* >::iterator it = requests.begin();
             it != requests.end(); ++it ) {
            CREAM_SAFE_LOG(
                           log_dev->infoStream()
                           << "*** Unparsing request <"
                           << (*it)->to_string()
                           << ">"
                           << log4cpp::CategoryStream::ENDLINE
                           );
            glite::wms::ice::iceAbsCommand* cmd;
            try {
                cmd = glite::wms::ice::iceCommandFactory::mkCommand( *it );
            } catch( std::exception& ex ) {
                CREAM_SAFE_LOG( log_dev->log(log4cpp::Priority::ERROR, ex.what() ) );
                CREAM_SAFE_LOG( log_dev->log(log4cpp::Priority::INFO, "Removing BAD request..." ) );
                iceManager->removeRequest( *it );
                continue;
            }

            // Submit to the thread pool
            threadPool->add_request( cmd );

        }
        sleep(1);
    }
    return 0;
}
