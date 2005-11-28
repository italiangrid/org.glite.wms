

#include "ice-core.h"
#include "iceAbsCommand.h"
#include "iceCommandFactory.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include <string>
#include <iostream>
#include <unistd.h>

using namespace std;
using namespace glite::ce::cream_client_api;

#define USE_STATUS_POLLER false
#define USE_STATUS_LISTENER true

int main(int argc, char*argv[]) {
  
  /**
   * - creates an ICE object
   * - initializes the job cache
   * - starts the async event consumer
   * - opens the WM's and the NS's filelist
   * 
   * - main's params:
   *                  argv[1]: Network server filelist
   *                  argv[2]: WM filelist
   *                  argv[3]: job cache persistency file
   *                  argv[4]: TCP port for event status listener
   */

  if ( argc!=5 ) {
      cout << "Usage: " << argv[0]
           << " <ns> <wm> <cache> <port>" << endl
           << "\t<ns> Network Server filelist" << endl
           << "\t<wm> WM filelist" << endl
           << "\t<cache> Job Cache persistency file" << endl
           << "\t<port> TCP port for event status listener" << endl;
      return -1;
  } 

  glite::wms::ice::ice* iceManager;
  try {
    iceManager = new glite::wms::ice::ice(argv[1], argv[2], argv[3]);
  } catch(glite::wms::ice::iceInit_ex& ex) {
    cerr << ex.what() <<endl;
    exit(1);
  } catch(...) {
    cerr << "something catched..."<<endl;
    exit(1);
  }
  
  vector<string> requests;
  requests.reserve(1000);
  soap_proxy::CreamProxyFactory::initProxy(true);
  if(!soap_proxy::CreamProxyFactory::getProxy())
    {
      cerr << "CreamProxy creation went wrong. Stop"<<endl;
      exit(1);
    }
  soap_proxy::CreamProxyFactory::getProxy()->printOnConsole( true );
  soap_proxy::CreamProxyFactory::getProxy()->printDebug( true );
  
  if(USE_STATUS_LISTENER) iceManager->startListener(atoi(argv[4]));
  if(USE_STATUS_POLLER) iceManager->startPoller(10);

  vector<string> url_jid;
  url_jid.reserve(2);
  
  while(true) {
    
    iceManager->getNextRequests(requests);
    
    if( requests.size() )
      cout << "************* Found " 
	   << requests.size() << " new request(s)" << endl;
    
    for(unsigned int j=0; j < requests.size( ); j++) {
      cout << "-----> Unparsing request <"<<requests[j]<<">"<<endl;
      glite::wms::ice::iceAbsCommand* cmd = 0;
      try {
	cmd = glite::wms::ice::iceCommandFactory::mkCommand( requests[j] );
      }
      catch(std::exception& ex) {
	cerr << "\tunparse ex: "<<ex.what()<<endl;
	cout << "\tRemoving BAD request..."<<endl;
	iceManager->removeRequest(j);
	continue;
      }
      cout << "\tUnparse successfull..."<<endl;
      try {
          cmd->execute( );
      } catch ( glite::wms::ice::iceCommandFatal_ex& ex ) {
          cerr << "Command execution got FATAL exception: " 
               << ex.what() << endl;
      } catch ( glite::wms::ice::iceCommandTransient_ex& ex ) {
          cerr << "Command execution got TRANSIENT exception: " 
               << ex.what() << endl;
      }
      cout << "\tRemoving submitted request from WM/ICE's filelist..."<<endl;
      try { 
	iceManager->removeRequest(j);
      } catch(exception& ex) {
	cerr << "Error removing request from FL: "<<ex.what()<<endl;
	exit(1);
      }
    }
    sleep(1);
    requests.clear();
  }
  return 0;
}
