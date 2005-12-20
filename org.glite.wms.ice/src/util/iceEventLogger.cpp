//
// This file is heavily based on org.glite.wms.jobsubmission/src/common/EventLogger.cpp
//

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
#include "glite/lb/context.h"
#include "iceEventLogger.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

//#include "glite/wms/common/configuration/Configuration.h"
//#include "glite/wms/common/configuration/CommonConfiguration.h"
//#include "glite/wms/common/logger/manipulators.h"
//#include "glite/wms/common/logger/edglog.h"

//#include "SignalChecker.h"
//#include "EventLogger.h"

using namespace std;
using namespace glite::wms::ice::util;

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
    code = 0; // FIXME: always don't retry
#ifdef DONT_COMPILE
  const configuration::CommonConfiguration     *conf = configuration::Configuration::instance()->common();
  int          ret;
  string       cause, host_proxy;

  if( code ) {
    cause = this->getLoggingError( NULL );

    switch( code ) {
    case EINVAL:
      ts::edglog << logger::setlevel( logger::critical )
		 << "Critical error in L&B calls: EINVAL." << endl
		 << "Cause = \"" << cause << "\"." << endl;

      code = 0; // Don't retry...
      break;
    case EDG_WLL_ERROR_GSS:
      ts::edglog << logger::setlevel( logger::severe )
		 << "Severe error in GSS layer while communicating with L&B daemons." << endl
		 << "Cause = \"" << cause << "\"." << endl;

      if( this->el_hostProxy ) {
	ts::edglog << "The log with the host certificate has just been done. Giving up." << endl;

	code = 0; // Don't retry...
      }
      else {
	ts::edglog << logger::setlevel( logger::info )
		   << "Retrying using host proxy certificate..." << endl;

	host_proxy = conf->host_proxy_file();

	if( host_proxy.length() == 0 ) {
	  ts::edglog << logger::setlevel( logger::warning )
		     << "Host proxy file not set inside configuration file." << endl
		     << "Trying with a default NULL and hoping for the best." << endl;

	  ret = edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_X509_PROXY, NULL );
	}
	else {
	  ts::edglog << logger::setlevel( logger::info )
		     << "Host proxy file found = \"" << host_proxy << "\"." << endl;

	  ret = edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_X509_PROXY, host_proxy.c_str() );
	}

	if( ret ) {
	  ts::edglog << logger::setlevel( logger::severe )
		     << "Cannot set the host proxy inside the context. Giving up." << endl;

	  code = 0; // Don't retry.
	}
	else this->el_hostProxy = true; // Set and retry (code is still != 0)
      }

      break;
    default:
      if( ++this->el_count > el_s_retries ) {
	ts::edglog << logger::setlevel( logger::error )
		   << "L&B call retried " << this->el_count << " times always failed." << endl
		   << "Ignoring." << endl;

	code = 0; // Don't retry anymore
      }
      else {
	ts::edglog << logger::setlevel( logger::warning )
		   << "L&B call got a transient error. Waiting " << el_s_sleep << " seconds and trying again." << endl
		   << logger::setlevel( logger::info )
		   << "Try n. " << this->el_count << "/" << el_s_retries << endl;

	sleep( el_s_sleep );
      }
      break;
    }
  }
  else // The logging call worked fine, do nothing
    ts::edglog << logger::setlevel( logger::debug ) << "L&B call succeeded." << endl;

  SignalChecker::instance()->throw_on_signal();

  return;
#endif

}

iceEventLogger::iceEventLogger( void ) throw ( iceLoggerException& ) : 
    el_remove( true ),
    el_flag( EDG_WLL_SEQ_NORMAL ),
    el_count( 0 ), 
    el_context( NULL ), 
    el_proxy( ),
    log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{
  this->el_context = new edg_wll_Context;

  if( edg_wll_InitContext(this->el_context) )
    throw iceLoggerException( "Cannot initialize logging context" );
}

