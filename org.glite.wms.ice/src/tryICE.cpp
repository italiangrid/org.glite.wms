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
  log_dev->setPriority( log4cpp::Priority::DEBUG );
  logger_instance->setLogfileEnabled( true );
  string logfile = iceUtil::iceConfManager::getInstance()->getLogFile();
  logger_instance->setLogFile(logfile.c_str());
//   logger_instance->log(log4cpp::Priority::INFO, 
// 		       string("Logfile is ["+logfile+"]"), false, false, true);
  cout << "Logfile is [" << logfile << "]" << endl;


  /*****************************************************************************
   * Gets the distinguished name from the host proxy certificate
   ****************************************************************************/
  string hostcert = iceUtil::iceConfManager::getInstance()->getHostProxyFile();
  string hostdn   = soap_proxy::CreamProxyFactory::getProxy()->getDN(hostcert);
  logger_instance->log(log4cpp::Priority::INFO, 
		       string("Host proxyfile is [") + hostcert + "]",
		       true, true, true);
  //  cout << "Host proxyfile is [" <<hostcert<<"]"<<endl;
  logger_instance->log(log4cpp::Priority::INFO, 
		       string("Host DN is ["+hostdn+"]"),
		       true, true, true);
  //  cout << "Host DN is ["<<hostdn<<"]"<<endl;
//   cout << "Initializing jobCache with journal file ["
//        << iceUtil::iceConfManager::getInstance()->getCachePersistFile() 
//        << "] and snapshot file ["
//        << iceUtil::iceConfManager::getInstance()->getCachePersistFile()+".snapshot" 
//        << "]..."<<endl;
 


  /*****************************************************************************
   * Initializes job cache
   ****************************************************************************/ 
  string jcachefile = iceUtil::iceConfManager::getInstance()->getCachePersistFile();
  string jsnapfile  = iceUtil::iceConfManager::getInstance()->getCachePersistFile()+".snapshot";
  logger_instance->log(log4cpp::Priority::INFO, 
		       string("Initializing jobCache with journal file ["
			      +jcachefile 
			      +"] and snapshot file ["
			      +jsnapfile+"]..."),
		       true, true, true);
  iceUtil::jobCache::setJournalFile(jcachefile);
  iceUtil::jobCache::setSnapshotFile(jsnapfile);
  
  iceUtil::jobCache::getInstance();


  /*****************************************************************************
   * Initializes ice manager
   ****************************************************************************/ 
  glite::wms::ice::ice* iceManager;
  try {
    iceManager = new glite::wms::ice::ice(iceUtil::iceConfManager::getInstance()->getWMInputFile(), iceUtil::iceConfManager::getInstance()->getICEInputFile());
  } catch(glite::wms::ice::iceInit_ex& ex) {
    logger_instance->log(log4cpp::Priority::ERROR, ex.what(), true, true, true);
    exit(1);
  } catch(...) {
    //cerr << "something catched..."<<endl;
    logger_instance->log(log4cpp::Priority::ERROR, 
			 "Catched unknown exception", true, true, true);
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
      logger_instance->log(log4cpp::Priority::ERROR, 
			   "CreamProxy creation failed! Stop", true, true, true);
      exit(1);
    }
  soap_proxy::CreamProxyFactory::getProxy()->printOnConsole( true );
  soap_proxy::CreamProxyFactory::getProxy()->printDebug( true );
  try {
    soap_proxy::CreamProxyFactory::getProxy()->setSOAPHeaderID(hostdn);
  } catch(soap_proxy::auth_ex& ex) {
    //cerr << ex.what()<<endl;
    logger_instance->log(log4cpp::Priority::ERROR, ex.what(), true, true, true);
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
  
  
  
  /*****************************************************************************
   * Main loop that fetch requests from input filelist, submit/cancel the jobs,
   * removes requests from input filelist.
   ****************************************************************************/
  while(true) {
    
    iceManager->getNextRequests(requests);
    
    ostringstream os("");
    os << "*** Found " << requests.size() << " new request(s)";

    if( requests.size() )
      logger_instance->log(log4cpp::Priority::INFO, 
			   os.str(), 
			   true, true, true);
//       cout << "************* Found " 
// 	   << requests.size() << " new request(s)" << endl;
    
    for(unsigned int j=0; j < requests.size( ); j++) {
      logger_instance->log(log4cpp::Priority::INFO, 
			   string("*** Unparsing request <"
				  + requests[j] + ">"), 
			   true, true, true);
      //      cout << "-----> Unparsing request <"<<requests[j]<<">"<<endl;
      glite::wms::ice::iceAbsCommand* cmd = 0;
      try {
	cmd = glite::wms::ice::iceCommandFactory::mkCommand( requests[j] );
      }
      catch(std::exception& ex) {
	//cerr << "\tunparse ex: "<<ex.what()<<endl;
	logger_instance->log(log4cpp::Priority::ERROR,
			     ex.what(),
			     true, true, true);
	//cout << "\tRemoving BAD request..."<<endl;
	logger_instance->log(log4cpp::Priority::INFO,
			     "Removing BAD request...",
			     true, true, true);
	iceManager->removeRequest(j);
	continue;
      }
      //cout << "\tUnparse successfull..."<<endl;
      try {
          cmd->execute( );
      } catch ( glite::wms::ice::iceCommandFatal_ex& ex ) {
	logger_instance->log(log4cpp::Priority::ERROR,
			     string("Command execution got FATAL exception: ")
				    +ex.what(),
			     true, true, true);
//           cerr << "Command execution got FATAL exception: " 
//                << ex.what() << endl;
      } catch ( glite::wms::ice::iceCommandTransient_ex& ex ) {
	logger_instance->log(log4cpp::Priority::ERROR,
			     string("Command execution got TRANSIENT exception: ")
				    +ex.what(),
			     true, true, true);
	logger_instance->log(log4cpp::Priority::INFO,
			     "Request will be resubmitted",
			     true, true, true);
//           cerr << "Command execution got TRANSIENT exception: " 
//                << ex.what() << endl
//                << "Request will be resubmitted" << endl;
          
      }
      logger_instance->log(log4cpp::Priority::INFO,
			     "Removing submitted request from WM/ICE's filelist...",
			     true, true, true);
      //      cout << "\tRemoving submitted request from WM/ICE's filelist..."<<endl;
      try { 
	iceManager->removeRequest(j);
      } catch(exception& ex) {
	logger_instance->log(log4cpp::Priority::ERROR,
			     string("Error removing request from FL: ")
				    +ex.what(),
			     true, true, true);
	//	cerr << "Error removing request from FL: "<<ex.what()<<endl;
	exit(1);
      }
    }
    sleep(1);
    requests.clear();
  }
  return 0;
}
