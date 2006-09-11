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
 * ICE thread pool class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_THREADPOOL_H
#define GLITE_WMS_ICE_THREADPOOL_H

#include "iceThread.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

// Forward declaration
namespace log4cpp {
    class Category;
}

namespace glite {
  namespace wms {
    namespace ice {

        class iceAbsCommand;

      namespace util {

          struct iceThreadPoolState;

          class iceThreadPool {
          public:
              virtual ~iceThreadPool( );

              /**
               * Gets the singleton instance of this class.
               *
               * @return a pointer to the singleton instance of this class
               */
              static iceThreadPool* instance( );

              /**
               * Adds a request to the thread pool. The request is
               * assigned (and executed) immediately if a thread is
               * available; otherwise, the request is enqueued and
               * will be served in FIFO order.
               *
               * @param req The request (command) to enqueue/execute
               */
              void add_request( glite::wms::ice::iceAbsCommand* req );

          protected:

              iceThreadPool( );

              /**
               * The class of worker threads
               */
              class iceThreadPoolWorker : public iceThread {
              public:
                  iceThreadPoolWorker( iceThreadPoolState* st );
                  virtual ~iceThreadPoolWorker( );
              protected:
                  void body( );

                  boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_proxy;
                  iceThreadPoolState* m_state;
                  const int m_threadNum;
                  static int s_threadNum;
              };

              boost::scoped_ptr< iceThreadPoolState > m_state;
              boost::thread_group m_all_threads;
              log4cpp::Category* m_log_dev;

              static iceThreadPool* s_instance;

          };
          
      } // namespace util
    } // namespace ice
  } // namespace wms
} // namespace glite

#endif
