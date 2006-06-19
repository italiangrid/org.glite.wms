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
 * ICE LB Logger
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceLBLogger.h"
#include "iceLBContext.h"
#include "iceLBEvent.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#ifdef GLITE_WMS_HAVE_LBPROXY
#include <openssl/pem.h>
#include <openssl/x509.h>
#endif

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;
using namespace glite::wms::ice::util;

iceLBLogger* iceLBLogger::s_instance = 0;
boost::recursive_mutex iceLBLogger::s_mutex;

#ifdef GLITE_WMS_HAVE_LBPROXY

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
// iceLBLogger
//
//////////////////////////////////////////////////////////////////////////////
iceLBLogger* iceLBLogger::instance( void )
{
    boost::recursive_mutex::scoped_lock L( s_mutex );
    if ( 0 == s_instance ) {
        s_instance = new iceLBLogger( );
    }
    return s_instance;
}

iceLBLogger::iceLBLogger( void ) :
    m_ctx( new iceLBContext() ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{

}


iceLBLogger::~iceLBLogger( void )
{

}

void iceLBLogger::logEvent( iceLBEvent* ev )
{
    if ( ev ) {

        boost::recursive_mutex::scoped_lock L( s_mutex );
        boost::scoped_ptr< iceLBEvent > scoped_ev( ev );
        
        try {
            m_ctx->setLoggingJob( ev->getJob(), ev->getSrc() );
        } catch( iceLBException& ex ) {
	  CREAM_SAFE_LOG(m_log_dev->errorStream()
			 << "iceLBLogger::logEvent() - Error logging " << ev->describe()
			 << " GridJobID=[" << ev->getJob().getGridJobID() << "]"
			 << " CreamJobID=[" << ev->getJob().getJobID() << "]"
			 << ". Caught exception " << ex.what()
			 << log4cpp::CategoryStream::ENDLINE);
            return;
        }
    
        m_ctx->startLogging();

        int res = 0;
        do {
            CREAM_SAFE_LOG(m_log_dev->infoStream() 
			   << "iceLBLogger::logEvent() - Logging " << ev->describe( )
			   << " GridJobID=[" << ev->getJob().getGridJobID() << "]"
			   << " CreamJobID=[" << ev->getJob().getJobID() << "]"
			   << log4cpp::CategoryStream::ENDLINE);
            
            res = ev->execute( m_ctx );
            CREAM_SAFE_LOG(m_log_dev->infoStream() 
			   << "iceLBLogger::logEvent() - ...Got return code " << res 
			   << log4cpp::CategoryStream::ENDLINE);
            
            m_ctx->testCode( res );
            
        } while( res != 0 );        
        
        m_ctx->update_and_store_job( ev->getJob() );    
    }
}
