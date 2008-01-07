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
 * Council for the Central Laboratory of the Research Councils (CCLRfC), United Kingdom
 *
 * ICE command submit class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// Define the following macro if you are testing ICE without the WMS
// If the macro is defined, then an additional L&B call is performed
// to register the job to the L&B server. This call is NOT done if ICE
// is used inside the WMS, as the UI takes care of L&B job
// registration.

// Local includes
#include "iceCommandSubmit.h"
#include "subscriptionManager.h"
#include "subscriptionProxy.h"
#include "DNProxyManager.h"
#include "Delegation_manager.h"
#include "iceConfManager.h"
#include "iceSubscription.h"
#include "jobCache.h"
#include "creamJob.h"
#include "ice-core.h"
#include "eventStatusListener.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "CreamProxyMethod.h"
#include "Request.h"
#include "Request_source_purger.h"
#include "iceUtils.h"

/**
 *
 * Cream Client API Headers
 *
 */
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/JobIdWrapper.h"
#include "glite/ce/cream-client-api-c/AbsCreamProxy.h"
#include "glite/ce/cream-client-api-c/ResultWrapper.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/ce/cream-client-api-c/JobDescriptionWrapper.h"

#include "glite/wms/common/utilities/scope_guard.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

// Boost stuff
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"
#include "boost/format.hpp"

// C++ stuff
#include <ctime>
#include <cstring> // for memset

using namespace std;
namespace ceurl_util = glite::ce::cream_client_api::util::CEUrl;
namespace cream_api = glite::ce::cream_client_api::soap_proxy;
namespace api_util = glite::ce::cream_client_api::util;
namespace wms_utils = glite::wms::common::utilities;
namespace iceUtil = glite::wms::ice::util;
using namespace glite::wms::ice;

boost::recursive_mutex iceCommandSubmit::s_localMutexForSubscriptions;

namespace { // Anonymous namespace
    
    // 
    // This class is used by a scope_guard to delete a job from the
    // job cache if something goes wrong during the submission.
    //
    class remove_job_from_cache {
    protected:
        const std::string m_grid_job_id;
        iceUtil::jobCache* m_cache;
        
    public:
        /**
         * Construct a remove_job_from_cache object which will remove
         * the job with given grid_job_id from the cache.
         */
        remove_job_from_cache( const std::string& grid_job_id ) :
            m_grid_job_id( grid_job_id ),
            m_cache( iceUtil::jobCache::getInstance() )
        { };
        /**
         * Actually removes the job from cache. If the job is no
         * longer in the job cache, nothing is done.
         */
        void operator()( void ) {
            boost::recursive_mutex::scoped_lock M( iceUtil::jobCache::mutex );
            iceUtil::jobCache::iterator it( m_cache->lookupByGridJobID( m_grid_job_id ) );
            m_cache->erase( it );
        }
    };       
    
}; // end anonymous namespace

