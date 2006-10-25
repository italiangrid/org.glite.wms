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
#include <set>

namespace glite {
namespace wms {
namespace ice {

    class iceAbsCommand;

namespace util {

    class iceThreadPoolState {
    public:
        iceThreadPoolState( ) { }
        
        int m_num_running; ///< Number of running threads
        boost::recursive_mutex m_mutex; ///< Mutex to protect the shared state object
        boost::condition m_no_requests_available; ///< Condition triggered when there is a new request in the queue
        std::list< iceAbsCommand* > m_requests_queue; ///< The queue of requests (commands to be issued to CREAM)
        std::set< std::string > m_pending_jobs; ///< Grei Job IDs of pending jobs (jobs being processed)
    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif
