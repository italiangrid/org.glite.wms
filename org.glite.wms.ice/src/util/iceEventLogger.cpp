//
// This file is heavily based on org.glite.wms.jobsubmission/src/common/EventLogger.cpp
//
// See also:
// org.glite.lb.client-interface/build/producer.h
// org.glite.lb.client-interface/doc/C/latex/refman.pdf

#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <iostream>
#include <string>

#include <classad_distribution.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;

#include <openssl/pem.h>
#include <openssl/x509.h>

#include "glite/lb/producer.h"
#include "iceEventLogger.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"

#include <netdb.h>

using namespace std;
using namespace glite::wms::ice::util;
namespace configuration = glite::wms::common::configuration;

#ifdef DONT_COMPILE

namespace {

  // retrieve the subject_name from a given x509_proxy (thx to giaco)
  std::string get_proxy_subject(std::string const& x509_proxy)
  {
    static std::string const null_string;

    std::FILE* fd = std::fopen(x509_proxy.c_str(), "r");
    if (!fd) return null_string;
    boost::shared_ptr<std::FILE> fd_(fd, std::fclose);

    ::X509* const cert = ::PEM_read_X509(fd, 0, 0, 0);
    if (!cert) return null_string;
    boost::shared_ptr< ::X509> cert_(cert, ::X509_free);

    char* const s = ::X509_NAME_oneline(::X509_get_subject_name(cert), 0, 0);
    if (!s) return null_string;
    boost::shared_ptr<char> s_(s, ::free);

    return std::string(s);
  }

}

#endif

// static member definitions
unsigned int iceEventLogger::el_s_retries = 3;
unsigned int iceEventLogger::el_s_sleep = 60;
const char *iceEventLogger::el_s_notLogged = "Event not logged, context unset.";
const char *iceEventLogger::el_s_unavailable = "unavailable";
const char *iceEventLogger::el_s_OK = "OK";
const char *iceEventLogger::el_s_failed = "Failed";
iceEventLogger* iceEventLogger::_instance = 0;

//////////////////////////////////////////////////////////////////////////////
//
// iceLogger Exception
//
//////////////////////////////////////////////////////////////////////////////
iceLoggerException::iceLoggerException( const char *reason ) : le_reason( reason ? reason : "" )
{}

iceLoggerException::iceLoggerException( const string &reason ) : le_reason( reason )
{}

iceLoggerException::~iceLoggerException( void ) throw() {}

const char *iceLoggerException::what( void ) const throw()
{
  return this->le_reason.c_str();
}

//////////////////////////////////////////////////////////////////////////////
// 
// iceEventLogger
//
//////////////////////////////////////////////////////////////////////////////
iceEventLogger* iceEventLogger::instance( void )
{
    if ( 0 == _instance ) {
        _instance = new iceEventLogger( );
    }
    return _instance;
}

iceEventLogger::iceEventLogger( void ) :
    el_count( 0 ), 
    el_context( new edg_wll_Context ), 
    log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    el_s_localhost_name( )
{
    edg_wll_InitContext( el_context );
    char name[256];

    if ( gethostname(name, 256) < 0 ) {
        el_s_localhost_name = "(unknown host name )";
    } else {
        struct hostent *H = gethostbyname(name);
        if ( !H ) {
            el_s_localhost_name = "(unknown host name )";            
        } else {
            el_s_localhost_name = H->h_name;
        }
    }
}


iceEventLogger::~iceEventLogger( void )
{

}

string iceEventLogger::getLoggingError( const char *preamble )
{
  string       cause( preamble ? preamble : "" );

  if( preamble ) cause.append( 1, ' ' ); 

  char        *text, *desc;

  edg_wll_Error( *this->el_context, &text, &desc );
  cause.append( text );
  cause.append( " - " ); cause.append( desc );

  free( desc ); free( text );

  return cause;
}

