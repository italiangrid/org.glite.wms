// Local includes
#include "iceCommandSubmit.h"
#include "subscriptionCache.h"
#include "subscriptionManager.h"
#include "iceConfManager.h"
#include "jobCache.h"
#include "creamJob.h"
#include "eventStatusListener.h"
#include "iceEventLogger.h"

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

using namespace glite::wms::ice;
using namespace std;

namespace ceurl_util = glite::ce::cream_client_api::util::CEUrl;
namespace cream_api = glite::ce::cream_client_api;
namespace api_util = glite::ce::cream_client_api::util;
namespace ice_util = glite::wms::ice::util;

//______________________________________________________________________________
iceCommandSubmit::iceCommandSubmit( const string& request )
  throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) :
  iceAbsCommand( ),
  log_dev( api_util::creamApiLogger::instance()->getLogger()),
  confMgr( ice_util::iceConfManager::getInstance()), // no need of mutex here because the getInstance does that
  //ceS(),
  //T( confMgr->getICETopic()),
  //P(5000),
  _ev_logger( ice_util::iceEventLogger::instance() )
{
    char name[256];
    memset((void*)name, 0, 256);
    if(gethostname(name, 256) == -1)
    {
        log_dev->fatalStream() << "iceCommandSubmit::CTOR - Couldn't resolve local hostname: "
                               << strerror(errno)
                               << log4cpp::CategoryStream::ENDLINE;

      exit(1);
    }
    struct hostent *H = gethostbyname(name);
    if(!H) {
        log_dev->fatalStream() << "iceCommandSubmit::CTOR() - Couldn't resolve local hostname: "
                               << strerror(h_errno)
                               << log4cpp::CategoryStream::ENDLINE;
        exit(1);
    }
    
    myname_url = boost::str( boost::format("http://%1%:%2%") % H->h_name % confMgr->getListenerPort() );

    // Sample classad:
    // ( this is the NEW version )
    //  [ requirements = ( ( ( ( other.GlueCEInfoHostName == "lxb2022.cern.ch" ) ) ) && ( other.GlueCEStateStatus == "Production" ) ) && ( other.GlueCEStateStatus == "Production" );
    //     RetryCount = 0;
    //     ce_id = "grid005.pd.infn.it:8443/cream_lsf_grid002";
    //     edg_jobid = "https://gundam.cnaf.infn.it:9000/ZLM4zE60OqHIcqH_PNbs-w";
    //     Arguments = "136100001 herrala_27411 360 20";
    //     OutputSandboxPath = "/var/glitewms/SandboxDir/ZL/https_3a_2f_2fgundam.cnaf.infn.it_3a9000_2fZLM4zE60OqHIcqH_5fPNbs-w/output";
    //     JobType = "normal";
    //     Executable = "glite-dev.tt_ch_160_tb20.testrun.sh.sh";
    //     CertificateSubject = "/O=Grid/O=NorduGrid/OU=hip.fi/CN=Juha Herrala";
    //     StdOutput = "cstdout";
    //     X509UserProxy = "/var/glitewms/SandboxDir/ZL/https_3a_2f_2fgundam.cnaf.infn.it_3a9000_2fZLM4zE60OqHIcqH_5fPNbs-w/user.proxy";
    //     OutputSandbox = { "cstdout","cstderr","testrun.log","testrun.root" };
    //     LB_sequence_code = "UI=000003:NS=0000000003:WM=000000:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000";
    //     InputSandboxPath = "/var/glitewms/SandboxDir/ZL/https_3a_2f_2fgundam.cnaf.infn.it_3a9000_2fZLM4zE60OqHIcqH_5fPNbs-w/input";
    //     VirtualOrganisation = "EGEE";
    //     rank =  -other.GlueCEStateEstimatedResponseTime;
    //     Type = "job";
    //     StdError = "cstderr";
    //     DefaultRank =  -other.GlueCEStateEstimatedResponseTime;
    //     InputSandbox = { "ExeTar.tt_ch_160_tb20.testrun.sh","glite-dev.tt_ch_160_tb20.testrun.sh.sh" } ] ]

    classad::ClassAdParser parser;
    classad::ClassAd *_rootAD = parser.ParseClassAd( request );

    if (!_rootAD)
        throw ice_util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request by iceCommandSubmit");

    string _commandStr;
    // Parse the "command" attribute
    if ( !_rootAD->EvaluateAttrString( "command", _commandStr ) ) {
        throw ice_util::JobRequest_ex("attribute 'command' not found or is not a string");
    }
    boost::trim_if( _commandStr, boost::is_any_of("\"") );

    if ( 0 != _commandStr.compare( "jobsubmit" ) ) {
        throw ice_util::JobRequest_ex("wrong command ["+_commandStr+"] parsed by iceCommandSubmit" );
    }

    string _versionStr;
    // Parse the "version" attribute
    if ( !_rootAD->EvaluateAttrString( "version", _versionStr ) ) {
        throw ice_util::JobRequest_ex("attribute 'version' not found or is not a string");
    }
    // Check if the version is exactly 1.0.0
    if ( _versionStr.compare("1.0.0") ) {
        throw ice_util::JobRequest_ex("Wrong \"version\" for jobRequest: expected 1.0.0, got " + _versionStr );
    }

    classad::ClassAd *_argumentsAD = 0;
    // Parse the "arguments" attribute
    if ( !_rootAD->EvaluateAttrClassAd( "arguments", _argumentsAD ) ) {
        throw ice_util::JobRequest_ex("attribute 'arguments' not found or is not a classad");
    }

    classad::ClassAd *_adAD = 0;
    // Look for "ad" attribute inside "arguments"
    if ( !_argumentsAD->EvaluateAttrClassAd( "ad", _adAD ) ) {
        throw ice_util::JobRequest_ex("Attribute 'ad' not found inside 'arguments', or is not a classad" );
    }

    // initializes the _jdl attribute
    classad::ClassAdUnParser unparser;
    unparser.Unparse( _jdl, _argumentsAD->Lookup( "ad" ) );

}

