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
 * ICE LB Logger
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */


//
// This file is heavily based on org.glite.wms.jobsubmission/src/common/EventLogger.h
//
#ifndef ICELBLOGGER_H
#define ICELBLOGGER_H

//#include "boost/thread/recursive_mutex.hpp"
#include "creamJob.h"

// Forward declaration
namespace log4cpp {
    class Category;
};

namespace glite {
    namespace wms {
        namespace ice {
            namespace util {

                // Forward declarations
                class iceLBEvent;
                // class iceLBContext;

                /**
                 * This class implements ICE LB logger.
                 * The iceLBLogger is a singleton class 
                 */
                class iceLBLogger {
                public:

                    /**
                     * Returns the singleton instance of this class.
                     *
                     * @return the singleton instance of this class
                     */
                    static iceLBLogger* instance( void );

                    /**
                     * Logs an event to the LB service. Caller
                     * transfers ownership of the passed parameter,
                     * which is automatically destroyed inside this
                     * method.  Note that this method updates the job
                     * cache with the new sequence code assigned to
                     * the job after it has been logged.
                     *
                     * @param ev the event to log; if ev==0, this
                     * method calls abort().
                     *
                     * @return the (modified) CreamJob whose status
                     * has been logged. The returned CreamJob differs
                     * from the one stored in the logged event ev in
                     * the sequence code only.
                     */
                    CreamJob logEvent( iceLBEvent* ev );

                    ~iceLBLogger( void );
                protected:
                    iceLBLogger( );

                    static iceLBLogger* s_instance;
                    log4cpp::Category* m_log_dev;
                    bool m_lb_enabled;
                };

            } // namespace util
        } // namespace ice
    } // namespace wns
}; // namespace glite

#endif
