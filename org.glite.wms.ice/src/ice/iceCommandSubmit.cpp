#include "iceCommandSubmit.h"
#include "jobCache.h"
#include "creamJob.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "boost/algorithm/string.hpp"
#include "glite/ce/cream-client-api-c/CEUrl.h"

#include <ctime>

using namespace glite::wms::ice;
using namespace std;
using namespace glite::ce::cream_client_api;
namespace ceurl_util = glite::ce::cream_client_api::util::CEUrl;

iceCommandSubmit::iceCommandSubmit( const std::string& request ) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) :
    iceAbsCommand( )
{
    // Sample classad:
    // ( this is the NEW version )
    //  [ requirements = ( ( ( ( other.GlueCEInfoHostName == "lxb2022.cern.ch" ) ) ) && ( other.GlueCEStateStatus == "Production" ) ) && ( other.GlueCEStateStatus == "Production" );
    //     RetryCount = 0;
    //     ce_id = "grid005.pd.infn.it:8443/cream_lsf_grid002";
    //     edg_jobid = "https://gundam.cnaf.infn.it:9000/ZLM4zE60OqHIcqH_PNbs-w";
    //     Arguments = "136100001 herrala_27411 360 20";
    //     OutputSandboxPath = "/var/glitewms/SandboxDir/ZL/https_3a_2f_2fgundam.cnaf.infn.it_3a9000_2fZLM4zE60OqHIcqH_5fPNbs-w/output";
    //     JobType = "normal";
    //     Executable = "glite-dev.tt_ch_160_tb20.testrun.sh.sh";
    //     CertificateSubject = "/O=Grid/O=NorduGrid/OU=hip.fi/CN=Juha Herrala";
    //     StdOutput = "cstdout";
    //     X509UserProxy = "/var/glitewms/SandboxDir/ZL/https_3a_2f_2fgundam.cnaf.infn.it_3a9000_2fZLM4zE60OqHIcqH_5fPNbs-w/user.proxy";
    //     OutputSandbox = { "cstdout","cstderr","testrun.log","testrun.root" };
    //     LB_sequence_code = "UI=000003:NS=0000000003:WM=000000:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000";
    //     InputSandboxPath = "/var/glitewms/SandboxDir/ZL/https_3a_2f_2fgundam.cnaf.infn.it_3a9000_2fZLM4zE60OqHIcqH_5fPNbs-w/input";
    //     VirtualOrganisation = "EGEE";
    //     rank =  -other.GlueCEStateEstimatedResponseTime;
    //     Type = "job";
    //     StdError = "cstderr";
    //     DefaultRank =  -other.GlueCEStateEstimatedResponseTime;
    //     InputSandbox = { "ExeTar.tt_ch_160_tb20.testrun.sh","glite-dev.tt_ch_160_tb20.testrun.sh.sh" } ] ]
    
    classad::ClassAdParser parser;
    classad::ClassAd *_rootAD = parser.ParseClassAd( request );

    if (!_rootAD)
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");

    string _commandStr;
    // Parse the "command" attribute
    if ( !_rootAD->EvaluateAttrString( "command", _commandStr ) ) {
        throw util::JobRequest_ex("attribute 'command' not found or is not a string");
    }
    boost::trim_if( _commandStr, boost::is_any_of("\"") );

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
    if ( !_adAD->EvaluateAttrString( "edg_jobid", _gridJobId ) ) {
        throw util::JobRequest_ex( "attribute 'edg_jobid' inside 'ad' not found, or is not a string" );
    }
    boost::trim_if(_gridJobId, boost::is_any_of("\"") );
}


void iceCommandSubmit::execute( void )
{
  try {
    vector<string> url_jid;

    cout << "\tThis request is a Submission..."<<endl;

    util::CreamJob theJob( _jdl,
			   "",
			   /* _gridJobId, */
			   job_statuses::UNKNOWN,
			   time(NULL));

    string ceid = theJob.getCEID();
    vector<string> ceid_pieces;
    ceurl_util::parseCEID( ceid, ceid_pieces );
    string bsname = ceid_pieces[2];
    string qname = ceid_pieces[3];

    // Update jdl to insert two new attributes needed by cream:
    // QueueName and BatchSystem.

    classad::ClassAdParser parser;
    classad::ClassAd *_root = parser.ParseClassAd( _jdl );
    _root->InsertAttr( "QueueName", qname );
    _root->InsertAttr( "BatchSystem", bsname );

    string modified_jdl;
    classad::ClassAdUnParser unparser;
    unparser.Unparse( modified_jdl, _root );
    
    cout << "\tSubmiting JDL " << modified_jdl << " to ["
	 << theJob.getCreamURL() << "][" << theJob.getCreamDelegURL()
	 << "]" << endl; 
    
    soap_proxy::CreamProxyFactory::getProxy()->Authenticate(theJob.getUserProxyCertificate());
    
    soap_proxy::CreamProxyFactory::getProxy()->Register( 
	theJob.getCreamURL().c_str(),
	theJob.getCreamDelegURL().c_str(),
        "", // deleg ID not needed because this client
        // will always do auto_delegation
        modified_jdl, 
        theJob.getUserProxyCertificate(),
        url_jid,
        true /*autostart*/ 
        );
    
    cout << "\tReturned CREAM-JOBID ["<<url_jid[1]<<"]"<<endl;

    // no failure: put jobids and status in cache
    // and remove last request from WM's filelist

    theJob.setJobID(url_jid[1]);
    theJob.setStatus(job_statuses::PENDING);

    //put(...) accepts arg by reference, but
    // the implementation puts the arg in the memory hash by copying it. So
    // passing a *pointer should not produce problems
    util::jobCache::getInstance()->put( theJob );

  } catch(util::ClassadSyntax_ex& ex) {
    cerr << ex.what()<<endl;
    exit(1);
  } catch(soap_proxy::auth_ex& ex) {
    cerr << "\tauth_ex: " << ex.what() << endl;
    exit(1);
  } 
  catch(soap_proxy::soap_ex& ex) {
    cerr << "\tsoap ex: "<<ex.what() << endl;
    // MUST LOG TO LB
    // HERE MUST RESUBMIT
    exit(1);
  } catch(cream_exceptions::BaseException& base) {
    cout << "cream_exception::BaseException: " 
	 << base.what() << endl;
    exit(1);
    // MUST LOG TO LB
    cerr << "Base ex: "<<base.what()<<endl;
//     submitter->ungetRequest(j);
//     submitter->removeRequest(j);
//    continue; // process next request
  } catch(cream_exceptions::InternalException& intern) {
    // TODO
    // MUST LOG TO LB
    cerr << "Internal ex: "<<intern.what()<<endl;
    exit(1);
  } catch(util::jnlFile_ex& ex) {
    cerr << "put in cache raised an ex: "<<ex.what()<<endl;
    exit(1);
  } catch(util::jnlFileReadOnly_ex& ex) {
    cerr << "put in cache raised an ex: "<<ex.what()<<endl;
    exit(1);
  } 
}