void iceEventLogger::testCode( int &code, bool retry )
{
    if ( code != 0 ) {
        string err( getLoggingError( 0 ) );
        log_dev->errorStream() << "Got error " << err
                               << log4cpp::CategoryStream::ENDLINE;
        
    }

    const configuration::CommonConfiguration *conf = configuration::Configuration::instance()->common();
    int          ret;
    string       cause, host_proxy;

    if( code ) {
        cause = this->getLoggingError( NULL );

        switch( code ) {
        case EINVAL:
            log_dev->errorStream()
                << "Critical error in L&B calls: EINVAL. "
                << "Cause = \"" << cause << "\"."
                << log4cpp::CategoryStream::ENDLINE;
            
            code = 0; // Don't retry...
            break;
        case EDG_WLL_ERROR_GSS:
            log_dev->errorStream()
                << "Severe error in GSS layer while communicating with L&B daemons. " 
                << "Cause = \"" << cause << "\"." 
                << log4cpp::CategoryStream::ENDLINE;

            if( this->el_hostProxy ) {
                log_dev->infoStream()
                    << "The log with the host certificate has just been done. Giving up." 
                    << log4cpp::CategoryStream::ENDLINE;
                
                code = 0; // Don't retry...
            }
            else {
                host_proxy = conf->host_proxy_file();

                log_dev->infoStream()
                    << "Retrying using host proxy certificate [" 
                    << host_proxy << "]" 
                    << log4cpp::CategoryStream::ENDLINE;


                if( host_proxy.length() == 0 ) {
                    log_dev->warnStream()
                        << "Host proxy file not set inside configuration file. " 
                        << "Trying with a default NULL and hoping for the best." 
                        << log4cpp::CategoryStream::ENDLINE;

                    ret = edg_wll_SetParam( *el_context, EDG_WLL_PARAM_X509_PROXY, NULL );
                }
                else {
                    log_dev->infoStream()
                        << "Host proxy file found = [" << host_proxy << "]."
                        << log4cpp::CategoryStream::ENDLINE;

                    ret = edg_wll_SetParam( *el_context, EDG_WLL_PARAM_X509_PROXY, host_proxy.c_str() );
                }

                if( ret ) {
                    log_dev->errorStream()
                        << "Cannot set the host proxy inside the context. Giving up." 
                        << log4cpp::CategoryStream::ENDLINE;

                    code = 0; // Don't retry.
                }
                else this->el_hostProxy = true; // Set and retry (code is still != 0)
            }

            break;
        default:
            if( ++this->el_count > el_s_retries ) {
                log_dev->errorStream()
                    << "L&B call retried " << this->el_count << " times always failed. "
                    << "Ignoring." 
                    << log4cpp::CategoryStream::ENDLINE;

                code = 0; // Don't retry anymore
            }
            else {
                log_dev->warnStream()
                    << "L&B call got a transient error. Waiting " << el_s_sleep << " seconds and trying again. " 
                    << "Try n. " << this->el_count << "/" << el_s_retries 
                    << log4cpp::CategoryStream::ENDLINE;

                sleep( el_s_sleep );
            }
            break;
        }
    }
    else // The logging call worked fine, do nothing
        log_dev->debugStream() 
            << "L&B call succeeded." 
            << log4cpp::CategoryStream::ENDLINE;

    // SignalChecker::instance()->throw_on_signal();

    return;

}

void iceEventLogger::registerJob( const util::CreamJob& theJob )
{
    int res;
    edg_wlc_JobId   id;

    setLoggingJob( theJob, EDG_WLL_SOURCE_JOB_SUBMISSION );

    log_dev->infoStream() 
        << "Registering jobid=[" << theJob.getGridJobID() << "]"
        << log4cpp::CategoryStream::ENDLINE;
    
    edg_wlc_JobIdParse( theJob.getGridJobID().c_str(), &id );

    res = edg_wll_RegisterJob( *el_context, id, EDG_WLL_JOB_SIMPLE, theJob.getJDL().c_str(), theJob.getEndpoint().c_str(), 0, 0, 0 );
    edg_wlc_JobIdFree( id );
    if( res != 0 ) {
        log_dev->errorStream() 
            << "Cannot register jobid=[" << theJob.getGridJobID()
            << "]. LB error code=" << res
            << log4cpp::CategoryStream::ENDLINE;
    }
}


