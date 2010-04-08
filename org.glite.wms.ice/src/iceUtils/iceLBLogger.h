
/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */

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