//
//
//____________________________________________________________________________
iceCommandSubmit::iceCommandSubmit( iceUtil::Request* request )
  throw( iceUtil::ClassadSyntax_ex&, iceUtil::JobRequest_ex& ) :
    iceAbsCommand( ),
    m_theIce( Ice::instance() ),
    m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
    m_configuration( iceUtil::iceConfManager::getInstance()->getConfiguration() ),
    m_lb_logger( iceUtil::iceLBLogger::instance() ),
    m_request( request )
{
    try {
        m_myname = iceUtil::getHostName();
	if( m_configuration->ice()->listener_enable_authn() ) {
            m_myname_url = boost::str( boost::format("https://%1%:%2%") % m_myname % m_configuration->ice()->listener_port() );
	} else {
            m_myname_url = boost::str( boost::format("http://%1%:%2%") % m_myname % m_configuration->ice()->listener_port() );   
	}
    } catch( runtime_error& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->fatalStream() 
                       << "iceCommandSubmit::CTOR() - "
                       << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
	abort();
    }
    
    /*

[
  stream_error = false;
  edg_jobid = "https://cert-rb-03.cnaf.infn.it:9000/YeyOVNkR84l6QMHl_PY6mQ";
  GlobusScheduler = "gridit-ce-001.cnaf.infn.it:2119/jobmanager-lcgpbs";
  ce_id = "gridit-ce-001.cnaf.infn.it:2119/jobmanager-lcgpbs-cert";
  Transfer_Executable = true;
  Output = "/var/glite/jobcontrol/condorio/Ye/https_3a_2f_2fcert-rb-03.cnaf.infn.it_3a9000_2fYeyOVNkR84l6QMHl_5fPY6mQ/StandardOutput";
  Copy_to_Spool = false;
  Executable = "/var/glite/jobcontrol/submit/Ye/JobWrapper.https_3a_2f_2fcert-rb-03.cnaf.infn.it_3a9000_2fYeyOVNkR84l6QMHl_5fPY6mQ.sh";
  X509UserProxy = "/var/glite/SandboxDir/Ye/https_3a_2f_2fcert-rb-03.cnaf.infn.it_3a9000_2fYeyOVNkR84l6QMHl_5fPY6mQ/user.proxy"; 
  Error_ = "/var/glite/jobcontrol/condorio/Ye/https_3a_2f_2fcert-rb-03.cnaf.infn.it_3a9000_2fYeyOVNkR84l6QMHl_5fPY6mQ/StandardError";
  LB_sequence_code = "UI=000002:NS=0000000003:WM=000004:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000:LBS=000000"; 
  Notification = "never"; 
  stream_output = false; 
  GlobusRSL = "(queue=cert)(jobtype=single)"; 
  Type = "job"; 
  Universe = "grid"; 
  UserSubjectName = "/C=IT/O=INFN/OU=Personal Certificate/L=CNAF/CN=Marco Cecchi"; 
  Log = "/var/glite/logmonitor/CondorG.log/CondorG.log"; 
  grid_type = "globus" 
]

*/
            
                
    classad::ClassAdParser parser;
    classad::ClassAd *rootAD = parser.ParseClassAd( request->to_string() );

    if (!rootAD) {
        throw iceUtil::ClassadSyntax_ex( boost::str( boost::format( "iceCommandSubmit: ClassAd parser returned a NULL pointer parsing request: %1%" ) % request->to_string() ) );        
    }
    
    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( rootAD );

    string commandStr;
    // Parse the "command" attribute
    if ( !classad_safe_ptr->EvaluateAttrString( "command", commandStr ) ) {
        throw iceUtil::JobRequest_ex( boost::str( boost::format( "iceCommandSubmit: attribute 'command' not found or is not a string in request: %1%") % request->to_string() ) );
    }
    boost::trim_if( commandStr, boost::is_any_of("\"") );

    if ( !boost::algorithm::iequals( commandStr, "submit" ) ) {
        throw iceUtil::JobRequest_ex( boost::str( boost::format( "iceCommandSubmit:: wrong command parsed: %1%" ) % commandStr ) );
    }

    string protocolStr;
    // Parse the "version" attribute
    if ( !classad_safe_ptr->EvaluateAttrString( "Protocol", protocolStr ) ) {
        throw iceUtil::JobRequest_ex("attribute \"Protocol\" not found or is not a string");
    }
    // Check if the version is exactly 1.0.0
    if ( protocolStr.compare("1.0.0") ) {
        throw iceUtil::JobRequest_ex("Wrong \"Protocol\" for jobRequest: expected 1.0.0, got " + protocolStr );
    }

    classad::ClassAd *argumentsAD = 0; // no need to free this
    // Parse the "arguments" attribute
    if ( !classad_safe_ptr->EvaluateAttrClassAd( "arguments", argumentsAD ) ) {
        throw iceUtil::JobRequest_ex("attribute 'arguments' not found or is not a classad");
    }

    classad::ClassAd *adAD = 0; // no need to free this
    // Look for "ad" attribute inside "arguments"
    if ( !argumentsAD->EvaluateAttrClassAd( "jobad", adAD ) ) {
        throw iceUtil::JobRequest_ex("Attribute \"JobAd\" not found inside 'arguments', or is not a classad" );
    }

    // initializes the m_jdl attribute
    classad::ClassAdUnParser unparser;
    unparser.Unparse( m_jdl, argumentsAD->Lookup( "jobad" ) );

    try {
        m_theJob.setJdl( m_jdl );
        m_theJob.setStatus( glite::ce::cream_client_api::job_statuses::UNKNOWN );
    } catch( iceUtil::ClassadSyntax_ex& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream() 
                       << "iceCommandSubmit::CTOR() - Cannot instantiate a job from jdl=" << m_jdl
                       << " due to classad excaption: " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
        throw( iceUtil::ClassadSyntax_ex( ex.what() ) );
    }
    
    //m_theProxy.reset( _theProxy );
}

//
//
//____________________________________________________________________________
void iceCommandSubmit::execute( void ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
    static const char* method_name="iceCommandSubmit::execute() - ";

    // api_util::scoped_timer tmp_timer( "iceCommandSubmit::execute" );
    
    CREAM_SAFE_LOG(
                   m_log_dev->infoStream()
                   << "iceCommandSubmit::execute() - "
                   << "This request is a Submission..."
                   << log4cpp::CategoryStream::ENDLINE
                   );   
    
    iceUtil::jobCache* cache( iceUtil::jobCache::getInstance() );
    
    //vector<string> url_jid;
    Request_source_purger purger_f( m_request );
    wms_utils::scope_guard remove_request_guard( purger_f );

    // We start by checking whether the job already exists in ICE
    // cache.  This may happen if ICE already tried to submit the job,
    // but crashed before removing the request from the filelist. If
    // we find the job already in ICE cache, we simply give up
    // (logging an information message), and the purge_f object will
    // take care of actual removal.
    {
      boost::recursive_mutex::scoped_lock M( iceUtil::jobCache::mutex );
        //iceUtil::jobCache::const_iterator it( cache->lookupByGridJobID( m_theJob.getGridJobID() ) );
	iceUtil::jobCache::iterator it( cache->lookupByGridJobID( m_theJob.getGridJobID() ) );
        if ( cache->end() != it ) {
            CREAM_SAFE_LOG( m_log_dev->warnStream()
                            << "iceCommandSubmit::execute() - "
                            << "Submit request for job "
                            << m_theJob.describe()
                            << " is related to a job already in ICE cache. "
                            << "Removing the request and going ahead."
                            << log4cpp::CategoryStream::ENDLINE
                            );   
            return;
        }
    } // unlocks the cache
        

    /**
     *
     * Retrieve all usefull cert info
     *
     */
    cream_api::VOMSWrapper V( m_theJob.getUserProxyCertificate() );
    if( !V.IsValid( ) ) {
      //      throw cream_api::auth_ex( V.getErrorMessage() );
      CREAM_SAFE_LOG(
		     m_log_dev->errorStream()
		     << "iceCommandSubmit::execute() - "
		     << "Unable to submit gridJobID=" 
		     << m_theJob.getGridJobID()
		     << " due to authentication error:" << V.getErrorMessage()
		     << log4cpp::CategoryStream::ENDLINE
		     );
      m_theJob.set_failure_reason( V.getErrorMessage() );
      m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, V.getErrorMessage() ) );
      m_theJob.set_failure_reason( boost::str( boost::format( "Submission to CREAM failed due to exception: %1%" ) % V.getErrorMessage() ) );
      m_theJob = m_lb_logger->logEvent( new iceUtil::job_done_failed_event( m_theJob ) ); // Ref: Ale, 25 jan 2007
      m_theIce->resubmit_job( m_theJob, boost::str( boost::format( "Resubmitting because of authentication exception %1%" ) % V.getErrorMessage() ) );
      throw( iceCommandFatal_ex( V.getErrorMessage() ) );
    }

    /**
     * In order to make the userDN an index in the BDb's secondary database
     * it must be available already at the first put.
     * So the following block of code must remain here.
     */
