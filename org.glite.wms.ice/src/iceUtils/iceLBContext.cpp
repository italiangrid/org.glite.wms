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
 * ICE LB Logger Context
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

//
// This file is heavily based on org.glite.wms.jobsubmission/src/common/EventLogger.cpp
//
// See also:
// org.glite.lb.client-interface/build/producer.h
// org.glite.lb.client-interface/doc/C/latex/refman.pdf

#include <cstdlib>
#include <cstdio>
//#include <cerrno>

#include <iostream>
#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;

#include "glite/lb/producer.h"
#include "iceLBContext.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
//#include "glite/ce/cream-client-api-c/scoped_timer.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
// #include "jobCache.h"
#include "iceUtils.h"

//#include <netdb.h>

using namespace std;
using namespace glite::wms::ice::util;
namespace configuration = glite::wms::common::configuration;

// static member definitions
unsigned int iceLBContext::s_el_s_retries = 3;
unsigned int iceLBContext::s_el_s_sleep = 5; // Delay between retries
const char *iceLBContext::el_s_notLogged = "Event not logged, context unset.";
const char *iceLBContext::el_s_unavailable = "unavailable";
const char *iceLBContext::el_s_OK = "OK";
const char *iceLBContext::el_s_failed = "Failed";
const char *iceLBContext::el_s_succesfully = "Job Terminated Successfully";

#ifdef GLITE_WMS_HAVE_LBPROXY

#include <openssl/pem.h>
#include <openssl/x509.h>

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
//////////////////////////////////////////////////////////////////////////////
//
// iceLogger Exception
//
//////////////////////////////////////////////////////////////////////////////
iceLBException::iceLBException( const char *reason ) : m_le_reason( reason ? reason : "" )
{}

iceLBException::iceLBException( const string &reason ) : m_le_reason( reason )
{}

iceLBException::~iceLBException( void ) throw() {}

const char *iceLBException::what( void ) const throw()
{
  return this->m_le_reason.c_str();
}

string iceLBContext::s_localHostName( "" );

//////////////////////////////////////////////////////////////////////////////
// 
// iceLBContext
//
//////////////////////////////////////////////////////////////////////////////

//____________________________________________________________________________
iceLBContext::iceLBContext( void ) :
    el_context( new edg_wll_Context ), 
    el_s_localhost_name( ),
    m_el_hostProxy( false ),
    m_el_count( 0 ), 
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
    // m_cache( jobCache::getInstance() )
{

  {
    //api_util::scoped_timer T( "iceLBContext::iceLBContext::edg_wll_InitContext(...)" );
    edg_wll_InitContext( el_context );
    
  }

    try {
      if( s_localHostName.empty() ) {
        el_s_localhost_name = getHostName();  
        s_localHostName = el_s_localhost_name;
      } else {
        el_s_localhost_name = s_localHostName;
      }
    } catch( runtime_error& ex) {
        CREAM_SAFE_LOG(m_log_dev->errorStream() 
                       << "iceLBContext::CTOR() - getHostName() returned an ERROR: "
                       << ex.what()
                       );
        el_s_localhost_name = "(unknown host name )"; 
	s_localHostName = "";
    }
}

iceLBContext::~iceLBContext( void )
{
  edg_wll_FreeContext( *el_context );
  delete el_context;
}

string iceLBContext::getLoggingError( const char *preamble )
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

void iceLBContext::testCode( int &code, bool retry )
{
    if ( code != 0 ) {
        string err( getLoggingError( 0 ) );
        CREAM_SAFE_LOG(m_log_dev->errorStream() << "iceLBContext::testCode() - Got error " << err
		       );
        
    }

    const configuration::ICEConfiguration *conf = configuration::Configuration::instance()->ice();
    int          ret;
    string       cause, host_proxy;

    if( code ) {
        cause = this->getLoggingError( NULL );

        switch( code ) {
        case EINVAL:
	  CREAM_SAFE_LOG(m_log_dev->errorStream()
			 << "iceLBContext::testCode() - Critical error in L&B calls: EINVAL. "
			 << "Cause = \"" << cause << "\"."
			 );
            
            code = 0; // Don't retry...
            break;
        case EDG_WLL_ERROR_GSS:
	  CREAM_SAFE_LOG(m_log_dev->errorStream()
			 << "iceLBContext::testCode() - Severe error in GSS layer while communicating with L&B daemons. " 
			 << "Cause = \"" << cause << "\"." 
			 );

            if( this->m_el_hostProxy ) {
	      CREAM_SAFE_LOG(m_log_dev->debugStream()
			     << "iceLBContext::testCode() - The log with the host certificate has just been done. Giving up." 
			     );
                
                code = 0; // Don't retry...
            }
            else {
	      host_proxy = conf->ice_host_cert();

                CREAM_SAFE_LOG(m_log_dev->debugStream()
			       << "Retrying using host proxy certificate [" 
			       << host_proxy << "]" 
			       );


                if( host_proxy.length() == 0 ) {
		  CREAM_SAFE_LOG(m_log_dev->warnStream()
				 << "iceLBContext::testCode() - Host proxy file not set inside configuration file. " 
				 << "Trying with a default NULL and hoping for the best." 
				 );

                    ret = edg_wll_SetParam( *el_context, EDG_WLL_PARAM_X509_PROXY, NULL );
                }
                else {
		  CREAM_SAFE_LOG(m_log_dev->debugStream()
				 << "iceLBContext::testCode() - Host proxy file found = [" << host_proxy << "]."
				 );

                    ret = edg_wll_SetParam( *el_context, EDG_WLL_PARAM_X509_PROXY, host_proxy.c_str() );
                }

                if( ret ) {
		  CREAM_SAFE_LOG(m_log_dev->errorStream()
				 << "iceLBContext::testCode() - Cannot set the host proxy inside the context. Giving up." 
				 );

                    code = 0; // Don't retry.
                }
                else this->m_el_hostProxy = true; // Set and retry (code is still != 0)
            }

            break;
        default:
            if( ++this->m_el_count > s_el_s_retries ) {
	      CREAM_SAFE_LOG(m_log_dev->errorStream()
			     << "iceLBContext::testCode() - L&B call retried " << this->m_el_count << " times always failed. "
			     << "Ignoring." 
			     );

                code = 0; // Don't retry anymore
            }
            else {
	      CREAM_SAFE_LOG(m_log_dev->warnStream()
			     << "iceLBContext::testCode() - L&B call got a transient error (code=" << code << "). Waiting " << s_el_s_sleep << " seconds and trying again. " 
			     << "Try n. " << this->m_el_count << "/" << s_el_s_retries 
			     );

                sleep( s_el_s_sleep );
            }
            break;
        }
    }
    else // The logging call worked fine, do nothing
      CREAM_SAFE_LOG(m_log_dev->debugStream() 
		     << "iceLBContext::testCode() - L&B call succeeded." 
		     );

    // SignalChecker::instance()->throw_on_signal();

    return;

}

