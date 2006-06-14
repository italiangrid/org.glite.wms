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
 * ICE core class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_ICE_CORE_H
#define GLITE_WMS_ICE_ICE_CORE_H

#include "iceInit_ex.h"
#include "eventStatusListener.h"
#include "subscriptionUpdater.h"
#include "eventStatusPoller.h"
#include "leaseUpdater.h"
#include "proxyRenewal.h"
#include "jobKiller.h"
#include "creamJob.h"
#include "iceThread.h"

#include "glite/wms/common/utilities/FLExtractor.h"

#include "ClassadSyntax_ex.h"

#include <string>

typedef glite::wms::common::utilities::FLExtractor<std::string>::iterator FLEit;

namespace boost {
  class thread;
};

namespace log4cpp {
  class Category;
};

namespace glite {
  namespace wms {
    namespace ice {

      class Ice {

          class IceThreadHelper { 
          public:
              IceThreadHelper( const std::string& name );
              virtual ~IceThreadHelper( );

              /**
               * Associates a newly created iceThread object
               * to this thread, and starts it. 
               *
               * @param obj a pointer to the iceThread object from
               * which a thread should be created. The caller
               * transfers ownership of parameter obj.
               */
              void start( util::iceThread* obj ) throw( iceInit_ex& );

          protected:

              /**
               * Stops this thread. This method has no effect if the
               * start() method was not called first, or if the
               * start() method raised an exception.
               */
              void stop( void );

              const std::string m_name; //! The name of this thread (can be any string)
              boost::thread* m_thread; //! The dynamically created thread object. 
              boost::shared_ptr< util::iceThread > m_ptr_thread; //! Used to instantiate the thread.
              log4cpp::Category* m_log_dev; //! Logging device
          };

          IceThreadHelper m_listener_thread;
          IceThreadHelper m_poller_thread;
          IceThreadHelper m_updater_thread;
          IceThreadHelper m_lease_updater_thread;
          IceThreadHelper m_proxy_renewer_thread;
	  IceThreadHelper m_job_killer_thread;

          std::string m_ns_filelist;
          std::vector<FLEit> m_requests;
          glite::wms::common::utilities::FLExtractor<std::string> m_fle;
          glite::wms::common::utilities::FileList<std::string> m_flns;
          
          log4cpp::Category* m_log_dev;
          
      public:
          Ice( const std::string& NS_FL, const std::string& WM_FL )
              throw(glite::wms::ice::iceInit_ex&);
          
          virtual ~Ice();
          
          void clearRequests();
          void getNextRequests(std::vector<std::string>&);
          void removeRequest( unsigned int );
          void ungetRequest( unsigned int );
          void startListener( int );
          void startPoller( int );
          void startLeaseUpdater( void );
          void startProxyRenewer( void );
	  void startJobKiller( void );
          virtual void resubmit_job( util::CreamJob& j );

      }; // class ice

    } // namespace ice
  } // namespace ce
} // namespace glite

#endif