//     try {
//       m_theJob.setUserDN( glite::ce::cream_client_api::certUtil::getDNFQAN(m_theJob.getUserProxyCertificate()) );
//     } catch(exception& ex) {
//       CREAM_SAFE_LOG(
//                    m_log_dev->errorStream()
//                    << "iceCommandSubmit::execute() - Cannot set the user DN for current job: "
// 		   << ex.what() << ""
//                    << log4cpp::CategoryStream::ENDLINE
//                    );
//       m_theJob.set_failure_reason( ex.what() );
//       m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, ex.what()  ) );
//       // The next event is used to show the failure reason in the status info
//       // JC+LM log transfer-fail / aborted in case of condor transfers fail
//       m_theJob.set_failure_reason( boost::str( boost::format( "Transfer to CREAM failed due to exception: %1%" ) % ex.what() ) );
//       m_theJob = m_lb_logger->logEvent( new iceUtil::job_done_failed_event( m_theJob ) );
//       m_theIce->resubmit_job( m_theJob, boost::str( boost::format( "Resubmitting because of exception %1%" ) % ex.what() ) ); // Try to resubmit
//       throw( iceCommandFatal_ex( ex.what() ) );
//     }

    m_theJob.setUserDN( V.getDNFQAN() );

    // This must be left AFTER the above code. The remove_job_guard
    // object will REMOVE the job from the cache when being destroied.
    remove_job_from_cache remove_f( m_theJob.getGridJobID() );
    wms_utils::scope_guard remove_job_guard( remove_f );
    
    m_theJob = m_lb_logger->logEvent( new iceUtil::wms_dequeued_event( m_theJob, m_configuration->ice()->input() ) );
    m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_start_event( m_theJob ) );
    
    string modified_jdl;
    try {    
        // It is important to get the jdl from the job itself, rather
        // than using the m_jdl attribute. This is because the
        // sequence_code attribute inside the jdl classad has been
        // modified by the L&B calls, and we have to pass to CREAM the
        // "last" sequence code as the job wrapper will need to log
        // the "really running" event.
        modified_jdl = creamJdlHelper( m_theJob.getJDL() );
    } catch( iceUtil::ClassadSyntax_ex& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream() 
                       << "iceCommandSubmit::execute() - Cannot convert jdl=" << m_jdl
                       << " due to classad exception:" << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
        m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, boost::str( boost::format( "iceCommandSubmit cannot convert jdl=%1% due to classad exception=%2%") % m_jdl % ex.what() ) ) );
        m_theJob.set_failure_reason( boost::str( boost::format( "iceCommandSubmit cannot convert jdl=%1% due to classad exception=%2%") % m_jdl % ex.what() ) );
        m_theJob = m_lb_logger->logEvent( new iceUtil::job_aborted_event( m_theJob ) );
        throw( iceCommandFatal_ex( ex.what() ) );
    }
    
    CREAM_SAFE_LOG( m_log_dev->log(log4cpp::Priority::INFO, "iceCommandSubmit::execute() - Submitting") );
    CREAM_SAFE_LOG(
                   m_log_dev->debugStream() 
                   << "iceCommandSubmit::execute() - JDL " << modified_jdl << " to [" 
                   << m_theJob.getCreamURL() <<"]["
                   << m_theJob.getCreamDelegURL() << "]"
                   << log4cpp::CategoryStream::ENDLINE
                   );

    //
    // Authenticate    
    //
//     try {
//         // api_util::scoped_timer autenticate_timer( "iceCommandSubmit::Authenticate" );
//         //m_theProxy->Authenticate(m_theJob.getUserProxyCertificate());

//       cream_api::VOMSWrapper V( m_theJob.getUserProxyCertificate() );
//       if( !V.IsValid( ) ) {
// 	throw cream_api::auth_ex( V.getErrorMessage() );
//       }

