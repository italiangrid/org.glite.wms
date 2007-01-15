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
//#define ICE_STANDALONE

// Local includes
#include "iceCommandSubmit.h"
#include "subscriptionManager.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include "cemonUrlCache.h"
#include "creamJob.h"
#include "ice-core.h"
#include "eventStatusListener.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "CreamProxyMethod.h"
#include "filelist_request.h"
#include "filelist_request_purger.h"

#ifdef ICE_STANDALONE
#include "iceLBContext.h" // FIXME: To be removed when job registration to the LB service will be thrown away from ICE
#endif

#include "iceUtils.h"

// Other glite includes
#include "glite/ce/cream-client-api-c/CreamProxy.h"
// #include "glite/ce/cream-client-api-c/scoped_timer.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
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
namespace cream_api = glite::ce::cream_client_api;
namespace api_util = glite::ce::cream_client_api::util;
namespace wms_utils = glite::wms::common::utilities;
namespace wms_conf = glite::wms::common::configuration;

using namespace glite::wms::ice;

namespace { // Anonymous namespace
    
    // 
    // This class is used by a scope_guard to delete a job from the
    // job cache if something goes wrong during the submission.
    //
    class remove_job_from_cache {
    protected:
        const std::string m_grid_job_id;
        util::jobCache* m_cache;
        
    public:
        /**
         * Construct a remove_job_from_cache object which will remove
         * the job with given grid_job_id from the cache.
         */
        remove_job_from_cache( const std::string& grid_job_id ) :
            m_grid_job_id( grid_job_id ),
            m_cache( util::jobCache::getInstance() )
        { };
        /**
         * Actually removes the job from cache. If the job is no
         * longer in the job cache, nothing is done.
         */
        void operator()( void ) {
            boost::recursive_mutex::scoped_lock M( util::jobCache::mutex );
            util::jobCache::iterator it( m_cache->lookupByGridJobID( m_grid_job_id ) );
            m_cache->erase( it );
        }
    };       
    
}; // end anonymous namespace

