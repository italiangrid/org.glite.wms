//
// This file is heavily based on org.glite.wms.jobsubmission/src/common/EventLogger.h
//
#ifndef __ICEEVENTLOGGER_H__
#define __ICEEVENTLOGGER_H__

#include <exception>

#include "glite/lb/context.h"
#include "creamJob.h"

// Forward declaration
namespace log4cpp {
    class Category;
};

typedef  struct _edg_wll_Context  *edg_wll_Context;

namespace classad { class ClassAd; }

namespace glite {
    namespace wms {
        namespace ice {
            namespace util {

                class iceLoggerException : public std::exception {
                public:
                    iceLoggerException( const char *reason );
                    iceLoggerException( const std::string &reason );
                    virtual ~iceLoggerException( void ) throw();
                    
                    virtual const char *what( void ) const throw();
                    
                private:
                    std::string     le_reason;
                };

                class iceEventLogger {
                public:
                    
                    static iceEventLogger* instance( void );

                    ~iceEventLogger( void );

                    /**
                     * Job transfer events. The job is transferred
                     * from ICE to CREAM. This event is logged by ICE
                     * behaving as the Job Controller.
                     *
                     * @param theJob the job being transferred
                     */
                    void cream_transfer_start_event( const CreamJob& theJob );
                    void cream_transfer_ok_event( const CreamJob& theJob  );
                    void cream_transfer_fail_event( const CreamJob& theJob, const std::string& reason );

                    /**
                     * job accepted event. The job has been accepted
                     * by CREAM.
                     *
                     * @param theJob the job being accepted
                     */
                    void cream_accepted_event( const CreamJob& theJob );

                    /**
                     * Job accepted event. The job has been accepted
                     * by the LRMS 
                     *
                     * @param theJob the job being accepted by the LRMS
                     */
                     void lrms_accepted_event( const CreamJob& theJob );

                    /**
                     * Job running event. The job is being executed by
                     * the LRMS.
                     *
                     * @param theJob the job which is running
                     * @param host the host where the job is being executed
                     */
                    void job_running_event( const CreamJob& theJob, const std::string& host );

                    /**
                     * Logs a job refused event. The job is being refused
                     * by CREAM.
                     *
                     * @param theJob the job being refused
                     * @param reason the reason why the job is refused
                     */
                    void cream_refused_event( const CreamJob& theJob, const std::string& reason );


                    // job cancel events.
                    void cream_cancel_request_event( const CreamJob& theJob );
                    void cream_cancel_refuse_event( const CreamJob& theJob, const std::string& reason );

                    /**
                     * Logs a job_done (cancelled) event. This notifies
                     * that a job has been cancelled by the user.
                     *
                     * @param theJob the job which has been cancelled
                     */
                    void job_cancelled_event( const CreamJob& theJob );

                    void job_suspended_event( const CreamJob& theJob );
                    void job_done_failed_event( const CreamJob& theJob );
                    void job_done_ok_event( const CreamJob& theJob );

                    // job scheduled event
                    void cream_scheduled_event( const CreamJob& theJob );

                    /**
                     * Logs a job status change event. This means that
                     * the job passed as parameter changed status; the
                     * new status is logged to L&B service.  All
                     * information needed for logging are retrieved
                     * from the creamJob object passed as parameter.
                     *
                     * @param theJob the job being logged
                     */
                    void log_job_status_change( const CreamJob& theJob );

                    static void set_lb_retries( unsigned int r ) { el_s_retries = r; return; }
                    static void set_lb_interval( unsigned int sec ) { el_s_sleep = sec; return; }
                    static void set_lb_destination( const char* dest ) { el_s_destination = dest; return; }

                    /**
                     * @obsolete{Registers a new job (this would
                     * normally be done by the UI).}
                     */
                    void registerJob( const CreamJob& theJob );

                protected:
                    inline void startLogging( void ) { this->el_count = 0; this->el_hostProxy = false; }

                    void setLoggingJob( const CreamJob& theJob, edg_wll_Source src  ) throw( iceLoggerException& );

                    void testCode( int &code, bool retry = false );
                    std::string getLoggingError( const char *preamble );

                    /**
                     * Default costructor
                     * 
                     * This method builds a new iceEventLogger object;
                     * moreover, a new edg_wll_Context is created and
                     * associated to this logger.
                     */
                    iceEventLogger( void );


                    bool el_hostProxy;
                    unsigned int el_count;
                    edg_wll_Context* el_context;
                    log4cpp::Category *log_dev;

                    static unsigned int el_s_retries, el_s_sleep;
                    static const char *el_s_notLogged, *el_s_unavailable, *el_s_OK, *el_s_failed;
                    static const char *el_s_destination; 

                    static iceEventLogger* _instance;
                };

            } // namespace util
        } // namespace ice
    } // namespace wns
}; // namespace glite

#endif
