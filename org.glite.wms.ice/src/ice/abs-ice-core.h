#ifndef __GLITE_WMS_ICE_ABS_ICE_CORE_H__
#define __GLITE_WMS_ICE_ABS_ICE_CORE_H__

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