//     } catch ( cream_api::auth_ex& ex ) {
//         CREAM_SAFE_LOG(
//                        m_log_dev->errorStream()
//                        << "iceCommandSubmit::execute() - "
//                        << "Unable to submit gridJobID=" 
//                        << m_theJob.getGridJobID()
//                        << " due to authentication error:" << ex.what()
//                        << log4cpp::CategoryStream::ENDLINE
//                        );
//         m_theJob.set_failure_reason( ex.what() );
//         m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, ex.what() ) );
//         m_theJob.set_failure_reason( boost::str( boost::format( "Submission to CREAM failed due to exception: %1%" ) % ex.what() ) );
//         m_theJob = m_lb_logger->logEvent( new iceUtil::job_done_failed_event( m_theJob ) ); // Ref: Ale, 25 jan 2007
//         m_theIce->resubmit_job( m_theJob, boost::str( boost::format( "Resubmitting because of SOAP exception %1%" ) % ex.what() ) );
//         throw( iceCommandFatal_ex( ex.what() ) );
//     }
    
    string delegID; // empty delegation id

    int newLease = m_configuration->ice()->lease_delta_time();

    CREAM_SAFE_LOG(
                   m_log_dev->debugStream()
                   << "iceCommandSubmit::execute() - "
                   << "Sequence code for job "
                   << m_theJob.describe()
                   << " is "
                   << m_theJob.getSequenceCode()
                   << log4cpp::CategoryStream::ENDLINE                   
                   );

    bool force_delegation = false;

    cream_api::AbsCreamProxy::RegisterArrayResult res;

    while( 1 ) {
        
        // 
        // Delegates the proxy
        //
       delegID = iceUtil::Delegation_manager::instance()->delegate( m_theJob, force_delegation );
        
        //
        // Registers the job (with autostart)
        //
        try {	    
            // api_util::scoped_timer register_timer( "iceCommandSubmit::Register" );
            CREAM_SAFE_LOG(
                           m_log_dev->debugStream()
                           << "iceCommandSubmit::execute() - "
                           << "Going to REGISTER Job ["
                           << m_theJob.getGridJobID() 
                           << "]..."
                           << log4cpp::CategoryStream::ENDLINE
                           );
//             iceUtil::CreamProxy_Register 
//                 pr( m_theJob.getCreamURL(), m_theJob.getCreamDelegURL(), delegID,
//                     modified_jdl, m_theJob.getGridJobID(),
//                     m_theJob.getUserProxyCertificate(), url_jid, newLease, false );

	    cream_api::AbsCreamProxy::RegisterArrayRequest req;
	    
	    // FIXME: must check what to set the 3rd and 4th arguments (delegationProxy, leaseID)
	    // FIXME: TODO -> LeaseManager
	    // last asrgument is irrelevant now, because we register jobs one by one
	    cream_api::JobDescriptionWrapper jd(m_theJob.getJDL(), 
				     delegID, 
				     ""/* delegPRoxy */, 
				     ""/* leaseID */, 
				     false, /* NO autostart */
				     "foo");

	    req.push_back( &jd );
	    iceUtil::CreamProxy_Register( m_theJob.getCreamURL(),
					  m_theJob.getUserProxyCertificate(),
					  (const cream_api::AbsCreamProxy::RegisterArrayRequest*)&req,
					  &res).execute( 3 );
            
            //pr.execute(m_theProxy.get(), 3 );

	    // FIXME: put code for lease time negotiation

            //newLease = pr.retrieveNewLeaseTime();

            break; // exit the while(1) loop

        } catch ( glite::ce::cream_client_api::cream_exceptions::DelegationException& ex ) {
            CREAM_SAFE_LOG(
                           m_log_dev->warnStream()
                           << method_name
                           << "Cannot register GridJobID ["
                           << m_theJob.getGridJobID() 
                           << "] due to Delegation Exception:" << ex.what()
                           << log4cpp::CategoryStream::ENDLINE
                           );
            force_delegation = true;
        } catch( exception& ex ) {
            CREAM_SAFE_LOG(
                           m_log_dev->errorStream()
                           << method_name
                           << "Cannot register GridJobID ["
                           << m_theJob.getGridJobID() 
                           << "] Exception:" << ex.what()
                           << log4cpp::CategoryStream::ENDLINE
                           );
            m_theJob.set_failure_reason( ex.what() );
            m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, ex.what()  ) );
            // The next event is used to show the failure reason in the status info
            // JC+LM log transfer-fail / aborted in case of condor transfers fail
            m_theJob.set_failure_reason( boost::str( boost::format( "Transfer to CREAM failed due to exception: %1%" ) % ex.what() ) );
            m_theJob = m_lb_logger->logEvent( new iceUtil::job_done_failed_event( m_theJob ) );
            m_theIce->resubmit_job( m_theJob, boost::str( boost::format( "Resubmitting because of exception %1%" ) % ex.what() ) ); // Try to resubmit
            throw( iceCommandFatal_ex( ex.what() ) );
        }
    } // end while(1)
    
    bool ok = res.begin()->second.get<0>();
    
    if( !ok )
      {
	string err = res.begin()->second.get<2>();
	CREAM_SAFE_LOG(
		       m_log_dev->errorStream()
		       << method_name
		       << "Cannot register GridJobID ["
		       << m_theJob.getGridJobID() 
		       << "] Error:" << err
		       << log4cpp::CategoryStream::ENDLINE
		       );
	m_theJob.set_failure_reason( err );
	m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, err  ) );
	// The next event is used to show the failure reason in the status info
	// JC+LM log transfer-fail / aborted in case of condor transfers fail
	m_theJob.set_failure_reason( boost::str( boost::format( "Transfer to CREAM failed due to service fault: %1%" ) % err ) );
	m_theJob = m_lb_logger->logEvent( new iceUtil::job_done_failed_event( m_theJob ) );
	m_theIce->resubmit_job( m_theJob, boost::str( boost::format( "Resubmitting because of CREAM fault %1%" ) % err ) ); // Try to resubmit
	throw( iceCommandFatal_ex( err ) );
      }
    
    string jobId      = res.begin()->second.get<1>().getCreamJobID();
    string __creamURL = res.begin()->second.get<1>().getCreamURL();

    // FIXME: should we check that __creamURL == m_theJob.getCreamURL() ?!?
    // If it is not it's VERY severe server error, and I think it is not our businness

    CREAM_SAFE_LOG(
                   m_log_dev->infoStream()
                   << method_name
		   << "For GridJobID [" << m_theJob.getGridJobID() << "]" 
		   << " CREAM Returned CREAM-JOBID ["
                   << jobId <<"]"
                   << log4cpp::CategoryStream::ENDLINE
                   );

    cream_api::ResultWrapper startRes;

    try {
      CREAM_SAFE_LOG(
		     m_log_dev->debugStream()
		     << method_name
		     << "Going to START CreamJobID ["
		     << jobId <<"] related to GridJobID ["
		     << m_theJob.getGridJobID() 
		     << "]..."
		     << log4cpp::CategoryStream::ENDLINE
		     );
      
      // FIXME: must create the request JobFilterWrapper for the Start operation
      vector<cream_api::JobIdWrapper> toStart;
      toStart.push_back( cream_api::JobIdWrapper( (const string&)jobId, (const string&)__creamURL, vector<cream_api::JobPropertyWrapper>()));

      cream_api::JobFilterWrapper jw(toStart, vector<string>(), -1, -1, "", "");

      //      iceUtil::CreamProxy_Start pr( m_theJob.getCreamURL(), url_jid[1]);
      iceUtil::CreamProxy_Start( __creamURL, 
				 m_theJob.getUserProxyCertificate(), 
				 (const cream_api::JobFilterWrapper *)&jw, 
				 &startRes ).execute( 7 );

	//      pr.execute(m_theProxy.get(), 7 );
    } catch( exception& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream()
                       << method_name
                       << "Cannot start CreamJobID ["
		       << jobId << "] GridJobID ["
                       << m_theJob.getGridJobID() << "]"
                       << " Exception:" << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
        m_theJob.set_failure_reason( ex.what() );
        m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, ex.what()  ) );
        // The next event is used to show the failure reason in the status info
        // JC+LM log transfer-fail / aborted in case of condor transfers fail
        m_theJob.set_failure_reason( boost::str( boost::format( "Transfer to CREAM failed due to exception: %1%" ) % ex.what() ) );
        m_theJob = m_lb_logger->logEvent( new iceUtil::job_done_failed_event( m_theJob ) );
        m_theIce->resubmit_job( m_theJob, boost::str( boost::format( "Resubmitting because of exception %1%" ) % ex.what() ) ); // Try to resubmit
        throw( iceCommandFatal_ex( ex.what() ) );
    }

    // FIXME: Very orrible. 
    // Must look for a better and more elegant way for doing the following
    // the following code accumulate all the results in tmp list
    list<pair<cream_api::JobIdWrapper, string> > tmp;
    startRes.getNotExistingJobs( tmp );
    if(tmp.empty())
      startRes.getNotMatchingStatusJobs( tmp );
    if(tmp.empty())
      startRes.getNotMatchingDateJobs( tmp );
    if(tmp.empty())
      startRes.getNotMatchingProxyDelegationIdJobs( tmp );
    if(tmp.empty())
      startRes.getNotMatchingLeaseIdJobs( tmp );
    
    // It is sufficient look for "empty-ness" because
    // we've started only one job
    if(!tmp.empty()) {
      pair<cream_api::JobIdWrapper, string> wrong = *( tmp.begin() ); // we trust there's only one element because we've started ONLY ONE job
      string errMex = wrong.second;
      
      CREAM_SAFE_LOG(m_log_dev->errorStream()
		     << "iceCommandSubmit::execute() - "
		     << "Cannot start job " 
		     << m_theJob.describe()
		     << ". Reason is: " << errMex
		     << log4cpp::CategoryStream::ENDLINE);

      m_theJob.set_failure_reason( errMex );
      m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, errMex  ) );
      // The next event is used to show the failure reason in the status info
      // JC+LM log transfer-fail / aborted in case of condor transfers fail
      m_theJob.set_failure_reason( boost::str( boost::format( "Transfer to CREAM failed due to service fault: %1%" ) % errMex ) );
      m_theJob = m_lb_logger->logEvent( new iceUtil::job_done_failed_event( m_theJob ) );
      m_theIce->resubmit_job( m_theJob, boost::str( boost::format( "Resubmitting because of exception %1%" ) % errMex ) ); // Try to resubmit
      throw( iceCommandFatal_ex( errMex ) );
    }

    

    // no failure: put jobids and status in cache
    // and remove last request from WM's filelist
    
    m_theJob.setCreamJobID( jobId );
    m_theJob.setStatus(glite::ce::cream_client_api::job_statuses::PENDING);


    // FIXME: Must handle lease negotiation
    m_theJob.setEndLease( time(0) + newLease );
    
    m_theJob.setDelegationId( delegID );
    m_theJob.setProxyCertMTime( time(0) ); // FIXME: should be the modification time of the proxy file?
    m_theJob.set_wn_sequence_code( m_theJob.getSequenceCode() );
    

    m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_ok_event( m_theJob ) );
    
    // now the job is in cache and has been registered
    // we can save its proxy into the DN-Proxy Manager's cache
    //    {
    // boost::recursive_mutex::scoped_lock M( iceUtil::DNProxyManager::mutex );
    iceUtil::DNProxyManager::getInstance()->setUserProxyIfLonger( m_theJob.getUserDN(), m_theJob.getUserProxyCertificate() );
    //}

    /*
     * here must check if we're subscribed to the CEMon service
     * in order to receive the status change notifications
     * of job just submitted. But only if listener is ON
     */
    if( m_theIce->is_listener_started() ) {
	
        //this->doSubscription( m_theJob.getCreamURL() );
      this->doSubscription( m_theJob );
	
    }

    remove_job_guard.dismiss(); // dismiss guard, job will NOT be removed from cache
    
    {
      boost::recursive_mutex::scoped_lock M( iceUtil::jobCache::mutex );
      m_theJob.setLastSeen( time(0) );
      cache->put( m_theJob );
    }
} // execute


