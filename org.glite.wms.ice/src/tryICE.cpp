

#include "ice-core.h"
#include "iceAbsCommand.h"
#include "iceCommandFactory.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"

#include <string>
#include <iostream>
#include <unistd.h>

using namespace std;
using namespace glite::ce::cream_client_api;

#define USE_STATUS_POLLER true
#define USE_STATUS_LISTENER false


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
   *                  argv[5]: host's certificate
   */

  if(argc<6) return 1;

//   string CREAM  = string("https://")+argv[5]+"/ce-cream/services/CREAM";
//   string CREAMD = string("https://")+argv[5]+"/ce-cream/services/CREAMDelegation";

  glite::wms::ice::ice* submitter;
  try {
    submitter = new glite::wms::ice::ice(argv[1], 
					 argv[2], 
					 argv[3], 
					 atoi(argv[4]), 
					 USE_STATUS_LISTENER, 
					 USE_STATUS_POLLER, 
					 10,
					 argv[5]);
  } catch(glite::wms::ice::iceInit_ex& ex) {
    cerr << ex.what() <<endl;
    exit(1);
  } catch(...) {
    cerr << "something catched..."<<endl;
    exit(1);
  }
  
    vector<string> requests;
    requests.reserve(1000);
    soap_proxy::CreamProxy creamClient( /*automatic_delegation*/ true );
    creamClient.printOnConsole( true );
    creamClient.printDebug( true );
  

    vector<string> url_jid;
    url_jid.reserve(2);
    //  glite::wms::ice::jobRequest R;
  
    while(true) {

        //cout << "********** Getting requests from filelist..."<<endl;

        submitter->getNextRequests(requests);
    
        if(requests.size( ))
            cout << "************* Found " << requests.size( ) << " new request(s)"<<endl;
    
        //sleep(1000);

        for(unsigned int j=0; j < requests.size( ); j++) {
            cout << "-----> Unparsing request <"<<requests[j]<<">"<<endl;
            glite::wms::ice::iceAbsCommand* cmd = 0;
            try {
                cmd = glite::wms::ice::iceCommandFactory::mkCommand( requests[j] );
            }
            catch(std::exception& ex) {
                cerr << "\tunparse ex: "<<ex.what()<<endl;
                cout << "\tRemoving BAD request..."<<endl;
                submitter->removeRequest(j);
                continue;
            }
            cout << "\tUnparse successfull..."<<endl;
            
            //cout << "This request is a ["<<R.getCommand( )<<"]"<<endl;
            
            cmd->execute( &creamClient );
            
            cout << "\tRemoving submitted request from WM/ICE's filelist..."<<endl;
            submitter->removeRequest(j);
        }
        sleep(1);
        requests.clear();
    }
    
    return 0;

#ifdef PIPPO
    for(unsigned int j=0; j < requests.size( ); j++)
      {
	cout << "----->  Unparsing request..."<<endl;// <"<<requests[j]<<">"<<endl;
	try {R.unparse(requests[j]);}
	catch(std::exception& ex) {
	  cerr << "\tunparse ex: "<<ex.what()<<endl;
	  cout << "\tRemoving BAD request..."<<endl;
	  submitter->removeRequest(j);
	  continue;
	}

	cout << "\tUnparse successfull..."<<endl;

	//cout << "This request is a ["<<R.getCommand( )<<"]"<<endl;
	string newJDL = R.getUserJDL();
	glite::wms::ice::util::CreamJob *cj;
	try {
	  cj = new glite::wms::ice::util::CreamJob(newJDL, 
						   "", 
						   R.getGridJobID( ), 
						   job_statuses::PENDING);
	} catch(glite::wms::ice::util::ClassadSyntax_ex& ex) {
	  cerr << ex.what()<<endl;
	  exit(1);
	}
      
	
	if(R.getCommand( ) == R.jobsubmit) {

	  cout << "\tThis request is a Submission..."<<endl;
	  try {
	    //string newJDL = JDLHelper.manipulate(R.getUserJDL());
	    
	    cout << "\tAuthenticating with proxy ["
		 << R.getProxyCertificate()<<"]"<<endl;
	    creamClient.Authenticate( R.getProxyCertificate() );

	    cout << "\tSubmiting JDL <"<<newJDL<<"> to ["
		 << cj->getCreamURL() << "]["
		 << cj->getCreamDelegURL() << "]" << endl; 

	    creamClient.Register( cj->getCreamURL().c_str(), 
				  cj->getCreamDelegURL().c_str(), 
				  "", // deleg ID not needed because this client
				  // will always do auto_delegation
				  newJDL, // JDL
				  R.getProxyCertificate(), // cert file for auto deleg.
				  url_jid,
				  true /*autostart*/ );
	    
	    cj->setJobID(url_jid[1]);

	    cout << "\tReturned CREAM-JOBID ["<<url_jid[1]<<"]"<<endl;
	  } catch(soap_proxy::soap_ex& ex) {
	    cerr << "\tsoap ex: "<<ex.what() << endl;
	    // MUST LOG TO LB
	    // HERE MUST RESUBMIT
	    submitter->ungetRequest(j);
	    // Removing current request from WM's output FL
	    submitter->removeRequest(j);
	    continue; // process next request
	    exit(1);
	  } catch(soap_proxy::auth_ex& ex) {
	    cerr << "\tauthN ex: " << ex.what() << endl;
	    // The CreamProxy::Authenticate(...) failed
	    // or the certificate is expired
	    // MUST LOG TO LB
	    exit(1);
	  } catch(cream_exceptions::BaseException& base) {
	    // MUST LOG TO LB
	    // Resubmitting request to WM's input FL
	    cerr << "\tBase ex: "<<base.what()<<endl;
	    submitter->ungetRequest(j);
	    // Removing current request from WM's output FL
	    submitter->removeRequest(j);
	    continue; // process next request
	  } catch(cream_exceptions::InternalException& intern) {
	    // TODO
	    // MUST LOG TO LB
	    cerr << "\tInternal ex: "<<intern.what()<<endl;
	    exit(1);
	  } catch(cream_exceptions::DelegationException& deleg) {
	    // MUST LOG TO LB
	    // Resubmitting request to WM's input FL
	    cerr << "\tDelegation ex: "<<deleg.what()<<endl;
	    submitter->ungetRequest(j);
	    // Removing current request from WM's output FL
	    submitter->removeRequest(j);
	    continue; // process next request
	  }

	  // no failure: put jobids and status in cache
	  // and remove last request from WM's filelist
	  try {
	    cout << "\tGoing to put submitted job in cache ["
		 << R.getGridJobID( ) << "] ["<<url_jid[1]<<"] ["
		 << job_statuses::PENDING<<"]"<<endl;
// 	    glite::wms::ice::util::jobCache::getInstance()->put(R.getGridJobID( ), 
// 								url_jid[1], 
// 								job_statuses::PENDING);
	    glite::wms::ice::util::jobCache::getInstance()->put( *cj );

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
#endif

}
