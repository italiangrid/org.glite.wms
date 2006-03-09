#ifndef __ICELBEVENTFACTORY_H__
#define __ICELBEVENTFACTORY_H__

#include "creamJob.h"

namespace glite {
    namespace wms {
        namespace ice {
            namespace util {

                class iceLBEvent; // forward declaration
                class CreamJob; // forward declaration

                /**
                 * This class is a factory used to build a logging
                 * event corresponding to a job status change.
                 */
                class iceLBEventFactory {
                public:
                    virtual ~iceLBEventFactory( ) { };
                    /**
                     * Factory method used to create an iceLBEvent
                     * object which corresponds to the last (most
                     * recent) status change for job j. The caller
                     * owns the pointer which is returned; thus, the
                     * caller is responsible for freeing the pointer
                     * when necessary.
                     *
                     * @param j the job whose more recent status is to
                     * be logged 
                     *
                     * @return a logging event corresponding to the
                     * most recent status of job j; if the current job
                     * status for j does not correspond to any valid
                     * LB event, then the null pointer is returned.
                     */
                    static iceLBEvent* mkEvent( const CreamJob& j );
                protected:
                    iceLBEventFactory( ) { };
                };

            } // namespace util
        } // namespace ice
    } // namespace wms
} // namespace glite

#endif