//____________________________________________________________________________
string iceCommandSubmit::creamJdlHelper( const string& oldJdl ) throw( iceUtil::ClassadSyntax_ex& )
{
    classad::ClassAdParser parser;
    classad::ClassAd *root = parser.ParseClassAd( oldJdl );

    if ( !root ) {
        throw iceUtil::ClassadSyntax_ex( boost::str( boost::format( "ClassAd parser returned a NULL pointer parsing request=[%1%]") % oldJdl ) );
    }
    
    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( root );
    
    string ceid;
    if ( !classad_safe_ptr->EvaluateAttrString( "ce_id", ceid ) ) {
        throw iceUtil::ClassadSyntax_ex( "ce_id attribute not found" );
    }
    boost::trim_if( ceid, boost::is_any_of("\"") );

    vector<string> ceid_pieces;
    ceurl_util::parseCEID( ceid, ceid_pieces );
    string bsname = ceid_pieces[2];
    string qname = ceid_pieces[3];

    // Update jdl to insert two new attributes needed by cream:
    // QueueName and BatchSystem.
    classad_safe_ptr->InsertAttr( "QueueName", qname );
    classad_safe_ptr->InsertAttr( "BatchSystem", bsname );

    updateIsbList( classad_safe_ptr.get() );
    updateOsbList( classad_safe_ptr.get() );

    string newjdl;
    classad::ClassAdUnParser unparser;
    unparser.Unparse( newjdl, classad_safe_ptr.get() ); // this is safe: Unparse doesn't deallocate its second argument
    return newjdl;
}

