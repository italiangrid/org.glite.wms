#include "ice_timer.h"

namespace iceutil=glite::wms::ice::util;

boost::recursive_mutex iceutil::ice_timer::s_mutex;

//______________________________________________________________________________
iceutil::ice_timer::ice_timer( const std::string& loc ) 
  : m_location( loc ) 
{
  gettimeofday( &m_beforeT, 0 );
}

//______________________________________________________________________________
iceutil::ice_timer::~ice_timer() 
{
  gettimeofday( &m_afterT, 0 );
  double delay = ( (double)m_afterT.tv_sec - (double)m_beforeT.tv_sec )
    + ( (double)m_afterT.tv_usec - (double)m_beforeT.tv_usec )/1000000;
  boost::recursive_mutex::scoped_lock M( s_mutex );
  std::cout << "ice_timer::~ice_timer - CALLER=" 
	    << m_location << " - Time = " << delay
	    << std::endl;
}
