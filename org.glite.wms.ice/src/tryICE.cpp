#include "ice-core.h"
#include "iceAbsCommand.h"
#include "iceCommandFactory.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include "iceConfManager.h"
#include "iceEventLogger.h"
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
  log_dev->setPriority( iceUtil::iceConfManager::getInstance()->getLogLevel() );
  logger_instance->setLogfileEnabled( iceUtil::iceConfManager::getInstance()->logOnFile() );
  logger_instance->setConsoleEnabled( iceUtil::iceConfManager::getInstance()->logOnConsole() );
  string logfile = iceUtil::iceConfManager::getInstance()->getLogFile();
  logger_instance->setLogFile(logfile.c_str());

  cout << "Logfile is [" << logfile << "]" << endl;


  /*****************************************************************************
   * Gets the distinguished name from the host proxy certificate
   ****************************************************************************/
  string hostcert = iceUtil::iceConfManager::getInstance()->getHostProxyFile();
  string hostdn;
  try {
    hostdn   = soap_proxy::CreamProxyFactory::getProxy()->getDN(hostcert);
  } catch ( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
    logger_instance->log(log4cpp::Priority::ERROR, 
			 "Unable to extract user DN from Proxy File " 
			 + hostcert, true, true, true);
    exit(1);
  }
  log_dev->log(log4cpp::Priority::INFO, 
               string("Host proxyfile is [") + hostcert + "]" );
  log_dev->log(log4cpp::Priority::INFO, 
               string("Host DN is ["+hostdn+"]") );


  /*****************************************************************************
   * Initializes job cache
   ****************************************************************************/ 
  string jcachefile = iceUtil::iceConfManager::getInstance()->getCachePersistFile();
  string jsnapfile  = iceUtil::iceConfManager::getInstance()->getCachePersistFile()+".snapshot";
  log_dev->infoStream() 
      << "Initializing jobCache with journal file ["
      << jcachefile 
      << "] and snapshot file ["
      << jsnapfile
      << "]..." 
      << log4cpp::CategoryStream::ENDLINE;

  iceUtil::jobCache::setJournalFile(jcachefile);
  iceUtil::jobCache::setSnapshotFile(jsnapfile);
  
  try {iceUtil::jobCache::getInstance();}
  catch(exception& ex) {
    log_dev->errorStream() << ex.what() << log4cpp::CategoryStream::ENDLINE;
  }


  /*****************************************************************************
   * Initializes ice manager
   ****************************************************************************/ 
  glite::wms::ice::ice* iceManager;
  try {
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
  if(iceUtil::iceConfManager::getInstance()->startListener()) 
    iceManager->startListener(iceUtil::iceConfManager::getInstance()->getListenerPort());

  if(iceUtil::iceConfManager::getInstance()->startPoller()) 
    iceManager->startPoller(iceUtil::iceConfManager::getInstance()->getPollerDelay());

  vector<string> url_jid;
  url_jid.reserve(2);


  
  /*
   * Initializes the L&B logger (this code is not compiled at the moment)
   */
#ifdef DO_NOT_COMPILE
  iceUtil::iceEventLogger* iceLogger = iceUtil::iceEventLogger::instance();
  iceUtil::ProxySet *ps = new iceUtil::ProxySet();
  ps->ps_x509Proxy = "/tmp/x509up_u219";
  ps->ps_x509Key = "/home/marzolla/.globus/userkey.pem";
  ps->ps_x509Cert = "/home/marzolla/.globus/usercert.pem";
  iceLogger->initialize_ice_context( ps );
#endif

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
      glite::wms::ice::iceAbsCommand* cmd = 0;
      try {
          cmd = glite::wms::ice::iceCommandFactory::mkCommand( requests[j] );
      }
      catch(std::exception& ex) {
          log_dev->log(log4cpp::Priority::ERROR, ex.what() );
          log_dev->log(log4cpp::Priority::INFO, "Removing BAD request..." );
	iceManager->removeRequest(j);
	continue;
      }
      try {
          cmd->execute( );
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