void iceEventLogger::setLoggingJob( const util::CreamJob& theJob, edg_wll_Source src ) throw ( iceLoggerException& )
{
    edg_wlc_JobId   id;
    int res = 0;

    res = edg_wlc_JobIdParse( theJob.getGridJobID().c_str(), &id );    

    char *lbserver;
    unsigned int lbport;
    edg_wlc_JobIdGetServerParts( id, &lbserver, &lbport );
    log_dev->infoStream() 
        << "iceEventLogger::setLoggingJob: "
        << "Setting log job to jobid=[" << theJob.getGridJobID() << "] "
        << "LB server=[" << lbserver << ":" << lbport << "] "
        << "(port is not used, actually...)"
        << log4cpp::CategoryStream::ENDLINE;
    res |= edg_wll_SetParam( *el_context, EDG_WLL_PARAM_SOURCE, src );        
    res |= edg_wll_SetParam( *el_context, EDG_WLL_PARAM_DESTINATION, lbserver );
    if ( lbserver ) free( lbserver );

    if ( !theJob.getSequenceCode().empty() ) {
        res |= edg_wll_SetLoggingJob( *el_context, id, theJob.getSequenceCode().c_str(), EDG_WLL_SEQ_NORMAL );
    }

    edg_wlc_JobIdFree( id );

    if( res != 0 ) {
        log_dev->errorStream()
            << "iceEventLogger::setLoggingJob: "
            << "Unable to set logging job to jobid=["
            << theJob.getGridJobID()
            << "]. LB error is "
            << getLoggingError( 0 )
            << log4cpp::CategoryStream::ENDLINE;
        throw iceLoggerException( this->getLoggingError("Cannot set logging job:") );
    }

    // Set user proxy for L&B stuff
    fs::path    pf( theJob.getUserProxyCertificate(), fs::native);
    pf.normalize();

    if( fs::exists(pf) ) {

        res = edg_wll_SetParam( *el_context, EDG_WLL_PARAM_X509_PROXY, 
                                theJob.getUserProxyCertificate().c_str() );

        if( res ) {
            log_dev->errorStream()
                << "iceEventLogger::setLoggingJob: "
                << "Unable to set logging job to jobid=["
                << theJob.getGridJobID()
                << "]. "
                << getLoggingError( 0 )
                << log4cpp::CategoryStream::ENDLINE;            
            throw iceLoggerException( this->getLoggingError("Cannot set proxyfile path inside context:") );
        }
    } else {
        log_dev->errorStream()
            << "iceEventLogger::setLoggingJob: "
            << "Unable to set logging job to jobid=["
            << theJob.getGridJobID()
            << "]. Proxy file "
            << theJob.getUserProxyCertificate()
            << " does not exist."
            << log4cpp::CategoryStream::ENDLINE;

    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Transfer Start event
//
//////////////////////////////////////////////////////////////////////////////
void iceEventLogger::cream_transfer_start_event( const util::CreamJob& theJob )
{
    int           res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_transfer_start_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging CREAM transfer start, dest_host = " 
            << theJob.getCreamURL()
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogTransferSTART( *el_context, 
                                        EDG_WLL_SOURCE_LRMS, 
                                        theJob.getCreamURL().c_str(),
                                        el_s_unavailable,
                                        theJob.getJDL().c_str(),  
                                        el_s_unavailable,
                                        el_s_unavailable );
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        testCode( res );
    } while( res != 0 );        
    
    return;
}

//////////////////////////////////////////////////////////////////////////////
//
// transfer OK event
//
//////////////////////////////////////////////////////////////////////////////
void iceEventLogger::cream_transfer_ok_event( const util::CreamJob& theJob )
{
    int           res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_transfer_ok_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging cream transfer OK event, "
            << " jobid=" << theJob.getGridJobID()
            << " dest_host=" << theJob.getCreamURL()
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogTransferOK( *el_context, 
                                     EDG_WLL_SOURCE_LRMS, 
                                     theJob.getCreamURL().c_str(),
                                     el_s_unavailable,
                                     theJob.getJDL().c_str(),  
                                     el_s_unavailable,
                                     theJob.getJobID().c_str() );
        
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        
        testCode( res );
    } while( res != 0 );        
    
    return;
}

