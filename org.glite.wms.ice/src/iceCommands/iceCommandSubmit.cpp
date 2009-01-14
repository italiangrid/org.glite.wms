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
//#include "glite/ce/cream-client-api-c/scoped_timer.h"

#include "glite/wms/common/utilities/scope_guard.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "Lease_manager.h"

// Boost stuff
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"
#include "boost/format.hpp"

// C++ stuff
#include <ctime>

using namespace std;
namespace ceurl_util = glite::ce::cream_client_api::util::CEUrl;
namespace cream_api = glite::ce::cream_client_api::soap_proxy;
namespace api_util = glite::ce::cream_client_api::util;
namespace wms_utils = glite::wms::common::utilities;
namespace iceUtil = glite::wms::ice::util;
namespace configuration = glite::wms::common::configuration;

using namespace glite::wms::ice;

boost::recursive_mutex iceCommandSubmit::s_localMutexForSubscriptions;

//
//
//____________________________________________________________________________
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
//	  api_util::scoped_timer tmp_timer( "remove_job_from_cache::operator()" );
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
    iceAbsCommand( "iceCommandSubmit" ),
    m_theIce( Ice::instance() ),
    m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
    m_configuration( iceUtil::iceConfManager::getInstance()->getConfiguration() ),
    m_lb_logger( iceUtil::iceLBLogger::instance() ),
    m_request( request )
{
  m_myname = m_theIce->getHostName();
  if( m_configuration->ice()->listener_enable_authn() ) {
    m_myname_url = boost::str( boost::format("https://%1%:%2%") % m_myname % m_configuration->ice()->listener_port() );
  } else {
    m_myname_url = boost::str( boost::format("http://%1%:%2%") % m_myname % m_configuration->ice()->listener_port() );   
  }

// [
//   stream_error = false;
//   edg_jobid = "https://cert-rb-03.cnaf.infn.it:9000/YeyOVNkR84l6QMHl_PY6mQ";
//   GlobusScheduler = "gridit-ce-001.cnaf.infn.it:2119/jobmanager-lcgpbs";
//   ce_id = "gridit-ce-001.cnaf.infn.it:2119/jobmanager-lcgpbs-cert";
//   Transfer_Executable = true;
//   Output = "/var/glite/jobcontrol/condorio/Ye/https_3a_2f_2fcert-rb-03.cnaf.infn.it_3a9000_2fYeyOVNkR84l6QMHl_5fPY6mQ/StandardOutput";
//   Copy_to_Spool = false;
//   Executable = "/var/glite/jobcontrol/submit/Ye/JobWrapper.https_3a_2f_2fcert-rb-03.cnaf.infn.it_3a9000_2fYeyOVNkR84l6QMHl_5fPY6mQ.sh";
//   X509UserProxy = "/var/glite/SandboxDir/Ye/https_3a_2f_2fcert-rb-03.cnaf.infn.it_3a9000_2fYeyOVNkR84l6QMHl_5fPY6mQ/user.proxy"; 
//   Error_ = "/var/glite/jobcontrol/condorio/Ye/https_3a_2f_2fcert-rb-03.cnaf.infn.it_3a9000_2fYeyOVNkR84l6QMHl_5fPY6mQ/StandardError";
//   LB_sequence_code = "UI=000002:NS=0000000003:WM=000004:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000:LBS=000000"; 
//   Notification = "never"; 
//   stream_output = false; 
//   GlobusRSL = "(queue=cert)(jobtype=single)"; 
//   Type = "job"; 
//   Universe = "grid"; 
//   UserSubjectName = "/C=IT/O=INFN/OU=Personal Certificate/L=CNAF/CN=Marco Cecchi"; 
//   Log = "/var/glite/logmonitor/CondorG.log/CondorG.log"; 
//   grid_type = "globus" 
// ]


  string commandStr;
  string protocolStr;
  
  {// Classad-mutex protected region  
    boost::recursive_mutex::scoped_lock M_classad( Ice::ClassAd_Mutex );
    
    classad::ClassAdParser parser;
    classad::ClassAd *rootAD = parser.ParseClassAd( request->to_string() );
    
    if (!rootAD) {
      throw iceUtil::ClassadSyntax_ex( boost::str( boost::format( "iceCommandSubmit: ClassAd parser returned a NULL pointer parsing request: %1%" ) % request->to_string() ) );        
    }
    
    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( rootAD );
    
    // Parse the "command" attribute
    if ( !classad_safe_ptr->EvaluateAttrString( "command", commandStr ) ) {
      throw iceUtil::JobRequest_ex( boost::str( boost::format( "iceCommandSubmit: attribute 'command' not found or is not a string in request: %1%") % request->to_string() ) );
    }
    boost::trim_if( commandStr, boost::is_any_of("\"") );
    
    if ( !boost::algorithm::iequals( commandStr, "submit" ) ) {
      throw iceUtil::JobRequest_ex( boost::str( boost::format( "iceCommandSubmit:: wrong command parsed: %1%" ) % commandStr ) );
    }
    
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
    // Look for "JobAd" attribute inside "arguments"
    if ( !argumentsAD->EvaluateAttrClassAd( "jobad", adAD ) ) {
      throw iceUtil::JobRequest_ex("Attribute \"JobAd\" not found inside 'arguments', or is not a classad" );
    }
    
    // initializes the m_jdl attribute
    classad::ClassAdUnParser unparser;
    unparser.Unparse( m_jdl, argumentsAD->Lookup( "jobad" ) );

  } // end classad-mutex protected regions
  
  try {
    m_theJob.setJdl( m_jdl );
    m_theJob.setStatus( glite::ce::cream_client_api::job_statuses::UNKNOWN );
  } catch( iceUtil::ClassadSyntax_ex& ex ) {
    CREAM_SAFE_LOG(
		   m_log_dev->errorStream() 
		   << "iceCommandSubmit::CTOR() - Cannot instantiate a job from jdl=" << m_jdl
		   << " due to classad excaption: " << ex.what()
		   
		   );
    throw( iceUtil::ClassadSyntax_ex( ex.what() ) );
  }
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
                   << method_name
                   << "This request is a Submission..."
                   
                   );   
    
    iceUtil::jobCache* cache( iceUtil::jobCache::getInstance() );
    
    Request_source_purger purger_f( m_request );
    wms_utils::scope_guard remove_request_guard( purger_f );

    // We start by checking whether the job already exists in ICE
    // cache.  This may happen if ICE already tried to submit the job,
    // but crashed before removing the request from the filelist. If
    // we find the job already in ICE cache, we simply give up
    // (logging an information message), and the purge_f object will
    // take care of actual removal.
    {
//      api_util::scoped_timer tmp_timer( "iceCommandSubmit::execute() - First mutex: Check of GridJobID" );
        boost::recursive_mutex::scoped_lock M( iceUtil::jobCache::mutex );
	iceUtil::jobCache::iterator it( cache->lookupByGridJobID( m_theJob.getGridJobID() ) );
        if ( cache->end() != it ) {
            CREAM_SAFE_LOG( m_log_dev->warnStream()
                            << method_name
                            << "Submit request for job "
                            << m_theJob.describe()
                            << " is related to a job already in ICE cache. "
                            << "Removing the request and going ahead."
                            );
            return;
        }
    } // unlocks the cache


    // This must be left AFTER the above code. The remove_job_guard
    // object will REMOVE the job from the cache when being destroied.
    remove_job_from_cache remove_f( m_theJob.getGridJobID() );
    wms_utils::scope_guard remove_job_guard( remove_f );

    /**
     * We now try to actually submit the job. Any exception raised by
     * the try_to_submit() method is catched, and triggers the
     * appropriate actions (logging to LB and resubmitting).
     */        
    try {
        try_to_submit( );        
    } catch( const iceCommandFatal_ex& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream() 
                       << method_name 
                       << "Error during submission of jdl=" << m_jdl
                       << " Fatal Exception is:" << ex.what()
                       
                       );
        m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, boost::str( boost::format( "Transfer to CREAM failed due to exception: %1%") % ex.what() ) ) );
        m_theJob.set_failure_reason( boost::str( boost::format( "Transfer to CREAM failed due to exception: %1%") % ex.what() ) );
        m_theJob = m_lb_logger->logEvent( new iceUtil::job_aborted_event( m_theJob ) );
        throw( iceCommandFatal_ex( ex.what() ) );
    } catch( const iceCommandTransient_ex& ex ) {

        // The next event is used to show the failure reason in the
        // status info JC+LM log transfer-fail / aborted in case of
        // condor transfers fail
        m_theJob.set_failure_reason( boost::str( boost::format( "Transfer to CREAM failed due to exception: %1%" ) % ex.what() ) );
        m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_fail_event( m_theJob, ex.what()  ) );
        m_theJob = m_lb_logger->logEvent( new iceUtil::job_done_failed_event( m_theJob ) );
        m_theIce->resubmit_job( m_theJob, boost::str( boost::format( "Resubmitting because of exception %1%" ) % ex.what() ) ); // Try to resubmit
        throw( iceCommandFatal_ex( ex.what() ) ); // Yes, we throw an iceCommandFatal_ex in both cases

    }
    
    remove_job_guard.dismiss(); // dismiss guard, job will NOT be removed from cache
}

