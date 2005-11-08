#include "iceCommandCancel.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

using namespace glite::wms::ice;
using namespace std;
using namespace glite::ce::cream_client_api;

iceCommandCancel::iceCommandCancel( const std::string& request ) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) :
    iceAbsCommand( )
{
    // Sample classad
    // [ Arguments = [ Force = false; LogFile = "/var/glitewms/logmonitor/CondorG.log/CondorG.1130511338.log"; ProxyFile = "/var/glitewms/SandboxDir/GZ/https_3a_2f_2ftigerman.cnaf.infn.it_3a9000_2fGZnxUvN6Ktfda5TRS68YZg/user.proxy"; SequenceCode = "UI=000000:NS=0000000008:WM=000002:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000"; JobId = "https://tigerman.cnaf.infn.it:9000/GZnxUvN6Ktfda5TRS68YZg" ]; Command = "Cancel"; Source = 2; Protocol = "1.0.0" ]
    
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

    if ( 0 != _commandStr.compare( "jobcancel" ) ) {
        throw util::JobRequest_ex("wrong command ["+_commandStr+"] parsed by iceCommandCancel" );
    }

    string _protocolStr;
    // Parse the "version" attribute
    if ( !_rootAD->EvaluateAttrString( "Protocol", _protocolStr ) ) {
        throw util::JobRequest_ex("attribute 'Protocol' not found or is not a string");
    }
    // Check if the version is exactly 1.0.0
    if ( _protocolStr.compare("1.0.0") ) {
        throw util::JobRequest_ex("Wrong 'Protocol' for jobCancel: expected 1.0.0, got " + _protocolStr );
    }

    classad::ClassAd *_argumentsAD = 0;
    // Parse the "arguments" attribute
    if ( !_rootAD->EvaluateAttrClassAd( "Arguments", _argumentsAD ) ) {
        throw util::JobRequest_ex("attribute 'Arguments' not found or is not a classad");
    }

    // Look for "JobId" attribute inside "Arguments"
    if ( !_argumentsAD->EvaluateAttrString( "JobId", _gridJobId ) ) {
        throw util::JobRequest_ex( "attribute 'JobId' inside 'Arguments' not found, or is not a string" );
    }

    // Look for "ProxyFile" attribute inside "Arguments"
    // FIXME: Is this necessary?
    if ( !_argumentsAD->EvaluateAttrString("Proxyfile", _certfile) ) {
        throw util::JobRequest_ex("attribute 'Proxyfile' not found in 'Arguments', or is not a string");
    }

    glite::ce::cream_client_api::util::string_manipulation::trim(_certfile, "\"");
}

void iceCommandCancel::execute( soap_proxy::CreamProxy* c, const string& cream, const string& creamd )
{
    log4cpp::Category* log_dev = glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger();

    cout << "\tThis request is a Submission..."<<endl;
    
    try {

        log_dev->log( log4cpp::Priority::INFO, 
                      "Autenticating with proxy [" + _certfile + "]" );
        c->Authenticate( _certfile );
        util::CreamJob _theJob( util::jobCache::getInstance()->getJobByGridJobID(_gridJobId) );
        vector<string> url_jid(1);   
        url_jid[1] = _theJob.getJobID();
        log_dev->log( log4cpp::Priority::INFO,
                      "Removing job gridJobId [" + _gridJobId + "], "
                      "creamJobId [" + url_jid[1] +"]" );
        c->Cancel( cream.c_str(), url_jid );
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
//         cerr << "Base ex: "<<base.what()<<endl;
//         submitter->ungetRequest(j);
//         submitter->removeRequest(j);
//        continue; // process next request
    } catch(cream_exceptions::InternalException& intern) {
        // TODO
        // MUST LOG TO LB
        cerr << "Internal ex: "<<intern.what()<<endl;
        exit(1);
    }
    // no failure: put jobids and status in cache
    // and remove last request from WM's filelist
    try {
        util::jobCache::getInstance()->remove_by_grid_jobid( _gridJobId );
    } catch(exception& ex) {
        cerr << "put in cache raised an ex: "<<ex.what()<<endl;
        exit(1);
    }
}
