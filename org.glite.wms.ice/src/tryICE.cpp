
#include "ice-core.h"
#include "jobRequest.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"

#include <string>
#include <iostream>
#include <unistd.h>

using namespace std;
using namespace glite::ce::cream_client_api;

int main(int argc, char*argv[]) {
  
  /**
   * - creates an XYZ object
   * - initializes the job cache
   * - starts the async event consumer
   * - opens the WM's and the NS's filelist
   * 
   * - main's params:
   *                  argv[1]: Network server filelist
   *                  argv[2]: WM filelist
   *                  argv[3]: job cache persistency file
   *                  argv[4]: TCP port for event status listener
   *                  argv[5]: Cream endpoint (host:port)
   *                  argv[6]: host's certificate
   */

  if(argc<7) return 1;

  string CREAM  = string("https://")+argv[5]+"/ce-cream/services/CREAM";
  string CREAMD = string("https://")+argv[5]+"/ce-cream/services/CREAMDelegation";

  glite::wms::ice::ice* submitter;
  try {
    submitter = new glite::wms::ice::ice(argv[1], 
					 argv[2], 
					 argv[3], 
					 atoi(argv[4]), 
					 false/* do not start listener */, 
					 true/* start poller */, 
					 10, 
					 CREAM,
					 argv[6]);
  } catch(glite::wms::ice::iceInit_ex& ex) {
    cerr << ex.what() <<endl;
    exit(1);
  } catch(...) {
    cerr << "something catched..."<<endl;
    exit(1);
  }
  
  vector<string> requests;
  soap_proxy::CreamProxy creamClient( /*automatic_delegation*/ true );
  creamClient.printOnConsole( true );
  creamClient.printDebug( true );
  

  vector<string> url_jid;
  glite::wms::ice::jobRequest R;
  while(true) {

    //cout << "********** Getting requests from filelist..."<<endl;

    submitter->getNextRequests(requests);
    
    if(requests.size( ))
      cout << "************* Found " << requests.size( ) << " request(s)"<<endl;
    
    for(unsigned int j=0; j < requests.size( ); j++)
      {
	cout << "-----> Unparsing request ["<<requests[j]<<"]"<<endl;
	try {R.unparse(requests[j]);}
	catch(std::exception& ex) {
	  cerr << "\tunaprse ex: "<<ex.what()<<endl;
	  cout << "\tRemoving BAD request..."<<endl;
	  submitter->removeRequest(j);
	  continue;
	}

	cout << "\tUnparse successfull..."<<endl;

	//cout << "This request is a ["<<R.getCommand( )<<"]"<<endl;

	if(R.getCommand( ) == R.jobsubmit) {

	  cout << "\tThis request is a Submission..."<<endl;

	  try {
	    //string newJDL = JDLHelper.manipulate(R.getUserJDL());
	    string newJDL = R.getUserJDL();
	    
	    cout << "\tAuthenticating with proxy ["
		 << R.getProxyCertificate()<<"]"<<endl;
	    creamClient.Authenticate( R.getProxyCertificate() );

	    cout << "\tSubmiting JDL ["<<newJDL<<"] to ["
		 <<CREAM.c_str()<<"]["<<CREAMD.c_str()<<"]"<<endl; 
	    
	    creamClient.Register( CREAM.c_str(), 
				  CREAMD.c_str(), 
				  "", // deleg ID not needed because this client
				  // will always do auto_delegation
				  newJDL, // JDL
				  R.getProxyCertificate(), // cert file for auto deleg.
				  url_jid,
				  true /*autostart*/ );
	    
	    cout << "\tReturned CREAM-JOBID ["<<url_jid[1]<<"]"<<endl;
	  } catch(soap_proxy::soap_ex& ex) {
	    cerr << "\tsoap ex: "<<ex.what() << endl;
	    // MUST LOG TO LB
	    // HERE MUST RESUBMIT
	    exit(1);
	  } catch(soap_proxy::auth_ex& ex) {
	    cerr << "\tauthN ex: " << ex.what() << endl;
	    // MUST LOG TO LB
	    exit(1);
	  } catch(cream_exceptions::BaseException& base) {
	    // MUST LOG TO LB
	    cerr << "\tBase ex: "<<base.what()<<endl;
	    submitter->ungetRequest(j);
	    submitter->removeRequest(j);
	    continue; // process next request
	  } catch(cream_exceptions::InternalException& intern) {
	    // TODO
	    // MUST LOG TO LB
	    cerr << "\tInternal ex: "<<intern.what()<<endl;
	    exit(1);
	  }
	  // no failure: put jobids and status in cache
	  // and remove last request from WM's filelist
	  try {
	    cout << "\tGoing to put submitted job in cache ["
		 << R.getGridJobID( ) << "] ["<<url_jid[1]<<"] ["
		 << job_statuses::PENDING<<"]"<<endl;
	    glite::wms::ice::util::jobCache::getInstance()->put(R.getGridJobID( ), 
								url_jid[1], 
								job_statuses::PENDING);
	  } catch(exception& ex) {
	    cerr << "\tput in cache raised an ex: "<<ex.what()<<endl;
	    exit(1);
	  } catch(...) {
	    cerr << "\tCatched unknown exception..."<<endl;
	    exit(1);
	  }
	  cout << "\tRemoving submitted request from WM/ICE's filelist..."<<endl;
	  submitter->removeRequest(j);
	}
	if(R.getCommand() == R.jobcancel) {
	  cout << "\tThis request is a Cancel..."<<endl;
	  try {
	    cout << "\tWill cancel job [" << R.getGridJobID() << "]" << endl;
	    // HERE MUST CANCEL JOB
	    //	    creamClient.Cancel( )
	  } catch(soap_proxy::soap_ex& ex) {
	    cerr << ex.what() << endl;
	    // MUST LOG TO LB
	  }
	}
      }
    sleep(1);
    requests.clear();
  }
  
  return 0;
}