//______________________________________________________________________________
void iceCommandSubmit::execute( ice* _ice ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& )
{
    log_dev->log(log4cpp::Priority::INFO, "iceCommandSubmit::execute() - This request is a Submission...");

    vector<string> url_jid;
    ice_util::CreamJob theJob;
    cream_api::soap_proxy::CreamProxy* theProxy = cream_api::soap_proxy::CreamProxyFactory::getProxy();

    try {
        theJob.setJdl( _jdl );
        theJob.setStatus( cream_api::job_statuses::UNKNOWN, time(NULL) );
    } catch( ice_util::ClassadSyntax_ex& ex ) {
        log_dev->errorStream() 
            << "Cannot instantiate a job from jdl="
            << _jdl
            << " due to classad excaption: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        throw( iceCommandFatal_ex( ex.what() ) );
        // TODO: L&B?
    }

    log_dev->infoStream() << "iceCommandSubmit::execute() - Registering "
                          << "gridJobID=\"" << theJob.getGridJobID()
                          << "\" to L&B service with user proxy=\"" 
                          << theJob.getUserProxyCertificate() 
                          << "\""
                          << log4cpp::CategoryStream::ENDLINE;

    _ev_logger->registerJob( theJob ); // FIXME: to be removed
    _ev_logger->cream_transfer_start_event( theJob );

    string modified_jdl;
    try {    
        modified_jdl = creamJdlHelper( _jdl );
    } catch( ice_util::ClassadSyntax_ex& ex ) {
        log_dev->errorStream() << "Cannot convert jdl="
                               << _jdl
                               << " due to classad exception:"
                               << ex.what()
                               << log4cpp::CategoryStream::ENDLINE;
        _ev_logger->cream_transfer_fail_event( theJob, ex.what() );
        throw( iceCommandFatal_ex( ex.what() ) );
    }
    
    log_dev->log(log4cpp::Priority::INFO, "iceCommandSubmit::execute() - Submitting");
    log_dev->debugStream() << "JDL " << modified_jdl
                           << " to [" << theJob.getCreamURL() <<"]["
                           << theJob.getCreamDelegURL() << "]"
                           << log4cpp::CategoryStream::ENDLINE;

    try {
        theProxy->Authenticate(theJob.getUserProxyCertificate());
    } catch ( cream_api::soap_proxy::auth_ex& ex ) {
        _ev_logger->cream_transfer_fail_event( theJob, ex.what() );
        log_dev->errorStream()
            << "Unable to submit gridJobID=" << theJob.getGridJobID()
            << " due to authentication error:" << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        throw( iceCommandFatal_ex( ex.what() ) );
    }
    
    { 
        // lock the listener:
        // this prevents the eventStatusListener::acceptJobStatus()
        // to process a notification of a just submitted job that is not
        // yet present in the jobCache
        boost::recursive_mutex::scoped_lock lockAccept( util::eventStatusListener::mutexJobStatusUpdate );
        try {
            theProxy->Register(
                               theJob.getCreamURL().c_str(),
                               theJob.getCreamDelegURL().c_str(),
                               "", // deleg ID not needed because this client
                               // will always do auto_delegation
                               modified_jdl,
                               theJob.getUserProxyCertificate(),
                               url_jid,
			       -1,
                               true /*autostart*/
                               );
        } catch( exception& ex ) {
            log_dev->errorStream()
                << "Cannot register jobID="
                << theJob.getGridJobID()
                << " Exception:" << ex.what()
                << log4cpp::CategoryStream::ENDLINE;
            _ev_logger->cream_transfer_fail_event( theJob, ex.what() );
            throw( iceCommandFatal_ex( ex.what() ) );
        }

        log_dev->infoStream()
            << "iceCommandSubmit::execute() - Returned CREAM-JOBID ["
            << url_jid[1] <<"]"
            << log4cpp::CategoryStream::ENDLINE;
        
        // no failure: put jobids and status in cache
        // and remove last request from WM's filelist

        theJob.setJobID(url_jid[1]);
        theJob.setStatus(cream_api::job_statuses::PENDING, time(NULL) );

        _ev_logger->cream_transfer_ok_event( theJob );
        _ev_logger->cream_accepted_event( theJob );

        boost::recursive_mutex::scoped_lock M( util::jobCache::mutex );

        // put(...) accepts arg by reference, but
        // the implementation puts the arg in the memory hash by copying it. So
        // passing a *pointer should not produce problems
        util::jobCache::getInstance()->put( theJob );
    } // this end-scope unlock the listener that now can

    /**
     * here must check if we're subscribed to the CEMon service
     * in order to receive the status change notifications
     * of job just submitted. But only if listener is ON
     */
    /* ....... */

    bool _tmp_start_listener;
    {
      boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
      _tmp_start_listener = confMgr->getStartListener();
    }

    if( _tmp_start_listener ) {
      string cemon_url = confMgr->getCEMonUrlPrefix() + theJob.getEndpoint()
                         + confMgr->getCEMonUrlPostfix();
      if( !util::subscriptionCache::getInstance()->has(cemon_url) ) {
        /* MUST SUBSCRIBE TO THIS CEMON */
        log_dev->infoStream()
            << "iceCommandSubmit::execute() - Not subscribed to ["
            << cemon_url << "]. Going to subscribe to it..."
            << log4cpp::CategoryStream::ENDLINE;
        //try {

            /* ceS.authenticate(confMgr->getHostProxyFile().c_str(), "/");
            ceS.setServiceURL(cemon_url);
            ceS.setSubscribeParam(myname_url.c_str(),
                                  T,
                                  P,
                                  confMgr->getSubscriptionDuration()
                                  );
	    */

            log_dev->infoStream()
                << "iceCommandSubmit::execute() - Subscribing the consumer ["
                << myname_url << "] to ["<<cemon_url
                << "] with duration="
                << confMgr->getSubscriptionDuration()
                << " secs"
                << log4cpp::CategoryStream::ENDLINE;
            {
              /**
  	       * This is the 1st call of subscriptionManager::getInstance()
  	       * and it's safe because the singleton has already been created by
	       * ice-core module.
	       */
	      boost::recursive_mutex::scoped_lock M( util::subscriptionManager::mutex );
	      if( !util::subscriptionManager::getInstance()->subscribe(cemon_url) )
	      {
                log_dev->errorStream()
                  << "iceCommandSubmit::execute() - Subscribe to ["
                  << cemon_url << "] failed! Will not receive status notifications from it..."
                  << log4cpp::CategoryStream::ENDLINE;
	      } else {
                log_dev->infoStream()
                  << "iceCommandSubmit::execute() - Subscribed with ID ["
                  << util::subscriptionManager::getInstance()->getLastSubscriptionID() << "]"
                  << log4cpp::CategoryStream::ENDLINE;

		{
		  boost::recursive_mutex::scoped_lock M( util::subscriptionCache::mutex );
	          util::subscriptionCache::getInstance()->insert(cemon_url);
		}
              }
	    }
            //glite::wms::ice::util::subscriptionCache::getInstance()->insert(cemon_url);
//         } catch( exception& ex ) {
//             log_dev->errorStream()
//                 << "Problem while subscribing to notifications for jobID="
//                 << theJob.getGridJobID()
//                 << " Exception:" << ex.what()
//                 << log4cpp::CategoryStream::ENDLINE;
//         }
      }
    }
}

