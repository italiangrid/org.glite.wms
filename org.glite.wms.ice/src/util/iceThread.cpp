#include "iceThread.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

using namespace glite::wms::ice::util;
namespace apiLogger = glite::ce::cream_client_api::util;

iceThread::iceThread( const std::string& name ) :
    _name( name ),
    _running( false ),
    _stopped( false )
{

}

void iceThread::operator()()
{
    _running = true;
    apiLogger::creamApiLogger::instance()->getLogger()->infoStream()
        << "Thread " << getName() << " starting..."
        << log4cpp::CategoryStream::ENDLINE;
    while( ! isStopped() ) {
        body( );
    }
    apiLogger::creamApiLogger::instance()->getLogger()->infoStream()
        << "Thread " << getName() << " finished"
        << log4cpp::CategoryStream::ENDLINE;
    _running = false;
}
