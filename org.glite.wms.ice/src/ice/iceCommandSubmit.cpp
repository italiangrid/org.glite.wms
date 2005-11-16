#include "iceCommandSubmit.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
#include "jobCache.h"
#include "creamJob.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"

using namespace glite::wms::ice;
using namespace std;
using namespace glite::ce::cream_client_api;

iceCommandSubmit::iceCommandSubmit( const std::string& request ) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) :
    iceAbsCommand( )
{
    // Sample classad: 
    // [arguments = [ ad = [ QueueName="grid01"; BatchSystem="lsf"; X509UserProxy="/tmp/x509up_u202"; id="Job_di_Alvise"; VirtualOrganisation="EGEE"; executable="/bin/echo" ] ]; command = "jobsubmit"; version = "1.0.0" ]

    classad::ClassAdParser parser;
    classad::ClassAd *_rootAD = parser.ParseClassAd( request );

    if (!_rootAD)
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");

    string _commandStr;
    // Parse the "command" attribute
    if ( !_rootAD->EvaluateAttrString( "command", _commandStr ) ) {
        throw util::JobRequest_ex("attribute 'command' not found or is not a string");
    }
    glite::ce::cream_client_api::util::string_manipulation::trim(_commandStr, "\"");

    if ( 0 != _commandStr.compare( "jobsubmit" ) ) {
        throw util::JobRequest_ex("wrong command ["+_commandStr+"] parsed by iceCommandSubmit" );
    }

    string _versionStr;
    // Parse the "version" attribute
    if ( !_rootAD->EvaluateAttrString( "version", _versionStr ) ) {
        throw util::JobRequest_ex("attribute 'version' not found or is not a string");
    }
    // Check if the version is exactly 1.0.0
    if ( _versionStr.compare("1.0.0") ) {
        throw util::JobRequest_ex("Wrong \"version\" for jobRequest: expected 1.0.0, got " + _versionStr );
    }

    classad::ClassAd *_argumentsAD = 0;
    // Parse the "arguments" attribute
    if ( !_rootAD->EvaluateAttrClassAd( "arguments", _argumentsAD ) ) {
        throw util::JobRequest_ex("attribute 'arguments' not found or is not a classad");
    }

    classad::ClassAd *_adAD = 0;
    // Look for "ad" attribute inside "arguments"
    if ( !_argumentsAD->EvaluateAttrClassAd( "ad", _adAD ) ) {
        throw util::JobRequest_ex("Attribute 'ad' not found inside 'arguments', or is not a classad" );
    }

    // initializes the _jdl attribute
    classad::ClassAdUnParser unparser;
    unparser.Unparse( _jdl, _argumentsAD->Lookup( "ad" ) );

    // Look for "id" attribute inside "ad"
    if ( !_adAD->EvaluateAttrString( "id", _gridJobId ) ) {
        throw util::JobRequest_ex( "attribute 'id' inside 'ad' not found, or is not a string" );
    }
    glite::ce::cream_client_api::util::string_manipulation::trim(_gridJobId, "\"");

//     // Look for "X509UserProxy" attribute inside "ad"
//     if ( !_adAD->EvaluateAttrString("X509UserProxy", _certfile) ) {
//         throw util::JobRequest_ex("attribute 'X509UserProxy' not found in JDL");
//     }

//     glite::ce::cream_client_api::util::string_manipulation::trim(_certfile, "\"");

}

void iceCommandSubmit::execute( /* soap_proxy::CreamProxy* c */ )
{
    vector<string> url_jid;

    cout << "\tThis request is a Submission..."<<endl;

    util::CreamJob *theJob;
    try {
      theJob = new util::CreamJob( _jdl,
				   "",
				   _gridJobId,
				   job_statuses::UNKNOWN );
    } catch(util::ClassadSyntax_ex& ex) {
      cerr << ex.what()<<endl;
      delete( theJob );
      exit(1);
    }

    try {

        cout << "\tSubmiting JDL " << _jdl << " to ["
             << theJob->getCreamURL() << "][" << theJob->getCreamDelegURL()
	     << "]" << endl; 
	
	soap_proxy::CreamProxyFactory::getProxy()->Authenticate(theJob->getUserProxyCertificate());

        soap_proxy::CreamProxyFactory::getProxy()->Register( 
		    theJob->getCreamURL().c_str(),
		    theJob->getCreamDelegURL().c_str(),
		    "", // deleg ID not needed because this client
		    // will always do auto_delegation
		    _jdl, 
		    theJob->getUserProxyCertificate(),
		    url_jid,
		    true /*autostart*/ 
		    );
	    
        cout << "\tReturned CREAM-JOBID ["<<url_jid[1]<<"]"<<endl;
    } catch(soap_proxy::auth_ex& ex) {
      cerr << "\tauth_ex: " << ex.what() << endl;
      delete( theJob );
      exit(1);
    } 
    catch(soap_proxy::soap_ex& ex) {
        cerr << "\tsoap ex: "<<ex.what() << endl;
        // MUST LOG TO LB
        // HERE MUST RESUBMIT
	delete( theJob );
        exit(1);
    } catch(cream_exceptions::BaseException& base) {
        cout << "cream_exception::BaseException: " 
             << base.what() << endl;
	delete( theJob );
        exit(1);
        // MUST LOG TO LB
//         cerr << "Base ex: "<<base.what()<<endl;
//         submitter->ungetRequest(j);
//         submitter->removeRequest(j);
//        continue; // process next request
    } catch(cream_exceptions::InternalException& intern) {
        // TODO
        // MUST LOG TO LB
        cerr << "Internal ex: "<<intern.what()<<endl;
	delete( theJob );
        exit(1);
    }
    // no failure: put jobids and status in cache
    // and remove last request from WM's filelist

    theJob->setJobID(url_jid[1]);
    theJob->setStatus(job_statuses::PENDING);

    try {
      //        util::CreamJob theJob( _jdl, url_jid[1], _gridJobId, job_statuses::PENDING );
       
      //put(...) accepts arg by reference, but
      // the implementation puts the arg in the memory hash by copying it. So
      // passing a *pointer should not produce problems
      util::jobCache::getInstance()->put( *theJob );
    } catch(exception& ex) {
        cerr << "put in cache raised an ex: "<<ex.what()<<endl;
	delete( theJob );
        exit(1);
    }
    delete( theJob );
}
