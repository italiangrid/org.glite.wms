#ifndef __GLITE_WMS_ICE_UTIL_LEASEUPDATER_H__
#define __GLITE_WMS_ICE_UTIL_LEASEUPDATER_H__

#include "iceThread.h"
#include "creamJob.h"
#include "boost/thread/recursive_mutex.hpp"
#include "boost/scoped_ptr.hpp"
//#include <vector>
//#include <string>

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

          /**
           *
           */
          class leaseUpdater : public iceThread {

          protected:

              time_t threshold; //! Residual lease durations less than this threshold are prolonged.
              time_t delay; //! Delay between two updates, in seconds.
              time_t delta; //! The amount of the lease update, in seconds.

              log4cpp::Category *log_dev;
              jobCache *cache;
              boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > creamClient;

              /**
               * Actually updates the lease for all active jobs in the cache.
               * This uses the is_active() method of CreamJob to check
               * whether a job is not terminated; for all active jobs,
               * the lease which is about to expire is increased.
               */
              void update_lease( void );

              /**
               * Updates the lease for a single job. No check is done
               * to see whether the job lease is about to expire.
               *
               * @param j the job whose lease is to be updated
               */
              void update_lease_for_job( CreamJob& j );

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
