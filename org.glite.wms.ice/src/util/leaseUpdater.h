#ifndef __GLITE_WMS_ICE_UTIL_LEASEUPDATER_H__
#define __GLITE_WMS_ICE_UTIL_LEASEUPDATER_H__

#include "iceThread.h"
//#include "glite/ce/monitor-client-api-c/CEConsumer.h"

//#undef soapStub_H
//#include "glite/ce/monitor-client-api-c/CEPing.h"
//#include "glite/ce/monitor-client-api-c/CESubscription.h"
//#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
//#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "boost/thread/recursive_mutex.hpp"
#include <vector>
#include <string>

// Forward declaratino for the CreamProxy
namespace glite {
  namespace ce {
    namespace cream_client_api {
      namespace soap_proxy {
	class CreamProxy;
      }
    }
  }
};


// Forward declaration for the logger
namespace log4cpp {
    class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
          
          class jobCache;
          class iceConfManager;
          class iceEventLogger;

          /**
           *
           */
          class leaseUpdater : public iceThread {

          protected:

              int delay; //! Delay between two updates, in seconds. Hardcoded for now, should be fixed in the future
              log4cpp::Category *log_dev;
              jobCache *cache;
              glite::ce::cream_client_api::soap_proxy::CreamProxy* creamClient;

              std::vector< std::string > getJobsToUpdate( void );

              //! This function actually performs the job
              virtual void body( void );

          public:

              leaseUpdater( );
              virtual ~leaseUpdater( );

          };

      }
    }
  }
}

#endif