void iceLBContext::registerJob( const util::CreamJob& theJob )
{
    int res;
    edg_wlc_JobId   id;

    setLoggingJob( theJob, EDG_WLL_SOURCE_JOB_SUBMISSION );

    CREAM_SAFE_LOG(m_log_dev->infoStream() 
		   << "iceLBContext::registerJob() - Registering jobid=[" << theJob.getGridJobID() << "]"
		   );
    
    edg_wlc_JobIdParse( theJob.getGridJobID().c_str(), &id );

#ifdef GLITE_WMS_HAVE_LBPROXY
    res = edg_wll_RegisterJobProxy( *el_context, id, EDG_WLL_JOB_SIMPLE, theJob.getJDL().c_str(), theJob.getEndpoint().c_str(), 0, 0, 0 );
#else
    res = edg_wll_RegisterJob( *el_context, id, EDG_WLL_JOB_SIMPLE, theJob.getJDL().c_str(), theJob.getEndpoint().c_str(), 0, 0, 0 );
#endif
    edg_wlc_JobIdFree( id );
    if( res != 0 ) {
      CREAM_SAFE_LOG(m_log_dev->errorStream() 
		     << "iceLBContext::registerJob() - Cannot register jobid=[" << theJob.getGridJobID()
		     << "]. LB error code=" << res
		     );
    }
}


void iceLBContext::setLoggingJob( const util::CreamJob& theJob, edg_wll_Source src ) throw ( iceLBException& )
{
    static const char* method_name = "iceLBContext::setLoggingJob - ";

    edg_wlc_JobId   id;
    int res = 0;

    res = edg_wlc_JobIdParse( theJob.getGridJobID().c_str(), &id );    

    char *lbserver;
    unsigned int lbport;
    edg_wlc_JobIdGetServerParts( id, &lbserver, &lbport );
    CREAM_SAFE_LOG(m_log_dev->infoStream() << method_name
		   << "Setting log job to jobid=[" << theJob.getGridJobID() << "] "
		   << "LB server=[" << lbserver << ":" << lbport << "] "
		   << "(port is not used, actually...)"
		   );
    res |= edg_wll_SetParam( *el_context, EDG_WLL_PARAM_SOURCE, src );        
    res |= edg_wll_SetParam( *el_context, EDG_WLL_PARAM_DESTINATION, lbserver );
    if ( lbserver ) free( lbserver );

    if ( !theJob.getSequenceCode().empty() ) {
#ifdef GLITE_WMS_HAVE_LBPROXY
        string const user_dn( get_proxy_subject( theJob.getBetterProxy() ) );

        res |= edg_wll_SetLoggingJobProxy( *el_context, id, theJob.getSequenceCode().c_str(), user_dn.c_str(), EDG_WLL_SEQ_NORMAL );
#else
        res |= edg_wll_SetLoggingJob( *el_context, id, theJob.getSequenceCode().c_str(), EDG_WLL_SEQ_NORMAL );
#endif
    }

//     CREAM_SAFE_LOG(m_log_dev->infoStream()
//                    << "After SetLoggingJob: "
//                    << " Seq code from job=[" << theJob.getSequenceCode() << "]"
//                    << " Seq code from ctx=[" << edg_wll_GetSequenceCode( *el_context ) << "]"
//                    );

    edg_wlc_JobIdFree( id );

    if( res != 0 ) {
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Unable to set logging job to jobid=["
		     << theJob.getGridJobID()
		     << "]. LB error is "
		     << getLoggingError( 0 )
		     );
        throw iceLBException( this->getLoggingError("Cannot set logging job:") );
    }

    // Set user proxy for L&B stuff
    fs::path    pf( theJob.getBetterProxy(), fs::native);
    pf.normalize();

    if( fs::exists(pf) ) {
        res = edg_wll_SetParam( *el_context, EDG_WLL_PARAM_X509_PROXY, 
                                theJob.getBetterProxy().c_str() );

        if( res ) {
            CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
			 << "Unable to set logging job to jobid=["
			 << theJob.getGridJobID()
			 << "]. "
			 << getLoggingError( 0 )
			 );
            throw iceLBException( this->getLoggingError("Cannot set proxyfile path inside context:") );
        }
    } else {
        CREAM_SAFE_LOG(m_log_dev->errorStream() << method_name
		     << "Unable to set logging job to jobid=["
		     << theJob.getGridJobID()
		     << "]. Proxy file "
		     << theJob.getBetterProxy()
		     << " does not exist. "
		     << "Trying to use the host proxy cert, and hoping for the best..."
		     );
    }
}
