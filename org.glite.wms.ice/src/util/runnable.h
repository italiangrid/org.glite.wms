
#ifndef __RUNNABLE_H__
#define __RUNNABLE_H__

#include <cstdio>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	class runnable {
	public:
	  virtual void run() {};//= 0;
	  virtual void stop() {};//= 0;
	  virtual void operator()();/*  { */
/* 	    printf("Executing runnable::operator()() !!!\n"); */
/* 	  };//= 0; // needed to be used with boost::thread */
	};
      }
    }
  }
}

#endif
