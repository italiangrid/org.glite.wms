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
#include "jobCache.h"
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

CreamJob iceLBLogger::logEvent( iceLBEvent* ev )
{
    // Abortg if trying to log the NULL event
    if ( ! ev ) {
        CREAM_SAFE_LOG(m_log_dev->fatalStream()
                       << "Trying to log NULL event"
                       << log4cpp::CategoryStream::ENDLINE);
        abort();        
    }

    // Destroys the parameter "ev" when exiting this function
    boost::scoped_ptr< iceLBEvent > scoped_ev( ev );

    std::string new_seq_code;
    {
        boost::recursive_mutex::scoped_lock L( s_mutex );
        
        try {
            m_ctx->setLoggingJob( ev->getJob(), ev->getSrc() );
        } catch( iceLBException& ex ) {
            CREAM_SAFE_LOG(m_log_dev->errorStream()
                           << "iceLBLogger::logEvent() - Error logging " 
                           << ev->describe()
                           << " GridJobID=[" 
                           << ev->getJob().getGridJobID() << "]"
                           << " CreamJobID=[" << ev->getJob().getJobID() << "]"
                           << ". Caught exception " << ex.what()
                           << log4cpp::CategoryStream::ENDLINE);
            return ev->getJob();
        }
    
        m_ctx->startLogging();

        int res = 0;
        do {
            CREAM_SAFE_LOG(m_log_dev->infoStream() 
			   << "iceLBLogger::logEvent() - Logging " 
                           << ev->describe( )
			   << " GridJobID=[" 
                           << ev->getJob().getGridJobID() << "]"
			   << " CreamJobID=[" << ev->getJob().getJobID() << "]"
                           // << " Seq code BEFORE from job=[" << ev->getJob().getSequenceCode() << "]"
                           // << " Seq code BEFORE from ctx=[" << edg_wll_GetSequenceCode( *(m_ctx->el_context) ) << "]"

			   << log4cpp::CategoryStream::ENDLINE);
            
            res = ev->execute( m_ctx );
            
            m_ctx->testCode( res );
            
        } while( res != 0 );        
        
        new_seq_code = edg_wll_GetSequenceCode( *(m_ctx->el_context) );
        CREAM_SAFE_LOG(m_log_dev->infoStream() 
                       << "iceLBLogger::logEvent() - ...Got return code " 
                       << res 
                       // << " Seq code AFTER from job=[" << ev->getJob().getSequenceCode() << "]"
                       // << " Seq code AFTER from ctx=[" << new_seq_code << "]"

                       << log4cpp::CategoryStream::ENDLINE);

    } // unlocks the LB

    { // Now, locks the cache
        jobCache* m_cache( jobCache::getInstance() );
        boost::recursive_mutex::scoped_lock( m_cache->mutex );
        CreamJob theJob( ev->getJob() );
        theJob.setSequenceCode( new_seq_code );    
        m_cache->put( theJob );
        return theJob;
    } // Unlocks the job cache
}
