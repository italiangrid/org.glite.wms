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
#include <ctime>

// Forward declaration for the logger
namespace log4cpp {
    class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
          
          /**
           *
           */
          class leaseUpdater : public iceThread {

          protected:
              
              time_t m_delay; //! Delay between two updates, in seconds.

              log4cpp::Category *m_log_dev;

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
