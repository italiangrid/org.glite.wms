/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE lease updater
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_LEASEUPDATER_H
#define GLITE_WMS_ICE_UTIL_LEASEUPDATER_H

#include "iceThread.h"
#include "creamJob.h"

#include "boost/thread/recursive_mutex.hpp"
#include "boost/scoped_ptr.hpp"

#include <ctime>

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
              
              time_t m_threshold; //! Residual lease durations less than this threshold are prolonged.
              time_t m_delay; //! Delay between two updates, in seconds.
              time_t m_delta; //! The amount of the lease update, in seconds.

              log4cpp::Category *m_log_dev;
              jobCache *m_cache;
              boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_creamClient;

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
              void update_lease_for_job( CreamJob j );

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
