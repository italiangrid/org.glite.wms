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

/**
 *
 * ICE  Headers
 *
 */
#include "iceCommandCancel.h"
#include "jobCache.h"
#include "ice-core.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "CreamProxyMethod.h"
#include "Request_source_purger.h"
#include "Request.h"
#include "DNProxyManager.h"

/**
 *
 * Cream Client API Headers
 *
 */
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/ce/cream-client-api-c/ResultWrapper.h"
#include "glite/ce/cream-client-api-c/JobIdWrapper.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/wms/common/utilities/scope_guard.h"

#include <boost/algorithm/string.hpp>

namespace cream_api = glite::ce::cream_client_api::soap_proxy;
namespace cream_ex  = glite::ce::cream_client_api::cream_exceptions;
namespace wms_utils = glite::wms::common::utilities;
using namespace std;
using namespace glite::wms::ice;

//
//
//______________________________________________________________________________
iceCommandCancel::iceCommandCancel( /*glite::ce::cream_client_api::soap_proxy::CreamProxy* _theProxy, */util::Request* request ) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) :
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
    SequenceCode = "UI=000002:NS=0000000007:WM=000002:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000:LBS=000000"; 
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
                        << "iceCommandCancel::execute() - Cancel request does not have a "
                        << "\"sequencecode\" attribute. "
                        << "Fine for now, should not happen in the future"
                        << log4cpp::CategoryStream::ENDLINE
                        );
    } else {
        boost::trim_if(m_sequence_code, boost::is_any_of("\""));        
    }
    
    //m_theProxy.reset( _theProxy );
}

//
//
//______________________________________________________________________________
void iceCommandCancel::execute( ) throw ( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
    CREAM_SAFE_LOG( 
                   m_log_dev->infoStream()
                   << "iceCommandCancel::execute() - This request is a Cancel..."
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
                       << "iceCommandCancel::execute() - Cancel operation cannot locate jobid=["
                       << m_gridJobId 
                       << "] in the jobCache. Giving up"
                       << log4cpp::CategoryStream::ENDLINE
                       );

        throw iceCommandFatal_ex( string("ICE cannot cancel job with grid job id=[") 
				  + m_gridJobId 
				  + string("], as the job does not appear to exist") );
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

    //vector<string> url_jid(1);   
    //url_jid[0] = theJob.getCreamJobID();
    CREAM_SAFE_LOG(
                   m_log_dev->infoStream()
                   << "iceCommandCancel::execute() - Removing job gridJobId [" 
                   << m_gridJobId
                   << "], creamJobId [" 
                   << theJob.getCompleteCreamJobID() 
                   << "]"
                   << log4cpp::CategoryStream::ENDLINE
                   );

    CREAM_SAFE_LOG(    
                   m_log_dev->infoStream()
                   << "iceCommandCancel::execute() - Sending cancellation request to ["
                   << theJob.getCreamURL() << "]"
                   << log4cpp::CategoryStream::ENDLINE
                   );

    /**
     * Getting betterproxy for current job. Note that this betterproxy should be there
     * becase this procedure already checked that this job is in the cache and then
     * it has been already submitted (that implies that its proxy has been put in the 
     * DNProxyManager's cache of betterproxies).
     */
    string betterproxy;

    betterproxy = util::DNProxyManager::getInstance()->getBetterProxyByDN( theJob.getUserDN() );

    try {
      //m_theProxy->Authenticate( betterproxy /* theJob.getUserProxyCertificate() */ );

      cream_api::VOMSWrapper V( betterproxy );
      if( !V.IsValid( ) ) {
        throw cream_api::auth_ex( V.getErrorMessage() );
      }

        theJob.set_failure_reason( "Aborted by user" );
        util::jobCache::getInstance()->put( theJob );

	vector<cream_api::JobIdWrapper> toCancel;
	toCancel.push_back( cream_api::JobIdWrapper(theJob.getCreamJobID(), 
						    theJob.getCreamURL(), 
						    std::vector<cream_api::JobPropertyWrapper>())
			    );

	cream_api::JobFilterWrapper req( toCancel, vector<string>(), -1, -1, "", "");
	cream_api::ResultWrapper res;

        util::CreamProxy_Cancel( theJob.getCreamURL(), betterproxy, &req, &res ).execute( 3 );

	// theProxy->Cancel( theJob.getCreamURL().c_str(), url_jid );
 	
	list< pair<cream_api::JobIdWrapper, string> > tmp;
	res.getOkJobs( tmp );
	if(!tmp.empty()) { // the unique job we cancelled is 
			   // in the OkJobs array. Then it has been
			   // cancelled.
	  return;
	}
	res.getNotExistingJobs( tmp );
	res.getNotMatchingStatusJobs( tmp );
	res.getNotMatchingDateJobs( tmp );
	res.getNotMatchingProxyDelegationIdJobs( tmp );
	res.getNotMatchingLeaseIdJobs( tmp );
	
	// We tried to cancel only one job.
	// Then if the operation went wrong
	// tmp contains only one element, the first one!
	if( tmp.begin() == tmp.end() )
	{
	  // Should not be empty. Something went wrong in the server
	  string errMex = "iceCommandCancel::execute() - The job to cancel [";
	  errMex += theJob.describe() + "] is not in the OK list neither in the ";
	  errMex += " failed list. Something went wrong in the server or in the ";
	  errMex += "SOAP communication.";
	   CREAM_SAFE_LOG(    
                   	   m_log_dev->errorStream()
                   	   << errMex
			   << log4cpp::CategoryStream::ENDLINE
	                  );
	  m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("Error: ") + errMex ) );
	  throw iceCommandFatal_ex( errMex );
	}
	
	pair<cream_api::JobIdWrapper, string> errorJob = *(tmp.begin());
	
	string errMex = "iceCommandCancel::execute() - Cancellation of the [";
	errMex += theJob.describe() + "] went wrong: ";
	errMex += errorJob.second + "]";
	
	CREAM_SAFE_LOG(    
                   m_log_dev->errorStream()
                   << errMex
                   << log4cpp::CategoryStream::ENDLINE
                   );
		   
	m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("Error: ") + errMex ) );
	throw iceCommandFatal_ex( errMex );
	
    } catch(cream_api::auth_ex& ex) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("Authentication Exception: ") + ex.what() ) );
        throw iceCommandFatal_ex( string("auth_ex: ") + ex.what() );
    } catch(cream_api::soap_ex& ex) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("SOAP Exception: ") + ex.what() ) );
        throw iceCommandTransient_ex( string("soap_ex: ") + ex.what() );
    } catch(cream_ex::BaseException& base) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("BaseException: ") + base.what() ) );
        throw iceCommandFatal_ex( string("BaseException: ") + base.what() );
    } catch(cream_ex::InternalException& intern) {
        m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("InternalException: ") + intern.what() ) );
        throw iceCommandFatal_ex( string("InternalException: ") + intern.what() );
    }

}
