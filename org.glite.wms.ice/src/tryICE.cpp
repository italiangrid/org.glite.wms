#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
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

#include "boost/scoped_ptr.hpp"

#include <string>
#include <iostream>
#include <unistd.h>

using namespace std;
using namespace glite::ce::cream_client_api;
namespace iceUtil = glite::wms::ice::util;

#define USE_STATUS_POLLER true
#define USE_STATUS_LISTENER false

int main(int argc, char*argv[]) {
  
  /**
   * - creates an ICE object
   * - initializes the job cache
   * - starts the async event consumer and status poller
   * - opens the WM's and the NS's filelist
   * 
   * - main's params:
   *                  argv[1]: configuration file
   */

  if ( argc!=2 ) {
      cout << "Usage: " << argv[0]
           << " <config_file>" << endl;
      return 1;
  } 



  /*****************************************************************************
   * Initializes configuration manager (that in turn loads configurations)
   ****************************************************************************/
  iceUtil::iceConfManager::init(argv[1]);
  try{
    iceUtil::iceConfManager::getInstance();
  }
  catch(iceUtil::ConfigurationManager_ex& ex) {
    cerr << ex.what() << endl;
    exit(1);
  }
 


  /*****************************************************************************
   * Sets the log file
   ****************************************************************************/
  util::creamApiLogger* logger_instance = util::creamApiLogger::instance();
  log4cpp::Category* log_dev = logger_instance->getLogger();
  string hostcert, logfile, hostdn;
  {
    boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
    log_dev->setPriority( iceUtil::iceConfManager::getInstance()->getLogLevel() );
    logger_instance->setLogfileEnabled( iceUtil::iceConfManager::getInstance()->getLogOnFile() );
    logger_instance->setConsoleEnabled( iceUtil::iceConfManager::getInstance()->getLogOnConsole() );
    logfile = iceUtil::iceConfManager::getInstance()->getLogFile();
    hostcert = iceUtil::iceConfManager::getInstance()->getHostProxyFile();
  }
  logger_instance->setLogFile(logfile.c_str());

  cout << "Logfile is [" << logfile << "]" << endl;

//   cout << "Poller Threshold time="<<iceUtil::iceConfManager::getInstance()->getPollerStatusThresholdTime()<<endl;
  /*****************************************************************************
   * Gets the distinguished name from the host proxy certificate
   ****************************************************************************/

  //string hostcert = iceUtil::iceConfManager::getInstance()->getHostProxyFile();
  //string hostdn;
  // Set the creation of CreamProxy with automatic delegetion ON
  soap_proxy::CreamProxyFactory::initProxy( true );
  log_dev->log(log4cpp::Priority::INFO,
               string("Host proxyfile is [") + hostcert + "]" );
  try {
    hostdn = soap_proxy::CreamProxyFactory::getProxy()->getDN(hostcert);
    if((soap_proxy::CreamProxyFactory::getProxy()->getProxyTimeLeft(hostcert)<=0) || (hostdn=="") ) {
        log_dev->errorStream() << "Host proxy certificate is expired. Won't start Listener"
					<< log4cpp::CategoryStream::ENDLINE;

        // this must be set because other pieces of code
        // have a behaviour that depends on the listener is running or not
	boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
	iceUtil::iceConfManager::getInstance()->setStartListener( false );
    } else {
	log_dev->log(log4cpp::Priority::INFO,
                     string( "Host DN is [" + hostdn + "]") );
    }
  } catch ( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
    logger_instance->log(log4cpp::Priority::ERROR,
			 "Unable to extract user DN from Proxy File "
			 + hostcert + ". Won't start Listener", true, true, true);

    // this must be set because other pieces of code
    // have a behaviour that depends on the listener is running or not
    boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
    iceUtil::iceConfManager::getInstance()->setStartListener( false );
  }

  /*****************************************************************************
   * Initializes job cache
   ****************************************************************************/
  string jcachefile;
  {
    boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
    jcachefile = iceUtil::iceConfManager::getInstance()->getCachePersistFile();
  }
  string jsnapfile  = jcachefile+".snapshot";
  log_dev->infoStream() << "Initializing jobCache with journal file ["
  			<< jcachefile
			<< "] and snapshot file ["
   			<< jsnapfile
			<< "]..."
			<< log4cpp::CategoryStream::ENDLINE;

  iceUtil::jobCache::setJournalFile(jcachefile);
  iceUtil::jobCache::setSnapshotFile(jsnapfile);

  try {
      iceUtil::jobCache::getInstance();
  }
  catch(exception& ex) {
      log_dev->log( log4cpp::Priority::FATAL, ex.what() );
      exit( 1 );
  }


  /*****************************************************************************
   * Initializes ice manager
   ****************************************************************************/ 
  glite::wms::ice::ice* iceManager;
  try {
    boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
    iceManager = new glite::wms::ice::ice(iceUtil::iceConfManager::getInstance()->getWMInputFile(), iceUtil::iceConfManager::getInstance()->getICEInputFile());
  } catch(glite::wms::ice::iceInit_ex& ex) {
      log_dev->log(log4cpp::Priority::ERROR, ex.what() );
    exit(1);
  } catch(...) {
      log_dev->log(log4cpp::Priority::ERROR, 
                   "Catched unknown exception" );
    exit(1);
  }


  
  /*****************************************************************************
   * Prepares a vector that will contains requests fetched from input file
   * list. Its initial capacity is set large enough... to tune...
   ****************************************************************************/
  vector<string> requests;
  requests.reserve(1000);

  /*****************************************************************************
   * Initializes CREAM client
   ****************************************************************************/
  soap_proxy::CreamProxyFactory::initProxy(true);
  if(!soap_proxy::CreamProxyFactory::getProxy())
    {
      log_dev->log(log4cpp::Priority::ERROR,
                   "CreamProxy creation failed! Stop" );
      exit(1);
    }
  soap_proxy::CreamProxyFactory::getProxy()->printOnConsole( true );
  soap_proxy::CreamProxyFactory::getProxy()->printDebug( true );
  try {
    soap_proxy::CreamProxyFactory::getProxy()->setSOAPHeaderID(hostdn);
  } catch(soap_proxy::auth_ex& ex) {
      log_dev->log(log4cpp::Priority::ERROR, ex.what() );
    exit(1);
  }



  /*****************************************************************************
   * Starts status poller and/or listener if specified in the config file
   ****************************************************************************/
  {
    boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
    if(iceUtil::iceConfManager::getInstance()->getStartListener())
      iceManager->startListener(iceUtil::iceConfManager::getInstance()->getListenerPort());

    if(iceUtil::iceConfManager::getInstance()->getStartPoller())
      iceManager->startPoller(iceUtil::iceConfManager::getInstance()->getPollerDelay());
  }
  // iceManager->startLeaseUpdater( ); // FIXME: starting this should be user-configurable

  vector<string> url_jid;
  url_jid.reserve(2);


  
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
        log_dev->infoStream()
            << "*** Found " << requests.size() << " new request(s)"
            << log4cpp::CategoryStream::ENDLINE;

    for(unsigned int j=0; j < requests.size( ); j++) {
        log_dev->infoStream()
            << "*** Unparsing request <"
            << requests[j] 
            << ">"
            << log4cpp::CategoryStream::ENDLINE;
        boost::scoped_ptr< glite::wms::ice::iceAbsCommand > cmd;
        try {
            glite::wms::ice::iceAbsCommand* tmp = glite::wms::ice::iceCommandFactory::mkCommand( requests[j] );
            cmd.reset( tmp ); // boost::scoped_ptr<>.reset() requires its argument not to throw anything
        }
        catch(std::exception& ex) {
            log_dev->log(log4cpp::Priority::ERROR, ex.what() );
            log_dev->log(log4cpp::Priority::INFO, "Removing BAD request..." );
            iceManager->removeRequest(j);
            continue;
        }
      try {
          cmd->execute( iceManager );
      } catch ( glite::wms::ice::iceCommandFatal_ex& ex ) {
          log_dev->errorStream()
              << "Command execution got FATAL exception: "
              << ex.what()
              << log4cpp::CategoryStream::ENDLINE;
      } catch ( glite::wms::ice::iceCommandTransient_ex& ex ) {
	log_dev->errorStream()
            << "Command execution got TRANSIENT exception: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;

	log_dev->log(log4cpp::Priority::INFO,
                     "Request will be resubmitted" );
          
      }
      log_dev->log(log4cpp::Priority::INFO,
                   "Removing submitted request from WM/ICE's filelist..." );
      try { 
	iceManager->removeRequest(j);
      } catch(exception& ex) {
          log_dev->log(log4cpp::Priority::ERROR,
                       string("Error removing request from FL: ")
                       +ex.what() );
	exit(1);
      }
    }
    sleep(1);
    requests.clear();
  }
  return 0;
}
