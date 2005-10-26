
#ifndef __THREAD_H__
#define __THREAD_H__

#include "runnable.h"
#include "thread_start_ex.h"

#include <string>
#include <pthread.h>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class thread {
	  runnable*       _object;
	  pthread_attr_t  attr;
	  pthread_t       t;

	public:
	  thread(runnable&);
	  virtual ~thread() { }
	  void start() throw(thread_start_ex&);
	  void stop();
	  void join();
	  void detach();
	};
      }
    }
  }
}

#endif