#ifdef DONT_COMPILE
iceEventLogger::iceEventLogger( edg_wll_Context *cont, int flag ) : 
    el_remove( false ), 
    el_hostProxy( false ),
    el_flag( flag ), 
    el_count( 0 ), 
    el_context( cont ),
    el_proxy( ),
    log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{

}
#endif

iceEventLogger::~iceEventLogger( void )
{
  if( this->el_context && this->el_remove ) {
    edg_wll_FreeContext( *this->el_context );

    delete this->el_context;
  }
}


iceEventLogger &iceEventLogger::initialize_ice_context( ProxySet *ps ) throw ( iceLoggerException& )
{
  int   res = 0;

  if( this->el_context ) {
      log_dev->infoStream() << "Initializing ICE L&B context"
                            << log4cpp::CategoryStream::ENDLINE;

    res = edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_SOURCE, EDG_WLL_SOURCE_JOB_SUBMISSION );
    res != edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_DESTINATION, "grid005.pd.infn.it" ); // FIXME: Remove hardcoded default
    res |= edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_INSTANCE, "unique" );

    if( ps ) {
        log_dev->infoStream() << "Setting L&B Proxy to "
                              << ( ps->ps_x509Proxy ? ps->ps_x509Proxy : "(null)" )
                              << log4cpp::CategoryStream::ENDLINE;

        res |= edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_X509_PROXY, ps->ps_x509Proxy ); 

        log_dev->infoStream() << "Setting L&B Key to "
                              << ( ps->ps_x509Key ? ps->ps_x509Key : "(null)" )
                              << log4cpp::CategoryStream::ENDLINE;

        res |= edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_X509_KEY, ps->ps_x509Key );

        log_dev->infoStream() << "Setting L&B Cert to "
                              << ( ps->ps_x509Cert ? ps->ps_x509Cert : "(null)" )
                              << log4cpp::CategoryStream::ENDLINE;

        res |= edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_X509_CERT, ps->ps_x509Cert ); 
    }

    if( res ) throw iceLoggerException( "Invalid context parameter setting." );
  }

  return *this;
}


iceEventLogger &iceEventLogger::reset_user_proxy( const string &proxyfile )
{
  bool    erase = false;
  int     res;

  if( proxyfile.size() && (proxyfile != this->el_proxy) ) {
      // fs::path    pf(fs::normalize_path(proxyfile), fs::native);
      fs::path    pf( proxyfile, fs::native);
      pf.normalize();

    if( fs::exists(pf) ) {
      this->el_proxy.assign( proxyfile );

      res = edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_X509_PROXY, proxyfile.c_str() );

      if( res ) throw iceLoggerException( this->getLoggingError("Cannot set proxyfile path inside context:") );
    }
    else erase = true;
  }
  else if( proxyfile.size() == 0 ) erase = true;

  if( erase ) {
    this->el_proxy.erase();

    res = edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_X509_PROXY, NULL );

    if( res ) throw iceLoggerException( this->getLoggingError("Cannot reset proxyfile path inside context:") );
  }

  return *this;
}

#ifdef DONT_COMPILE
iceEventLogger &iceEventLogger::reset_context( const string &jobid, const string &sequence ) throw( iceLoggerException& )
{
  int             res;
  edg_wlc_JobId   id;

  if( this->el_context ) {
    edg_wlc_JobIdParse( jobid.c_str(), &id );
    res = edg_wll_SetLoggingJob( *this->el_context, id, sequence.c_str(), this->el_flag );
    edg_wlc_JobIdFree( id );
    if( res != 0 ) throw iceLoggerException( this->getLoggingError("Cannot reset logging context:") );
  }

  return *this;
}
#endif


