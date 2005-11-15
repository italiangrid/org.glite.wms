#ifndef __ABS_ICE_CORE_H__
#define __ABS_ICE_CORE_H__

#include <string>
#include "creamJob.h"

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	class CreamJob;
      }
    }
  }
};

namespace glite {
  namespace wms {
    namespace ice {
      
      class absice {
      public:
	virtual void doOnJobFailure(const std::string&) = 0;
      };

    }
  }
}

#endif

