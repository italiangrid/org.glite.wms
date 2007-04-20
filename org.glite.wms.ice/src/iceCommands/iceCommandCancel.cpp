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
#include "CreamProxyMethod.h"
#include "Request_source_purger.h"
#include "Request.h"

#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/wms/common/utilities/scope_guard.h"

#include "boost/algorithm/string.hpp"

namespace cream_api = glite::ce::cream_client_api;
namespace wms_utils = glite::wms::common::utilities;
using namespace std;
using namespace glite::wms::ice;

iceCommandCancel::iceCommandCancel( glite::ce::cream_client_api::soap_proxy::CreamProxy* _theProxy, util::Request* request ) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) :
    iceAbsCommand( ),
    m_log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()),
    m_lb_logger( util::iceLBLogger::instance() ),
    m_request( request )
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
    classad::ClassAd *rootAD = parser.ParseClassAd( request->to_string() );

    if (!rootAD)
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");

    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( rootAD );

    string commandStr;
    // Parse the "command" attribute
    if ( !classad_safe_ptr->EvaluateAttrString( "command", commandStr ) ) {
        throw util::JobRequest_ex("attribute \"command\" not found or is not a string");
    }
    boost::trim_if(commandStr, boost::is_any_of("\""));

    if ( !boost::algorithm::iequals( commandStr, "cancel" ) ) {
        throw util::JobRequest_ex("wrong command ["+commandStr+"] parsed by iceCommandCancel" );
    }

    string protocolStr;
    // Parse the "version" attribute
    if ( !classad_safe_ptr->EvaluateAttrString( "protocol", protocolStr ) ) {
        throw util::JobRequest_ex("attribute \"protocol\" not found or is not a string");
    }
    // Check if the version is exactly 1.0.0
    if ( protocolStr.compare("1.0.0") ) {
        throw util::JobRequest_ex("Wrong \"Protocol\" for jobCancel: expected 1.0.0, got " + protocolStr );
    }

    classad::ClassAd *argumentsAD = 0; // no need to free this
    // Parse the "arguments" attribute
    if ( !classad_safe_ptr->EvaluateAttrClassAd( "arguments", argumentsAD ) ) {
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
    
    m_theProxy.reset( _theProxy );
}

void iceCommandCancel::execute( ) throw ( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
    CREAM_SAFE_LOG( 
                   m_log_dev->infoStream()
                   << "This request is a Cancel..."
                   << log4cpp::CategoryStream::ENDLINE
                   );

    Request_source_purger r( m_request );
    wms_utils::scope_guard remove_request_guard( r );
    
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

    // According to the following mail, the sequence from a cancel
    // request should NOT be replaced. Hence, this code has been
    // commented out and will be removed
    //
    // Date: Wed, 6 Dec 2006 15:18:05 +0100 From: Zdenek Salvet <salvet@ics.muni.cz> To: Local EGEE JRA1 group <egee-jra1@lindir.ics.muni.cz> Cc: Milos Mulac <mulac@civ.zcu.cz>, Alessio Gianelle <gianelle@pd.infn.it>, Massimo Sgaravatto - INFN Padova <massimo.sgaravatto@pd.infn.it> Subject: Re: [Egee-jra1] Problem with LB & ICE On Wed, Dec 06, 2006 at 03:01:54PM +0100, Moreno Marzolla wrote:

    // I don't think the problem is caused by this race, LB and its
    // event sequence codes has been designed with this in mind.  It
    // appears to me that ICE(LogMonitor) replaces its stored LB
    // sequence code for the running job with the one coming in cancel
    // request. Then, ReallyRunning appears to be logically following
    // cancellation. It should not do that.

    // if ( ! m_sequence_code.empty() )  
    // it->setSequenceCode( m_sequence_code );

    util::CreamJob theJob( *it );

    // Log cancel request event
    theJob = m_lb_logger->logEvent( new util::cream_cancel_request_event( theJob, string("Cancel request issued by user") ) );    

    vector<string> url_jid(1);   
    url_jid[0] = theJob.getCreamJobID();
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

    try {
	m_theProxy->Authenticate( theJob.getUserProxyCertificate() );
        theJob.set_failure_reason( "Aborted by user" );
        util::jobCache::getInstance()->put( theJob );
        util::CreamProxy_Cancel( theJob.getCreamURL(), url_jid ).execute( m_theProxy.get(), 3 );
	// theProxy->Cancel( theJob.getCreamURL().c_str(), url_jid );
    } catch(cream_api::soap_proxy::auth_ex& ex) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("auth_ex: ") + ex.what() ) );
        throw iceCommandFatal_ex( string("auth_ex: ") + ex.what() );
    } catch(cream_api::soap_proxy::soap_ex& ex) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("soap_ex: ") + ex.what() ) );
        throw iceCommandTransient_ex( string("soap_ex: ") + ex.what() );
    } catch(cream_api::cream_exceptions::BaseException& base) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("BaseException: ") + base.what() ) );
        throw iceCommandFatal_ex( string("BaseException: ") + base.what() );
    } catch(cream_api::cream_exceptions::InternalException& intern) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("InternalException: ") + intern.what() ) );
        throw iceCommandFatal_ex( string("InternalException: ") + intern.what() );
    }

}