//
//
//______________________________________________________________________________
void iceCommandSubmit::try_to_submit( void ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
    static const char* method_name = "iceCommandSubmit::try_to_submit() - ";
    /**
     * Retrieve all usefull cert info.  In order to make the userDN an
     * index in the BDb's secondary database it must be available
     * already at the first put.  So the following block of code must
     * remain here.
     */
    cream_api::VOMSWrapper V( m_theJob.getUserProxyCertificate() );
    if( !V.IsValid( ) ) {
        throw( iceCommandTransient_ex( "Authentication error: " + V.getErrorMessage() ) );
    }
    m_theJob.setUserDN( V.getDNFQAN() );
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
        throw( iceCommandFatal_ex( boost::str( boost::format("Cannot convert jdl due to classad exception %1%" ) % ex.what() ) ) );
    }
    
    CREAM_SAFE_LOG(
                   m_log_dev->debugStream() 
                   << method_name << "Submitting JDL " 
		   << modified_jdl << " to [" 
                   << m_theJob.getCreamURL() <<"]["
                   << m_theJob.getCreamDelegURL() << "]"
                   
                   );
    
    CREAM_SAFE_LOG(
                   m_log_dev->debugStream()
                   << method_name
                   << "Sequence code for job "
                   << m_theJob.describe()
                   << " is "
                   << m_theJob.getSequenceCode()
                                      
                   );

    bool is_lease_enabled = ( m_configuration->ice()->lease_delta_time() > 0 );
    //pair<string, time_t> delegation;
    boost::tuple<string, time_t, int> delegation;
    string lease_id; // empty delegation id
    bool force_delegation = false;
    bool force_lease = false;  
    bool retry = true;  
    cream_api::AbsCreamProxy::RegisterArrayResult res;        

    //int delegation_duration;

    while( retry ) {

        //
        // Manage lease creation
        //
        if ( is_lease_enabled ) {
            
            if ( force_lease ) {
                CREAM_SAFE_LOG( m_log_dev->infoStream() << method_name
                                << "Lease is enabled, enforcing creation of a new lease "
                                << "for job " << m_theJob.describe()
                                 );        
            } else {
                CREAM_SAFE_LOG( m_log_dev->infoStream() << method_name
                                << "Lease is enabled, trying to get lease "
                                << "for job " << m_theJob.describe()
                                 );        
            }

            //
            // Get a (possibly existing) lease ID
            //
            try{
                lease_id = iceUtil::Lease_manager::instance()->make_lease( m_theJob, force_lease );
            } catch( const std::exception& ex ) {
                // something was wrong with the lease creation step. 
                string err_msg( boost::str( boost::format( "Failed to get lease_id for job %1%. Exception is %2%" ) % m_theJob.getGridJobID() % ex.what() ) );

                CREAM_SAFE_LOG( m_log_dev->errorStream()
                                << method_name << err_msg );
                throw( iceCommandTransient_ex( err_msg ) );
            } catch( ... ) {
                string err_msg( boost::str( boost::format( "Failed to get lease_id for job %1% due to unknown exception" ) % m_theJob.getGridJobID() ) );
                CREAM_SAFE_LOG( m_log_dev->errorStream()
                                << method_name << err_msg );
                throw( iceCommandTransient_ex( err_msg ) );
            }
            
            // lease creation OK
            CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                            << "Using lease ID " << lease_id << " for job "
                            << m_theJob.describe()
                             );        
        }

        // 
        // Delegates the proxy
        //
        try {
	  delegation = iceUtil::Delegation_manager::instance()->delegate( m_theJob, V, force_delegation, m_theJob.is_proxy_renewable() );
        } catch( const exception& ex ) {
            throw( iceCommandTransient_ex( boost::str( boost::format( "Failed to create a delegation id for job %1%: reason is %2%" ) % m_theJob.getGridJobID() % ex.what() ) ) );
	}

	//delegation_duration = delegation.second - time(0);

        //
        // Registers the job (without autostart)
        //
        try {
                
            CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                            << "Going to REGISTER Job "
                            << m_theJob.describe() << " with delegation ID ["
			    << delegation.get<0>() << "]..."
                             );
            
            cream_api::AbsCreamProxy::RegisterArrayRequest req;
            
            // FIXME: must check what to set the 3rd and 4th arguments
            // (delegationProxy, leaseID) last asrgument is irrelevant
            // now, because we register jobs one by one
            cream_api::JobDescriptionWrapper jd(modified_jdl, 
                                                delegation.get<0>(), 
                                                ""/* delegPRoxy */, 
                                                lease_id /* leaseID */, 
                                                false, /* NO autostart */
                                                "foo");
            
            req.push_back( &jd );
            
            string iceid = m_theIce->getHostDN();
            boost::trim_if(iceid, boost::is_any_of("/"));
            boost::replace_all( iceid, "/", "_" );
            boost::replace_all( iceid, "=", "_" );
            
            iceUtil::CreamProxy_Register( m_theJob.getCreamURL(),
                                          m_theJob.getUserProxyCertificate(),
                                          (const cream_api::AbsCreamProxy::RegisterArrayRequest*)&req,
                                          &res,
                                          iceid).execute( 3 );
            
        } catch ( glite::ce::cream_client_api::cream_exceptions::DelegationException& ex ) {
            if ( !force_delegation ) {
                CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
                                << "Cannot register GridJobID ["
                                << m_theJob.getGridJobID() 
                                << "] due to Delegation Exception: " 
                                << ex.what() << ". Will retry once..."
                                 );
                force_delegation = true;
            } else {
                throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register raised DelegationException %1%") % ex.what() ) ) ); // Rethrow
            }
        } catch ( glite::ce::cream_client_api::cream_exceptions::GenericException& ex ) {
            if ( is_lease_enabled && !force_lease ) {
                CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
                                << "Cannot register GridJobID ["
                                << m_theJob.getGridJobID() 
                                << "] due to Generic Fault: " 
                                << ex.what() << ". Will retry once by enforcing creation of a new lease ID..."
                                 );
                force_lease = true;
		continue;
            } else {
                throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register raised GenericFault %1%") % ex.what() ) ) ); // Rethrow
            }                        
        } catch ( exception& ex ) {
            CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
                            << "Cannot register GridJobID ["
                            << m_theJob.getGridJobID() 
                            << "] due to std::exception: " 
                            << ex.what() << "."
                             );
            throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register raised std::exception %1%") % ex.what() ) ) ); // Rethrow
        }

        // FIXME: This should be updated as soon as the C++ CREAM APIs
        // have been modified to check the correct return values
        cream_api::JobIdWrapper::RESULT result = res.begin()->second.get<0>();
        string err = res.begin()->second.get<2>();
            
        switch( result ) {
        case cream_api::JobIdWrapper::OK: // nothing to do
            retry = false;
            break;
        case cream_api::JobIdWrapper::DELEGATIONIDMISMATCH:
        case cream_api::JobIdWrapper::DELEGATIONPROXYERROR:
            if ( !force_delegation ) {
                CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
                                << "Cannot register GridJobID ["
                                << m_theJob.getGridJobID() 
                                << "] due to Delegation Error: " 
                                << err << ". Will retry once..."
                                 );
                force_delegation = true;
            } else {
                throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register returned delegation error \"%1%\"") % err ) ) );
            }            
            break;
        case cream_api::JobIdWrapper::LEASEIDMISMATCH:
            if ( is_lease_enabled && !force_lease ) {
                CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
                                << "Cannot register GridJobID ["
                                << m_theJob.getGridJobID() 
                                << "] due to Lease Error: " 
                                << err << ". Will retry once by enforcing creation of a new lease ID..."
                                 );
                force_lease = true;
            } else {
                throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register returned lease id mismatch \"%1%\"") % err ) ) );
            }     
            break;                               
        default:
            CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
                            << "Error while registering GridJobID ["
                            << m_theJob.getGridJobID() 
                            << "] due to Error: " 
                            << err
                             );
            throw( iceCommandTransient_ex( boost::str( boost::format( "CREAM Register returned error \"%1%\"") % err ) ) ); 
        }

    } // end while(1)
    
    string jobId      = res.begin()->second.get<1>().getCreamJobID();
    string __creamURL = res.begin()->second.get<1>().getCreamURL();
    
    string completeid = __creamURL;

    boost::replace_all( completeid, m_configuration->ice()->cream_url_postfix(), "" );

    completeid += "/" + jobId;

    // FIXME: should we check that __creamURL ==
    // m_theJob.getCreamURL() ?!?  If it is not it's VERY severe
    // server error, and I think it is not our businness

    CREAM_SAFE_LOG( m_log_dev->infoStream() << method_name
                    << "For GridJobID [" << m_theJob.getGridJobID() << "]" 
                    << " CREAM Returned CREAM-JOBID [" << completeid <<"]"
                     );

    cream_api::ResultWrapper startRes;
    
    try {
        CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                        << "Going to START CreamJobID ["
                        << completeid <<"] related to GridJobID ["
                        << m_theJob.getGridJobID() << "]..."
                         );
        
      // FIXME: must create the request JobFilterWrapper for the Start operation
      vector<cream_api::JobIdWrapper> toStart;
      toStart.push_back( cream_api::JobIdWrapper( (const string&)jobId, (const string&)__creamURL, vector<cream_api::JobPropertyWrapper>()));

      cream_api::JobFilterWrapper jw(toStart, vector<string>(), -1, -1, "", "");

      iceUtil::CreamProxy_Start( __creamURL, 
				 m_theJob.getUserProxyCertificate(), 
				 (const cream_api::JobFilterWrapper *)&jw, 
				 &startRes ).execute( 7 );
    } catch( exception& ex ) {
        throw iceCommandTransient_ex( boost::str( boost::format( "CREAM Start raised exception %1%") % ex.what() ) );
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
      
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Cannot start job " << m_theJob.describe()
		     << ". Reason is: " << errMex
		      );

      throw iceCommandTransient_ex( boost::str( boost::format( "CREAM Start failed due to error %1%") % errMex ) );
    }
   
    // no failure: put jobids and status in cache
    // and remove last request from WM's filelist
    
    m_theJob.setCreamJobID( jobId );
    m_theJob.setStatus(glite::ce::cream_client_api::job_statuses::PENDING);    
    m_theJob.setDelegationId( delegation.get<0>() );
    m_theJob.setDelegationExpirationTime( delegation.get<1>() );
    m_theJob.setDelegationDuration( delegation.get<2>() );
    m_theJob.set_lease_id( lease_id ); // FIXME: redundant??
    m_theJob.setProxyCertMTime( time(0) ); // FIXME: should be the modification time of the proxy file?
    m_theJob.set_wn_sequence_code( m_theJob.getSequenceCode() );
    
    m_theJob = m_lb_logger->logEvent( new iceUtil::cream_transfer_ok_event( m_theJob ) );
    
    // now the job is in cache and has been registered we can save its
    // proxy into the DN-Proxy Manager's cache
    iceUtil::DNProxyManager::getInstance()->setUserProxyIfLonger( m_theJob.getUserDN(), 
								  m_theJob.getUserProxyCertificate(), 
								  V.getProxyTimeEnd() );

    /*
     * here must check if we're subscribed to the CEMon service
     * in order to receive the status change notifications
     * of job just submitted. But only if listener is ON
     */
    if( m_theIce->is_listener_started() ) {	
        doSubscription( m_theJob );	
    }
           
    {
      //api_util::scoped_timer tmp_timer( "iceCommandSubmit::try_to_submit() - Put in cache" );
        boost::recursive_mutex::scoped_lock M( iceUtil::jobCache::mutex );
        m_theJob.setLastSeen( time(0) );
        iceUtil::jobCache::getInstance()->put( m_theJob );
    }
} // try_to_submit


