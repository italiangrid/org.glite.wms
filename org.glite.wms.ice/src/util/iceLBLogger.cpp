#include "iceLBLogger.h"
#include "iceLBContext.h"
#include "iceLBEvent.h"

#include "boost/scoped_ptr.hpp"

using namespace glite::wms::ice::util;

iceLBLogger* iceLBLogger::_instance = 0;


//////////////////////////////////////////////////////////////////////////////
// 
// iceLBLogger
//
//////////////////////////////////////////////////////////////////////////////
iceLBLogger* iceLBLogger::instance( void )
{
    if ( 0 == _instance ) {
        _instance = new iceLBLogger( );
    }
    return _instance;
}

iceLBLogger::iceLBLogger( void ) :
    _ctx( new iceLBContext() )
{

}


iceLBLogger::~iceLBLogger( void )
{

}

void iceLBLogger::logEvent( iceLBEvent* ev )
{
    if ( ev ) {
        boost::scoped_ptr< iceLBEvent > _scoped_ev( ev );
        ev->execute( _ctx );
    }
}
