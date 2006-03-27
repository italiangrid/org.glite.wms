#include "iceLBLogger.h"
#include "iceLBContext.h"
#include "iceLBEvent.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "boost/scoped_ptr.hpp"

using namespace glite::wms::ice::util;

iceLBLogger* iceLBLogger::_instance = 0;
boost::recursive_mutex iceLBLogger::_mutex;

//////////////////////////////////////////////////////////////////////////////
// 
// iceLBLogger
//
//////////////////////////////////////////////////////////////////////////////
iceLBLogger* iceLBLogger::instance( void )
{
    boost::recursive_mutex::scoped_lock L( _mutex );
    if ( 0 == _instance ) {
        _instance = new iceLBLogger( );
    }
    return _instance;
}

iceLBLogger::iceLBLogger( void ) :
    _ctx( new iceLBContext() ),
    log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{

}


iceLBLogger::~iceLBLogger( void )
{

}

void iceLBLogger::logEvent( iceLBEvent* ev )
{
    if ( ev ) {

        boost::recursive_mutex::scoped_lock L( _mutex );
        boost::scoped_ptr< iceLBEvent > _scoped_ev( ev );
        
        try {
            _ctx->setLoggingJob( ev->getJob(), ev->getSrc() );
        } catch( iceLBException& ex ) {
            log_dev->errorStream()
                << "Error logging " << ev->describe()
                << " GridJobID=[" << ev->getJob().getGridJobID() << "]"
                << " CreamJobID=[" << ev->getJob().getJobID() << "]"
                << ". Caught exception " << ex.what()
                << log4cpp::CategoryStream::ENDLINE;
            return;
        }
    
        _ctx->startLogging();

        int res = 0;
        do {
            log_dev->infoStream() 
                << "Logging " << ev->describe( )
                << " GridJobID=[" << ev->getJob().getGridJobID() << "]"
                << " CreamJobID=[" << ev->getJob().getJobID() << "]"
                << log4cpp::CategoryStream::ENDLINE;
            
            res = ev->execute( _ctx );
            log_dev->infoStream() 
                << "...Got return code " << res 
                << log4cpp::CategoryStream::ENDLINE;
            
            _ctx->testCode( res );
            
        } while( res != 0 );        
        
        _ctx->update_and_store_job( ev->getJob() );    
    }
}
