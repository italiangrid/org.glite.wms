/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE command cancel class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceCommandCancel.h"
#include "jobCache.h"
#include "ice-core.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"

#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"

#include "boost/algorithm/string.hpp"

using namespace std;
namespace cream_api = glite::ce::cream_client_api;

namespace glite {
namespace wms {
namespace ice {

iceCommandCancel::iceCommandCancel( const std::string& request ) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) :
    iceAbsCommand( ),
    m_log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()),
    m_lb_logger( util::iceLBLogger::instance() )
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
    classad::ClassAd *rootAD = parser.ParseClassAd( request );

    if (!rootAD)
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");

    string commandStr;
    // Parse the "command" attribute
    if ( !rootAD->EvaluateAttrString( "command", commandStr ) ) {
        throw util::JobRequest_ex("attribute \"command\" not found or is not a string");
    }
    boost::trim_if(commandStr, boost::is_any_of("\""));

    if ( !boost::algorithm::iequals( commandStr, "cancel" ) ) {
        throw util::JobRequest_ex("wrong command ["+commandStr+"] parsed by iceCommandCancel" );
    }

    string protocolStr;
    // Parse the "version" attribute
    if ( !rootAD->EvaluateAttrString( "protocol", protocolStr ) ) {
        throw util::JobRequest_ex("attribute \"protocol\" not found or is not a string");
    }
    // Check if the version is exactly 1.0.0
    if ( protocolStr.compare("1.0.0") ) {
        throw util::JobRequest_ex("Wrong \"Protocol\" for jobCancel: expected 1.0.0, got " + protocolStr );
    }

    classad::ClassAd *argumentsAD = 0;
    // Parse the "arguments" attribute
    if ( !rootAD->EvaluateAttrClassAd( "arguments", argumentsAD ) ) {
        throw util::JobRequest_ex("attribute \"arguments\" not found or is not a classad");
    }

    // Look for "id" attribute inside "Arguments"
    if ( !argumentsAD->EvaluateAttrString( "jobid", m_gridJobId ) ) {
        throw util::JobRequest_ex( "attribute \"jobid\" inside \"arguments\" not found, or is not a string" );
    }

    // Look for "lb_sequence_code" attribute inside "Arguments"
    if ( !argumentsAD->EvaluateAttrString( "sequencecode", m_sequence_code ) ) {
        // FIXME: This should be an error to throw. For now, we try anyway...
        CREAM_SAFE_LOG( m_log_dev->warnStream()
                        << "Cancel request does not have a "
                        << "\"sequencecode\" attribute. "
                        << "Fine for now, should not happen in the future"
                        << log4cpp::CategoryStream::ENDLINE
                        );
    } else {
        boost::trim_if(m_sequence_code, boost::is_any_of("\""));        
    }
}

void iceCommandCancel::execute( Ice* ice ) throw ( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
    CREAM_SAFE_LOG( 
                   m_log_dev->infoStream()
                   << "This request is a Cancel..."
                   << log4cpp::CategoryStream::ENDLINE
                   );

    boost::recursive_mutex::scoped_lock M( util::jobCache::mutex );

    // Lookup the job in the jobCache
    util::jobCache::iterator it = util::jobCache::getInstance()->lookupByGridJobID( m_gridJobId );
    if ( it == util::jobCache::getInstance()->end() ) {
        CREAM_SAFE_LOG( 
                       m_log_dev->errorStream()
                       << "Cancel operation cannot locate jobid=["
                       << m_gridJobId 
                       << "] in the jobCache. Giving up"
                       << log4cpp::CategoryStream::ENDLINE
                       );

        throw iceCommandFatal_ex( string("ICE cannot cancel job with grid job id=[") + m_gridJobId + string("], as the job does not appear to exist") );
    }

    // Set Sequence code, if any...
    // FIXME: Remove the if() statement. Sequence code MUST be present
    if ( ! m_sequence_code.empty() )  
        it->setSequenceCode( m_sequence_code );

    // Log cancel request event
    m_lb_logger->logEvent( new util::cream_cancel_request_event( *it, string("User issued cancel request") ) );    

    util::CreamJob theJob( *it );
    vector<string> url_jid(1);   
    url_jid[0] = theJob.getJobID();
    CREAM_SAFE_LOG(
                   m_log_dev->infoStream()
                   << "Removing job gridJobId [" 
                   << m_gridJobId
                   << "], creamJobId [" 
                   << url_jid[0] 
                   << "]"
                   << log4cpp::CategoryStream::ENDLINE
                   );

    CREAM_SAFE_LOG(    
                   m_log_dev->infoStream()
                   << "Sending cancellation request to ["
                   << theJob.getCreamURL() << "]"
                   << log4cpp::CategoryStream::ENDLINE
                   );

    cream_api::soap_proxy::CreamProxy* theProxy( cream_api::soap_proxy::CreamProxyFactory::getProxy() );
    
    try {
	theProxy->Authenticate( theJob.getUserProxyCertificate() );
	theProxy->Cancel( theJob.getCreamURL().c_str(), url_jid );
    } catch(cream_api::soap_proxy::auth_ex& ex) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("auth_ex: ") + ex.what() ) );
        throw iceCommandFatal_ex( string("auth_ex: ") + ex.what() );
    } catch(cream_api::soap_proxy::soap_ex& ex) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("soap_ex: ") + ex.what() ) );
        throw iceCommandTransient_ex( string("soap_ex: ") + ex.what() );
        // HERE MUST RESUBMIT
    } catch(cream_api::cream_exceptions::BaseException& base) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("BaseException: ") + base.what() ) );
        throw iceCommandFatal_ex( string("BaseException: ") + base.what() );
    } catch(cream_api::cream_exceptions::InternalException& intern) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("InternalException: ") + intern.what() ) );
        throw iceCommandFatal_ex( string("InternalException: ") + intern.what() );
    }
    m_lb_logger->logEvent( new util::cream_cancel_done_event( theJob, "User Cancelled" ) );

    // no failure: put jobids and status in cache
    // and remove last request from WM's filelist
    util::jobCache::getInstance()->erase( it );
}


} // namespace ice
} // namespace wms
} // namespace glite
