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

#include "boost/thread/recursive_mutex.hpp"

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
                class iceLBContext;

                /**
                 * This class implements ICE LB logger.
                 * The iceLBLogger is a singleton class 
                 */
                class iceLBLogger {
                public:
                    static iceLBLogger* instance( void );
                    /**
                     * Logs an event to the LB service. 
                     *
                     * @param ev the event to log; if ev==0, nothing is done
                     */
                    void logEvent( iceLBEvent* ev );

                    /**
                     * This method is only used to register a new job
                     * to the LB service. Given that this task is
                     * normally done by the UI, this method will be
                     * REMOVED once ICE is fully integrated with the
                     * WMS.
                     */
                    iceLBContext* getLBContext( void ) const { return m_ctx; };
                    ~iceLBLogger( void );
                protected:
                    iceLBLogger( );

                    static iceLBLogger* s_instance;
                    static boost::recursive_mutex s_mutex;
                    iceLBContext* m_ctx;
                    log4cpp::Category* m_log_dev;
                };

            } // namespace util
        } // namespace ice
    } // namespace wns
}; // namespace glite

#endif
