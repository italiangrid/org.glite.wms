#ifndef __GLITE_WMS_ICE_UTIL_JOBKILLER_H__
#define __GLITE_WMS_ICE_UTIL_JOBKILLER_H__

#include "iceConfManager.h"
#include "iceThread.h"
#include "creamJob.h"

namespace log4cpp {
  class Category;
};

namespace glite {
  namespace ce {
    namespace cream_client_api {
      namespace soap_proxy {
	
	class CreamProxy;

      }
    }
  }
}

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

      class jobKiller : public iceThread {
	  bool m_valid;
	  log4cpp::Category *m_log_dev;
	  glite::ce::cream_client_api::soap_proxy::CreamProxy* m_theProxy;
	  time_t m_threshold_time;
	  time_t m_delay;

	  void killJob( const glite::wms::ice::util::CreamJob& ); 

	public:
	  jobKiller();
	  virtual ~jobKiller();
	  bool isValid( void ) const { return m_valid; }
	  
	  virtual void body( void );

	};

      }
    }
  }
}

#endif
