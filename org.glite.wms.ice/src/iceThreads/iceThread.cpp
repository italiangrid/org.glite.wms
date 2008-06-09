#include "iceThread.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

using namespace glite::wms::ice::util;
namespace apiLogger = glite::ce::cream_client_api::util;

iceThread::iceThread( const std::string& name ) :
    m_name( name ),
    m_running( false ),
    m_stopped( false )
{

}

iceThread::iceThread( ) :
    m_name( ),
    m_running( false ),
    m_stopped( false )
{

}

void iceThread::operator()()
{
    m_running = true;
    CREAM_SAFE_LOG(apiLogger::creamApiLogger::instance()->getLogger()->debugStream()
		   << "iceThread::operator() - Thread " << getName() << " starting..."
		   );
    body( );
    CREAM_SAFE_LOG(apiLogger::creamApiLogger::instance()->getLogger()->debugStream()
		   << "iceThread::operator() - Thread " << getName() << " finished"
		   );
    m_running = false;
}

void iceThread::stop() 
{
  m_stopped = true;
  CREAM_SAFE_LOG(apiLogger::creamApiLogger::instance()->getLogger()->debugStream()
		 << "iceThread::stop() - Thread [" << getName() << "] Called STOP."
		 );
}
