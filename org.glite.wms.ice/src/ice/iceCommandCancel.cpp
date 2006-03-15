#include "iceCommandCancel.h"
#include "jobCache.h"
#include "ice-core.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"

#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"

#include "boost/algorithm/string.hpp"

using namespace glite::wms::ice;
using namespace std;
using namespace glite::ce::cream_client_api;

iceCommandCancel::iceCommandCancel( const std::string& request ) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) :
    iceAbsCommand( ),
    log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()),
    _lb_logger( util::iceLBLogger::instance() )
{

    /*

[ Arguments = 
  [ Force = false; 
    LogFile = "/var/glite/logmonitor/CondorG.log/CondorG.1140166320.log"; 
    ProxyFile = "/var/glite/SandboxDir/tC/https_3a_2f_2fcert-rb-03.cnaf.infn.it_3a9000_2ftCDrNTu0b0uPbeUEDlpTjg/user.proxy";
    SequenceCode = "UI=000002:NS=0000000007:WM=000002:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000"; 
    JobId = "https://cert-rb-03.cnaf.infn.it:9000/tCDrNTu0b0uPbeUEDlpTjg" 
  ]; 
  Command = "Cancel"; 
  Source = 2; 
  Protocol = "1.0.0" 
]

*/
    classad::ClassAdParser parser;
    classad::ClassAd *_rootAD = parser.ParseClassAd( request );

    if (!_rootAD)
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");

    string _commandStr;
    // Parse the "command" attribute
    if ( !_rootAD->EvaluateAttrString( "command", _commandStr ) ) {
        throw util::JobRequest_ex("attribute \"command\" not found or is not a string");
    }
    boost::trim_if(_commandStr, boost::is_any_of("\""));

    if ( !boost::algorithm::iequals( _commandStr, "cancel" ) ) {
        throw util::JobRequest_ex("wrong command ["+_commandStr+"] parsed by iceCommandCancel" );
    }

    string _protocolStr;
    // Parse the "version" attribute
    if ( !_rootAD->EvaluateAttrString( "protocol", _protocolStr ) ) {
        throw util::JobRequest_ex("attribute \"protocol\" not found or is not a string");
    }
    // Check if the version is exactly 1.0.0
    if ( _protocolStr.compare("1.0.0") ) {
        throw util::JobRequest_ex("Wrong \"Protocol\" for jobCancel: expected 1.0.0, got " + _protocolStr );
    }

    classad::ClassAd *_argumentsAD = 0;
    // Parse the "arguments" attribute
    if ( !_rootAD->EvaluateAttrClassAd( "arguments", _argumentsAD ) ) {
        throw util::JobRequest_ex("attribute \"arguments\" not found or is not a classad");
    }

    // Look for "id" attribute inside "Arguments"
    if ( !_argumentsAD->EvaluateAttrString( "jobid", _gridJobId ) ) {
        throw util::JobRequest_ex( "attribute \"jobid\" inside \"arguments\" not found, or is not a string" );
    }

    // Look for "lb_sequence_code" attribute inside "Arguments"
    if ( !_argumentsAD->EvaluateAttrString( "sequencecode", _sequence_code ) ) {
        // FIXME: This should be an error to throw. For now, we try anyway...
        log_dev->warnStream()
            << "Cancel request does not have a \"sequencecode\" attribute. "
            << "Fine for now, should not happen in the future"
            << log4cpp::CategoryStream::ENDLINE;
    } else {
        boost::trim_if(_sequence_code, boost::is_any_of("\""));        
    }
}

void iceCommandCancel::execute( ice* _ice ) throw ( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
    log_dev->infoStream()
        << "This request is a Cancel..."
        << log4cpp::CategoryStream::ENDLINE;

    boost::recursive_mutex::scoped_lock M( util::jobCache::mutex );

    // Lookup the job in the jobCache
    util::jobCache::iterator it = util::jobCache::getInstance()->lookupByGridJobID( _gridJobId );
    if ( it == util::jobCache::getInstance()->end() ) {
        log_dev->errorStream()
            << "Cancel operation cannot locate jobid=["
            << _gridJobId 
            << "] in the jobCache. Giving up"
            << log4cpp::CategoryStream::ENDLINE;

        throw iceCommandFatal_ex( string("ICE cannot cancel job with grid job id=[") + _gridJobId + string("], as the job does not appear to exist") );
    }

    // Set Sequence code, if any...
    // FIXME: Remove the if() statement. Sequence code MUST be present
    if ( ! _sequence_code.empty() )  
        it->setSequenceCode( _sequence_code );

    // Log cancel request event
    _lb_logger->logEvent( new util::cream_cancel_request_event( *it ) );    

    util::CreamJob _theJob( *it );
    vector<string> url_jid(1);   
    url_jid[0] = _theJob.getJobID();
    log_dev->infoStream()
        << "Removing job gridJobId [" 
        << _gridJobId
        << "], creamJobId [" 
        << url_jid[0] 
        << "]"
        << log4cpp::CategoryStream::ENDLINE;
    
    log_dev->infoStream()
        << "Sending cancellation requesto to ["
        << _theJob.getCreamURL() << "]"
        << log4cpp::CategoryStream::ENDLINE;
    
    try {

	soap_proxy::CreamProxyFactory::getProxy()->Authenticate(_theJob.getUserProxyCertificate());

	soap_proxy::CreamProxyFactory::getProxy()->Cancel( _theJob.getCreamURL().c_str(), url_jid );

    } catch(soap_proxy::auth_ex& ex) {
        _lb_logger->logEvent( new util::cream_cancel_refuse_event( _theJob, string("auth_ex: ") + ex.what() ) );
        throw iceCommandFatal_ex( string("auth_ex: ") + ex.what() );
    } catch(soap_proxy::soap_ex& ex) {
        _lb_logger->logEvent( new util::cream_cancel_refuse_event( _theJob, string("soap_ex: ") + ex.what() ) );
        throw iceCommandTransient_ex( string("soap_ex: ") + ex.what() );
        // HERE MUST RESUBMIT
    } catch(cream_exceptions::BaseException& base) {
        _lb_logger->logEvent( new util::cream_cancel_refuse_event( _theJob, string("BaseException: ") + base.what() ) );
        throw iceCommandFatal_ex( string("BaseException: ") + base.what() );
    } catch(cream_exceptions::InternalException& intern) {
        _lb_logger->logEvent( new util::cream_cancel_refuse_event( _theJob, string("InternalException: ") + intern.what() ) );
        throw iceCommandFatal_ex( string("InternalException: ") + intern.what() );
    }

    // no failure: put jobids and status in cache
    // and remove last request from WM's filelist
    util::jobCache::getInstance()->erase( it );
}