//______________________________________________________________________________
void iceCommandSubmit::updateIsbList( classad::ClassAd* jdl )
{
    string default_isbURI = "gsiftp://";
    default_isbURI.append( m_myname );
    default_isbURI.push_back( '/' );
    string isbPath;
    if ( jdl->EvaluateAttrString( "InputSandboxPath", isbPath ) ) {
        default_isbURI.append( isbPath );
    } else {
        CREAM_SAFE_LOG(
                       m_log_dev->warnStream()
                       << "iceCommandSubmit::updateIsbList() found no "
                       << "\"InputSandboxPath\" attribute in the JDL. "
                       << "Hope this is correct..."
                       << log4cpp::CategoryStream::ENDLINE
                       );     
    }

    // If the InputSandboxBaseURI attribute is defined, remove it
    // after saving its value; the resulting jdl will NEVER have
    // the InputSandboxBaseURI attribute defined.
    string isbURI;
    if ( jdl->EvaluateAttrString( "InputSandboxBaseURI", isbURI ) ) {
        boost::trim_if( isbURI, boost::is_any_of("\"") );
        boost::trim_right_if( isbURI, boost::is_any_of("/") );
        // remove the attribute
        jdl->Delete( "InputSandboxBaseURI" );
    } else {
        isbURI = default_isbURI;
    }

    pathName isbURIobj( isbURI );

    // OK, not check each item in the InputSandbox and modify it if
    // necessary
    classad::ExprList* isbList;
    if ( jdl->EvaluateAttrList( "InputSandbox", isbList ) ) {

      /**
       * this pointer is used below as argument of ClassAd::Insert. The classad doc
       * says that that ptr MUST NOT be deallocated by the caller: 
       * http://www.cs.wisc.edu/condor/classad/c++tut.html#insert
       */
        classad::ExprList* newIsbList = new classad::ExprList();

	/*CREAM_SAFE_LOG(m_log_dev->debugStream()
            << "iceCommandSubmit::updateIsbList() - "
            << "Starting InputSandbox manipulation..."
            << log4cpp::CategoryStream::ENDLINE);
	*/
        string newPath;
        for ( classad::ExprList::iterator it=isbList->begin(); it != isbList->end(); it++ ) {
            
            classad::Value v;
            string s;
            if ( (*it)->Evaluate( v ) && v.IsStringValue( s ) ) {
                pathName isbEntryObj( s );
                pathName::pathType_t pType( isbEntryObj.getPathType() );

                switch( pType ) {
                case pathName::absolute:
                    newPath = default_isbURI + '/' + isbEntryObj.getFileName();
                    break;
                case pathName::relative:
                    newPath = isbURI + '/' + isbEntryObj.getFileName();
                    break;
                case pathName::invalid: // should abort??
                case pathName::uri:
                    newPath = s;
                    break;
                }                
            }
	    CREAM_SAFE_LOG(m_log_dev->debugStream()
                << "iceCommandSubmit::updateIsbList() - "
                << s << " became " << newPath
                << log4cpp::CategoryStream::ENDLINE);

            // Builds a new value
            classad::Value newV;
            newV.SetStringValue( newPath );
            // Builds the new string
            newIsbList->push_back( classad::Literal::MakeLiteral( newV ) );
        } 
	/**
	 * The pointer newIsbList pointer is used as argument of ClassAd::Insert. The classad doc
	 * says that that ptr MUST NOT be deallocated by the caller: 
	 * http://www.cs.wisc.edu/condor/classad/c++tut.html#insert
	 */
        jdl->Insert( "InputSandbox", newIsbList );
    }
}

