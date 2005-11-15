#ifndef __ABS_ICE_CORE_H__
#define __ABS_ICE_CORE_H__

#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      
      class absice {
      public:
	virtual void doOnJobFailure(const std::string& gid) = 0;
      };
    }
  }
}

#endif
