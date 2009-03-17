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
//#include "jobCache.h"
#include "glite/ce/cream-client-api-c/scoped_timer.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include "iceDb/UpdateJobSequenceCode.h"
#include "iceDb/CheckGridJobID.h"
#include "iceDb/Transaction.h"
#include "iceDb/CreateJob.h"

#include "creamJob.h"

#include <boost/scoped_ptr.hpp>

namespace api_util = glite::ce::cream_client_api::util;
using namespace glite::wms::ice::util;

iceLBLogger* iceLBLogger::s_instance = 0;

//////////////////////////////////////////////////////////////////////////////
// 
// iceLBLogger
//
//////////////////////////////////////////////////////////////////////////////
iceLBLogger* iceLBLogger::instance( void )
{
    if ( 0 == s_instance ) {
        s_instance = new iceLBLogger();
    }
    return s_instance;
}

iceLBLogger::iceLBLogger( void ) :
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    m_lb_enabled( true )
{
    //
    // To disable logging to LB, just set an environment variable
    //
    // GLITE_WMS_ICE_DISABLE_LB
    //
    // with whatever value you want.
    //
    if ( 0 != getenv( "GLITE_WMS_ICE_DISABLE_LB" ) ) {
        m_lb_enabled = false;
    }
}


iceLBLogger::~iceLBLogger( void )
{

}

CreamJob iceLBLogger::logEvent( iceLBEvent* ev )
{
    static const char* method_name = "iceLBLogger::logEvent() - ";
#ifdef ICE_PROFILE_ENABLE
    api_util::scoped_timer T( "logEvent()" );
#endif
    // Aborts if trying to log the NULL event
    if ( ! ev ) {
        CREAM_SAFE_LOG(m_log_dev->fatalStream()
                       << method_name
		       << "Cannot log a NULL event"
                       );
        abort();        
    }

    // Destroys the parameter "ev" when exiting this function
    boost::scoped_ptr< iceLBEvent > scoped_ev( ev );

    // Allocates a new (temporary) LB context
    boost::scoped_ptr< iceLBContext > m_ctx( new iceLBContext() );

    std::string new_seq_code;
        
    try {
        m_ctx->setLoggingJob( ev->getJob(), ev->getSrc() );
    } catch( iceLBException& ex ) {
        CREAM_SAFE_LOG(m_log_dev->errorStream()
                       << method_name
                       << ev->describe()
                       << " GridJobID=[" 
                       << ev->getJob().getGridJobID() << "]"
                       << " CreamJobID=[" << ev->getJob().getCompleteCreamJobID() << "]"
                       << ". Caught exception " << ex.what()
                       );
        return ev->getJob();
    }
    
    m_ctx->startLogging();

    int res = 0;
    do {
        CREAM_SAFE_LOG(m_log_dev->infoStream() 
                       << method_name
                       << ev->describe( )
                       << " GridJobID=[" 
                       << ev->getJob().getGridJobID() << "]"
                       << " CreamJobID=[" << ev->getJob().getCompleteCreamJobID() << "]"
                       // << " Seq code BEFORE from job=[" << ev->getJob().getSequenceCode() << "]"
                       // << " Seq code BEFORE from ctx=[" << edg_wll_GetSequenceCode( *(m_ctx->el_context) ) << "]"
                       
                       );
        if ( m_lb_enabled ) {

	  //api_util::scoped_timer T( "logEvent::ev->execute()" );

            res = ev->execute( m_ctx.get() );
            m_ctx->testCode( res );
	}
        
    } while( res != 0 );        

    char* _tmp_seqcode = edg_wll_GetSequenceCode( *(m_ctx->el_context) );

    if ( _tmp_seqcode ) { // update the sequence code only if it is non null
        new_seq_code = _tmp_seqcode;
	free( _tmp_seqcode );
        { // Lock the job cache
// #ifdef ICE_PROFILE_ENABLE
// 	  api_util::scoped_timer T( "logEvent::mutex_aquisition-EntireBlock" );//126
// #endif
	  //            jobCache* m_cache( jobCache::getInstance() );
	  //            boost::recursive_mutex::scoped_lock( m_cache->mutex );
	  boost::recursive_mutex::scoped_lock M( glite::wms::ice::util::CreamJob::globalICEMutex );

	    //api_util::scoped_timer T2( "logEvent::CreamJob_Creation+setSeqCode+put_in_cache" );//10
            CreamJob theJob( ev->getJob() );
	    
            theJob.setSequenceCode( new_seq_code );
// #ifdef ICE_PROFILE_ENABLE
// 	    api_util::scoped_timer T3( "logEvent::put_in_cache" );//10
// #endif
            //m_cache->put( theJob );
	    
	    //glite::wms::ice::db::CheckGridJobID check( theJob.getGridJobID() );
	    //{
	    //  glite::wms::ice::db::Transaction tnx;
	    //  tnx.execute( &check );
	   // }

	    //if( check.found() ) {
	    //  glite::wms::ice::db::UpdateJobSequenceCode upd( theJob.getGridJobID(), new_seq_code );
	    //  glite::wms::ice::db::Transaction tnx;
	    //  tnx.execute( &upd );
	    //} else {
	      glite::wms::ice::db::CreateJob aJob( theJob );
	      glite::wms::ice::db::Transaction tnx;
	      tnx.execute( &aJob );
	    //}
	    
            return theJob;

        } 
    } else {
        return ev->getJob(); // Make the compiler happy
    }
}