//______________________________________________________________________________
void iceCommandSubmit::updateOsbList( classad::ClassAd* jdl )
{
    // If no OutputSandbox attribute is defined, then nothing has to be done
    if ( 0 == jdl->Lookup( "OutputSandbox" ) )
        return;

    string default_osbdURI = "gsiftp://";
    default_osbdURI.append( m_myname );
    default_osbdURI.push_back( '/' );
    string osbPath;
    if ( jdl->EvaluateAttrString( "OutputSandboxPath", osbPath ) ) {
        default_osbdURI.append( osbPath );
    } else {
        CREAM_SAFE_LOG(m_log_dev->warnStream()
            << "iceCommandSubmit::updateOsbList() - found no "
            << "\"OutputSandboxPath\" attribute in the JDL. "
            << "Hope this is correct..."
            << log4cpp::CategoryStream::ENDLINE);        
    }

    if ( 0 != jdl->Lookup( "OutputSandboxDestURI" ) ) {

        // Remove the OutputSandboxBaseDestURI from the classad
        // OutputSandboxDestURI and OutputSandboxBaseDestURI cannot
        // be given at the same time.
        jdl->Delete( "OutputSandboxBaseDestURI" );

        // Check if all the entries in the OutputSandboxDestURI
        // are absolute URIs

        classad::ExprList* osbDUList;
	/**
	 * this pointer is used below as argument of ClassAd::Insert. The classad doc
	 * says that that ptr MUST NOT be deallocated by the caller: 
	 * http://www.cs.wisc.edu/condor/classad/c++tut.html#insert
	 */
        classad::ExprList* newOsbDUList = new classad::ExprList();
	
        if ( jdl->EvaluateAttrList( "OutputSandboxDestURI", osbDUList ) ) {
/*
            CREAM_SAFE_LOG(m_log_dev->infoStream()
                << "iceCommandSubmit::updateOsbList() - "
                << "Starting OutputSandboxDestURI manipulation..."
                << log4cpp::CategoryStream::ENDLINE);        
  */        
            string newPath;
            for ( classad::ExprList::iterator it=osbDUList->begin(); 
                  it != osbDUList->end(); it++ ) {
                
                classad::Value v;
                string s;
                if ( (*it)->Evaluate( v ) && v.IsStringValue( s ) ) {
                    pathName osbEntryObj( s );
                    pathName::pathType_t pType( osbEntryObj.getPathType() );
                    
                    switch( pType ) {
                    case pathName::absolute:
                        newPath = default_osbdURI + '/' + osbEntryObj.getFileName();
                        break;
                    case pathName::relative:
                        newPath = default_osbdURI + '/' + osbEntryObj.getFileName();
                        break;
                    case pathName::invalid: // should abort??
                    case pathName::uri:
                        newPath = s;
                        break;
                    }                
                }

		CREAM_SAFE_LOG(m_log_dev->debugStream()
                    << "iceCommandSubmit::updateOsbList() - After input sandbox manipulation, "
                    << s << " became " << newPath
                    << log4cpp::CategoryStream::ENDLINE);        

                // Builds a new value
                classad::Value newV;
                newV.SetStringValue( newPath );
                // Builds the new string
                newOsbDUList->push_back( classad::Literal::MakeLiteral( newV ) );
            }

	    /**
	     * The pointer newOsbDUList pointer is used as argument of ClassAd::Insert. The classad doc
	     * says that that ptr MUST NOT be deallocated by the caller: 
	     * http://www.cs.wisc.edu/condor/classad/c++tut.html#insert
	     */
            jdl->Insert( "OutputSandboxDestURI", newOsbDUList );
        }
    } else {       
        if ( 0 == jdl->Lookup( "OutputSandboxBaseDestURI" ) ) {
            // Put a default OutpuSandboxBaseDestURI attribute
            jdl->InsertAttr( "OutputSandboxBaseDestURI",  default_osbdURI );
        }
    }
}

//-----------------------------------------------------------------------------
// URI utility class
//-----------------------------------------------------------------------------
iceCommandSubmit::pathName::pathName( const string& p ) :
  m_log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()),
  m_fullName( p ),
  m_pathType( invalid )
{
    boost::regex uri_match( "gsiftp://[^/]+(:[0-9]+)?/([^/]+/)*([^/]+)" );
    boost::regex rel_match( "([^/]+/)*([^/]+)" );
    boost::regex abs_match( "(file://)?/([^/]+/)*([^/]+)" );
    boost::smatch what;

    CREAM_SAFE_LOG(
                   m_log_dev->debugStream()
                   << "iceCommandSubmit::pathName::CTOR() - Trying to unparse " << p
                   << log4cpp::CategoryStream::ENDLINE
                   );

    if ( boost::regex_match( p, what, uri_match ) ) {
        // is a uri
        m_pathType = uri;

        m_fileName = '/';
        m_fileName.append(what[3].first,what[3].second);
        if ( what[2].first != p.end() )
            m_pathName.assign(what[2].first,what[2].second);
        m_pathName.append( m_fileName );
    } else if ( boost::regex_match( p, what, rel_match ) ) {
        // is a relative path
        m_pathType = relative;

        m_fileName.assign(what[2].first,what[2].second);
        if ( what[1].first != p.end() )
            m_pathName.assign( what[1].first, what[1].second );
        m_pathName.append( m_fileName );
    } else if ( boost::regex_match( p, what, abs_match ) ) {
        // is an absolute path
        m_pathType = absolute;
        
        m_pathName = '/';
        m_fileName.assign( what[3].first, what[3].second );
        if ( what[2].first != p.end() ) 
            m_pathName.append( what[2].first, what[2].second );
        m_pathName.append( m_fileName );
    }

    CREAM_SAFE_LOG(
                   m_log_dev->debugStream()
                   << "iceCommandSubmit::pathName::CTOR() - "
                   << "Unparsed as follows: filename=[" 
                   << m_fileName << "] pathname={"
                   << m_pathName << "]"
                   << log4cpp::CategoryStream::ENDLINE
                   );

}

