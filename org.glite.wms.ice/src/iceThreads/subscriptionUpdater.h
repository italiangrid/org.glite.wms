#ifndef __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__
#define __GLITE_WMS_ICE_UTIL_CESUBUPDATER_H__

#include "iceConfManager.h"
#include "iceThread.h"

class Topic;
class Policy;

namespace log4cpp {
    class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class subscriptionManager;

	class subscriptionUpdater : public iceThread {

		int m_iteration_delay;

        public:
		subscriptionUpdater( );

		virtual ~subscriptionUpdater() { }
		virtual void body( void );
		
	};
        
      }
    }
  }
}

#endif
