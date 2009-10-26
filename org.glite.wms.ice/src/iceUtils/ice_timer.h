#ifndef ICE_TIMER_H
#define ICE_TIMER_H


#include <sys/time.h>
#include <iostream>

#include <boost/thread/recursive_mutex.hpp>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class ice_timer {
	  struct timeval m_beforeT, m_afterT;
	  std::string m_location;
	  static boost::recursive_mutex  s_mutex;

	public:
	  ice_timer(const std::string& location = "");
	  ~ice_timer();

	};

      }
    }
  }
}

#endif