//______________________________________________________________________________
void  iceCommandSubmit::doSubscription( const iceUtil::CreamJob& aJob )
{
  //boost::recursive_mutex localMutex; // FIXME: this is a temporary trick that avoid to acquire a subscriptionManager's mutex that could produce dead-lock. 
  boost::recursive_mutex::scoped_lock cemonM( s_localMutexForSubscriptions );
  

  string cemon_url;
  iceUtil::subscriptionManager* subMgr( iceUtil::subscriptionManager::getInstance() );
  iceUtil::DNProxyManager* dnprxMgr( iceUtil::DNProxyManager::getInstance() );
  
  string userDN    = aJob.getUserDN();
  string userProxy = aJob.getUserProxyCertificate();
  string ce        = aJob.getCreamURL();

  {
    //boost::recursive_mutex::scoped_lock cemonM( iceUtil::subscriptionManager::mutex );
    subMgr->getCEMonURL( userProxy, ce, cemon_url ); // also updated the internal subMgr's cache cream->cemon
  }
  
  CREAM_SAFE_LOG(
  		 m_log_dev->debugStream() 
		 << "iceCommandSubmit::doSubscription() - "
		 << "For current CREAM, subscriptionManager returned CEMon URL ["
		 << cemon_url << "]"
		 << log4cpp::CategoryStream::ENDLINE
		 );
  
  // Try to determine if the current user userDN is subscribed to 'cemon_url' by
  // asking the cemonUrlCache
  
  bool foundSubscription;

  foundSubscription = subMgr->hasSubscription( userProxy, cemon_url );
  
  if ( foundSubscription )
    {
      // if this was a ghost subscription (i.e. it does exist in the cemonUrlCache's cache
      // but not actually in the CEMon
      // the subscriptionUpdater will fix it soon
      CREAM_SAFE_LOG(m_log_dev->debugStream()
                     << "iceCommandSubmit::doSubscription() - "
                     << "User [" << userDN << "] is already subsdcribed to CEMon ["
                     << cemon_url << "] (found in subscriptionManager's cache)"
                     << log4cpp::CategoryStream::ENDLINE);

      // If the current proxy expires later than that one in the subscriptionManager's
      // map, we must update it with the current one.
      // This is because it is better to be sure that the subscription Updater
      // will use always the most long-lived proxy to renew subscriptions.
      //subMgr->setUserProxyIfLonger( userDN, userProxy );
      //      {

      dnprxMgr->setUserProxyIfLonger( userDN, userProxy );

      return;
    }	   
  
  
  
  
  // try to determine if the current user userProxy is subscribed to 'cemon_url'
  // with a direct SOAP query to CEMon (can block for SOAP_TIMETOUT seconds,
  // but executed only the first time)
  
  bool subscribed;
  iceUtil::iceSubscription sub;
  
  try {
    
    subscribed = iceUtil::subscriptionProxy::getInstance()->subscribedTo( userProxy, cemon_url, sub );
    
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "iceCommandSubmit::doSubscription() - "
		   << "Couldn't determine if user [" << userDN << "] is subscribed to ["
		   << cemon_url << "]. Another job could trigger a successful subscription."
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  }
  
  // OK: we're definitely subscribed to this CEMon with userProxy proxyfile, but the cached information is lost
  // for some reason (e.g. ICE has been recently re-started).
  
  if( subscribed ) {
    if( m_configuration->ice()->listener_enable_authz() ) {
      // If AUTHZ is ON, before caching the current CEMon to the list
      // of CEMons we're subscribed to, we must ask ot its DN that is 
      // an important information to cache in oder to authorize
      // notifications coming from this CEMon.
      string DN;
      //boost::recursive_mutex::scoped_lock cemonM( iceUtil::subscriptionManager::mutex );
      if( subMgr->getCEMonDN( userProxy, cemon_url, DN ) ) {
	    
	subMgr->insertSubscription( userProxy, cemon_url, sub );
	dnprxMgr->setUserProxyIfLonger( userDN, userProxy );
	
      } else {
	CREAM_SAFE_LOG(m_log_dev->errorStream()
		       << "iceCommandSubmit::doSubscription() - "
		       << "CEMon [" << cemon_url 
		       << "] reported that we're already subscribed to it, "
		       << "but couldn't get its DN. "
		       << "Will not authorize its job status "
		       << "notifications."
		       << log4cpp::CategoryStream::ENDLINE);
	return; 
      }
    } // unlock the subscriptionManager::mutex
    else {

      subMgr->insertSubscription( userProxy, cemon_url, sub );

      dnprxMgr->setUserProxyIfLonger( userDN, userProxy );

    }

    CREAM_SAFE_LOG(m_log_dev->debugStream()
		   << "iceCommandSubmit::doSubscription() - "
		   << "User DN [" << userDN << "] is already subscribed to CEMon ["
		   << cemon_url << "] (asked to CEMon itself)"
		   << log4cpp::CategoryStream::ENDLINE
		   );
  } else {
    // MUST subscribe
    
    bool can_subscribe = true;
    string DN;
    if ( m_configuration->ice()->listener_enable_authz() ) {
      if( !subMgr->getCEMonDN( userProxy, cemon_url, DN ) ) {
	// Cannot subscribe to a CEMon without it's DN
	can_subscribe = false;
	CREAM_SAFE_LOG(m_log_dev->errorStream()
		       << "iceCommandSubmit::doSubscription() - "
		       << "Notification authorization is enabled and couldn't "
		       << "get CEMon's DN. Will not subscribe to it."
		       << log4cpp::CategoryStream::ENDLINE
		       );
      } 
    } // unlock subscriptionManager::mutex
    
    if(can_subscribe) {
      iceUtil::iceSubscription sub;
      if( iceUtil::subscriptionProxy::getInstance()->subscribe( userProxy, cemon_url, sub ) ) {
	{
	  subMgr->insertSubscription( userProxy, cemon_url, sub );
	}

	dnprxMgr->setUserProxyIfLonger( userDN, userProxy );

      } else {
	CREAM_SAFE_LOG(
		       m_log_dev->errorStream()
		       << "iceCommandSubmit::doSubscription() - "
		       << "Couldn't subscribe to [" 
		       << cemon_url << "] with userDN [" << userDN<< "]. Will not"
		       << " receive job status notification from it for this user. "
		       << "Hopefully the subscriptionUpdater will retry."
		       << log4cpp::CategoryStream::ENDLINE
		       );
      }
    } // if(can_subscribe)
  } // else -> if(subscribedTo...)
  
} // end function, also unlocks subscriptionManager's mutex
