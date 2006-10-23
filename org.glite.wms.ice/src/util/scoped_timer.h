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
 * Scoped timer for ICE 
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef ICE_SCOPED_TIMER_H
#define ICE_SCOPED_TIMER_H

#include <string>
#include <sys/time.h>

namespace log4cpp {
    class Category;
}

namespace glite {
    namespace wms {
        namespace ice {
            namespace util {

                /**
                 * @brief Implementation of a scoped timer
                 *
                 * This class implements a "scoped timer", that is,
                 * instances of this class will log into the log file
                 * the time elapsed from their creation to their
                 * description. Thus, this class can be used to have a
                 * rough estimate of the time spent inside a given
                 * scope.
                 *
                 * Example:
                 *
                 * {
                 *   scoped_timer my_timer( "foo" );
                 *   ...
                 *   ...
                 * } // at this point, a message is logged in the log file
                 * // scoped_timer foo xxx
                 * // where xxx is the elapese time in seconds; 
                 * // the elapsed time is a floating point number.
                 */
                class scoped_timer {
                protected:
                    const std::string m_name;
                    struct timeval m_start_time;
                    log4cpp::Category  *m_log_dev;
                public:

                    /**
                     * Creates a scoped timer with a given name. The name
                     * will appear in the log file. The creation time is
                     * set to the current time.
                     *
                     * @param name The name given to this timer. The name
                     * is only used to display the result in the log file,
                     * so no check for uniqueness is performed.
                     */
                    scoped_timer( const std::string& name );
                    
                    /**
                     * The descructor displays in the log file (at
                     * INFO level) the string "scoped_timer", followed
                     * by the timer name (i.e., the string used in the
                     * costructor), followed by the elapsed time since
                     * creation, in seconds.
                     */
                    virtual ~scoped_timer( );
                };

            } // namespace util
        } // namespace ice
    } // namespace wms
}; // namespace glite

#endif