//______________________________________________________________________________
string iceCommandSubmit::creamJdlHelper( const string& oldJdl ) throw( ice_util::ClassadSyntax_ex& )
{
    classad::ClassAdParser parser;
    classad::ClassAd *_root = parser.ParseClassAd( oldJdl );

    if ( !_root ) {
        throw ice_util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");
    }

    string ceid;
    if ( !_root->EvaluateAttrString( "ce_id", ceid ) ) {
        throw ice_util::ClassadSyntax_ex( "ce_id attribute not found" );
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
	log_dev->log(log4cpp::Priority::INFO,"\tStarting InputSandbox manipulation...");
        //cout << "Starting InputSandbox manipulation..." << endl;
	//	log_dev
	
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
	    log_dev->log(log4cpp::Priority::DEBUG,
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
	  //            cout << "Starting OutputSandboxDestURI manipulation..." << endl;
	  log_dev->log(log4cpp::Priority::INFO,
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
                //cout << s << " became " << newPath << endl;
		log_dev->log(log4cpp::Priority::DEBUG,
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
  log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()),
  _fullName( p ),
  _pathType( invalid )
{
    boost::regex uri_match( "gsiftp://[^/]+(:[0-9]+)?/([^/]+/)*([^/]+)" );
    boost::regex rel_match( "([^/]+/)*([^/]+)" );
    boost::regex abs_match( "(file://)?/([^/]+/)*([^/]+)" );
    boost::smatch what;

    //cout << "Trying to unparse " << p << endl;
    log_dev->log(log4cpp::Priority::INFO,
		 string("iceCommandSubmit::pathName::CTOR() - Trying to unparse ")+ p );
    if ( boost::regex_match( p, what, uri_match ) ) {
        // is a uri
        _pathType = uri;

        _fileName = '/';
        _fileName.append(what[3].first,what[3].second);
        if ( what[2].first != p.end() )
            _pathName.assign(what[2].first,what[2].second);
        _pathName.append( _fileName );
    } else if ( boost::regex_match( p, what, rel_match ) ) {
        // is a relative path
        _pathType = relative;

        _fileName.assign(what[2].first,what[2].second);
        if ( what[1].first != p.end() )
            _pathName.assign( what[1].first, what[1].second );
        _pathName.append( _fileName );
    } else if ( boost::regex_match( p, what, abs_match ) ) {
        // is an absolute path
        _pathType = absolute;
        
        _pathName = '/';
        _fileName.assign( what[3].first, what[3].second );
        if ( what[2].first != p.end() ) 
            _pathName.append( what[2].first, what[2].second );
        _pathName.append( _fileName );
    }
    log_dev->log(log4cpp::Priority::DEBUG,
		 string("iceCommandSubmit::pathName::CTOR() - Unparsed as follows: filename=[")
		 +_fileName + "] pathname={"
		 +_pathName+"]");
}