#ifdef DONT_COMPILE
iceEventLogger &iceEventLogger::set_ice_context( const string &jobid, const string &sequence, const string &proxyfile)
{ 
  bool              erase = false;
  int               res;
  edg_wlc_JobId     id;
                                                                                
  if( proxyfile.size() && (proxyfile != this->el_proxy) ) {
      // fs::path    pf( fs::normalize_path(proxyfile), fs::native );
      fs::path    pf( proxyfile, fs::native );
      pf.normalize();

    if( fs::exists(pf) ) {
      this->el_proxy.assign( proxyfile );
      
      res = edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_X509_PROXY, proxyfile.c_str() );

      if( res ) throw iceLoggerException( this->getLoggingError("Cannot set proxyfile path inside context:") );
    }
    else erase = true;
  }
  else if( proxyfile.size() == 0 ) erase = true;
                                                                                
  if( erase ) {
    this->el_proxy.erase();
                                                                                
    res = edg_wll_SetParam( *this->el_context, EDG_WLL_PARAM_X509_PROXY, NULL );  
    if( res ) throw iceLoggerException( this->getLoggingError("Cannot reset proxyfile path inside context:") );
  }
                                                                                
  if( this->el_context ) {
    std::string const user_dn = get_proxy_subject( proxyfile );
    edg_wlc_JobIdParse( jobid.c_str(), &id );
    res = edg_wll_SetLoggingJobProxy( *this->el_context, id, sequence.c_str(), user_dn.c_str(), this->el_flag );
    edg_wlc_JobIdFree( id );
    if( res != 0 ) throw iceLoggerException( this->getLoggingError("Cannot set LBProxy context:") );
  }                                                                                
  return *this;
}
#endif


void iceEventLogger::unhandled_event( const char *descr )
{
    // FIXME nothing to do
  return;
}

void iceEventLogger::setLoggingJob( const char* jobid ) throw( iceLoggerException& )
{
    int res;
    edg_wlc_JobId id;
    
    if( this->el_context ) {
        edg_wlc_JobIdParse( jobid, &id );
        res = edg_wll_SetLoggingJob( *el_context, id, edg_wll_GetSequenceCode( *el_context ), EDG_WLL_SEQ_NORMAL );
        edg_wlc_JobIdFree( id );
        if( res != 0 ) throw iceLoggerException( this->getLoggingError("Cannot reset logging context:") );
    }
}


void iceEventLogger::execute_event( const char* jobid, const char *host ) throw( iceLoggerException& )
{

    int           res;
    
    if( this->el_context ) {
        setLMPersonality( );
        setLoggingJob( jobid );

        this->startLogging();

        do {
            log_dev->infoStream() << "Logging execute event, host = " << host
                                  << log4cpp::CategoryStream::ENDLINE;
            
            res = edg_wll_LogRunning( *this->el_context, host );
            log_dev->infoStream() << "...Got return code " << res 
                                  << log4cpp::CategoryStream::ENDLINE;
            this->testCode( res );
        } while( res != 0 );
    } else {
        log_dev->warnStream() << "Got job execute event (not logging, as the context is null), host = " << host
                              << log4cpp::CategoryStream::ENDLINE;
    }
    
    return;
}

string iceEventLogger::sequence_code( void )
{
  char          *seqcode;
  string         res( "undefined" );

  if( this->el_context ) {
    seqcode = edg_wll_GetSequenceCode( *this->el_context );

    res.assign( seqcode );
    free( seqcode );
  }

  return res;
}

void iceEventLogger::setJCPersonality( void ) throw( iceLoggerException& )
{
    if ( !el_context ) 
        throw iceLoggerException( "No context initialized" );

    int res = edg_wll_SetParam( *this->el_context, 
                                EDG_WLL_PARAM_SOURCE, 
                                EDG_WLL_SOURCE_JOB_SUBMISSION );

    if( res ) throw iceLoggerException( "Invalid context parameter setting." );    
}

void iceEventLogger::setLMPersonality( void ) throw( iceLoggerException& )
{
    if ( !el_context ) 
        throw iceLoggerException( "No context initialized" );

    int res = edg_wll_SetParam( *this->el_context, 
                                EDG_WLL_PARAM_SOURCE, 
                                EDG_WLL_SOURCE_LOG_MONITOR );

    if( res ) throw iceLoggerException( "Invalid context parameter setting." );
}
