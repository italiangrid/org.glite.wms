
#ifndef __RUNNABLE_H__
#define __RUNNABLE_H__

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	class runnable {
	public:
	  virtual void run() = 0;
	  virtual void stop() = 0;
	};
      }
    }
  }
}

#endif