//____________________________________________________________________________
string iceCommandSubmit::creamJdlHelper( const string& oldJdl ) throw( iceUtil::ClassadSyntax_ex& )
{ // Classad-mutex protected region

  boost::recursive_mutex::scoped_lock M_classad( Ice::ClassAd_Mutex );
  const configuration::WMConfiguration* WM_conf = m_configuration->wm();
  
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

  if ( 0 == classad_safe_ptr->Lookup( "maxOutputSandboxSize" ) && WM_conf ) {
      classad_safe_ptr->InsertAttr( "maxOutputSandboxSize", WM_conf->max_output_sandbox_size());
  }

  updateIsbList( classad_safe_ptr.get() );
  updateOsbList( classad_safe_ptr.get() );

#ifdef PIPPO  
  // Set CERequirements
  if ( WM_conf ) {
      std::vector<std::string> reqs_to_forward(WM_conf->ce_forward_parameters());

      // In order to forward the required attributes, these
      // have to be *removed* from the CE ad that is used
      // for flattening.
      classad::ClassAd* local_ce_ad(new classad::ClassAd(*ce_ad));
      classad::ClassAd* local_job_ad(new classad::ClassAd(*job_ad));

      std::vector<std::string>::const_iterator cur_req;
      std::vector<std::string>::const_iterator req_end = 
          reqs_to_forward.end();
      for (cur_req = reqs_to_forward.begin();
           cur_req != req_end; cur_req++)
          {
              local_ce_ad->Remove(*cur_req);
              // Don't care if it doesn't succeed. If the attribute is
              // already missing, so much the better.
          }
      
      // Now, flatten the Requirements expression of the Job Ad
      // with whatever info is left from the CE ad.
      // Recipe received from Nick Coleman, ncoleman@cs.wisc.edu
      // on Tue, 8 Nov 2005
      classad::MatchClassAd mad;
      mad.ReplaceLeftAd( local_job_ad );
      mad.ReplaceRightAd( local_ce_ad );
      
      classad::ExprTree *req = mad.GetLeftAd()->Lookup( "Requirements" );
      classad::ExprTree *flattened_req = 0;
      classad::Value fval;
      if( ! ( mad.GetLeftAd()->Flatten( req, fval, flattened_req ) ) ) {
          // Error while flattening. Result is undefined.
          return result;
      }
      
      // No remaining requirement. Result is undefined.
      if (!flattened_req) return result;
  }
#endif
  // Produce resulting JDL

  string newjdl;
  classad::ClassAdUnParser unparser;
  unparser.Unparse( newjdl, classad_safe_ptr.get() ); // this is safe: Unparse doesn't deallocate its second argument
  
  return newjdl;
} // end of creamJdlHelper(...) and ClassAd-mutex releasing


