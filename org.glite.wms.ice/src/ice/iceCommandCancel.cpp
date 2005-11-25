#include "iceCommandCancel.h"
#include "boost/algorithm/string.hpp"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"

using namespace glite::wms::ice;
using namespace std;
using namespace glite::ce::cream_client_api;

iceCommandCancel::iceCommandCancel( const std::string& request ) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) :
    iceAbsCommand( )
{
    // Sample classad
    // [ version = "1.0.0"; command = "jobcancel"; arguments = [ id = "https://gundam.cnaf.infn.it:9000/mm6jfVKnjutb9JNFaF1HuQ"; lb_sequence_code = "UI=000004:NS=0000000006:WM=000000:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000" ] ]

    classad::ClassAdParser parser;
    classad::ClassAd *_rootAD = parser.ParseClassAd( request );

    if (!_rootAD)
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");

    string _commandStr;
    // Parse the "command" attribute
    if ( !_rootAD->EvaluateAttrString( "command", _commandStr ) ) {
        throw util::JobRequest_ex("attribute 'command' not found or is not a string");
    }
    boost::trim_if(_commandStr, boost::is_any_of("\""));

    if ( 0 != _commandStr.compare( "jobcancel" ) ) {
        throw util::JobRequest_ex("wrong command ["+_commandStr+"] parsed by iceCommandCancel" );
    }

    string _versionStr;
    // Parse the "version" attribute
    if ( !_rootAD->EvaluateAttrString( "version", _versionStr ) ) {
        throw util::JobRequest_ex("attribute 'version' not found or is not a string");
    }
    // Check if the version is exactly 1.0.0
    if ( _versionStr.compare("1.0.0") ) {
        throw util::JobRequest_ex("Wrong 'Protocol' for jobCancel: expected 1.0.0, got " + _versionStr );
    }

    classad::ClassAd *_argumentsAD = 0;
    // Parse the "arguments" attribute
    if ( !_rootAD->EvaluateAttrClassAd( "arguments", _argumentsAD ) ) {
        throw util::JobRequest_ex("attribute 'arguments' not found or is not a classad");
    }

    // Look for "id" attribute inside "Arguments"
    if ( !_argumentsAD->EvaluateAttrString( "id", _gridJobId ) ) {
        throw util::JobRequest_ex( "attribute 'id' inside 'arguments' not found, or is not a string" );
    }

}

void iceCommandCancel::execute( void ) throw ( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
    log4cpp::Category* log_dev = glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger();

    cout << "\tThis request is a Cancel..."<<endl;
    
    try {
        util::CreamJob _theJob( util::jobCache::getInstance()->getJobByGridJobID(_gridJobId) );
        vector<string> url_jid(1);   
        url_jid[1] = _theJob.getJobID();
        log_dev->log( log4cpp::Priority::INFO,
                      "Removing job gridJobId [" + _gridJobId + "], "
                      "creamJobId [" + url_jid[1] +"]" );

	cout <<"Sending cancellation requesto to ["<<_theJob.getCreamURL()<<"]"<<endl;

	soap_proxy::CreamProxyFactory::getProxy()->Authenticate(_theJob.getUserProxyCertificate());

	soap_proxy::CreamProxyFactory::getProxy()->Cancel( _theJob.getCreamURL().c_str(), url_jid );

    } catch(soap_proxy::auth_ex& ex) {
        throw iceCommandFatal_ex( string("auth_ex: ") + ex.what() );
    } catch(soap_proxy::soap_ex& ex) {
        throw iceCommandTransient_ex( string("soap_ex: ") + ex.what() );
        // MUST LOG TO LB
        // HERE MUST RESUBMIT
    } catch(cream_exceptions::BaseException& base) {
        throw iceCommandFatal_ex( string("BaseException: ") + base.what() );
        // MUST LOG TO LB
    } catch(cream_exceptions::InternalException& intern) {
        throw iceCommandFatal_ex( string("InternalException: ") + intern.what() );
        // TODO
        // MUST LOG TO LB
    } catch( util::elementNotFound_ex& ex ) {
        throw iceCommandFatal_ex( string("ElementNotFound_ex: ") + ex.what() );
    }
    // no failure: put jobids and status in cache
    // and remove last request from WM's filelist
    try {
        util::jobCache::getInstance()->remove_by_grid_jobid( _gridJobId );
    } catch(exception& ex) {
        throw iceCommandFatal_ex( string("put in cache raised an exception: ") + ex.what() );
    }
}
