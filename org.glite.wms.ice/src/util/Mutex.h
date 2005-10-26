
#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <pthread.h>

//______________________________________________________________________________
namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	class Mutex {
	  pthread_mutex_t *_mx;
	public:
	  Mutex(pthread_mutex_t *mx) : _mx(mx) { pthread_mutex_lock(_mx); }
	  ~Mutex() { pthread_mutex_unlock(_mx); }
	};
      }
    }
  }
}

#endif