//______________________________________________________________________________
void iceCommandSubmit::updateIsbList( classad::ClassAd* jdl )
{ // synchronized block because the caller is Classad-mutex synchronized
    const static char* method_name = "iceCommandSubmit::updateIsbList() - ";
    string default_isbURI = "gsiftp://";
    default_isbURI.append( m_myname );
    default_isbURI.push_back( '/' );
    string isbPath;
    if ( jdl->EvaluateAttrString( "InputSandboxPath", isbPath ) ) {
        default_isbURI.append( isbPath );
    } else {
        CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
                        << "\"InputSandboxPath\" attribute in the JDL. "
                        << "Hope this is correct..."
                        
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
            );
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
	    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
                           << s << " became " << newPath
                           );

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
            );        
    }

    if ( 0 != jdl->Lookup( "OutputSandboxDestURI" ) ) {

        // Remove the OutputSandboxBaseDestURI from the classad
        // OutputSandboxDestURI and OutputSandboxBaseDestURI cannot
        // be given at the same time.
      if( 0 != jdl->Lookup( "OutputSandboxBaseDestURI") )
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
                    );        

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
                   << m_fileName << "] pathname=["
                   << m_pathName << "]"
                   
                   );

}

//______________________________________________________________________________
void  iceCommandSubmit::doSubscription( const iceUtil::CreamJob& aJob )
{
  static const char* method_name = "iceCommandSubmit::doSubscription() - ";
  boost::recursive_mutex::scoped_lock cemonM( s_localMutexForSubscriptions );
  
  string cemon_url;
  iceUtil::subscriptionManager* subMgr( iceUtil::subscriptionManager::getInstance() );
  //  iceUtil::DNProxyManager* dnprxMgr( iceUtil::DNProxyManager::getInstance() );
  
  //string userDN    = aJob.getUserDN();
  //string userProxy = aJob.getUserProxyCertificate();
  // string ce        = aJob.getCreamURL();

  subMgr->getCEMonURL( aJob.getUserProxyCertificate(), aJob.getCreamURL(), cemon_url ); // also updated the internal subMgr's cache cream->cemon
  
  CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                  << "For current CREAM, subscriptionManager returned CEMon URL ["
                  << cemon_url << "]"
                  
                  );
  
  // Try to determine if the current user userDN is subscribed to 'cemon_url' by
  // asking the cemonUrlCache
  
  bool foundSubscription;

  foundSubscription = subMgr->hasSubscription( aJob.getUserProxyCertificate(), cemon_url );
  
  if ( foundSubscription )
    {
      // if this was a ghost subscription (i.e. it does exist in the cemonUrlCache's cache
      // but not actually in the CEMon
      // the subscriptionUpdater will fix it soon
      CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		     << "User [" << aJob.getUserDN() << "] is already subsdcribed to CEMon ["
		     << cemon_url << "] (found in subscriptionManager's cache)"
		     );
      
      return;
    }	   
  
  
  
  
  // try to determine if the current user userProxy is subscribed to 'cemon_url'
  // with a direct SOAP query to CEMon (can block for SOAP_TIMETOUT seconds,
  // but executed only the first time)
  
  bool subscribed;
  iceUtil::iceSubscription sub;
  
  try {
    
    subscribed = iceUtil::subscriptionProxy::getInstance()->subscribedTo( aJob.getUserProxyCertificate(), cemon_url, sub );
    
  } catch(exception& ex) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
                     << "Couldn't determine if user [" << aJob.getUserDN() 
                     << "] is subscribed to [" << cemon_url 
                     << "]. Another job could trigger a successful subscription."
                     );
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

      if( subMgr->getCEMonDN( aJob.getUserProxyCertificate(), cemon_url, DN ) ) {
	    
	subMgr->insertSubscription( aJob.getUserProxyCertificate(), cemon_url, sub );
	//dnprxMgr->setUserProxyIfLonger( aJob.getUserDN(), aJob.getUserProxyCertificate() );
	
      } else {
          CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		       << "CEMon [" << cemon_url 
		       << "] reported that we're already subscribed to it, "
		       << "but couldn't get its DN. "
		       << "Will not authorize its job status "
		       << "notifications."
		       );
	return; 
      }
    } 
    else {

      subMgr->insertSubscription( aJob.getUserProxyCertificate(), cemon_url, sub );

      //dnprxMgr->setUserProxyIfLonger( aJob.getUserDN(), aJob.getUserProxyCertificate() );

    }

    CREAM_SAFE_LOG(m_log_dev->debugStream() << method_name
		   << "User DN [" << aJob.getUserDN() << "] is already subscribed to CEMon ["
		   << cemon_url << "] (asked to CEMon itself)"
		   
		   );
  } else {
    // MUST subscribe
    
    bool can_subscribe = true;
    string DN;
    if ( m_configuration->ice()->listener_enable_authz() ) {
      if( !subMgr->getCEMonDN( aJob.getUserProxyCertificate(), cemon_url, DN ) ) {
	// Cannot subscribe to a CEMon without it's DN
	can_subscribe = false;
	CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		       << "Notification authorization is enabled and couldn't "
		       << "get CEMon's DN. Will not subscribe to it."
		       
		       );
      } 
    }
    
    if(can_subscribe) {
      iceUtil::iceSubscription sub;
      if( iceUtil::subscriptionProxy::getInstance()->subscribe( aJob.getUserProxyCertificate(), cemon_url, sub ) ) {
	{
	  subMgr->insertSubscription( aJob.getUserProxyCertificate(), cemon_url, sub );
	}

	//dnprxMgr->setUserProxyIfLonger( aJob.getUserDN(), aJob.getUserProxyCertificate() );

      } else {
          CREAM_SAFE_LOG( m_log_dev->errorStream() << method_name
		       << "Couldn't subscribe to [" 
		       << cemon_url << "] with userDN [" << aJob.getUserDN()<< "]. Will not"
		       << " receive job status notification from it for this user. "
		       << "Hopefully the subscriptionUpdater will retry."
		       
		       );
      }
    } // if(can_subscribe)
  } // else -> if(subscribedTo...)
  
} // end function, also unlocks the iceCommandSubmit's s_localMutexForSubscription mutex
