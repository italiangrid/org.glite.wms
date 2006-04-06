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

// Local includes
#include "iceCommandSubmit.h"
#include "subscriptionCache.h"
#include "subscriptionManager.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include "cemonUrlCache.h"
#include "creamJob.h"
#include "ice-core.h"
#include "eventStatusListener.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "iceLBContext.h" // FIXME: To be removed when job registration to the LB service will be thrown away from ICE

// Other glite includes
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"

// Boost stuff
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"
#include "boost/format.hpp"

// C++ stuff
#include <ctime>
#include <netdb.h>
#include <cstring> // for memset

extern int h_errno;

using namespace std;

namespace ceurl_util = glite::ce::cream_client_api::util::CEUrl;
namespace cream_api = glite::ce::cream_client_api;
namespace api_util = glite::ce::cream_client_api::util;

namespace glite {
namespace wms {
namespace ice {

//____________________________________________________________________________
iceCommandSubmit::iceCommandSubmit( const string& request )
  throw( util::ClassadSyntax_ex&, util::JobRequest_ex& ) :
    iceAbsCommand( ),
    m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
    m_confMgr( util::iceConfManager::getInstance()), // no need of mutex here because the getInstance does that
    m_lb_logger( util::iceLBLogger::instance() )
{
    char name[256];
    memset((void*)name, 0, 256);
    if(gethostname(name, 256) == -1)
    {
        m_log_dev->fatalStream() 
            << "iceCommandSubmit::CTOR - Couldn't resolve local hostname: "
            << strerror(errno)
            << log4cpp::CategoryStream::ENDLINE;

      exit(1);
    }
    struct hostent *H = gethostbyname(name);
    if(!H) {
        m_log_dev->fatalStream() 
            << "iceCommandSubmit::CTOR() - Couldn't resolve local hostname: "
            << strerror(h_errno)
            << log4cpp::CategoryStream::ENDLINE;
        exit(1);
    }

    {
      boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
      m_myname_url = boost::str( boost::format("http://%1%:%2%") % H->h_name % m_confMgr->getListenerPort() );
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
    classad::ClassAd *_rootAD = parser.ParseClassAd( request );

    if (!_rootAD) {
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request by iceCommandSubmit");
    }

    string _commandStr;
    // Parse the "command" attribute
    if ( !_rootAD->EvaluateAttrString( "command", _commandStr ) ) {
        throw util::JobRequest_ex("attribute 'command' not found or is not a string");
    }
    boost::trim_if( _commandStr, boost::is_any_of("\"") );

    if ( !boost::algorithm::iequals( _commandStr, "submit" ) ) {
        throw util::JobRequest_ex("wrong command ["+_commandStr+"] parsed by iceCommandSubmit" );
    }

    string _protocolStr;
    // Parse the "version" attribute
    if ( !_rootAD->EvaluateAttrString( "Protocol", _protocolStr ) ) {
        throw util::JobRequest_ex("attribute \"Protocol\" not found or is not a string");
    }
    // Check if the version is exactly 1.0.0
    if ( _protocolStr.compare("1.0.0") ) {
        throw util::JobRequest_ex("Wrong \"Protocol\" for jobRequest: expected 1.0.0, got " + _protocolStr );
    }

    classad::ClassAd *_argumentsAD = 0;
    // Parse the "arguments" attribute
    if ( !_rootAD->EvaluateAttrClassAd( "arguments", _argumentsAD ) ) {
        throw util::JobRequest_ex("attribute 'arguments' not found or is not a classad");
    }

    classad::ClassAd *_adAD = 0;
    // Look for "ad" attribute inside "arguments"
    if ( !_argumentsAD->EvaluateAttrClassAd( "jobad", _adAD ) ) {
        throw util::JobRequest_ex("Attribute \"JobAd\" not found inside 'arguments', or is not a classad" );
    }

    // initializes the m_jdl attribute
    classad::ClassAdUnParser unparser;
    unparser.Unparse( m_jdl, _argumentsAD->Lookup( "jobad" ) );

}

//______________________________________________________________________________
void iceCommandSubmit::execute( Ice* _ice ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
    m_log_dev->log(log4cpp::Priority::INFO, "iceCommandSubmit::execute() - This request is a Submission...");

    util::jobCache* _cache = util::jobCache::getInstance();

    vector<string> url_jid;
    util::CreamJob theJob;
    cream_api::soap_proxy::CreamProxy* theProxy = cream_api::soap_proxy::CreamProxyFactory::getProxy();

    try {
        theJob.setJdl( m_jdl );
        theJob.setStatus( cream_api::job_statuses::UNKNOWN, time(NULL) );
    } catch( util::ClassadSyntax_ex& ex ) {
        m_log_dev->errorStream() 
            << "Cannot instantiate a job from jdl="
            << m_jdl
            << " due to classad excaption: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        throw( iceCommandFatal_ex( ex.what() ) );
        // TODO: L&B?
    }

    // Put job in the cache; remember position in job_pos
    /**
     * This mutex protects all cache accesses in the execute method
     */
    boost::recursive_mutex::scoped_lock M( util::jobCache::mutex );
    util::jobCache::iterator job_pos = _cache->put( theJob );

    m_log_dev->infoStream() 
        << "iceCommandSubmit::execute() - Registering "
        << "gridJobID=\"" << theJob.getGridJobID()
        << "\" to L&B service with user proxy=\"" 
        << theJob.getUserProxyCertificate() 
        << "\""
        << log4cpp::CategoryStream::ENDLINE;

    m_lb_logger->getLBContext()->registerJob( theJob ); // FIXME: to be removed
    m_lb_logger->logEvent( new util::cream_transfer_start_event( theJob ) );

    string modified_jdl;
    try {    
        modified_jdl = creamJdlHelper( m_jdl );
    } catch( util::ClassadSyntax_ex& ex ) {
        m_log_dev->errorStream() 
            << "Cannot convert jdl="
            << m_jdl
            << " due to classad exception:"
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        m_lb_logger->logEvent( new util::cream_transfer_fail_event( theJob, ex.what() ) );
        _cache->erase( job_pos );
        throw( iceCommandFatal_ex( ex.what() ) );
    }
    
    m_log_dev->log(log4cpp::Priority::INFO, "iceCommandSubmit::execute() - Submitting");
    m_log_dev->debugStream() 
        << "JDL " << modified_jdl
        << " to [" << theJob.getCreamURL() <<"]["
        << theJob.getCreamDelegURL() << "]"
        << log4cpp::CategoryStream::ENDLINE;

    try {
        theProxy->Authenticate(theJob.getUserProxyCertificate());
    } catch ( cream_api::soap_proxy::auth_ex& ex ) {
        m_lb_logger->logEvent( new util::cream_transfer_fail_event( theJob, ex.what() ) );
        m_log_dev->errorStream()
            << "Unable to submit gridJobID=" << theJob.getGridJobID()
            << " due to authentication error:" << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        _ice->resubmit_job( theJob );
        _cache->erase( job_pos );
        throw( iceCommandFatal_ex( ex.what() ) );
    }

    { 
        // lock the listener:
        // this prevents the eventStatusListener::acceptJobStatus()
        // to process a notification of a just submitted job that is not
        // yet present in the jobCache
        boost::recursive_mutex::scoped_lock lockAccept( util::eventStatusListener::mutexJobStatusUpdate );
	string delegID = "";
        try {
	    
            theProxy->Register(
                               theJob.getCreamURL().c_str(),
                               theJob.getCreamDelegURL().c_str(),
                               delegID, // deleg ID not needed because this client
                               // will always do auto_delegation
                               modified_jdl,
                               theJob.getUserProxyCertificate(),
                               url_jid,
			       // -1,
                               2, // 2 minute lease
                               true /*autostart*/
                               );
        } catch( exception& ex ) {
            m_log_dev->errorStream()
                << "Cannot register jobID="
                << theJob.getGridJobID()
                << " Exception:" << ex.what()
                << log4cpp::CategoryStream::ENDLINE;
            m_lb_logger->logEvent( new util::cream_transfer_fail_event( theJob, ex.what()  ) );
            _ice->resubmit_job( theJob ); // Try to resubmit
            _cache->erase( job_pos );
            throw( iceCommandFatal_ex( ex.what() ) );
        }

        m_log_dev->infoStream()
            << "iceCommandSubmit::execute() - Returned CREAM-JOBID ["
            << url_jid[1] <<"]"
            << log4cpp::CategoryStream::ENDLINE;
        
        // no failure: put jobids and status in cache
        // and remove last request from WM's filelist

        theJob.setJobID(url_jid[1]);
        theJob.setStatus(cream_api::job_statuses::PENDING, time(0) );
        theJob.setDelegationId( delegID );
        theJob.setProxyCertMTime( time(0) ); // FIXME: should be the modification time of the proxy file?

        m_lb_logger->logEvent( new util::cream_transfer_ok_event( theJob ) );
        m_lb_logger->logEvent( new util::cream_accepted_event( theJob ) );

        // put(...) accepts arg by reference, but
        // the implementation puts the arg in the memory hash by copying it. So
        // passing a *pointer should not produce problems

        // The following is redundant, as logEvent as a (wanted) side
        // effect stores the job
        _cache->put( theJob );
    } // this end-scope unlock the listener that now can accept new notifications

    /*
     * here must check if we're subscribed to the CEMon service
     * in order to receive the status change notifications
     * of job just submitted. But only if listener is ON
     */
    bool _tmp_start_listener;
    {
        boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
        _tmp_start_listener = m_confMgr->getStartListener();
    }

    if( _tmp_start_listener ) {
      string cemon_url;
      {
        boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
	boost::recursive_mutex::scoped_lock cemonM( util::cemonUrlCache::mutex );
        //cemon_url  m_confMgr->getCEMonUrlPrefix() + theJob.getEndpoint()
        //    + m_confMgr->getCEMonUrlPostfix();
	cemon_url = util::cemonUrlCache::getInstance()->getCEMonUrl( theJob.getCreamURL() );
	m_log_dev->infoStream() << "iceCommandSubmit::execute() - "
			      << "For current CREAM, cemonUrlCache returned CEMon URL ["
			      << cemon_url<<"]"
			      << log4cpp::CategoryStream::ENDLINE;

	if( cemon_url.empty() ) {
	  try {
	      cream_api::soap_proxy::CreamProxyFactory::getProxy()->Authenticate( m_confMgr->getHostProxyFile() );
	      cream_api::soap_proxy::CreamProxyFactory::getProxy()->GetCEMonURL( theJob.getCreamURL().c_str(), cemon_url );
	      m_log_dev->infoStream() 
                  << "iceCommandSubmit::execute() - "
                  << "For current CREAM, query to CREAM service returned "
                  << "CEMon URL [" << cemon_url<<"]"
                  << log4cpp::CategoryStream::ENDLINE;
	      util::cemonUrlCache::getInstance()->putCEMonUrl( theJob.getCreamURL(), cemon_url );
	    } catch(exception& ex) {

	      m_log_dev->errorStream() 
                  << "iceCommandSubmit::execute() - Error retrieving"
                  <<" CEMon's URL from CREAM's URL: "
                  << ex.what()
                  << ". Composing URL from configuration file..."
                  << log4cpp::CategoryStream::ENDLINE;
	      cemon_url = theJob.getCreamURL();
	      boost::replace_first(cemon_url,
                                   m_confMgr->getCreamUrlPostfix(),
                                   m_confMgr->getCEMonUrlPostfix()
                                  );
	      m_log_dev->infoStream() 
                  << "Using CEMon URL ["
                  << cemon_url << "]" 
                  << log4cpp::CategoryStream::ENDLINE;
	      util::cemonUrlCache::getInstance()->putCEMonUrl( theJob.getCreamURL(), cemon_url );
	    }
	}
      }
      boost::recursive_mutex::scoped_lock M( util::subscriptionCache::mutex );
      if( !util::subscriptionCache::getInstance()->has(cemon_url) ) {
            /* MUST SUBSCRIBE TO THIS CEMON */
            m_log_dev->infoStream()
                << "iceCommandSubmit::execute() - Not subscribed to ["
                << cemon_url << "]. Going to subscribe to it..."
                << log4cpp::CategoryStream::ENDLINE;

      {
	    boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
            m_log_dev->infoStream()
                << "iceCommandSubmit::execute() - Subscribing the consumer ["
                << m_myname_url << "] to ["<<cemon_url
                << "] with duration="
                << m_confMgr->getSubscriptionDuration()
                << " secs"
                << log4cpp::CategoryStream::ENDLINE;
      }

      /**
      * This is the 1st call of
      * subscriptionManager::getInstance() and it's safe
      * because the singleton has already been created by
      * ice-core module.
      */
       boost::recursive_mutex::scoped_lock M( util::subscriptionManager::mutex );
       if( !util::subscriptionManager::getInstance()->subscribe(cemon_url) ) {
         m_log_dev->errorStream()
             << "iceCommandSubmit::execute() - Subscribe to ["
             << cemon_url << "] failed! "
             << "Will not receive status notifications from it..."
             << log4cpp::CategoryStream::ENDLINE;
       } else {
         m_log_dev->infoStream()
             << "iceCommandSubmit::execute() - Subscribed with ID ["
             << util::subscriptionManager::getInstance()->getLastSubscriptionID() << "]"
             << log4cpp::CategoryStream::ENDLINE;

         util::subscriptionCache::getInstance()->insert(cemon_url);
       }
     }
   }
} // execute

//____________________________________________________________________________
string iceCommandSubmit::creamJdlHelper( const string& oldJdl ) throw( util::ClassadSyntax_ex& )
{
    classad::ClassAdParser parser;
    classad::ClassAd *_root = parser.ParseClassAd( oldJdl );

    if ( !_root ) {
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");
    }

    string ceid;
    if ( !_root->EvaluateAttrString( "ce_id", ceid ) ) {
        throw util::ClassadSyntax_ex( "ce_id attribute not found" );
    }
    boost::trim_if( ceid, boost::is_any_of("\"") );

    vector<string> ceid_pieces;
    ceurl_util::parseCEID( ceid, ceid_pieces );
    string bsname = ceid_pieces[2];
    string qname = ceid_pieces[3];

    // Update jdl to insert two new attributes needed by cream:
    // QueueName and BatchSystem.
    _root->InsertAttr( "QueueName", qname );
    _root->InsertAttr( "BatchSystem", bsname );

    updateIsbList( _root );
    updateOsbList( _root );

    string newjdl;
    classad::ClassAdUnParser unparser;
    unparser.Unparse( newjdl, _root );
    return newjdl;
}

//______________________________________________________________________________
void iceCommandSubmit::updateIsbList( classad::ClassAd* jdl )
{
    const string default_isbURI = "gsiftp://hostnamedelwms.pd.infn.it/jobISBdir"; // FIXME

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
	m_log_dev->log(log4cpp::Priority::INFO,"\tStarting InputSandbox manipulation...");
        //cout << "Starting InputSandbox manipulation..." << endl;
	//	m_log_dev
	
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
	    m_log_dev->log(log4cpp::Priority::DEBUG,
			 string("\t")+s+" became "+newPath);
	    //            cout << s << " became " << newPath << endl;
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
    const string default_osbdURI = "gsiftp://hostnamedelwms.pd.infn.it/jobOSBdir"; // FIXME

    // If no OutputSandbox attribute is defined, then nothing has to be done
    if ( 0 == jdl->Lookup( "OutputSandbox" ) )
        return;

    if ( 0 != jdl->Lookup( "OutputSandboxDestURI" ) ) {

        // Check if all the entries in the OutputSandboxDestURI
        // are absolute URIs

        classad::ExprList* osbDUList;
        classad::ExprList* newOsbDUList = new classad::ExprList();
        if ( jdl->EvaluateAttrList( "OutputSandboxDestURI", osbDUList ) ) {

	  m_log_dev->log(log4cpp::Priority::INFO,
		       "\tStarting OutputSandboxDestURI manipulation...");
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

		m_log_dev->log(log4cpp::Priority::DEBUG,
			     string("\t")+s+" became "+newPath);
                // Builds a new value
                classad::Value newV;
                newV.SetStringValue( newPath );
                // Builds the new string
                newOsbDUList->push_back( classad::Literal::MakeLiteral( newV ) );
            }
            jdl->Insert( "OutputSandboxDestURI", newOsbDUList );
        }
        return;
    }

    if ( 0 == jdl->Lookup( "OutputSandboxBaseDestURI" ) ) {
        // Put a default OutpuSandboxDestURI attribute
        jdl->InsertAttr( "OutputSandboxBaseDestURI",  default_osbdURI );
        return;
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

    m_log_dev->log(log4cpp::Priority::INFO,
		 string("iceCommandSubmit::pathName::CTOR() - Trying to unparse ")+ p );
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
    m_log_dev->log(log4cpp::Priority::DEBUG,
		 string("iceCommandSubmit::pathName::CTOR() - Unparsed as follows: filename=[")
		 +m_fileName + "] pathname={"
		 +m_pathName+"]");
}


} // namespace ice
} // namespace wms
} // namespace glite
