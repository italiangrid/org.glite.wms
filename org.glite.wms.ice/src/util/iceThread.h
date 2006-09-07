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
 * ICE thread class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_ICETHREAD_H
#define GLITE_WMS_ICE_ICETHREAD_H

#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

          /**
           * This class models a generic ICE thread. All ICE active
           * components (i.e., ICE threads) should inherit from this
           * class.
           */
          class iceThread {
          public:
              virtual ~iceThread( ) { };

              /**
               * This operator is used by boost threads to start the
               * computation. This method is defined in this class,
               * and must not be redefined. It simply calls body(),
               * setting the _running variable accordingly.
               */
              void operator()();

              // Accessors

              /**
               * Returns true iff the thread started execution (i.e.,
               * the operator()() method has been called).
               */
              bool isRunning( void ) const { return m_running; };

              /**
               * Returns true iff the thread has been stopped, by
               * setting to true the _stopped flag. Note that if
               * isStoped() == true, the thread is not necessarily
               * terminated! the thread is terminated if isStopped()
               * == true AND isRunning() == false;
               */
              bool isStopped( void ) const { return m_stopped; };

              /**
               * Returns the name of this thread.
               *
               * @return the name of the thread
               */
              const std::string& getName( void ) const { return m_name; };

              // Modifiers

              /**
               * Sets the _stopped flag to true. This means that at
               * the end of the current iteration, the thread will
               * terminate.
               */
              void stop( void ) { m_stopped = true; };

          protected:
              virtual void body( void ) = 0;

              iceThread( const std::string& name );
              iceThread( ); // needed by copy costructors...
          private:              
              std::string m_name;
              bool m_running;
              bool m_stopped;

          };

      } // namespace util
    } // namespace ice
  } // namespace wms
} // namespace glite

#endif
