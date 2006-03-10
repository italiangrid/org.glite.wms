//
// This file is heavily based on org.glite.wms.jobsubmission/src/common/EventLogger.h
//
#ifndef __ICELBLOGGER_H__
#define __ICELBLOGGER_H__


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
                    iceLBContext* getLBContext( void ) const { return _ctx; };
                    ~iceLBLogger( void );
                protected:
                    iceLBLogger( );

                    static iceLBLogger* _instance;
                    iceLBContext* _ctx;
                    log4cpp::Category* log_dev;
                };

            } // namespace util
        } // namespace ice
    } // namespace wns
}; // namespace glite

#endif