//////////////////////////////////////////////////////////////////////////////
//
// transfer FAIL event
//
//////////////////////////////////////////////////////////////////////////////
void iceEventLogger::cream_transfer_fail_event( const util::CreamJob& theJob, const string& reason )
{
    int           res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_transfer_fail_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging Transfer FAIL event, "
            << " jobid=" << theJob.getGridJobID()
            << " dest_host=" << theJob.getCreamURL()
            << " reason=" << reason
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogTransferFAIL( *el_context, 
                                       EDG_WLL_SOURCE_LRMS, 
                                       theJob.getCreamURL().c_str(),
                                       el_s_unavailable,
                                       theJob.getJDL().c_str(),  
                                       reason.c_str(), 
                                       el_s_unavailable );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        testCode( res );
    } while( res != 0 );        
    
    return;
}

//////////////////////////////////////////////////////////////////////////////
//
// cream accepted event
//
//////////////////////////////////////////////////////////////////////////////
void iceEventLogger::cream_accepted_event( const util::CreamJob& theJob )
{
    // FIXME: which is the source? which is the destination?
    int           res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_accepted_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging CREAM Accepted event, "
            << " jobid=[" << theJob.getGridJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogAccepted( *el_context, 
                                   EDG_WLL_SOURCE_JOB_SUBMISSION, 
                                   el_s_localhost_name.c_str(),
                                   el_s_unavailable,
                                   theJob.getJobID().c_str()
                                   );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        testCode( res );
    } while( res != 0 );        
    
    return;
}

//////////////////////////////////////////////////////////////////////////////
//
// lrms accepted event
//
//////////////////////////////////////////////////////////////////////////////
void iceEventLogger::lrms_accepted_event( const util::CreamJob& theJob )
{
    int           res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging lrms_accepted_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging LRMS Accepted event, "
            << " jobid=[" << theJob.getGridJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogAccepted( *el_context, 
                                   EDG_WLL_SOURCE_LRMS,
                                   theJob.getCEID().c_str(),
                                   el_s_unavailable,
                                   theJob.getJobID().c_str()
                                   );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        testCode( res );
    } while( res != 0 );        
    
    return;
}


//////////////////////////////////////////////////////////////////////////////
//
// accepted event
//
//////////////////////////////////////////////////////////////////////////////
void iceEventLogger::cream_refused_event( const util::CreamJob& theJob, const std::string& reason )
{
    int           res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_refused_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging Refused event, "
            << " jobid=[" << theJob.getGridJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogRefused( *el_context, 
                                  EDG_WLL_SOURCE_JOB_SUBMISSION,
                                  el_s_unavailable,
                                  el_s_unavailable,
                                  reason.c_str() );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        testCode( res );
    } while( res != 0 );        
    
    return;
}

//////////////////////////////////////////////////////////////////////////////
//
// cancel_request event
//
//////////////////////////////////////////////////////////////////////////////
void iceEventLogger::cream_cancel_request_event( const util::CreamJob& theJob )
{
    int           res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_cancel_request_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging Cancel Request event, "
            << " jobid=[" << theJob.getGridJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogCancelREQ( *el_context, 
                                    el_s_unavailable 
                                    );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        testCode( res );
    } while( res != 0 );        
    
    return;
}


