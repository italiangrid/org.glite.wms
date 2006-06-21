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

#include <boost/scoped_ptr.hpp>

using namespace glite::wms::ice::util;

iceLBLogger* iceLBLogger::s_instance = 0;
boost::recursive_mutex iceLBLogger::s_mutex;

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