//____________________________________________________________________________
iceCommandSubmit::iceCommandSubmit( glite::ce::cream_client_api::soap_proxy::CreamProxy* _theProxy, const filelist_request& request )
  throw( util::ClassadSyntax_ex&, util::JobRequest_ex& ) :
    iceAbsCommand( ),
    m_theIce( Ice::instance() ),
    m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
    m_configuration( util::iceConfManager::getInstance()->getConfiguration() ),
    m_lb_logger( util::iceLBLogger::instance() ),
    m_request( request )
{
    try {
        m_myname = util::getHostName();
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
        exit( 1 );
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
  LB_sequence_code = "UI=000002:NS=0000000003:WM=000004:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000"; 
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
    classad::ClassAd *rootAD = parser.ParseClassAd( request.get_request() );

    if (!rootAD) {
        throw util::ClassadSyntax_ex( boost::str( boost::format( "iceCommandSubmit: ClassAd parser returned a NULL pointer parsing request: %1%" ) % request.get_request()  ) );        
    }
    
    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( rootAD );

    string commandStr;
    // Parse the "command" attribute
    if ( !classad_safe_ptr->EvaluateAttrString( "command", commandStr ) ) {
        throw util::JobRequest_ex( boost::str( boost::format( "iceCommandSubmit: attribute 'command' not found or is not a string in request: %1%") % request.get_request() ) );
    }
    boost::trim_if( commandStr, boost::is_any_of("\"") );

    if ( !boost::algorithm::iequals( commandStr, "submit" ) ) {
        throw util::JobRequest_ex( boost::str( boost::format( "iceCommandSubmit:: wrong command parsed: %1%" ) % commandStr ) );
    }

    string protocolStr;
    // Parse the "version" attribute
    if ( !classad_safe_ptr->EvaluateAttrString( "Protocol", protocolStr ) ) {
        throw util::JobRequest_ex("attribute \"Protocol\" not found or is not a string");
    }
    // Check if the version is exactly 1.0.0
    if ( protocolStr.compare("1.0.0") ) {
        throw util::JobRequest_ex("Wrong \"Protocol\" for jobRequest: expected 1.0.0, got " + protocolStr );
    }

    classad::ClassAd *argumentsAD = 0; // no need to free this
    // Parse the "arguments" attribute
    if ( !classad_safe_ptr->EvaluateAttrClassAd( "arguments", argumentsAD ) ) {
        throw util::JobRequest_ex("attribute 'arguments' not found or is not a classad");
    }

    classad::ClassAd *adAD = 0; // no need to free this
    // Look for "ad" attribute inside "arguments"
    if ( !argumentsAD->EvaluateAttrClassAd( "jobad", adAD ) ) {
        throw util::JobRequest_ex("Attribute \"JobAd\" not found inside 'arguments', or is not a classad" );
    }

    // initializes the m_jdl attribute
    classad::ClassAdUnParser unparser;
    unparser.Unparse( m_jdl, argumentsAD->Lookup( "jobad" ) );

    try {
        m_theJob.setJdl( m_jdl );
        m_theJob.setStatus( cream_api::job_statuses::UNKNOWN );
    } catch( util::ClassadSyntax_ex& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream() 
                       << "Cannot instantiate a job from jdl=" << m_jdl
                       << " due to classad excaption: " << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
        throw( util::ClassadSyntax_ex( ex.what() ) );
    }
    
    m_theProxy.reset( _theProxy );
}

//____________________________________________________________________________
void iceCommandSubmit::execute( void ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
    // api_util::scoped_timer tmp_timer( "iceCommandSubmit::execute" );
    
    CREAM_SAFE_LOG(
                   m_log_dev->infoStream()
                   << "iceCommandSubmit::execute() - "
                   << "This request is a Submission..."
                   << log4cpp::CategoryStream::ENDLINE
                   );   
    
    util::jobCache* cache( util::jobCache::getInstance() );
    
    vector<string> url_jid;
    filelist_request_purger purger_f( m_request );
    wms_utils::scope_guard remove_request_guard( purger_f );
    remove_job_from_cache remove_f( m_theJob.getGridJobID() );
    wms_utils::scope_guard remove_job_guard( remove_f );
    
#ifdef ICE_STANDALONE
    CREAM_SAFE_LOG(
                   m_log_dev->infoStream() 
                   << "iceCommandSubmit::execute() - Registering "
                   << "gridJobID=\"" << m_theJob.getGridJobID()
                   << "\" to L&B service with user proxy=\"" 
                   << m_theJob.getUserProxyCertificate() << "\""
                   << log4cpp::CategoryStream::ENDLINE
                   );
    
    m_lb_logger->getLBContext()->registerJob( m_theJob ); // FIXME: to be used ONLY if ICE is being tested alone (i.e., not coupled with the WMS)
#endif
    m_theJob = m_lb_logger->logEvent( new util::wms_dequeued_event( m_theJob, m_configuration->ice()->input() ) );
    m_theJob = m_lb_logger->logEvent( new util::cream_transfer_start_event( m_theJob ) );
    
    string modified_jdl;
    try {    
        // It is important to get the jdl from the job itself, rather
        // than using the m_jdl attribute. This is because the
        // sequence_code attribute inside the jdl classad has been
        // modified by the L&B calls, and we have to pass to CREAM the
        // "last" sequence code as the job wrapper will need to log
        // the "really running" event.
        modified_jdl = creamJdlHelper( m_theJob.getJDL() );
    } catch( util::ClassadSyntax_ex& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream() 
                       << "Cannot convert jdl=" << m_jdl
                       << " due to classad exception:" << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
        m_theJob = m_lb_logger->logEvent( new util::cream_transfer_fail_event( m_theJob, boost::str( boost::format( "iceCommandSubmit cannot convert jdl=%1% due to classad exception=%2%") % m_jdl % ex.what() ) ) );
        m_theJob.set_failure_reason( boost::str( boost::format( "iceCommandSubmit cannot convert jdl=%1% due to classad exception=%2%") % m_jdl % ex.what() ) );
        m_theJob = m_lb_logger->logEvent( new util::job_aborted_event( m_theJob ) );
        throw( iceCommandFatal_ex( ex.what() ) );
    }
    
    CREAM_SAFE_LOG( m_log_dev->log(log4cpp::Priority::INFO, "iceCommandSubmit::execute() - Submitting") );
    CREAM_SAFE_LOG(
                   m_log_dev->debugStream() 
                   << "JDL " << modified_jdl << " to [" 
                   << m_theJob.getCreamURL() <<"]["
                   << m_theJob.getCreamDelegURL() << "]"
                   << log4cpp::CategoryStream::ENDLINE
                   );
    
    try {
        // api_util::scoped_timer autenticate_timer( "iceCommandSubmit::Authenticate" );
        m_theProxy->Authenticate(m_theJob.getUserProxyCertificate());
    } catch ( cream_api::soap_proxy::auth_ex& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream()
                       << "Unable to submit gridJobID=" 
                       << m_theJob.getGridJobID()
                       << " due to authentication error:" << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
        m_theJob.set_failure_reason( ex.what() );
        m_theJob = m_lb_logger->logEvent( new util::cream_transfer_fail_event( m_theJob, ex.what() ) );
        m_theJob.set_failure_reason( boost::str( boost::format( "Submission to CREAM failed due to exception: %1%" ) % ex.what() ) );
        m_theJob = m_lb_logger->logEvent( new util::job_aborted_event( m_theJob ) );
        m_theIce->resubmit_job( m_theJob, boost::str( boost::format( "Resubmitting because of SOAP exception %1%" ) % ex.what() ) );
        throw( iceCommandFatal_ex( ex.what() ) );
    }
    
    string delegID; // empty delegation id, as we do autodelegation
    try {	    
        // api_util::scoped_timer register_timer( "iceCommandSubmit::Register" );
        util::CreamProxy_Register( m_theJob.getCreamURL(), m_theJob.getCreamDelegURL(), delegID,
	modified_jdl,m_theJob.getUserProxyCertificate(), url_jid, m_configuration->ice()->lease_delta_time(), true ).execute(m_theProxy.get(), 3 );

    } catch( exception& ex ) {
        CREAM_SAFE_LOG(
                       m_log_dev->errorStream()
                       << "iceCommandSubmit::execute() - "
                       << "Cannot register jobID="
                       << m_theJob.getGridJobID() 
                       << " Exception:" << ex.what()
                       << log4cpp::CategoryStream::ENDLINE
                       );
        m_theJob.set_failure_reason( ex.what() );
        m_theJob = m_lb_logger->logEvent( new util::cream_transfer_fail_event( m_theJob, ex.what()  ) );
        // The next event is used to show the failure reason in the status info
        // JC+LM log transfer-fail / aborted in case of condor transfers fail
        m_theJob.set_failure_reason( boost::str( boost::format( "Transfer to CREAM failed due to exception: %1%" ) % ex.what() ) );
        m_theJob = m_lb_logger->logEvent( new util::job_aborted_event( m_theJob ) );
        m_theIce->resubmit_job( m_theJob, boost::str( boost::format( "Resubmitting because of exception %1%" ) % ex.what() ) ); // Try to resubmit
        throw( iceCommandFatal_ex( ex.what() ) );
    }
    
    CREAM_SAFE_LOG(
                   m_log_dev->infoStream()
                   << "iceCommandSubmit::execute() - Returned CREAM-JOBID ["
                   << url_jid[1] <<"]"
                   << log4cpp::CategoryStream::ENDLINE
                   );
    
    // no failure: put jobids and status in cache
    // and remove last request from WM's filelist
    
    m_theJob.setCreamJobID(url_jid[1]);
    m_theJob.setStatus(cream_api::job_statuses::PENDING);
    m_theJob.setEndLease( time(0) + m_configuration->ice()->lease_delta_time() );
    m_theJob.setDelegationId( delegID );
    m_theJob.setProxyCertMTime( time(0) ); // FIXME: should be the modification time of the proxy file?
    m_theJob.set_wn_sequence_code( m_theJob.getSequenceCode() );
    
    m_theJob = m_lb_logger->logEvent( new util::cream_transfer_ok_event( m_theJob ) );
    
    /*
     * here must check if we're subscribed to the CEMon service
     * in order to receive the status change notifications
     * of job just submitted. But only if listener is ON
     */
    if( m_theIce->is_listener_started() ) {
	
        this->doSubscription( m_theJob.getCreamURL() );
	
    }

    remove_job_guard.dismiss(); // dismiss guard, job will NOT be removed from cache
    
    boost::recursive_mutex::scoped_lock M( util::jobCache::mutex );
    m_theJob.setLastSeen( time(0) );
    cache->put( m_theJob );
} // execute


//____________________________________________________________________________
string iceCommandSubmit::creamJdlHelper( const string& oldJdl ) throw( util::ClassadSyntax_ex& )
{
    classad::ClassAdParser parser;
    classad::ClassAd *root = parser.ParseClassAd( oldJdl );

    if ( !root ) {
        throw util::ClassadSyntax_ex( boost::str( boost::format( "ClassAd parser returned a NULL pointer parsing request=[%1%]") % oldJdl ) );
    }
    
    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( root );
    
    string ceid;
    if ( !classad_safe_ptr->EvaluateAttrString( "ce_id", ceid ) ) {
        throw util::ClassadSyntax_ex( "ce_id attribute not found" );
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
        classad::ExprList* newIsbList = new classad::ExprList();
        	
	CREAM_SAFE_LOG(m_log_dev->infoStream()
            << "iceCommandSubmit::updateIsbList() "
            << "Starting InputSandbox manipulation..."
            << log4cpp::CategoryStream::ENDLINE);
	
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
                << "iceCommandSubmit::updateIsbList() "
                << s << " became " << newPath
                << log4cpp::CategoryStream::ENDLINE);

            // Builds a new value
            classad::Value newV;
            newV.SetStringValue( newPath );
            // Builds the new string
            newIsbList->push_back( classad::Literal::MakeLiteral( newV ) );
        }
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
            << "iceCommandSubmit::updateOsbList() found no "
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
        classad::ExprList* newOsbDUList = new classad::ExprList();
	
        if ( jdl->EvaluateAttrList( "OutputSandboxDestURI", osbDUList ) ) {

            CREAM_SAFE_LOG(m_log_dev->infoStream()
                << "iceCommandSubmit::updateOsbList() "
                << "Starting OutputSandboxDestURI manipulation..."
                << log4cpp::CategoryStream::ENDLINE);        
          
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
                    << "After input sandbox manipulation, "
                    << s << " became " << newPath
                    << log4cpp::CategoryStream::ENDLINE);        

                // Builds a new value
                classad::Value newV;
                newV.SetStringValue( newPath );
                // Builds the new string
                newOsbDUList->push_back( classad::Literal::MakeLiteral( newV ) );
            }
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
                   m_log_dev->infoStream()
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
void  iceCommandSubmit::doSubscription( const string& ce )
{
  string cemon_url;
  util::cemonUrlCache* cemon_cache( util::cemonUrlCache::getInstance() );
  
  /**
    The entire method must be mutex-protected, because even getCEMonURL modify the
    internal member of cemonUrlCache class.
    If these calls block, this should happen only the first time, because after
    a succesfull query to CREAM cemonUrlCache should update its internal data
    structures
   */
  boost::recursive_mutex::scoped_lock cemonM( util::cemonUrlCache::mutex );
  
  cemon_url = cemon_cache->getCEMonURL( ce );
           
  CREAM_SAFE_LOG(
  		 m_log_dev->infoStream() 
                  << "iceCommandSubmit::doSubscription() - "
                  << "For current CREAM, cemonUrlCache returned CEMon URL ["
                  << cemon_url << "]"
                  << log4cpp::CategoryStream::ENDLINE
                );
            
  // try to determine if we're subscribed to 'cemon_url' by
  // asking the cemonUrlCache
  
  bool foundSubscription = cemon_cache->hasCEMon( cemon_url );
  
  if ( foundSubscription ) {
      // if this a ghost subscription
      // the subscriptionUpdater will fix it soon
      CREAM_SAFE_LOG(m_log_dev->infoStream()
                     << "iceCommandSubmit::doSubscription() - "
                     << "Already subsdcribed to CEMon ["
                     << cemon_url << "] (found in cemonUrlCache)"
                     << log4cpp::CategoryStream::ENDLINE);
      return;
  }	   
  
  
  vector<Subscription> fake;
  
  // try to determine with a direct SOAP query to CEMon
  bool subscribed;
  try {
    subscribed = util::subscriptionManager::getInstance()->subscribedTo( cemon_url, fake );
  } catch(exception& ex) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
                     << "iceCommandSubmit::doSubscription() - "
                     << "Couldn't determine if we're subscribed to ["
                     << cemon_url << "]. Another job could trigger a successful subscription."
                     << log4cpp::CategoryStream::ENDLINE);
    return;
  }
  if( subscribed ) {
      if( m_configuration->ice()->listener_enable_authz() ) {
          string DN;
          if( cemon_cache->getCEMonDN( cemon_url, DN ) ) {
              
              cemon_cache->insertDN( DN );
              cemon_cache->insertCEMon( cemon_url );
              
          } else {
              CREAM_SAFE_LOG(m_log_dev->errorStream()
                             << "iceCommandSubmit::doSubscription() - "
                             << "CEMon [" << cemon_url 
                             << "] reported that we're subscribed to it, "
                             << "but couldn't get its DN. "
                             << "Will not authorize its job status "
                             << "notifications."
                             << log4cpp::CategoryStream::ENDLINE);
          }
      } else {
          cemon_cache->insertCEMon( cemon_url );
      }
      CREAM_SAFE_LOG(m_log_dev->infoStream()
                     << "iceCommandSubmit::doSubscription() - "
                     << "Already subscribed to CEMon ["
                     << cemon_url << "] (asked to CEMon itself)"
                     << log4cpp::CategoryStream::ENDLINE
                     );
  } else {
      // MUST subscribe
      string DN;
      bool can_subscribe = true;
      if ( m_configuration->ice()->listener_enable_authz() ) {
          if( !cemon_cache->getCEMonDN( cemon_url, DN ) ) {
              // Cannot subscribe to a CEMon without it's DN
              can_subscribe = false;
              CREAM_SAFE_LOG(m_log_dev->errorStream()
                             << "iceCommandSubmit::doSubscription() - "
                             << "Notification authorization is enabled but couldn't "
                             << "get CEMon's DN. Will not subscribe to it."
                             << log4cpp::CategoryStream::ENDLINE
                             );
          } else {
              CREAM_SAFE_LOG(m_log_dev->infoStream()
                             << "iceCommandSubmit::doSubscription() - "
                             << "Inserting this DN ["
                             << DN << "] into cemonUrlCache"
                             << log4cpp::CategoryStream::ENDLINE
                             );
              cemon_cache->insertDN( DN );	      
          }
      } // if(m_confMgr->getListenerEnableAuthZ() )
      
      if(can_subscribe) {
          if( util::subscriptionManager::getInstance()->subscribe( cemon_url ) ) {
              cemon_cache->insertCEMon( cemon_url );
              
          } else {
              CREAM_SAFE_LOG(
                             m_log_dev->errorStream()
                             << "iceCommandSubmit::doSubscription() - "
                             << "Couldn't subscribe to [" 
                             << cemon_url << "]. Will not"
                             << " receive job status notification from it. "
                             << "Hopefully the subscriptionUpdater will retry."
                             << log4cpp::CategoryStream::ENDLINE
                             );
          }
      } // if(can_subscribe)
  } // else -> if(subscribedTo...)
  
} // end function