//////////////////////////////////////////////////////////////////////////////
//
// cancel refuse event
//
//////////////////////////////////////////////////////////////////////////////
void iceEventLogger::cream_cancel_refuse_event( const util::CreamJob& theJob, const string& reason )
{
    int           res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_cancel_refuse_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging Cancel Refuse event, "
            << " jobid=[" << theJob.getGridJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogCancelREFUSE( *el_context, 
                                       reason.c_str()
                                       );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        testCode( res );
    } while( res != 0 );        
    
    return;
}

void iceEventLogger::job_running_event( const util::CreamJob& theJob, const string& host )
{
    int res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging job_running_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();

    do {
        log_dev->infoStream() 
            << "Logging job running event, host = " << host
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogRunning( *el_context, host.c_str() );
        
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        
        this->testCode( res );
    } while( res != 0 );        
    
    return;
}

void iceEventLogger::job_cancelled_event( const util::CreamJob& theJob )
{
    int res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging job_cancelled_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();

    do {
        log_dev->infoStream() 
            << "Logging job cancelled event..."
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogDoneCANCELLED( *el_context, el_s_unavailable, 0 ); // FIXME
        
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        
        this->testCode( res );
    } while( res != 0 );        
    
    return;
}

void iceEventLogger::job_suspended_event( const util::CreamJob& theJob )
{
    int res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging job_suspended_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();

    do {
        log_dev->infoStream() 
            << "Logging job suspended event..."
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogSuspend( *el_context, el_s_unavailable );
        
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        
        this->testCode( res );
    } while( res != 0 );        
    
    return;
}

void iceEventLogger::job_done_ok_event( const util::CreamJob& theJob )
{
    int res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging job_done_ok_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();

    do {
        log_dev->infoStream() 
            << "Logging job done_ok event..."
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogDoneOK( *el_context, el_s_unavailable, 0 ); // FIXME
        
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        
        this->testCode( res );
    } while( res != 0 );        
    
    return;
}

void iceEventLogger::job_done_failed_event( const util::CreamJob& theJob )
{
    int res;

    try {
        setLoggingJob( theJob, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLoggerException& ex ) {
        log_dev->errorStream()
            << "Error logging job_done_failed_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    startLogging();

    do {
        log_dev->infoStream() 
            << "Logging job done_failed event..."
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogDoneFAILED( *el_context, el_s_unavailable, 0 ); // FIXME
        
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        
        this->testCode( res );
    } while( res != 0 );        
    
    return;
}


//////////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////////
void iceEventLogger::log_job_status_change( const CreamJob& theJob )
{
    switch( theJob.getStatus() ) {
    case glite::ce::cream_client_api::job_statuses::REGISTERED:
        // FIXME: should never occur in ice, as we register with autostart
        break;
    case glite::ce::cream_client_api::job_statuses::PENDING:
        cream_accepted_event( theJob );
        break;
    case glite::ce::cream_client_api::job_statuses::IDLE:
        lrms_accepted_event( theJob );
        break;
    case glite::ce::cream_client_api::job_statuses::RUNNING:
        job_running_event( theJob, string( el_s_unavailable ) ); // FIXME
        break;
    case glite::ce::cream_client_api::job_statuses::CANCELLED:
        job_cancelled_event( theJob );
        break;
    case glite::ce::cream_client_api::job_statuses::HELD:
        job_suspended_event( theJob );
        break;
    case glite::ce::cream_client_api::job_statuses::ABORTED:
        job_done_failed_event( theJob );
        break;
    case glite::ce::cream_client_api::job_statuses::DONE_OK:
        job_done_ok_event( theJob );
        break;
    case glite::ce::cream_client_api::job_statuses::DONE_FAILED:
        job_done_failed_event( theJob );
        break;
    case glite::ce::cream_client_api::job_statuses::UNKNOWN:
        // FIXME??
        break;
    case glite::ce::cream_client_api::job_statuses::NA:
        // FIXME??
        break;

    }
}
