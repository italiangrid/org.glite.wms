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
 * ICE thread pool shared state
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICETHREADPOOLSTATE
#define GLITE_WMS_ICETHREADPOOLSTATE

#include <boost/thread/condition.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <list>

namespace glite {
namespace wms {
namespace ice {

    class iceAbsCommand;

namespace util {

    class iceThreadPoolState {
    public:
        iceThreadPoolState( ) { }
        
        int m_num_running; ///< Number of running threads
        boost::recursive_mutex m_mutex; ///< Mutex to protect
        boost::condition m_queue_empty; ///< Condition triggered when the queue is not empty
        std::list< iceAbsCommand* > m_requests_queue; ///< The queue of requests (commands to be issued to CREAM)
    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
